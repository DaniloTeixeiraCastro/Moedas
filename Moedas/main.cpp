#define _CRT_SECURE_NO_WARNINGS

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include "vc.h"
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

int main(int argc, const char* argv[]) {
    std::string videofile;
    if (argc == 2) {

        videofile = argv[1];
    }
    else {
        std::cout << "Escolha o vídeo para processar:\n";
        std::cout << "1 - C:/Moedas/videos/video1.mp4\n";
        std::cout << "2 - C:/Moedas/videos/video2.mp4\n";
        std::cout << "Opcao: ";
        int opcao = 0;
        std::cin >> opcao;
        if (opcao == 1) {
            videofile = "C:/Moedas/videos/video1.mp4";
        }
        else if (opcao == 2) {
            videofile = "C:/Moedas/videos/video2.mp4";
        }
        else {
            std::cerr << "Opção inválida!\n";
            return 1;
        }
    }

    cv::VideoCapture capture(videofile);
    if (!capture.isOpened()) {
        std::cerr << "Erro ao abrir ficheiro!\n";
        return 1;
    }

    int totalFrames = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_COUNT)),
        fps = static_cast<int>(capture.get(cv::CAP_PROP_FPS)),
        width = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH)),
        height = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_HEIGHT));

    cv::namedWindow("Visão por Computador - TP2", cv::WINDOW_AUTOSIZE);

    std::vector<OVC> passou;
    int cont = 0, mTotal = 0;
    float soma = 0.0;
    int m200 = 0, m100 = 0, m50 = 0, m20 = 0, m10 = 0, m5 = 0, m2 = 0, m1 = 0;

    FILE* fp = fopen("Moedas.txt", "a");
    if (!fp) {
        std::cerr << "Erro ao abrir o ficheiro de guardar moedas!\n";
        return 1;
    }

    cv::Mat frameorig;
    bool paused = false;
    std::cout << "Pressione 'q' para sair, 'p' para pausar.\n";
    while (true) {
        if (!paused) {
            capture >> frameorig;
            if (frameorig.empty()) {
                std::cout << "Fim do vídeo ou erro na captura.\n";
                break;
            }
        }

        int currentFrame = static_cast<int>(capture.get(cv::CAP_PROP_POS_FRAMES));

        cv::Mat framethr = cv::Mat::zeros(frameorig.size(), CV_8UC1);

        if (!idBlobs(frameorig, framethr, 12, 150, 35.0f, 255.0f, 20, 150)) {
            std::cerr << "Erro na segmentação HSV!\n"; continue;
        }

        int nMoedas = 0;
        OVC* moedas = vc_binary_blob_labelling(framethr, framethr, &nMoedas);
        if (!moedas && nMoedas > 0) { std::cerr << "Erro na etiquetagem!\n"; continue; }
        if (nMoedas > 0 && !vc_binary_blob_info(framethr, moedas, nMoedas)) {
            std::cerr << "Erro no cálculo de propriedades dos blobs!\n"; free(moedas); continue;
        }

        desenha_linhaVermelha(frameorig);
        for (int i = 0; i < nMoedas; i++) {
            if (moedas[i].area > 8000) {
                moedas[i].circularity = (moedas[i].perimeter > 0) ? 
                    (4.0f * CV_PI * moedas[i].area) / (moedas[i].perimeter * moedas[i].perimeter) : 0.0f;

                std::string text = "x: " + std::to_string(moedas[i].xc) + ", y: " + std::to_string(moedas[i].yc);
                cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc - 60),
                    cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 0), 2);
                cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc - 60),
                    cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 8, 255), 1);

                text = "AREA: " + std::to_string(moedas[i].area);
                cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc - 40),
                    cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 0), 2);
                cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc - 40),
                    cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 8, 255), 1);

                text = "PERIMETRO: " + std::to_string(moedas[i].perimeter);
                cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc - 20),
                    cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 0), 2);
                cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc - 20),
                    cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 8, 255), 1);

                text = "CIRCULARIDADE: " + std::to_string(moedas[i].circularity).substr(0, 5);
                cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc),
                    cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 0), 2);
                cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc),
                    cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 8, 255), 1);

                // Calcular média de cor do blob
                cv::Scalar meanColorScalar = cv::mean(frameorig(cv::Rect(moedas[i].x, moedas[i].y, moedas[i].width, moedas[i].height)));
                cv::Vec3b meanColor((uchar)meanColorScalar[0], (uchar)meanColorScalar[1], (uchar)meanColorScalar[2]);
                // Só processar moedas com circularidade razoável e tipo reconhecido
                int tipo = idMoeda(moedas[i].area, moedas[i].perimeter, moedas[i].circularity, meanColor);
                std::string tipoText;
                if (tipo != 0 && moedas[i].circularity > 0.55) {
                    switch (tipo) {
                    case 200: tipoText = "2 EUR"; break;
                    case 100: tipoText = "1 EUR"; break;
                    case 50: tipoText = "50 CENT"; break;
                    case 20: tipoText = "20 CENT"; break;
                    case 10: tipoText = "10 CENT"; break;
                    case 5: tipoText = "5 CENT"; break;
                    case 2: tipoText = "2 CENT"; break;
                    case 1: tipoText = "1 CENT"; break;
                    default: tipoText = "DESCONHECIDO"; break;
                    }
                    text = "TIPO: " + tipoText;
                    cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc + 20),
                        cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 0), 2);
                    cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc + 20),
                        cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 0), 1);

                    vc_desenha_bounding_box(frameorig, moedas[i]);

                    if (height / 4 >= moedas[i].yc - 15 && height / 4 <= moedas[i].yc + 20) {
                        desenha_linhaVerde(frameorig);
                        if (passou.size() == 0) {
                            passou.push_back(moedas[i]);
                            cont++; mTotal++;
                            switch (tipo) {
                            case 200: m200++; soma += 2.0f; break;
                            case 100: m100++; soma += 1.0f; break;
                            case 50: m50++; soma += 0.5f; break;
                            case 20: m20++; soma += 0.2f; break;
                            case 10: m10++; soma += 0.1f; break;
                            case 5: m5++; soma += 0.05f; break;
                            case 2: m2++; soma += 0.02f; break;
                            case 1: m1++; soma += 0.01f; break;
                            }
                        }
                        else {
                            int p = verificaPassouAntes(passou.data(), moedas[i], cont);
                            if (p == 1) {
                                passou.push_back(moedas[i]);
                                cont++; mTotal++;
                                switch (tipo) {
                                case 200: m200++; soma += 2.0f; break;
                                case 100: m100++; soma += 1.0f; break;
                                case 50: m50++; soma += 0.5f; break;
                                case 20: m20++; soma += 0.2f; break;
                                case 10: m10++; soma += 0.1f; break;
                                case 5: m5++; soma += 0.05f; break;
                                case 2: m2++; soma += 0.02f; break;
                                case 1: m1++; soma += 0.01f; break;
                                }
                            }
                        }
                    }
                }
            }
        }

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << soma;
        std::string text = "NR TOTAL DE MOEDAS: " + std::to_string(mTotal);
        cv::putText(frameorig, text, cv::Point(20, 30), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 0), 1);
        text = "TOTAL: €" + oss.str();
        cv::putText(frameorig, text, cv::Point(20, 50), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 0), 1);
        int y_offset = 70;
        text = "2 EUR: " + std::to_string(m200); cv::putText(frameorig, text, cv::Point(20, y_offset), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 0), 1); y_offset += 20;
        text = "1 EUR: " + std::to_string(m100); cv::putText(frameorig, text, cv::Point(20, y_offset), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 0), 1); y_offset += 20;
        text = "50 CENT: " + std::to_string(m50); cv::putText(frameorig, text, cv::Point(20, y_offset), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 0), 1); y_offset += 20;
        text = "20 CENT: " + std::to_string(m20); cv::putText(frameorig, text, cv::Point(20, y_offset), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 0), 1); y_offset += 20;
        text = "10 CENT: " + std::to_string(m10); cv::putText(frameorig, text, cv::Point(20, y_offset), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 0), 1); y_offset += 20;
        text = "5 CENT: " + std::to_string(m5); cv::putText(frameorig, text, cv::Point(20, y_offset), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 0), 1); y_offset += 20;
        text = "2 CENT: " + std::to_string(m2); cv::putText(frameorig, text, cv::Point(20, y_offset), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 0), 1); y_offset += 20;
        text = "1 CENT: " + std::to_string(m1); cv::putText(frameorig, text, cv::Point(20, y_offset), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 0), 1);

        cv::imshow("Visão por Computador - TP2", frameorig);
        cv::waitKey(1);
        int key = cv::waitKey(33);
        if (key == 'q') break;
        if (key == 'p') paused = !paused;

        if (moedas) { free(moedas); moedas = NULL; }
    }

    escreverInfo(fp, cont, mTotal, m200, m100, m50, m20, m10, m5, m2, m1, videofile.c_str());
    fclose(fp);
    capture.release();
    cv::destroyAllWindows();
    std::cout << "Programa terminado.\n";
    return 0;
}