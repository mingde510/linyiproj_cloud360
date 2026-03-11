#include "PersonDetectorApp.h"
#include <chrono>
#include <thread>
#include <iostream>

PersonDetectorApp::PersonDetectorApp(const Options& opt) : opt_(opt) {}

void PersonDetectorApp::setFrameCallback(std::function<void(const cv::Mat&)> cb) {
    frame_cb_ = std::move(cb);
}

cv::Mat PersonDetectorApp::acquireFrame() {
    if (!opt_.rtsp_url.empty()) {
        if (!rtsp_cap_) {
            rtsp_cap_ = std::make_unique<RtspCapture>(opt_.rtsp_url);
            if (!rtsp_cap_->open()) return {};
            rtsp_cap_->start();
        }
        return rtsp_cap_->getLatestFrame();
    }

    if (!cam_cap_) {
        cam_cap_ = std::make_unique<cv::VideoCapture>(opt_.camera_id);
        if (!cam_cap_->isOpened()) {
            std::cerr << "[PersonDetectorApp] open camera failed: " << opt_.camera_id << "\n";
            cam_cap_.reset();
            return {};
        }
    }

    cv::Mat frame;
    (*cam_cap_) >> frame;
    return frame;
}

bool PersonDetectorApp::run() {
    RoiManager roi;
    roi.load(opt_.roi_file);

    TrtYoloDetector detector(opt_.engine_path);
    if (!detector.init()) {
        std::cerr << "[PersonDetectorApp] detector init failed\n";
        return false;
    }
    detector.setConfidenceThreshold(opt_.conf_thres);

    running_ = true;

    while (running_) {
        cv::Mat frame = acquireFrame();
        if (frame.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        if (opt_.rotate_180) {
            cv::rotate(frame, frame, cv::ROTATE_180);
        }
        cv::resize(frame, frame, {opt_.display_width, opt_.display_height});

        auto dets = detector.infer(frame);
        bool alarm = false;
        Visualizer::drawRoi(frame, roi, false);
        Visualizer::drawDetections(frame, dets, roi, detector.personClassId(), alarm);

        if (frame_cb_) frame_cb_(frame);
    }

    cleanup();
    return true;
}

void PersonDetectorApp::stop() {
    running_ = false;
    cleanup();
}

void PersonDetectorApp::cleanup() {
    if (rtsp_cap_) {
        rtsp_cap_->stop();
        rtsp_cap_.reset();
    }
    if (cam_cap_) {
        cam_cap_->release();
        cam_cap_.reset();
    }
}
