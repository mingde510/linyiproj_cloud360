// RtspCapture.hpp
#pragma once
#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include <atomic>

class RtspCapture {
public:
    RtspCapture(const std::string& url, int maxFail = 50);
    ~RtspCapture();

    bool open();
    cv::Mat getLatestFrame();   // 拿最新帧拷贝
    void start();
    void stop();

private:
    void grabLoop();

    std::string rtspUrl_;
    int maxFail_;
    cv::VideoCapture cap_;

    std::thread th_;
    std::mutex mtx_;
    cv::Mat latest_;
    std::atomic<bool> running_{false};
    int failCount_{0};
};
