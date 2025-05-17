// Desabilita warnings de funções não seguras (fopen, etc.)
#define _CRT_SECURE_NO_WARNINGS

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include "vc.h"
#include <iostream>

// Programa principal para processamento de vídeo e detecção de moedas
int main(int argc, const char* argv[]) {
    // Menu de seleção de vídeo
    std::cout << "Escolha o vídeo para processar:\n";
    std::cout << "1 - C:/Moedas/videos/video1.mp4\n";
    std::cout << "2 - C:/Moedas/videos/video2.mp4\n";
    std::cout << "Opcao: ";
    int opcao = 0;
    std::cin >> opcao;
    std::string videofile;
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

    // Abrir vídeo
    cv::VideoCapture capture(videofile);
    if (!capture.isOpened()) {
        std::cerr << "Erro ao abrir ficheiro!\n";
        return 1;
    }

    // Obter propriedades do vídeo
    int totalFrames = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_COUNT)),
        fps = static_cast<int>(capture.get(cv::CAP_PROP_FPS)),
        width = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH)),
        height = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_HEIGHT));

    // Criar janela para exibição
    cv::namedWindow("Visão por Computador - TP2", cv::WINDOW_AUTOSIZE);

    // Variáveis para contagem de moedas
    std::vector<OVC> passou(100);
    int cont = 0; // Contador de moedas processadas
    int mTotal = 0; // Total de moedas detectadas
    float soma = 0.0; // Somatório do valor das moedas
    int m200 = 0, m100 = 0, m50 = 0, m20 = 0, m10 = 0, m5 = 0, m2 = 0, m1 = 0; // Contadores por tipo

    // Abrir ficheiro para salvar resultados
    FILE* fp = fopen("Moedas.txt", "a");
    if (!fp) {
        std::cerr << "Erro ao abrir o ficheiro de guardar moedas!\n";
        return 1;
    }

    cv::Mat frameorig;
    // Processar cada frame do vídeo
    while (true) {
        capture >> frameorig; // Capturar frame atual
        if (frameorig.empty()) break; // Sair se o vídeo terminou

        int currentFrame = static_cast<int>(capture.get(cv::CAP_PROP_POS_FRAMES)); // Obter número do frame atual

        // Inicializar matrizes para processamento
        cv::Mat framethr, frameaux;

        // Converter para espaço HSV para segmentação
        cv::Mat hsv;
        cv::cvtColor(frameorig, hsv, cv::COLOR_BGR2HSV);
        // Segmentar moedas por tonalidade e brilho
        cv::inRange(hsv, cv::Scalar(12, 35, 20), cv::Scalar(150, 255, 150), framethr);

        // Aplicar operação morfológica para remover ruído
        cv::morphologyEx(framethr, framethr, cv::MORPH_OPEN,
            cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)),
            cv::Point(-1, -1), 5);

        // Detectar contornos (blobs) das moedas
        int nMoedas = 0;
        std::vector<OVC> moedas;
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(framethr.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        nMoedas = static_cast<int>(contours.size());
        moedas.resize(nMoedas);

        // Calcular propriedades de cada blob
        for (int i = 0; i < nMoedas; i++) {
            OVC blob;
            cv::Rect bbox = cv::boundingRect(contours[i]);
            blob.x = bbox.x;
            blob.y = bbox.y;
            blob.width = bbox.width;
            blob.height = bbox.height;
            blob.area = static_cast<int>(cv::contourArea(contours[i]));
            blob.perimeter = static_cast<int>(cv::arcLength(contours[i], true));
            blob.xc = blob.x + blob.width / 2;
            blob.yc = blob.y + blob.height / 2;
            // Calcular circularidade (4π*área/perímetro²)
            blob.circularity = (blob.perimeter > 0) ? (4.0 * CV_PI * blob.area) / (blob.perimeter * blob.perimeter) : 0.0;
            moedas[i] = blob;
        }

        // Desenhar linha vermelha de referência
        cv::line(frameorig, cv::Point(0, frameorig.rows / 4),
            cv::Point(frameorig.cols, frameorig.rows / 4),
            cv::Scalar(0, 0, 255), 2);

        // Processar cada moeda detectada
        for (int i = 0; i < nMoedas; i++) {
            if (moedas[i].area > 8000) { // Filtrar blobs pequenos
                // Exibir coordenadas do centro de gravidade
                std::string text = "x: " + std::to_string(moedas[i].xc) + ", y: " + std::to_string(moedas[i].yc);
                cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc - 60),
                    cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 0), 2);
                cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc - 60),
                    cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 8, 255), 1);

                // Exibir área
                text = "AREA: " + std::to_string(moedas[i].area);
                cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc - 40),
                    cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 0), 2);
                cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc - 40),
                    cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 8, 255), 1);

                // Exibir perímetro
                text = "PERIMETRO: " + std::to_string(moedas[i].perimeter);
                cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc - 20),
                    cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 0), 2);
                cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc - 20),
                    cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 8, 255), 1);

                // Exibir circularidade
                text = "CIRCULARIDADE: " + std::to_string(moedas[i].circularity).substr(0, 5);
                cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc),
                    cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 0), 2);
                cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc),
                    cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 8, 255), 1);

                // Exibir tipo de moeda
                int tipo = idMoeda(moedas[i].area, moedas[i].perimeter);
                std::string tipoText;
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

                // Desenhar bounding box
                cv::rectangle(frameorig,
                    cv::Point(moedas[i].x, moedas[i].y),
                    cv::Point(moedas[i].x + moedas[i].width, moedas[i].y + moedas[i].height),
                    cv::Scalar(0, 255, 0), 2);

                // Verificar se a moeda cruzou a linha de referência
                if (frameorig.rows / 4 >= moedas[i].yc - 15 && frameorig.rows / 4 <= moedas[i].yc + 20) {
                    // Desenhar linha verde
                    cv::line(frameorig, cv::Point(0, frameorig.rows / 4),
                        cv::Point(frameorig.cols, frameorig.rows / 4),
                        cv::Scalar(0, 255, 0), 2);

                    // Verificar se a moeda é nova
                    int p = verificaPassouAntes(passou.data(), moedas[i], cont);
                    if (p == 1) {
                        // Atualizar contadores e soma
                        switch (idMoeda(moedas[i].area, moedas[i].perimeter)) {
                        case 200: m200++; soma += 2; break;
                        case 100: m100++; soma += 1; break;
                        case 50: m50++; soma += 0.5; break;
                        case 20: m20++; soma += 0.2; break;
                        case 10: m10++; soma += 0.1; break;
                        case 5: m5++; soma += 0.05; break;
                        case 2: m2++; soma += 0.02; break;
                        case 1: m1++; soma += 0.01; break;
                        }
                        passou[cont] = moedas[i];
                        cont++;
                        mTotal++;
                    }
                }
            }
        }

        // Exibir contadores globais e soma
        std::string text = "NR TOTAL DE MOEDAS: " + std::to_string(mTotal);
        cv::putText(frameorig, text, cv::Point(20, 30),
            cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 0), 1);
        text = "TOTAL: €" + std::to_string(soma).substr(0, std::to_string(soma).find(".") + 3);
        cv::putText(frameorig, text, cv::Point(20, 50),
            cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 0), 1);
        int y_offset = 70;
        text = "2 EUR: " + std::to_string(m200);
        cv::putText(frameorig, text, cv::Point(20, y_offset),
            cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 0), 1);
        y_offset += 20;
        text = "1 EUR: " + std::to_string(m100);
        cv::putText(frameorig, text, cv::Point(20, y_offset),
            cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 0), 1);
        y_offset += 20;
        text = "50 CENT: " + std::to_string(m50);
        cv::putText(frameorig, text, cv::Point(20, y_offset),
            cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 0), 1);
        y_offset += 20;
        text = "20 CENT: " + std::to_string(m20);
        cv::putText(frameorig, text, cv::Point(20, y_offset),
            cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 0), 1);
        y_offset += 20;
        text = "10 CENT: " + std::to_string(m10);
        cv::putText(frameorig, text, cv::Point(20, y_offset),
            cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 0), 1);
        y_offset += 20;
        text = "5 CENT: " + std::to_string(m5);
        cv::putText(frameorig, text, cv::Point(20, y_offset),
            cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 0), 1);
        y_offset += 20;
        text = "2 CENT: " + std::to_string(m2);
        cv::putText(frameorig, text, cv::Point(20, y_offset),
            cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 0), 1);
        y_offset += 20;
        text = "1 CENT: " + std::to_string(m1);
        cv::putText(frameorig, text, cv::Point(20, y_offset),
            cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 0), 1);

        // Exibir frame processado
        cv::imshow("Visão por Computador - TP2", frameorig);

        // Sair com a tecla 'q'
        int key = cv::waitKey(80);
        if (key == 'q') break;
    }

    // Liberar recursos
    cv::destroyWindow("Visão por Computador - TP2");
    capture.release();

    // Salvar informações no ficheiro
    escreverInfo(fp, cont, mTotal, m200, m100, m50, m20, m10, m5, m2, m1, videofile.c_str());
    fclose(fp);

    return 0;
}