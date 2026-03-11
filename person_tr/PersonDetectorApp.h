#pragma once
#include <opencv2/opencv.hpp>
#include <functional>
#include <atomic>
#include <memory>
#include <string>
#include "RtspCapture.hpp"
#include "RoiManager.hpp"
#include "TrtYoloDetector.hpp"
#include "Visualizer.hpp"

class PersonDetectorApp {
public:
    struct Options {
        int camera_id = 0;
        std::string rtsp_url;
        std::string engine_path = "person_tr/best_person_fp16.engine";
        float conf_thres = 0.5f;
        int display_width = 1080;
        int display_height = 720;
        std::string roi_file = "roi_points.txt";
        bool rotate_180 = true;
    };

    explicit PersonDetectorApp(const Options& opt);

    void setFrameCallback(std::function<void(const cv::Mat&)> cb);

    // 阻塞运行，直到 stop 被调用或捕获/推理失败
    bool run();
    void stop();

private:
    cv::Mat acquireFrame();
    void cleanup();

    Options opt_;
    std::function<void(const cv::Mat&)> frame_cb_;
    std::atomic<bool> running_{false};

    std::unique_ptr<RtspCapture> rtsp_cap_;
    std::unique_ptr<cv::VideoCapture> cam_cap_;
};
