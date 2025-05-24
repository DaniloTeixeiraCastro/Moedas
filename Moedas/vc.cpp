#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <opencv2/core.hpp>
#include "vc.h"

int idBlobs(cv::Mat frameIn, cv::Mat frameOut, int hueMin, int hueMax, float satMin, float satMax, int valueMin, int valueMax) {
    if (frameIn.empty() || frameOut.empty() || frameIn.cols <= 0 || frameIn.rows <= 0 || frameIn.channels() != 3 ||
        frameIn.cols != frameOut.cols || frameIn.rows != frameOut.rows) return 0;
    cv::Mat hsv;
    cv::cvtColor(frameIn, hsv, cv::COLOR_BGR2HSV);
    cv::inRange(hsv, cv::Scalar(hueMin, satMin, valueMin), cv::Scalar(hueMax, satMax, valueMax), frameOut);
    return 1;
}

int verificaPassouAntes(OVC* passou, OVC moedas, int cont) {
    if (cont == 0) return 1;
    return (moedas.xc < passou[cont - 1].xc - 10 || moedas.xc > passou[cont - 1].xc + 10) ? 1 : 0;
}


int idMoeda(int area, int perimeter, float circularity, cv::Vec3b meanColor) {
    // Nota: O parâmetro meanColor não é utilizado na classificação atual, 
    // mas está mantido para compatibilidade e possível uso futuro
    
    if (area > 35000 || area < 3000) return 0;
    if (circularity < 0.05) return 0;
 
    if (area >= 26000 && area < 29000 && perimeter >= 700 && perimeter < 850) return 200;

    else if (area >= 20610 && area < 24000 && perimeter >= 600 && perimeter < 750) return 100;

    else if (area >= 24000 && area < 26000 && perimeter >= 600 && perimeter < 800) return 50;

    else if (area >= 19500 && area < 22000 && perimeter >= 550 && perimeter < 650) return 20;
 
    else if (area >= 16000 && area < 17500 && perimeter >= 500 && perimeter < 650) return 10;

    else if (area >= 17500 && area < 20000 && perimeter >= 550 && perimeter < 700) return 5;
 
    else if (area >= 12500 && area < 15500 && perimeter >= 450 && perimeter < 600) return 2;
 
    else if (area >= 8000 && area < 12500 && perimeter >= 350 && perimeter < 950) return 1;
    
    else return 0;
}

void escreverInfo(FILE* fp, int cont, int mTotal, int m200, int m100, int m50, int m20, int m10, int m5, int m2, int m1, const char* videofile) {
    if (!fp) return;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(fp, "== VIDEO: %s | HORA: %d-%d-%d %d:%d:%d ==\n", videofile, tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
    fprintf(fp, "NUMERO DE MOEDAS: %d\n", mTotal);
    fprintf(fp, " -2 EUROS: %d\n", m200);
    fprintf(fp, " -1 EURO: %d\n", m100);
    fprintf(fp, " -50 CENT: %d\n", m50);
    fprintf(fp, " -20 CENT: %d\n", m20);
    fprintf(fp, " -10 CENT: %d\n", m10);
    fprintf(fp, " -5 CENT: %d\n", m5);
    fprintf(fp, " -2 CENT: %d\n", m2);
    fprintf(fp, " -1 CENT: %d\n\n", m1);
}



OVC* vc_binary_blob_labelling(cv::Mat src, cv::Mat dst, int* nlabels) {
    if (src.empty() || dst.empty() || src.cols <= 0 || src.rows <= 0 || src.channels() != 1 ||
        src.cols != dst.cols || src.rows != dst.rows || dst.channels() != 1) return NULL;
    cv::Mat gray = src.clone();
    for (int y = 0; y < gray.rows; ++y) {
        for (int x = 0; x < gray.cols; ++x) {
            uchar& pixel = gray.at<uchar>(y, x);
            pixel = (pixel > 127) ? 255 : 0;
        }
    }
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(gray, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    *nlabels = contours.size();
    if (*nlabels == 0) return NULL;
    OVC* blobs = (OVC*)calloc(*nlabels, sizeof(OVC));
    if (!blobs) return NULL;
    for (int i = 0; i < *nlabels; i++) blobs[i].label = i + 1;
    return blobs;
}

int vc_binary_blob_info(cv::Mat src, OVC* blobs, int nblobs) {
    if (src.empty() || src.cols <= 0 || src.rows <= 0 || src.channels() != 1) return 0;
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(src.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    for (int i = 0; i < nblobs && i < contours.size(); i++) {
        cv::Rect bbox = cv::boundingRect(contours[i]);
        blobs[i].x = bbox.x; blobs[i].y = bbox.y;
        blobs[i].width = bbox.width; blobs[i].height = bbox.height;
        blobs[i].area = cv::contourArea(contours[i]);
        blobs[i].perimeter = cv::arcLength(contours[i], true);
        cv::Moments m = cv::moments(contours[i]);
        blobs[i].xc = m.m10 / MAX(m.m00, 1); blobs[i].yc = m.m01 / MAX(m.m00, 1);
        blobs[i].circularity = (blobs[i].perimeter > 0) ? (4.0 * CV_PI * blobs[i].area) / (blobs[i].perimeter * blobs[i].perimeter) : 0.0;
    }
    return 1;
}

int vc_desenha_bounding_box(cv::Mat src, OVC blobs) {
    if (src.empty() || src.cols <= 0 || src.rows <= 0 || src.channels() != 3) return 0;
    
    float raio = sqrt(blobs.area / CV_PI);
    
    cv::circle(src, cv::Point(blobs.xc, blobs.yc), static_cast<int>(raio), cv::Scalar(0, 0, 255), 1);
    
    cv::circle(src, cv::Point(blobs.xc, blobs.yc), 3, cv::Scalar(0, 0, 255), -1);
    
    return 1;
}

int desenha_linhaVerde(cv::Mat frame) {
    if (frame.empty() || frame.cols <= 0 || frame.rows <= 0 || frame.channels() != 3) return 0;
    int y = frame.rows / 4;
    for (int x = 0; x < frame.cols; ++x) {
        frame.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 255, 0);
    }
    return 1;
}

int desenha_linhaVermelha(cv::Mat frame) {
    if (frame.empty() || frame.cols <= 0 || frame.rows <= 0 || frame.channels() != 3) return 0;
    int y = frame.rows / 4;
    for (int x = 0; x < frame.cols; ++x) {
        frame.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 255);
    }
    return 1;
}

