// Visualizer.hpp
#pragma once
#include <opencv2/opencv.hpp>
#include "RoiManager.hpp"
#include "TrtYoloDetector.hpp"

class Visualizer {
public:
    static void drawRoi(cv::Mat& img, const RoiManager& roi, bool drawing);
    static void drawDetections(cv::Mat& img, const std::vector<Detection>& dets, 
                               const RoiManager& roi, int personId, bool& alarm);
};
