#include <opencv2/opencv.hpp>
#include "RtspCapture.hpp"
#include "RoiManager.hpp"
#include "TrtYoloDetector.hpp"
#include "Visualizer.hpp"
#include <iostream>

static bool drawing = false;
static RoiManager roi;

void onMouse(int event, int x, int y, int, void*) {
    if (!drawing) return;
    if (event == cv::EVENT_LBUTTONDOWN) {
        roi.addPoint(x, y);
        std::cout << "[INFO] add ROI pt: " << x << "," << y << "\n";
    }
}

int main() {
    // ---- config ----
    std::string rtspUrl = "rtsp://admin:smartsense1008@192.168.31.64:554/Streaming/Channels/1";
    std::string roiFile = "roi_points.txt";
    int DISP_W=1080, DISP_H=720;

    // ---- init ----
    RtspCapture cap(rtspUrl);
    if (!cap.open()) return -1;
    cap.start();

    roi.load(roiFile);

    TrtYoloDetector detector("best_person_fp16.engine");
    if (!detector.init()) return -2;

    cv::namedWindow("Person ROI Alarm", 1);
    cv::setMouseCallback("Person ROI Alarm", onMouse);

    while (true) {
        cv::Mat frame = cap.getLatestFrame();
        //frame = cv::imread("test2.jpg");
        if (frame.empty()) { cv::waitKey(1); continue; }

        //cv::rotate(frame, frame, cv::ROTATE_180);
        cv::rotate(frame, frame, cv::ROTATE_180);
        cv::resize(frame, frame, {DISP_W, DISP_H});

        auto dets = detector.infer(frame);

        //std::cout << dets.size() << std::endl;

        bool alarm=false;
        Visualizer::drawRoi(frame, roi, drawing);
        Visualizer::drawDetections(frame, dets, roi, detector.personClassId(), alarm);

        cv::imshow("Person ROI Alarm", frame);

        int k = cv::waitKey(1);
        if (k == 27 || k=='q') break;         // ESC/Q quit
        if (k == 's') {                      // start draw
            roi.clear(); drawing=true;
            std::cout << "[INFO] start draw ROI\n";
        }
        if (k == 'f') {                      // finish draw
            if (roi.isDefined()) roi.save(roiFile);
            drawing=false;
            std::cout << "[INFO] finish draw ROI\n";
        }
    }

    cap.stop();
    return 0;
}
