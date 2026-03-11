#include "LidarProcessor.h"
#include <iostream>
#include <chrono>
#include <thread>

LidarProcessor::LidarProcessor(const Options& opt) : opt_(opt) {
    reader_.reset(createUnitreeLidarReader());
}

LidarProcessor::~LidarProcessor() { close(); }

bool LidarProcessor::open() {
    if (!reader_) return false;
    int ret = reader_->initializeSerial(
        opt_.port.c_str(),
        opt_.baudrate,
        opt_.lidar_type,
        opt_.use_imu,
        opt_.min_range,
        opt_.max_range
    );
    if (ret != 0) {
        std::cerr << "[LIDAR] init serial failed, code=" << ret << std::endl;
        return false;
    }
    running_ = true;
    return true;
}

void LidarProcessor::close() {
    running_ = false;
    if (reader_) reader_->closeSerial();
}

bool LidarProcessor::grabOneCloud(PointCloudUnitree& cloud_unitree, PCLCloudPtr& cloud_pcl) {
    int packetType = reader_->runParse();
    if (packetType == LIDAR_POINT_DATA_PACKET_TYPE) {
        if (reader_->getPointCloud(cloud_unitree)) {
            cloud_pcl.reset(new PCLCloud);
            UnitreeToPcl(cloud_unitree, cloud_pcl);
            return !cloud_pcl->empty();
        }
    }
    return false;
}

std::vector<LidarProcessor::PCLCloudPtr>
LidarProcessor::captureSeconds(double seconds,
    std::function<void(double,double)> progress_cb)
{
    std::vector<PCLCloudPtr> frames;
    PointCloudUnitree cloud_u;

    using clock = std::chrono::steady_clock;
    auto start = clock::now();

    while (running_) {
        PCLCloudPtr pcl_cloud;
        if (grabOneCloud(cloud_u, pcl_cloud)) {
            frames.push_back(pcl_cloud);
        }

        auto now = clock::now();
        double elapsed = std::chrono::duration<double>(now - start).count();
        if (progress_cb) progress_cb(elapsed, seconds);

        if (elapsed >= seconds) break;
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    std::cout << "[CAP] frames captured: " << frames.size() << std::endl;
    return frames;
}
