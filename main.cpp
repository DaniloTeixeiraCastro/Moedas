#include <iostream>
#include <opencv2/opencv.hpp>

bool idBlobs(const cv::Mat& frame, cv::Mat& frameThr, int minArea, int maxArea, float minHue, float maxHue, int minSat, int maxSat) {
    cv::Mat hsv;
    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

    cv::Mat mask;
    cv::inRange(hsv, cv::Scalar(minHue, minSat, 0), cv::Scalar(maxHue, maxSat, 255), mask);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty()) {
        return false;
    }

    for (const auto& contour : contours) {
        int area = cv::contourArea(contour);
        if (area < minArea || area > maxArea) {
            continue;
        }

        cv::Rect boundingRect = cv::boundingRect(contour);
        cv::Mat roi = frame(boundingRect);

        cv::Mat hsvRoi;
        cv::cvtColor(roi, hsvRoi, cv::COLOR_BGR2HSV);

        float hue = cv::mean(hsvRoi.col(0))[0];
        float sat = cv::mean(hsvRoi.col(1))[0];

        if (hue >= minHue && hue <= maxHue && sat >= minSat && sat <= maxSat) {
            cv::rectangle(frame, boundingRect, cv::Scalar(0, 255, 0), 2);
        }
    }

    return true;
}

int main() {
    cv::VideoCapture cap("video.mp4");
    if (!cap.isOpened()) {
        std::cerr << "Erro ao abrir o vídeo!\n";
        return -1;
    }

    cv::Mat frame, frameThr;

    while (true) {
        cap >> frame;
        if (frame.empty()) {
            break;
        }

        if (!idBlobs(frame, frameThr, 0, 170, 0.0f, 255.0f, 0, 220)) {
            std::cerr << "Erro na segmentação HSV!\n";
            continue;
        }

        cv::imshow("Frame", frame);
        cv::imshow("Frame Threshold", frameThr);

        if (cv::waitKey(30) == 'q') {
            break;
        }
    }

    return 0;
} 