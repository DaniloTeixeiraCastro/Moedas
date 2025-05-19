#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <opencv2/core.hpp>
#include "vc.h"

// Segmentar blobs com base em intervalos HSV
int idBlobs(cv::Mat frameIn, cv::Mat frameOut, int hueMin, int hueMax, float satMin, float satMax, int valueMin, int valueMax) {
    if (frameIn.empty() || frameOut.empty() || frameIn.cols <= 0 || frameIn.rows <= 0 || frameIn.channels() != 3 ||
        frameIn.cols != frameOut.cols || frameIn.rows != frameOut.rows) return 0;
    cv::Mat hsv;
    cv::cvtColor(frameIn, hsv, cv::COLOR_BGR2HSV);
    cv::inRange(hsv, cv::Scalar(hueMin, satMin, valueMin), cv::Scalar(hueMax, satMax, valueMax), frameOut);
    return 1;
}

// Verifica se a moeda já foi contada
int verificaPassouAntes(OVC* passou, OVC moedas, int cont) {
    if (cont == 0) return 1;
    return (moedas.xc < passou[cont - 1].xc - 10 || moedas.xc > passou[cont - 1].xc + 10) ? 1 : 0;
}

// Classifica o tipo de moeda com base em área, perímetro e circularidade
int idMoeda(int area, int perimeter, float circularity, cv::Vec3b meanColor) {
    // Filtros gerais
    if (area > 35000 || area < 3000) return 0;
    if (circularity < 0.05) return 0;

    // 2 EUR: 
    if (area >= 26000 && area < 29000 && perimeter >= 700 && perimeter < 800) return 200;

    // 1 EUR:
    else if (area >= 20500 && area < 24000 && perimeter >= 600 && perimeter < 750) return 100;

    // 50 CENT:
    else if (area >= 24000 && area < 26000 && perimeter >= 600 && perimeter < 800) return 50;

    // 20 CENT:
    else if (area >= 19500 && area < 22000 && perimeter >= 550 && perimeter < 650) return 20;

    // 10 CENT: 
    else if (area >= 16000 && area < 17500 && perimeter >= 500 && perimeter < 650) return 10;

    // 5 CENT:
    else if (area >= 17500 && area < 20000 && perimeter >= 550 && perimeter < 700) return 5;

    // 2 CENT: 
    else if (area >= 12500 && area < 15500 && perimeter >= 450 && perimeter < 550) return 2;

    // 1 CENT: 
    else if (area >= 8000 && area < 12500 && perimeter >= 350 && perimeter < 700) return 1;
    
    else return 0;
}

// Salva informações das moedas
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

// Aloca memória para uma imagem IVC
IVC* vc_image_new(int width, int height, int channels, int levels) {
    IVC* image = (IVC*)malloc(sizeof(IVC));
    if (!image || levels <= 0 || levels > 255) return NULL;
    image->width = width; image->height = height; image->channels = channels; image->levels = levels;
    image->bytesperline = width * channels;
    image->data = (unsigned char*)malloc(width * height * channels * sizeof(char));
    return image->data ? image : vc_image_free(image);
}

// Liberta memória de uma imagem IVC
IVC* vc_image_free(IVC* image) {
    if (image) {
        free(image->data);
        free(image);
    }
    return NULL;
}

// Lê uma imagem PBM, PGM ou PPM
IVC* vc_read_image(char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) return NULL;
    IVC* image = NULL; unsigned char* tmp; char tok[20]; int width, height, channels = 1, levels = 255, v;
    netpbm_get_token(file, tok, sizeof(tok));
    if (strcmp(tok, "P4") == 0) levels = 1;
    else if (strcmp(tok, "P5") == 0) channels = 1;
    else if (strcmp(tok, "P6") == 0) channels = 3;
    else { fclose(file); return NULL; }
    if (levels == 1) {
        if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
            sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1) {
            fclose(file); return NULL;
        }
        image = vc_image_new(width, height, channels, levels);
        if (!image) return NULL;
        long int sizeofbinarydata = (width / 8 + (width % 8 ? 1 : 0)) * height;
        tmp = (unsigned char*)malloc(sizeofbinarydata);
        if (!tmp || fread(tmp, 1, sizeofbinarydata, file) != sizeofbinarydata) {
            vc_image_free(image); fclose(file); free(tmp); return NULL;
        }
        bit_to_unsigned_char(tmp, image->data, width, height);
        free(tmp);
    }
    else {
        if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
            sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
            sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255) {
            fclose(file); return NULL;
        }
        image = vc_image_new(width, height, channels, levels);
        if (!image || fread(image->data, 1, width * height * channels, file) != width * height * channels) {
            vc_image_free(image); fclose(file); return NULL;
        }
    }
    fclose(file);
    return image;
}

// Escreve uma imagem PBM, PGM ou PPM
int vc_write_image(char* filename, IVC* image) {
    if (!image) return 0;
    FILE* file = fopen(filename, "wb");
    if (!file) return 0;
    if (image->levels == 1) {
        unsigned char* tmp = (unsigned char*)malloc((image->width / 8 + (image->width % 8 ? 1 : 0)) * image->height + 1);
        if (!tmp) { fclose(file); return 0; }
        fprintf(file, "P4 %d %d\n", image->width, image->height);
        long int totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
        if (fwrite(tmp, 1, totalbytes, file) != totalbytes) {
            fclose(file); free(tmp); return 0;
        }
        free(tmp);
    }
    else {
        fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);
        if (fwrite(image->data, image->bytesperline, image->height, file) != image->height) {
            fclose(file); return 0;
        }
    }
    fclose(file);
    return 1;
}

// Realiza a etiquetagem de blobs
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

// Extrai informações de blobs
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

// Desenha círculo em volta da moeda e centro
int vc_desenha_bounding_box(cv::Mat src, OVC blobs) {
    if (src.empty() || src.cols <= 0 || src.rows <= 0 || src.channels() != 3) return 0;
    
    // Calcular o raio baseado na área da moeda
    float raio = sqrt(blobs.area / CV_PI);
    
    // Desenhar círculo vermelho ao redor da moeda
    cv::circle(src, cv::Point(blobs.xc, blobs.yc), static_cast<int>(raio), cv::Scalar(0, 0, 255), 1);
    
    // Desenhar centroide (círculo pequeno vermelho)
    cv::circle(src, cv::Point(blobs.xc, blobs.yc), 3, cv::Scalar(0, 0, 255), -1);
    
    return 1;
}

// Desenha linha verde de referência
int desenha_linhaVerde(cv::Mat frame) {
    if (frame.empty() || frame.cols <= 0 || frame.rows <= 0 || frame.channels() != 3) return 0;
    int y = frame.rows / 4;
    for (int x = 0; x < frame.cols; ++x) {
        frame.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 255, 0);
    }
    return 1;
}

// Desenha linha vermelha de referência
int desenha_linhaVermelha(cv::Mat frame) {
    if (frame.empty() || frame.cols <= 0 || frame.rows <= 0 || frame.channels() != 3) return 0;
    int y = frame.rows / 4;
    for (int x = 0; x < frame.cols; ++x) {
        frame.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 255);
    }
    return 1;
}

// Lê um token de um ficheiro NetPBM
char* netpbm_get_token(FILE* file, char* tok, int len) {
    char* t; int c;
    for (;;) { while (isspace(c = getc(file))); if (c != '#') break; do c = getc(file); while (c != '\n' && c != EOF); if (c == EOF) break; }
    t = tok; if (c != EOF) do { *t++ = c; c = getc(file); } while (!isspace(c) && c != '#' && c != EOF && t - tok < len - 1);
    if (c == '#') ungetc(c, file); *t = 0; return tok;
}

// Converte dados de unsigned char para bits
long int unsigned_char_to_bit(unsigned char* datauchar, unsigned char* databit, int width, int height) {
    int x, y, countbits; long int pos, counttotalbytes; unsigned char* p = databit;
    *p = 0; countbits = 1; counttotalbytes = 0;
    for (y = 0; y < height; y++) for (x = 0; x < width; x++) {
        pos = width * y + x; if (countbits <= 8) { *p |= (datauchar[pos] == 0) << (8 - countbits); countbits++; }
        if (countbits > 8 || x == width - 1) { p++; *p = 0; countbits = 1; counttotalbytes++; }
    }
    return counttotalbytes;
}

// Converte dados de bits para unsigned char
void bit_to_unsigned_char(unsigned char* databit, unsigned char* datauchar, int width, int height) {
    int x, y, countbits; long int pos; unsigned char* p = databit;
    countbits = 1; for (y = 0; y < height; y++) for (x = 0; x < width; x++) {
        pos = width * y + x; if (countbits <= 8) datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;
        if (countbits > 8 || x == width - 1) { p++; countbits = 1; }
    }
}