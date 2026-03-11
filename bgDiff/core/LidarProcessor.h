#pragma once
#include "unitree_lidar_sdk.h"
#include "UnitreePclBridge.h"
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <memory>
#include <atomic>
#include <functional>

using namespace unilidar_sdk2;

class LidarProcessor {
public:
    using PCLCloud = pcl::PointCloud<PointType>;
    using PCLCloudPtr = PCLCloud::Ptr;

    struct Options {
        std::string port = "/dev/ttyACM0";
        int baudrate = 4000000;
        int lidar_type = 18;
        bool use_imu = true;
        float min_range = 0.1f;
        float max_range = 100.0f;
    };

    explicit LidarProcessor(const Options& opt);
    ~LidarProcessor();

    bool open();
    void close();

    // 采集 seconds 秒，返回多帧 pcl 点云
    std::vector<PCLCloudPtr> captureSeconds(
        double seconds,
        std::function<void(double elapsed, double total)> progress_cb = nullptr
    );

private:
    Options opt_;
    std::unique_ptr<UnitreeLidarReader> reader_;
    std::atomic<bool> running_{false};

    bool grabOneCloud(PointCloudUnitree& cloud_unitree, PCLCloudPtr& cloud_pcl);
};
