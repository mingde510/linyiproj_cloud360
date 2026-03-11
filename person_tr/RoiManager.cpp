// RoiManager.cpp
#include "RoiManager.hpp"
#include <fstream>
#include <iostream>

bool RoiManager::load(const std::string& path) {
    std::ifstream fin(path);
    if (!fin.is_open()) return false;
    points_.clear();
    int x, y;
    while (fin >> x >> y) points_.emplace_back(x, y);
    if (points_.size() < 3) {
        points_.clear();
        return false;
    }
    std::cout << "[INFO] ROI loaded, pts=" << points_.size() << "\n";
    return true;
}

bool RoiManager::save(const std::string& path) const {
    if (points_.size() < 3) return false;
    std::ofstream fout(path);
    if (!fout.is_open()) return false;
    for (auto& p : points_) fout << p.x << " " << p.y << "\n";
    std::cout << "[INFO] ROI saved to " << path << "\n";
    return true;
}

void RoiManager::clear() { points_.clear(); }
void RoiManager::addPoint(int x, int y) { points_.emplace_back(x, y); }

bool RoiManager::contains(int x, int y) const {
    if (points_.size() < 3) return false;
    double v = cv::pointPolygonTest(points_, cv::Point2f(x, y), false);
    return v >= 0;
}
