#if 0
#include "BackgroundModel.h"
#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/statistical_outlier_removal.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <iostream>
#include <cmath>

static BackgroundModel::PCLCloud::Ptr VoxelDownsample(
    const BackgroundModel::PCLCloud::Ptr& in, double voxel)
{
    if (!in || in->empty() || voxel <= 0) return in;
    auto out = BackgroundModel::PCLCloud::Ptr(new BackgroundModel::PCLCloud);
    pcl::VoxelGrid<PointType> vg;
    vg.setInputCloud(in);
    vg.setLeafSize(voxel, voxel, voxel);
    vg.filter(*out);
    return out;
}

static BackgroundModel::PCLCloud::Ptr StatisticalFilter(
    const BackgroundModel::PCLCloud::Ptr& in, int nb, double std)
{
    if (!in || in->empty()) return in;
    auto out = BackgroundModel::PCLCloud::Ptr(new BackgroundModel::PCLCloud);
    pcl::StatisticalOutlierRemoval<PointType> sor;
    sor.setInputCloud(in);
    sor.setMeanK(nb);
    sor.setStddevMulThresh(std);
    sor.filter(*out);
    return out;
}

BackgroundModel::BackgroundModel(const Options& opt) : opt_(opt) {
    bg_cloud_.reset(new PCLCloud);
}

void BackgroundModel::build(const std::vector<PCLCloudPtr>& bg_frames) {
    bg_cloud_->clear();
    if (bg_frames.empty()) {
        std::cerr << "[BG] no background frames." << std::endl;
        built_ = false;
        return;
    }

    // 合并所有背景帧
    for (auto &f : bg_frames) {
        auto ds = VoxelDownsample(f, opt_.voxel_size);
        auto flt = StatisticalFilter(ds, opt_.nb_neighbors, opt_.std_ratio);
        *bg_cloud_ += *flt;
    }

    // 再整体下采样+滤波一次提升密度与稳定性
    bg_cloud_ = VoxelDownsample(bg_cloud_, opt_.voxel_size);
    bg_cloud_ = StatisticalFilter(bg_cloud_, opt_.nb_neighbors, opt_.std_ratio);

    built_ = !bg_cloud_->empty();
    std::cout << "[BG] built background points: " << bg_cloud_->size() << std::endl;
}

BackgroundModel::PCLCloudPtr BackgroundModel::subtract(
    const std::vector<PCLCloudPtr>& fg_frames) const
{
    auto fg_merged = PCLCloudPtr(new PCLCloud);
    for (auto &f : fg_frames) *fg_merged += *f;

    fg_merged = VoxelDownsample(fg_merged, opt_.voxel_size);
    if (!built_ || bg_cloud_->empty() || fg_merged->empty()) {
        return fg_merged;
    }

    pcl::KdTreeFLANN<PointType> kdtree;
    kdtree.setInputCloud(bg_cloud_);

    auto result = PCLCloudPtr(new PCLCloud);
    std::vector<int> idx(1);
    std::vector<float> dist2(1);

    for (const auto &pt : fg_merged->points) {
        if (kdtree.nearestKSearch(pt, 1, idx, dist2) > 0) {
            double dist = std::sqrt(dist2[0]);
            if (dist > opt_.diff_thresh) {
                result->points.push_back(pt);
            }
        }
    }

    result->width = result->points.size();
    result->height = 1;
    result->is_dense = false;

    std::cout << "[SUB] foreground after subtract: " << result->size() << std::endl;
    return result;
}
#endif

#include "BackgroundModel.h"
#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/statistical_outlier_removal.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <unordered_set>
#include <iostream>
#include <cmath>

static BackgroundModel::PCLCloudPtr VoxelDownsample(
    const BackgroundModel::PCLCloudPtr& in, double voxel)
{
    if (!in || in->empty() || voxel <= 0) return in;
    auto out = BackgroundModel::PCLCloudPtr(new BackgroundModel::PCLCloud);
    pcl::VoxelGrid<PointType> vg;
    vg.setInputCloud(in);
    vg.setLeafSize(voxel, voxel, voxel);
    vg.filter(*out);
    return out;
}

static BackgroundModel::PCLCloudPtr StatisticalFilter(
    const BackgroundModel::PCLCloudPtr& in, int nb, double std)
{
    if (!in || in->empty()) return in;
    auto out = BackgroundModel::PCLCloudPtr(new BackgroundModel::PCLCloud);
    pcl::StatisticalOutlierRemoval<PointType> sor;
    sor.setInputCloud(in);
    sor.setMeanK(nb);
    sor.setStddevMulThresh(std);
    sor.filter(*out);
    return out;
}

BackgroundModel::BackgroundModel(const Options& opt) : opt_(opt) {
    bg_cloud_.reset(new PCLCloud);
}

// 把点坐标量化成 voxel key
static BackgroundModel::VoxelKey MakeKey(const PointType& p, double voxel) {
    return {
        (int)std::floor(p.x / voxel),
        (int)std::floor(p.y / voxel),
        (int)std::floor(p.z / voxel)
    };
}

void BackgroundModel::build(const std::vector<PCLCloudPtr>& bg_frames) {
    bg_cloud_->clear();
    built_ = false;
    if (bg_frames.empty()) {
        std::cerr << "[BG] no background frames.\n";
        return;
    }

    const int frame_num = (int)bg_frames.size();
    std::unordered_map<VoxelKey, int, VoxelKeyHash> voxel_counts;

    // 1) 逐帧统计 stable_voxel 命中次数（同一帧内重复命中只算一次）
    for (int fi = 0; fi < frame_num; ++fi) {
        auto f = bg_frames[fi];
        if (!f || f->empty()) continue;

        // 先用 stable_voxel 下采样，减少统计量
        auto f_ds = VoxelDownsample(f, opt_.stable_voxel);

        std::unordered_set<VoxelKey, VoxelKeyHash> touched;
        touched.reserve(f_ds->size());

        for (auto &pt : f_ds->points) {
            auto key = MakeKey(pt, opt_.stable_voxel);
            touched.insert(key);
        }

        for (auto &k : touched) {
            voxel_counts[k] += 1;
        }
    }

    // 2) 合并所有背景点，再按“出现率”过滤
    auto merged = PCLCloudPtr(new PCLCloud);
    for (auto &f : bg_frames) {
        if (!f || f->empty()) continue;
        *merged += *f;
    }

    // 基础下采样 + 滤波（让点更规整）
    merged = VoxelDownsample(merged, opt_.voxel_size);
    merged = StatisticalFilter(merged, opt_.nb_neighbors, opt_.std_ratio);

    // 按出现率筛稳定点
    auto stable_bg = PCLCloudPtr(new PCLCloud);
    stable_bg->points.reserve(merged->size());

    for (auto &pt : merged->points) {
        auto key = MakeKey(pt, opt_.stable_voxel);
        auto it = voxel_counts.find(key);
        if (it == voxel_counts.end()) continue;

        double ratio = (double)it->second / (double)frame_num;
        if (ratio >= opt_.min_presence_ratio) {
            stable_bg->points.push_back(pt);
        }
    }

    stable_bg->width = stable_bg->points.size();
    stable_bg->height = 1;
    stable_bg->is_dense = false;

    bg_cloud_ = stable_bg;
    built_ = !bg_cloud_->empty();
    has_bg_ = built_;
    
    std::cout << "[BG] frames=" << frame_num
              << ", stable_voxel=" << opt_.stable_voxel
              << ", presence_ratio>=" << opt_.min_presence_ratio
              << ", bg_points=" << bg_cloud_->size() << "\n";
}

BackgroundModel::PCLCloudPtr BackgroundModel::subtract(
    const std::vector<PCLCloudPtr>& fg_frames) const
{
    auto fg_merged = PCLCloudPtr(new PCLCloud);
    for (auto &f : fg_frames) {
        if (!f || f->empty()) continue;
        *fg_merged += *f;
    }

    fg_merged = VoxelDownsample(fg_merged, opt_.voxel_size);
    if (!built_ || bg_cloud_->empty() || fg_merged->empty()) {
        return fg_merged;
    }

    pcl::KdTreeFLANN<PointType> kdtree;
    kdtree.setInputCloud(bg_cloud_);

    auto result = PCLCloudPtr(new PCLCloud);
    result->points.reserve(fg_merged->size());

    std::vector<int> idx(1);
    std::vector<float> dist2(1);

    for (const auto &pt : fg_merged->points) {
        if (kdtree.nearestKSearch(pt, 1, idx, dist2) > 0) {
            double nn_dist = std::sqrt(dist2[0]);

            double r = std::sqrt(pt.x*pt.x + pt.y*pt.y + pt.z*pt.z);
            double adaptive_thr = opt_.diff_base + opt_.diff_k * r;

            if (nn_dist > adaptive_thr) {
                result->points.push_back(pt);
            }
        } else {
            // 没找到邻居也视为前景（可选）
            result->points.push_back(pt);
        }
    }

    result->width = result->points.size();
    result->height = 1;
    result->is_dense = false;

    std::cout << "[SUB] fg_points=" << fg_merged->size()
              << ", result=" << result->size()
              << " (base=" << opt_.diff_base
              << ", k=" << opt_.diff_k << ")\n";

    return result;
}

