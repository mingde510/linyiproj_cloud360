// RtspCapture.cpp
#include "RtspCapture.hpp"
#include <iostream>
#include <chrono>

RtspCapture::RtspCapture(const std::string& url, int maxFail)
    : rtspUrl_(url), maxFail_(maxFail) {}

RtspCapture::~RtspCapture() { stop(); }

bool RtspCapture::open() {
    std::lock_guard<std::mutex> lk(mtx_);
    if (cap_.isOpened()) cap_.release();
    std::cout << "[INFO] open RTSP: " << rtspUrl_ << std::endl;
    cap_.open(rtspUrl_, cv::CAP_FFMPEG);
    cap_.set(cv::CAP_PROP_BUFFERSIZE, 1);
    if (!cap_.isOpened()) {
        std::cerr << "[ERROR] cannot open RTSP\n";
        return false;
    }
    return true;
}

void RtspCapture::start() {
    running_ = true;
    th_ = std::thread(&RtspCapture::grabLoop, this);
}

void RtspCapture::stop() {
    running_ = false;
    if (th_.joinable()) th_.join();
    std::lock_guard<std::mutex> lk(mtx_);
    if (cap_.isOpened()) cap_.release();
}

cv::Mat RtspCapture::getLatestFrame() {
    std::lock_guard<std::mutex> lk(mtx_);
    return latest_.empty() ? cv::Mat() : latest_.clone();
}

void RtspCapture::grabLoop() {
    while (running_) {
        {
            std::lock_guard<std::mutex> lk(mtx_);
            if (!cap_.isOpened()) {
                open();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                continue;
            }
            cv::Mat frame;
            if (!cap_.read(frame)) {
                failCount_++;
                if (failCount_ >= maxFail_) {
                    std::cout << "[INFO] reconnect RTSP...\n";
                    open();
                    failCount_ = 0;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                continue;
            }
            failCount_ = 0;
            latest_ = frame;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
