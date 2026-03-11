#include "SessionManager.h"
#include <pcl/io/pcd_io.h>
#include <unordered_set>
#include <iostream>

SessionManager::SessionManager(const Options& opt)
    : opt_(opt), lidar_(opt.lidar_opt), bg_model_(opt.bg_opt)
{}

bool SessionManager::makeBackground(std::function<void(double,double)> progress_cb) {
    bg_frames_.clear();

    if (!lidar_.open()) return false;
    bg_frames_ = lidar_.captureSeconds(opt_.bg_seconds, progress_cb);
    lidar_.close();

    if (bg_frames_.empty()) return false;

    bg_model_.build(bg_frames_);
    return bg_model_.isBuilt();
}

bool SessionManager::recordForeground(std::function<void(double,double)> progress_cb) {
    fg_frames_.clear();

    if (!lidar_.open()) return false;
    fg_frames_ = lidar_.captureSeconds(opt_.fg_seconds, progress_cb);
    lidar_.close();

    return !fg_frames_.empty();
}

#if 0
bool SessionManager::doSubtract() {
    if (!hasBackground() || !hasForeground()) return false;
    result_ = bg_model_.subtract(fg_frames_);
    return hasResult();
}
#endif

bool SessionManager::exportResultPCD(const std::string& filename) const {
    if (!hasResult()) return false;
    return pcl::io::savePCDFileBinary(filename, *result_) == 0;
}

bool SessionManager::doSubtract() {
    if (!hasBackground() || !hasForeground()) return false;

    // ① 全场景差分
    result_full_ = bg_model_.subtract(fg_frames_);
    if (!result_full_ || result_full_->empty()) return false;
    result_ = result_full_; 

    // ② ROI 裁剪，仅用于几何/导出
    result_roi_ = RoiAnalyzer::cropAxisAligned(result_full_, roi_);
    geom_ = GeometryInfo();
    plane_inliers_roi_.reset(new PCLCloud);
    plane_indices_roi_.clear();

    if (!result_roi_ || result_roi_->empty()) {
        return true; // ROI 空也算差分成功
    }

    // ③ 平面拟合（拿 inliers）
    auto plane_opt = RoiAnalyzer::fitPlaneRansac(result_roi_, 0.02f, 1500);

    // ✅ 必须同时检查 inliers ptr 和 indices
    if (plane_opt && plane_opt->inliers && !plane_opt->inliers->indices.empty()) {

        last_plane_coeff_ = plane_opt->coeff;
        plane_indices_roi_ = plane_opt->inliers->indices;

        // 平面外边框（世界系）
        auto hull_opt = RoiAnalyzer::computePlaneHullWorld(result_roi_, *plane_opt);
        if (hull_opt) {
            plane_hull_world_ = *hull_opt;
        } else {
            plane_hull_world_.reset();
        }
        
        // 拿出 inliers 点云（用于标注/显示）
        plane_inliers_roi_->points.reserve(plane_indices_roi_.size());
        for (int idx : plane_indices_roi_) {
            if (idx >= 0 && idx < (int)result_roi_->points.size()) {
                plane_inliers_roi_->points.push_back(result_roi_->points[idx]);
            }
        }
        plane_inliers_roi_->width = plane_inliers_roi_->points.size();
        plane_inliers_roi_->height = 1;
        plane_inliers_roi_->is_dense = false;

        // ④ OBB几何（在 ROI 上）
        auto obb_opt = RoiAnalyzer::computePlaneOBB2D(result_roi_, *plane_opt);
        if (obb_opt) {
            geom_.valid = true;
            geom_.length = obb_opt->length;
            geom_.width  = obb_opt->width;
            geom_.yaw_deg= obb_opt->yaw_deg;
            geom_.plane_inlier_ratio = plane_opt->inlier_ratio;
        }
    } else {
        // 平面拟合失败 / inliers 空：保持 geom_ invalid
        // 这里不 return false，是因为差分仍成功，只是没平面结果
    }

    return true;
}


bool SessionManager::exportResultRoi(const std::string& filename) const {
    if (!result_roi_ || result_roi_->empty()) return false;

    auto out = PCLCloudPtr(new PCLCloud(*result_roi_));

    // 把 inliers 索引集做个 hash set
    std::unordered_set<size_t> inlier_set;
    if (plane_inliers_roi_ && !plane_inliers_roi_->empty()) {
        // 保险起见：我们用“坐标匹配”太慢；更稳是保存 indices
        // 这里假设你愿意保存 indices：plane_indices_roi_
        // 如果你没保存 indices，就直接在 doSubtract 保存一个 indices set。
    }

    // ✅ 推荐方式：在 doSubtract 里保存 plane_indices_roi_
    // 这里用 plane_indices_roi_ 标注：
    for (size_t i = 0; i < out->points.size(); ++i) {
        // 默认非平面
        out->points[i].intensity = 50.0f;
    }
    for (int idx : plane_indices_roi_) {
        if (idx >=0 && idx < (int)out->points.size()) {
            out->points[idx].intensity = 255.0f;
        }
    }

    int ret = pcl::io::savePCDFileBinary(filename, *out);
    return ret == 0;
}

bool SessionManager::saveBackgroundToFile(const std::string& path) const {
    if (!bg_model_.hasBackgroundCloud()) return false; 
    // ↑ 你如果没有这个接口，就改成判断 bg_model_ 内部背景云指针是否为空

    auto bg_cloud = bg_model_.backgroundCloud(); 
    // ↑ 你需要提供 backgroundCloud() getter（见下面 BackgroundModel 部分）

    if (!bg_cloud || bg_cloud->empty()) return false;
    return pcl::io::savePCDFileBinary(path, *bg_cloud) == 0;
}

bool SessionManager::loadBackgroundFromFile(const std::string& path) {
    PCLCloudPtr cloud(new PCLCloud);
    if (pcl::io::loadPCDFile<PointType>(path, *cloud) != 0) return false;
    if (cloud->empty()) return false;

    bg_model_.setBackgroundCloud(cloud);
    // ↑ 你需要提供 setBackgroundCloud()（见下面 BackgroundModel 部分）

    bg_frames_.clear();         // 旧帧清空，避免混
    fg_frames_.clear();
    return true;
}