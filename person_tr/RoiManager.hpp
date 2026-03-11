// RoiManager.hpp
#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

class RoiManager {
public:
    bool load(const std::string& path);
    bool save(const std::string& path) const;

    void clear();
    void addPoint(int x, int y);
    bool isDefined() const { return points_.size() >= 3; }

    const std::vector<cv::Point>& points() const { return points_; }
    bool contains(int x, int y) const;

private:
    std::vector<cv::Point> points_;
};
