// TrtYoloDetector.hpp
#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

struct Detection {
    cv::Rect box;
    float conf;
    int cls;
};

class TrtYoloDetector {
public:
    TrtYoloDetector(const std::string& enginePath, int inputW=640, int inputH=640);
    ~TrtYoloDetector();

    bool init();
    std::vector<Detection> infer(const cv::Mat& bgr);

    // 浣犵殑 names 閲?person 瀵瑰簲 id
    int personClassId() const { return personId_; }
    void setConfidenceThreshold(float c) { confThr_ = c; }

    static inline float sigmoid(float x) {
        return 1.f / (1.f + std::exp(-x));
    }

private:
    void preprocess(const cv::Mat& bgr, float* gpuInput);
    std::vector<Detection> postprocess(const float* gpuOutput);

private:
    std::string enginePath_;
    int inW_, inH_;
    int personId_{0}; // 渚濇嵁妯″瀷绫诲埆琛ㄨ缃?
    // TensorRT 璧勬簮锛堢渷鐣ュ叿浣撶被鍨嬶級
    void* runtime_{nullptr};
    void* engine_{nullptr};
    void* context_{nullptr};
    void* stream_{nullptr};

    void* dInput_{nullptr};
    void* dOutput_{nullptr};

    // TrtYoloDetector.hpp 閲?private 鍖哄煙鍔狅細
    int inputIndex_{-1}, outputIndex_{-1};
    int batch_{1}, c_{3};
    int outN_{0}, outC_{0};
    int origW_{0}, origH_{0};
    float scale_{1.f};
    int padLeft_{0}, padTop_{0};

    std::vector<float> hInput_{std::vector<float>(3*640*640)};
    std::vector<float> hOutput_;

    float confThr_{0.25f};
    float iouThr_{0.35f};
};
