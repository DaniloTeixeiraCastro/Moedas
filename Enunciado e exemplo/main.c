#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

extern "C" {
#include "vc.h"
}

// Timer function from the example code
void vc_timer(void) {
    static int running = 0;
    static clock_t start_time;
    
    if (running == 0) {
        running = 1;
        start_time = clock();
    }
    else {
        double elapsed_time = ((double)(clock() - start_time)) / CLOCKS_PER_SEC;
        printf("Tempo decorrido: %f segundos\n", elapsed_time);
        printf("Pressione qualquer tecla para continuar...\n");
        getchar();
        running = 0;
    }
}

// Estrutura para armazenar informações sobre moedas
typedef struct {
    int tipo;           // 1, 2, 5, 10, 20, 50 cents or 1, 2 euros
    double valor;       // Valor monetário
    int x, y;           // Centro da moeda
    double area;        // Área em pixels
    double perimetro;   // Perímetro em pixels
    int x1, y1, x2, y2; // Caixa delimitadora
    double circularidade; // Medida de circularidade
} InfoMoeda;

// Função para processar a imagem binária e detectar moedas
int detectarMoedas(IVC* imagem, IVC* imagemBinaria, InfoMoeda* moedas, int maxMoedas) {
    int x, y, i, j;
    int width = imagem->width;
    int height = imagem->height;
    int bytesperline = imagem->bytesperline;
    int channels = imagem->channels;
    unsigned char* data = imagem->data;
    unsigned char* dataBin = imagemBinaria->data;
    int numMoedas = 0;
    
    // Aqui implementaremos a lógica de detecção de moedas
    // Esta é uma versão simplificada. Você precisará adaptá-la para seu caso específico.
    
    // 1. Encontrar componentes conectados (moedas)
    // 2. Calcular propriedades (área, perímetro, circularidade)
    // 3. Classificar moedas com base nas propriedades
    
    // Exemplo de código para percorrer a imagem binária
    for (y = 0; y < height && numMoedas < maxMoedas; y++) {
        for (x = 0; x < width && numMoedas < maxMoedas; x++) {
            int pos = y * imagemBinaria->bytesperline + x;
            
            // Se encontrarmos um pixel branco, pode ser uma moeda
            if (dataBin[pos] == 255) {
                // Implementar algoritmo de flood fill para encontrar toda a moeda
                // Calcular área, perímetro, centro, etc.
                
                // Exemplo simplificado (não funcional):
                moedas[numMoedas].x = x;
                moedas[numMoedas].y = y;
                moedas[numMoedas].area = 1000; // Valor de exemplo
                moedas[numMoedas].perimetro = 100; // Valor de exemplo
                moedas[numMoedas].circularidade = 0.9; // Valor de exemplo
                
                // Classificar moeda com base na área (simplificado)
                if (moedas[numMoedas].area < 2000) {
                    moedas[numMoedas].tipo = 1; // 1 cent
                    moedas[numMoedas].valor = 0.01;
                } else if (moedas[numMoedas].area < 3000) {
                    moedas[numMoedas].tipo = 2; // 2 cents
                    moedas[numMoedas].valor = 0.02;
                } else if (moedas[numMoedas].area < 4000) {
                    moedas[numMoedas].tipo = 5; // 5 cents
                    moedas[numMoedas].valor = 0.05;
                } else if (moedas[numMoedas].area < 5000) {
                    moedas[numMoedas].tipo = 10; // 10 cents
                    moedas[numMoedas].valor = 0.10;
                } else if (moedas[numMoedas].area < 6000) {
                    moedas[numMoedas].tipo = 20; // 20 cents
                    moedas[numMoedas].valor = 0.20;
                } else if (moedas[numMoedas].area < 7000) {
                    moedas[numMoedas].tipo = 50; // 50 cents
                    moedas[numMoedas].valor = 0.50;
                } else if (moedas[numMoedas].area < 8000) {
                    moedas[numMoedas].tipo = 100; // 1 euro
                    moedas[numMoedas].valor = 1.0;
                } else {
                    moedas[numMoedas].tipo = 200; // 2 euros
                    moedas[numMoedas].valor = 2.0;
                }
                
                numMoedas++;
                
                // Marcar a moeda como processada para não detectá-la novamente
                // (Isso seria parte do algoritmo de flood fill)
            }
        }
    }
    
    return numMoedas;
}

// Função para segmentar a imagem e isolar as moedas
void segmentarImagem(IVC* imagemOriginal, IVC* imagemBinaria) {
    int x, y;
    int width = imagemOriginal->width;
    int height = imagemOriginal->height;
    int bytesperline = imagemOriginal->bytesperline;
    int channels = imagemOriginal->channels;
    unsigned char* data = imagemOriginal->data;
    unsigned char* dataBin = imagemBinaria->data;
    
    // Converter para HSV para melhor segmentação
    IVC* imagemHSV = vc_image_new(width, height, 3, 255);
    if (imagemHSV == NULL) {
        printf("Erro ao alocar memória para imagem HSV\n");
        return;
    }
    
    // Converter RGB para HSV
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            int pos = y * bytesperline + x * channels;
            
            // Obter componentes RGB
            unsigned char r = data[pos];
            unsigned char g = data[pos + 1];
            unsigned char b = data[pos + 2];
            
            // Converter para HSV (implementação simplificada)
            float max = (r > g) ? ((r > b) ? r : b) : ((g > b) ? g : b);
            float min = (r < g) ? ((r < b) ? r : b) : ((g < b) ? g : b);
            float delta = max - min;
            
            // Valores HSV
            float h = 0, s = 0, v = max;
            
            if (max != 0) {
                s = delta / max;
            }
            
            if (delta != 0) {
                if (max == r) {
                    h = (g - b) / delta;
                    if (h < 0) h += 6;
                } else if (max == g) {
                    h = 2 + (b - r) / delta;
                } else {
                    h = 4 + (r - g) / delta;
                }
                h *= 60;
            }
            
            // Armazenar valores HSV
            imagemHSV->data[pos] = (unsigned char)(h / 2);  // H (0-180)
            imagemHSV->data[pos + 1] = (unsigned char)(s * 255); // S (0-255)
            imagemHSV->data[pos + 2] = (unsigned char)v;    // V (0-255)
        }
    }
    
    // Segmentar moedas com base em valores HSV
    // Valores de limiar para detectar moedas (ajustar conforme necessário)
    int hMin = 0, hMax = 180;
    int sMin = 0, sMax = 30;
    int vMin = 100, vMax = 255;
    
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            int posHSV = y * bytesperline + x * channels;
            int posBin = y * imagemBinaria->bytesperline + x;
            
            unsigned char h = imagemHSV->data[posHSV];
            unsigned char s = imagemHSV->data[posHSV + 1];
            unsigned char v = imagemHSV->data[posHSV + 2];
            
            // Verificar se o pixel está dentro dos limiares definidos
            if (h >= hMin && h <= hMax && s >= sMin && s <= sMax && v >= vMin && v <= vMax) {
                dataBin[posBin] = 255; // Pixel branco (moeda)
            } else {
                dataBin[posBin] = 0;   // Pixel preto (fundo)
            }
        }
    }
    
    // Aplicar operações morfológicas para remover ruído
    // Implementar erosão e dilatação
    
    vc_image_free(imagemHSV);
}

// Função para desenhar informações na imagem
void desenharInformacoes(cv::Mat frame, InfoMoeda* moedas, int numMoedas) {
    int i;
    char texto[100];
    double valorTotal = 0.0;
    
    // Contar moedas por tipo
    int count_1c = 0, count_2c = 0, count_5c = 0, count_10c = 0;
    int count_20c = 0, count_50c = 0, count_1e = 0, count_2e = 0;
    
    for (i = 0; i < numMoedas; i++) {
        // Desenhar caixa delimitadora
        cv::rectangle(frame, cv::Point(moedas[i].x1, moedas[i].y1), 
                     cv::Point(moedas[i].x2, moedas[i].y2), 
                     cv::Scalar(0, 255, 0), 2);
        
        // Desenhar centro
        cv::circle(frame, cv::Point(moedas[i].x, moedas[i].y), 5, 
                  cv::Scalar(0, 0, 255), -1);
        
        // Exibir tipo de moeda
        if (moedas[i].tipo < 100) {
            sprintf(texto, "%d cents", moedas[i].tipo);
        } else {
            sprintf(texto, "%d euros", moedas[i].tipo / 100);
        }
        
        cv::putText(frame, texto, cv::Point(moedas[i].x - 30, moedas[i].y - 20), 
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
        
        // Acumular valor total
        valorTotal += moedas[i].valor;
        
        // Contar por tipo
        switch (moedas[i].tipo) {
            case 1: count_1c++; break;
            case 2: count_2c++; break;
            case 5: count_5c++; break;
            case 10: count_10c++; break;
            case 20: count_20c++; break;
            case 50: count_50c++; break;
            case 100: count_1e++; break;
            case 200: count_2e++; break;
        }
    }
    
    // Exibir estatísticas
    int y_pos = 150;
    
    // Total de moedas
    sprintf(texto, "Total moedas: %d", numMoedas);
    cv::putText(frame, texto, cv::Point(20, y_pos), 
               cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 1);
    y_pos += 25;
    
    // Valor total
    sprintf(texto, "Valor total: %.2f euros", valorTotal);
    cv::putText(frame, texto, cv::Point(20, y_pos), 
               cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 1);
    y_pos += 25;
    
    // Exibir contagem por tipo
    if (count_1c > 0) {
        sprintf(texto, "1 cent: %d", count_1c);
        cv::putText(frame, texto, cv::Point(20, y_pos), 
                   cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 1);
        y_pos += 25;
    }
    
    if (count_2c > 0) {
        sprintf(texto, "2 cents: %d", count_2c);
        cv::putText(frame, texto, cv::Point(20, y_pos), 
                   cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 1);
        y_pos += 25;
    }
    
    if (count_5c > 0) {
        sprintf(texto, "5 cents: %d", count_5c);
        cv::putText(frame, texto, cv::Point(20, y_pos), 
                   cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 1);
        y_pos += 25;
    }
    
    if (count_10c > 0) {
        sprintf(texto, "10 cents: %d", count_10c);
        cv::putText(frame, texto, cv::Point(20, y_pos), 
                   cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 1);
        y_pos += 25;
    }
    
    if (count_20c > 0) {
        sprintf(texto, "20 cents: %d", count_20c);
        cv::putText(frame, texto, cv::Point(20, y_pos), 
                   cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 1);
        y_pos += 25;
    }
    
    if (count_50c > 0) {
        sprintf(texto, "50 cents: %d", count_50c);
        cv::putText(frame, texto, cv::Point(20, y_pos), 
                   cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 1);
        y_pos += 25;
    }
    
    if (count_1e > 0) {
        sprintf(texto, "1 euro: %d", count_1e);
        cv::putText(frame, texto, cv::Point(20, y_pos), 
                   cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 1);
        y_pos += 25;
    }
    
    if (count_2e > 0) {
        sprintf(texto, "2 euros: %d", count_2e);
        cv::putText(frame, texto, cv::Point(20, y_pos), 
                   cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 1);
    }
}

int main(void) {
    // Vídeo
    char videofile[100] = "VC-PL-TP/VC-PL-TP/video1.mp4";
    cv::VideoCapture capture;
    struct {
        int width, height;
        int ntotalframes;
        int fps;
        int nframe;
    } video;
    
    // Outros
    char str[100];
    int key = 0;
    InfoMoeda moedas[100]; // Array para armazenar informações das moedas
    int numMoedas = 0;
    
    // Abrir o arquivo de vídeo
    capture.open(videofile);
    
    // Verificar se foi possível abrir o arquivo de vídeo
    if (!capture.isOpened()) {
        printf("Erro ao abrir o arquivo de vídeo!\n");
        return 1;
    }
    
    // Obter propriedades do vídeo
    video.ntotalframes = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
    video.fps = (int)capture.get(cv::CAP_PROP_FPS);
    video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
    video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);
    
    // Criar janela para exibir o vídeo
    cv::namedWindow("Detector de Moedas", cv::WINDOW_AUTOSIZE);
    
    // Iniciar o timer
    vc_timer();
    
    cv::Mat frame;
    while (key != 'q') {
        // Ler um frame do vídeo
        capture.read(frame);
        
        // Verificar se conseguiu ler o frame
        if (frame.empty()) break;
        
        // Número do frame atual
        video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);
        
        // Exibir informações do vídeo
        sprintf(str, "RESOLUCAO: %dx%d", video.width, video.height);
        cv::putText(frame, str, cv::Point(20, 25), 
                   cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 1);
        
        sprintf(str, "FRAME: %d/%d", video.nframe, video.ntotalframes);
        cv::putText(frame, str, cv::Point(20, 50), 
                   cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 1);
        
        sprintf(str, "FPS: %d", video.fps);
        cv::putText(frame, str, cv::Point(20, 75), 
                   cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 1);
        
        // Criar uma nova imagem IVC
        IVC* image = vc_image_new(video.width, video.height, 3, 255);
        if (image == NULL) {
            printf("Erro ao alocar memória para a imagem IVC\n");
            break;
        }
        
        // Copiar dados da imagem de cv::Mat para IVC
        memcpy(image->data, frame.data, video.width * video.height * 3);
        
        // Criar imagem binária para segmentação
        IVC* imagemBinaria = vc_image_new(video.width, video.height, 1, 255);
        if (imagemBinaria == NULL) {
            printf("Erro ao alocar memória para a imagem binária\n");
            vc_image_free(image);
            break;
        }
        
        // Segmentar a imagem para isolar as moedas
        segmentarImagem(image, imagemBinaria);
        
        // Detectar moedas na imagem binária
        numMoedas = detectarMoedas(image, imagemBinaria, moedas, 100);
        
        // Desenhar informações na imagem
        desenharInformacoes(frame, moedas, numMoedas);
        
        // Liberar memória das imagens IVC
        vc_image_free(image);
        vc_image_free(imagemBinaria);
        
        // Exibir o frame
        cv::imshow("Detector de Moedas", frame);
        
        // Sair se o usuário pressionar 'q'
        key = cv::waitKey(1);
    }
    
    // Parar o timer e exibir o tempo decorrido
    vc_timer();
    
    // Fechar a janela
    cv::destroyWindow("Detector de Moedas");
    
    // Fechar o arquivo de vídeo
    capture.release();
    
    return 0;
}
