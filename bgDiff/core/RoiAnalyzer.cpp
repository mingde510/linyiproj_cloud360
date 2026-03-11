#include "RoiAnalyzer.h"
#include <pcl/filters/passthrough.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/filters/project_inliers.h>
#include <pcl/common/centroid.h>
#include <pcl/segmentation/extract_clusters.h>
#include <pcl/search/kdtree.h>
#include <cmath>

RoiAnalyzer::CloudPtr RoiAnalyzer::cropAxisAligned(const CloudPtr& in, const Roi& roi) {
    if (!in || in->empty()) return CloudPtr(new Cloud);

    CloudPtr tmp(new Cloud), out(new Cloud);
    pcl::PassThrough<PointType> pass;

    pass.setInputCloud(in);
    pass.setFilterFieldName("x");
    pass.setFilterLimits(roi.xmin, roi.xmax);
    pass.filter(*tmp);

    pass.setInputCloud(tmp);
    pass.setFilterFieldName("y");
    pass.setFilterLimits(roi.ymin, roi.ymax);
    pass.filter(*tmp);

    pass.setInputCloud(tmp);
    pass.setFilterFieldName("z");
    pass.setFilterLimits(roi.zmin, roi.zmax);
    pass.filter(*out);

    return out;
}

std::optional<RoiAnalyzer::PlaneResult>
RoiAnalyzer::fitPlaneRansac(const CloudPtr& in, float dist_thresh, int max_iter) {
    if (!in || in->size() < 50) return std::nullopt;

    pcl::SACSegmentation<PointType> seg;
    seg.setOptimizeCoefficients(true);
    seg.setModelType(pcl::SACMODEL_PLANE);
    seg.setMethodType(pcl::SAC_RANSAC);
    seg.setDistanceThreshold(dist_thresh);
    seg.setMaxIterations(max_iter);
    seg.setInputCloud(in);

    pcl::PointIndices::Ptr inliers(new pcl::PointIndices);
    pcl::ModelCoefficients::Ptr coeff(new pcl::ModelCoefficients);
    seg.segment(*inliers, *coeff);

    if (inliers->indices.empty()) return std::nullopt;

    PlaneResult res;
    res.coeff = Eigen::Vector4f(coeff->values[0], coeff->values[1],
                               coeff->values[2], coeff->values[3]);
    res.normal = res.coeff.head<3>().normalized();
    res.inliers = inliers;
    res.inlier_ratio = float(inliers->indices.size()) / float(in->size());
    return res;
}

void RoiAnalyzer::buildPlaneFrame(const Eigen::Vector3f& n,
                                  Eigen::Vector3f& u,
                                  Eigen::Vector3f& v) {
    Eigen::Vector3f ref = (std::fabs(n.z()) < 0.9f) ? Eigen::Vector3f::UnitZ()
                                                   : Eigen::Vector3f::UnitY();
    u = n.cross(ref).normalized();
    v = n.cross(u).normalized();
}

std::optional<RoiAnalyzer::OBB2DResult>
RoiAnalyzer::computePlaneOBB2D(const CloudPtr& roi_cloud,
                               const PlaneResult& plane) {
    if (!roi_cloud || roi_cloud->size() < 50) return std::nullopt;
    if (!plane.inliers || plane.inliers->indices.size() < 30) return std::nullopt;

    // 提取 inliers 并投影到平面
    CloudPtr inlier_cloud(new Cloud);
    pcl::ExtractIndices<PointType> ex;
    ex.setInputCloud(roi_cloud);
    ex.setIndices(plane.inliers);
    ex.setNegative(false);
    ex.filter(*inlier_cloud);
    if (inlier_cloud->size() < 30) return std::nullopt;

    pcl::ProjectInliers<PointType> proj;
    proj.setModelType(pcl::SACMODEL_PLANE);
    pcl::ModelCoefficients::Ptr coeff(new pcl::ModelCoefficients);
    coeff->values = {plane.coeff[0], plane.coeff[1], plane.coeff[2], plane.coeff[3]};
    proj.setModelCoefficients(coeff);
    proj.setInputCloud(inlier_cloud);
    CloudPtr projected(new Cloud);
    proj.filter(*projected);
    if (projected->empty()) return std::nullopt;

    // 平面坐标系 u,v,n 与中心
    Eigen::Vector3f n = plane.normal;
    Eigen::Vector3f u, v;
    buildPlaneFrame(n, u, v);
    Eigen::Vector4f centroid4;
    pcl::compute3DCentroid(*projected, centroid4);
    Eigen::Vector3f c = centroid4.head<3>();

    // 投到 2D 并做 DBSCAN (欧式聚类) 取最大簇
    pcl::PointCloud<pcl::PointXYZ>::Ptr pts2d(new pcl::PointCloud<pcl::PointXYZ>);
    pts2d->points.reserve(projected->size());
    std::vector<Eigen::Vector2f> uv_list;
    uv_list.reserve(projected->size());
    for (auto &p : projected->points) {
        Eigen::Vector3f d(p.x, p.y, p.z);
        d -= c;
        float ucoord = d.dot(u);
        float vcoord = d.dot(v);
        uv_list.emplace_back(ucoord, vcoord);
        pts2d->points.emplace_back(ucoord, vcoord, 0.0f);
    }

    pcl::search::KdTree<pcl::PointXYZ>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZ>);
    tree->setInputCloud(pts2d);
    pcl::EuclideanClusterExtraction<pcl::PointXYZ> ec;
    ec.setClusterTolerance(0.05f);   // 5 cm
    ec.setMinClusterSize(20);
    ec.setMaxClusterSize(static_cast<int>(pts2d->size()));
    ec.setSearchMethod(tree);
    ec.setInputCloud(pts2d);
    std::vector<pcl::PointIndices> clusters;
    ec.extract(clusters);
    if (clusters.empty()) return std::nullopt;
    auto largest = std::max_element(
        clusters.begin(), clusters.end(),
        [](const auto& a, const auto& b){ return a.indices.size() < b.indices.size(); });

    // PCA 仅用最大簇
    Eigen::MatrixXf P2(2, largest->indices.size());
    for (size_t i=0;i<largest->indices.size();++i) {
        const auto idx = largest->indices[i];
        P2(0,i) = uv_list[idx].x();
        P2(1,i) = uv_list[idx].y();
    }

    Eigen::Vector2f mean = P2.rowwise().mean();
    Eigen::MatrixXf Q = P2.colwise() - mean;
    Eigen::Matrix2f cov = (Q * Q.transpose()) / float(Q.cols());
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix2f> es(cov);
    Eigen::Vector2f e1 = es.eigenvectors().col(1).normalized();
    Eigen::Vector2f e2(-e1.y(), e1.x());

    Eigen::VectorXf a1 = e1.transpose() * Q;
    Eigen::VectorXf a2 = e2.transpose() * Q;
    float min1=a1.minCoeff(), max1=a1.maxCoeff();
    float min2=a2.minCoeff(), max2=a2.maxCoeff();

    float length = (max1 - min1);
    float width  = (max2 - min2);
    float yaw = std::atan2(e1.y(), e1.x()) * 180.0f / float(M_PI);

    Eigen::Vector3f c_plane = c + mean.x() * u + mean.y() * v;

    OBB2DResult out;
    out.length = length;
    out.width  = width;
    out.yaw_deg = yaw;
    out.center_world = c_plane;
    out.R_world.col(0) = u;
    out.R_world.col(1) = v;
    out.R_world.col(2) = n;
    return out;
}

std::optional<pcl::PointCloud<pcl::PointXYZ>::Ptr>
RoiAnalyzer::computePlaneHullWorld(
    const CloudPtr& roi_cloud,
    const PlaneResult& plane)
{
    if (!roi_cloud || roi_cloud->empty()) return std::nullopt;
    if (!plane.inliers || plane.inliers->indices.empty()) return std::nullopt;

    // 1) 提取 inliers
    CloudPtr inlier_cloud(new Cloud);
    pcl::ExtractIndices<PointType> ex;
    ex.setInputCloud(roi_cloud);
    ex.setIndices(plane.inliers);
    ex.setNegative(false);
    ex.filter(*inlier_cloud);

    if (inlier_cloud->size() < 30) return std::nullopt;

    // 2) 投影到平面
    pcl::ProjectInliers<PointType> proj;
    proj.setModelType(pcl::SACMODEL_PLANE);
    pcl::ModelCoefficients::Ptr coeff(new pcl::ModelCoefficients);
    coeff->values = {plane.coeff[0], plane.coeff[1], plane.coeff[2], plane.coeff[3]};
    proj.setModelCoefficients(coeff);
    proj.setInputCloud(inlier_cloud);

    CloudPtr projected(new Cloud);
    proj.filter(*projected);
    if (projected->size() < 20) return std::nullopt;

    // 3) 平面坐标系
    Eigen::Vector3f n = plane.normal;
    Eigen::Vector3f u, v;
    buildPlaneFrame(n, u, v);
    Eigen::Vector4f centroid4;
    pcl::compute3DCentroid(*projected, centroid4);
    Eigen::Vector3f c = centroid4.head<3>();

    // 4) 投到 2D 并聚类（DBSCAN/欧式），保留最大簇
    pcl::PointCloud<pcl::PointXYZ>::Ptr pts2d(new pcl::PointCloud<pcl::PointXYZ>);
    pts2d->points.reserve(projected->size());
    std::vector<Eigen::Vector2f> uv_list;
    uv_list.reserve(projected->size());
    for (auto &p : projected->points) {
        Eigen::Vector3f d(p.x, p.y, p.z);
        d -= c;
        float uu = d.dot(u);
        float vv = d.dot(v);
        uv_list.emplace_back(uu, vv);
        pts2d->points.emplace_back(uu, vv, 0.0f);
    }

    pcl::search::KdTree<pcl::PointXYZ>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZ>);
    tree->setInputCloud(pts2d);
    pcl::EuclideanClusterExtraction<pcl::PointXYZ> ec;
    ec.setClusterTolerance(0.05f);   // 5 cm
    ec.setMinClusterSize(20);
    ec.setMaxClusterSize(static_cast<int>(pts2d->size()));
    ec.setSearchMethod(tree);
    ec.setInputCloud(pts2d);
    std::vector<pcl::PointIndices> clusters;
    ec.extract(clusters);
    if (clusters.empty()) return std::nullopt;
    auto largest = std::max_element(
        clusters.begin(), clusters.end(),
        [](const auto& a, const auto& b){ return a.indices.size() < b.indices.size(); });

    // 5) 用最大簇构建“矩形”外框（PCA in uv-plane）
    Eigen::MatrixXf P2(2, largest->indices.size());
    for (size_t i=0;i<largest->indices.size();++i) {
        const auto idx = largest->indices[i];
        P2(0,i) = uv_list[idx].x();   // uv 坐标系
        P2(1,i) = uv_list[idx].y();
    }

    Eigen::Vector2f mean = P2.rowwise().mean();
    Eigen::MatrixXf Q = P2.colwise() - mean;
    Eigen::Matrix2f cov = (Q * Q.transpose()) / float(Q.cols());
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix2f> es(cov);

    // 主轴（注意 eigenvalues 升序，所以 col(1) 是最大方向）
    Eigen::Vector2f e1 = es.eigenvectors().col(1).normalized();
    Eigen::Vector2f e2(-e1.y(), e1.x());

    // 在 PCA 轴上取范围
    Eigen::VectorXf a1 = e1.transpose() * Q;
    Eigen::VectorXf a2 = e2.transpose() * Q;
    float min1=a1.minCoeff(), max1=a1.maxCoeff();
    float min2=a2.minCoeff(), max2=a2.maxCoeff();

    // PCA 坐标系下的四角 (alpha1, alpha2)
    Eigen::Vector2f alphaCorners[4] = {
        {min1, min2}, {min1, max2},
        {max1, max2}, {max1, min2}
    };

    // 6) PCA -> uv -> world，并按逆时针排序避免画线交叉
    std::vector<Eigen::Vector2f> uvCorners;
    uvCorners.reserve(4);
    for (int i=0;i<4;++i) {
        const auto& a = alphaCorners[i];
        // ✅ PCA -> uv
        Eigen::Vector2f uv_corner = mean + e1 * a.x() + e2 * a.y();
        uvCorners.push_back(uv_corner);
    }

    // 逆时针排序（可视化连线才不会交叉）
    Eigen::Vector2f center2 = (uvCorners[0]+uvCorners[1]+uvCorners[2]+uvCorners[3]) / 4.0f;
    std::sort(uvCorners.begin(), uvCorners.end(),
            [&](const Eigen::Vector2f& p1, const Eigen::Vector2f& p2){
                return std::atan2(p1.y()-center2.y(), p1.x()-center2.x()) <
                        std::atan2(p2.y()-center2.y(), p2.x()-center2.x());
            });

    // uv -> world
    pcl::PointCloud<pcl::PointXYZ>::Ptr hull_pts(new pcl::PointCloud<pcl::PointXYZ>);
    hull_pts->points.reserve(5);
    for (const auto& uv_corner : uvCorners) {
        Eigen::Vector3f p3 = c + uv_corner.x()*u + uv_corner.y()*v;
        hull_pts->points.emplace_back(p3.x(), p3.y(), p3.z());
    }
    // 闭合
    hull_pts->points.push_back(hull_pts->points.front());


    hull_pts->width = hull_pts->points.size();
    hull_pts->height = 1;
    hull_pts->is_dense = true;

    return hull_pts;
}
