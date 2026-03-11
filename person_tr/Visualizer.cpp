// Visualizer.cpp
#include "Visualizer.hpp"

void Visualizer::drawRoi(cv::Mat& img, const RoiManager& roi, bool drawing) {
    const auto& pts = roi.points();
    if (pts.empty()) return;
    cv::polylines(img, pts, roi.isDefined(), 
                  drawing ? cv::Scalar(0,255,255) : cv::Scalar(255,0,0), 2);
    if (drawing) {
        for (auto& p: pts) cv::circle(img, p, 4, cv::Scalar(0,255,255), -1);
    }
}

void Visualizer::drawDetections(cv::Mat& img, const std::vector<Detection>& dets, 
                                const RoiManager& roi, int personId, bool& alarm) {
    alarm = false;
    for (auto& d : dets) {
        if (d.cls != personId) continue;
        int cx = d.box.x + d.box.width/2;
        int cy = d.box.y + d.box.height/2;

        bool inside = roi.contains(cx, cy);
        cv::Scalar color = inside ? cv::Scalar(0,0,255) : cv::Scalar(0,255,0);
        if (inside) alarm = true;

        cv::rectangle(img, d.box, color, 2);
        cv::circle(img, {cx,cy}, 3, color, -1);

        char text[64];
        snprintf(text, sizeof(text), "person %.2f", d.conf);
        int baseline=0;
        auto sz=cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX,0.5,1,&baseline);
        cv::rectangle(img, {d.box.x, d.box.y-sz.height-baseline},
                      {d.box.x+sz.width, d.box.y}, color, -1);
        cv::putText(img, text, {d.box.x, d.box.y-2},
                    cv::FONT_HERSHEY_SIMPLEX,0.5,{255,255,255},1);
    }

    if (alarm) {
        cv::putText(img, "ALARM IN ROI", {30,50},
                    cv::FONT_HERSHEY_SIMPLEX,1.2,{0,0,255},2);
    }
}
