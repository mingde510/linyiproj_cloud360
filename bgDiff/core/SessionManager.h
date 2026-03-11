#pragma once
#include "LidarProcessor.h"
#include "BackgroundModel.h"
#include <string>
#include <functional>
#include "RoiAnalyzer.h"
#include <Eigen/Dense>

class SessionManager {
public:
    using PCLCloud = pcl::PointCloud<PointType>;
    using PCLCloudPtr = LidarProcessor::PCLCloudPtr;

    struct Options {
        LidarProcessor::Options lidar_opt;
        BackgroundModel::Options bg_opt;
        double bg_seconds = 5.0;
        double fg_seconds = 5.0;
    };

    explicit SessionManager(const Options& opt);

    // 1) 制作背景
    bool makeBackground(std::function<void(double,double)> progress_cb=nullptr);

    // 2) 录入前景
    bool recordForeground(std::function<void(double,double)> progress_cb=nullptr);

    // 3) 差分
    bool doSubtract();

    // 4) 导出结果
    bool exportResultPCD(const std::string& filename) const;

    bool hasBackground() const { return bg_model_.hasBackgroundCloud(); }
    bool hasForeground()  const { return !fg_frames_.empty(); }
    bool hasResult()      const { return result_ && !result_->empty(); }

    int bgFrameCount() const { return static_cast<int>(bg_frames_.size()); }
    int fgFrameCount() const { return static_cast<int>(fg_frames_.size()); }
    int bgPointCount() const { return bg_model_.backgroundCloud() ? static_cast<int>(bg_model_.backgroundCloud()->size()) : 0; }
    int resultPointCount() const { return result_ ? static_cast<int>(result_->size()) : 0; }

    void updateOptions(const Options& opt) { opt_ = opt; }

    struct GeometryInfo {
        bool valid=false;
        float length=0, width=0, yaw_deg=0;
        float plane_inlier_ratio=0;
    };

    const Eigen::Vector4f& lastPlaneCoeff() const { return last_plane_coeff_; }
    const std::vector<int>& planeIndicesRoi() const { return plane_indices_roi_; }
    pcl::PointCloud<pcl::PointXYZ>::ConstPtr planeHullWorld() const { return plane_hull_world_; }

    void setRoi(const RoiAnalyzer::Roi& roi) { roi_ = roi; }
    GeometryInfo geometryInfo() const { return geom_; }

    // 新增接口
    PCLCloudPtr resultFull() const { return result_full_; }
    PCLCloudPtr resultRoi()  const { return result_roi_; }
    PCLCloudPtr planeInliersRoi() const { return plane_inliers_roi_; }
    bool exportResultRoi(const std::string& filename) const;

    bool saveBackgroundToFile(const std::string& path) const;
    bool loadBackgroundFromFile(const std::string& path);
    PCLCloudPtr backgroundCloud() const { return bg_model_.backgroundCloud(); }
    PCLCloudPtr foregroundCloud() const { return fg_frames_.empty() ? nullptr : fg_frames_.back(); }

private:
    Options opt_;
    LidarProcessor lidar_;
    BackgroundModel bg_model_;
    std::vector<PCLCloudPtr> bg_frames_;
    std::vector<PCLCloudPtr> fg_frames_;
    PCLCloudPtr result_;

    RoiAnalyzer::Roi roi_{-2,2,-2,2,-1,2};
    GeometryInfo geom_;

    PCLCloudPtr result_full_;       // 差分后的全场景结果
    PCLCloudPtr result_roi_;        // ROI 裁剪后的结果
    PCLCloudPtr plane_inliers_roi_; // ROI 内平面 inliers
    Eigen::Vector4f last_plane_coeff_{0, 0, 1, 0};
    std::vector<int> plane_indices_roi_;
    pcl::PointCloud<pcl::PointXYZ>::Ptr plane_hull_world_;
};
