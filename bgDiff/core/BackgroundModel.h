#if 0
#pragma once
#include "unitree_lidar_sdk.h"
#include "UnitreePclBridge.h"
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <memory>

using namespace unilidar_sdk2;

class BackgroundModel {
public:
    using PCLCloud = pcl::PointCloud<PointType>;
    using PCLCloudPtr = PCLCloud::Ptr;

    struct Options {
        double voxel_size = 0.05;     // 下采样体素
        int nb_neighbors = 20;        // 统计滤波邻居数
        double std_ratio = 2.0;       // 统计滤波标准差
        double diff_thresh = 0.15;    // 背景差分阈值（米）
    };

    explicit BackgroundModel(const Options& opt);

    // 输入多帧背景点云，建立背景模型（合并+滤波）
    void build(const std::vector<PCLCloudPtr>& bg_frames);

    // 用背景模型做差分，返回最终前景点云
    PCLCloudPtr subtract(const std::vector<PCLCloudPtr>& fg_frames) const;

    bool isBuilt() const { return built_; }
    PCLCloudPtr backgroundCloud() const { return bg_cloud_; }

private:
    Options opt_;
    bool built_ = false;
    PCLCloudPtr bg_cloud_;
};
#endif

#pragma once
#include "unitree_lidar_sdk.h"
#include "UnitreePointType.h"
#include <pcl/point_cloud.h>
#include <memory>
#include <unordered_map>

using namespace unilidar_sdk2;

class BackgroundModel {
public:
    using PCLCloud = pcl::PointCloud<PointType>;
    using PCLCloudPtr = PCLCloud::Ptr;

    struct Options {
        double voxel_size = 0.05;        // 通用下采样（前景/背景合并时）
        int nb_neighbors = 20;
        double std_ratio = 2.0;

        // --- 稳定背景 voxel 筛选 ---
        double stable_voxel = 0.15;      // 背景稳定性统计体素（可更大）
        double min_presence_ratio = 0.6; // 出现率阈值（>= 此比例才算稳定）

        // --- 自适应差分阈值 ---
        double diff_base = 0.05;         // r=0 时阈值
        double diff_k = 0.003;           // 阈值随距离增长系数
    };

    explicit BackgroundModel(const Options& opt);

    void build(const std::vector<PCLCloudPtr>& bg_frames);
    PCLCloudPtr subtract(const std::vector<PCLCloudPtr>& fg_frames) const;

    bool isBuilt() const { return built_; }
    PCLCloudPtr backgroundCloud() const { return bg_cloud_; }
    void setBackgroundCloud(PCLCloudPtr c) { bg_cloud_ = c; has_bg_ = (c && !c->empty()); built_ = has_bg_;}
    bool hasBackgroundCloud() const { return has_bg_; }

    struct VoxelKey {
        int x, y, z;
        bool operator==(const VoxelKey& o) const {
            return x==o.x && y==o.y && z==o.z;
        }
    };

private:

    struct VoxelKeyHash {
        std::size_t operator()(const VoxelKey& k) const {
            // 简单 hash
            return ((std::size_t)k.x * 73856093) ^
                   ((std::size_t)k.y * 19349663) ^
                   ((std::size_t)k.z * 83492791);
        }
    };

    Options opt_;
    bool built_ = false;
    PCLCloudPtr bg_cloud_;
    bool has_bg_ = false;
};

