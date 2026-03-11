#pragma once
#include "UnitreePointType.h"
#include <pcl/point_cloud.h>
#include <pcl/PointIndices.h>
#include <pcl/PolygonMesh.h>
#include <Eigen/Dense>
#include <optional>

class RoiAnalyzer {
public:
    using Cloud = pcl::PointCloud<PointType>;
    using CloudPtr = Cloud::Ptr;

    struct Roi {
        float xmin, xmax;
        float ymin, ymax;
        float zmin, zmax;
    };

    struct PlaneResult {
        Eigen::Vector4f coeff;
        Eigen::Vector3f normal;
        float inlier_ratio;
        pcl::PointIndices::Ptr inliers;   // ✅ 新增
    };

    struct OBB2DResult {
        float length;   // 平面内长
        float width;    // 平面内宽
        float yaw_deg;  // 平面内绕法向的旋转角（相对世界XY的投影角）
        Eigen::Vector3f center_world;
        Eigen::Matrix3f R_world; // 平面坐标系到世界系旋转
    };

    // 1) ROI 裁剪
    static CloudPtr cropAxisAligned(const CloudPtr& in, const Roi& roi);

    // 2) 平面拟合（RANSAC）
    static std::optional<PlaneResult> fitPlaneRansac(
        const CloudPtr& in,
        float dist_thresh = 0.02f,
        int max_iter = 2000
    );

    // 3) 平面投影 + 2D OBB（PCA）
    static std::optional<OBB2DResult> computePlaneOBB2D(
        const CloudPtr& roi_cloud,
        const PlaneResult& plane
    );

    // 平面外边框（convex hull）
    static std::optional<pcl::PointCloud<pcl::PointXYZ>::Ptr> computePlaneHullWorld(
        const CloudPtr& roi_cloud,
        const PlaneResult& plane
    );

private:
    static void buildPlaneFrame(
        const Eigen::Vector3f& n,
        Eigen::Vector3f& u,
        Eigen::Vector3f& v
    );
};
