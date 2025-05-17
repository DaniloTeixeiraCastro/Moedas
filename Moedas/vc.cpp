// Desabilita warnings de funções não seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#ifdef __APPLE__ && __MACH__
// Como não existe malloc para mac, usamos stdlib.h
#include <stdlib.h>
#else
    // Para Windows usar malloc
#include <malloc.h>
#endif
#include "vc.h"

// Função para segmentar blobs com base em intervalos HSV
// Retorna: 1 se bem-sucedido, 0 em caso de erro
int idBlobs(IplImage* src, IplImage* dst, int hueMin, int hueMax, float satMin, float satMax, int valueMin, int valueMax) {
    unsigned char* imageDatasrc = (unsigned char*)src->imageData;
    unsigned char* imageDatadst = (unsigned char*)dst->imageData;
    int width = src->width;
    int height = src->height;
    int bytesperline = src->widthStep;
    int channels = src->nChannels;
    float r, g, b, h, s, v;
    float max, min;
    int size;

    // Verificação de erros
    if ((src->width <= 0) || (src->height <= 0) || (src->imageData == NULL)) { return 0; }
    if ((src->width != dst->width) || (src->height != dst->height)) { return 0; }
    if ((src->nChannels != 3)) { return 0; }

    // Número de píxeis
    size = width * height * channels;

    // Percorre todos os píxeis
    for (int i = 0; i < size; i = i + channels) {
        // Inverte cores, IPLimage funciona com BGR
        b = (float)imageDatasrc[i];
        g = (float)imageDatasrc[i + 1];
        r = (float)imageDatasrc[i + 2];

        // Calcula max e min de BGR
        max = (r > g ? (r > b ? r : b) : (g > b ? g : b));
        min = (r < g ? (r < b ? r : b) : (g < b ? g : b));

        // Value tem valores entre 0 e 255
        v = max;

        if (v != 0.0) {
            s = ((max - min) / max) * (float)255.0;
            if (s != 0.0) {
                if (v == r && g >= b) { h = 60.0f * (g - b) / (v - min); }
                if (v == r && b > g) { h = 360.0f + 60.0f * (g - b) / (v - min); }
                if (v == g) { h = 120.0f + 60.0f * (b - r) / (v - min); }
                if (v == b) { h = 240.0f + 60.0f * (r - g) / (v - min); }
            }
            else {
                h = 0.0;
            }
        }
        else {
            h = 0.0;
            s = 0.0;
        }

        // Coloca tudo a preto, exceto se os valores estiverem dentro dos intervalos
        if ((h <= hueMax && h >= hueMin) && (s <= satMax && s >= satMin) && (v <= valueMax && v >= valueMin)) {
            imageDatadst[i / 3] = 255;
        }
        else {
            imageDatadst[i / 3] = 0;
        }
    }
    return 1;
    // Nota: Esta função usa IplImage. Para usar cv::Mat, substitua por cv::inRange no main.cpp.
}

// Verifica se a moeda já foi contada anteriormente
// Parâmetros:
// - passou: Array de moedas já processadas
// - moedas: Moeda atual
// - cont: Contador de moedas processadas
// Retorna: 1 se nova moeda, 0 se já foi contada
int verificaPassouAntes(OVC* passou, OVC moedas, int cont) {
    if (cont == 0) {
        return 1; // Primeira moeda, sempre nova
    }
    else if (moedas.xc < passou[cont - 1].xc - 10 || moedas.xc > passou[cont - 1].xc + 10) {
        return 1; // Diferença significativa em xc, é uma nova moeda
    }
    return 0; // Moeda já contada
}

// Classifica o tipo de moeda com base em área e perímetro
// Parâmetros:
// - area: Área da moeda em píxeis
// - perimeter: Perímetro da moeda em píxeis
// Retorna: Valor da moeda (1, 2, 5, 10, 20, 50 cêntimos; 100, 200 euros), ou 0 se desconhecido
int idMoeda(int area, int perimeter) {
    if (area >= 22000) {
        return 50; // 50 cêntimos
    }
    else if (area >= 19500 && area <= 22000 && perimeter > 490 && perimeter < 600) {
        return 20; // 20 cêntimos
    }
    else if (area >= 16000 && area <= 17500 && perimeter > 480 && perimeter < 600) {
        return 200; // 2 euros
    }
    else if (area >= 15500 && area <= 18000 && perimeter > 436 && perimeter <= 480) {
        return 10; // 10 cêntimos
    }
    else if (area >= 17500 && area <= 20500) {
        return 5; // 5 cêntimos
    }
    else if (area >= 11000 && perimeter > 600) {
        return 100; // 1 euro
    }
    else if (area >= 13000 && area < 16500) {
        return 2; // 2 cêntimos
    }
    else if (area < 13000) {
        return 1; // 1 cêntimo
    }
    return 0; // Desconhecido
    // Nota: Para maior precisão, considere adicionar circularity como parâmetro.
    // Exemplo: if (area >= 22000 && circularity > 0.8) { return 50; }
}

// Salva informações das moedas no ficheiro Moedas.txt
// Parâmetros:
// - fp: Ponteiro para o ficheiro
// - cont: Número de moedas processadas
// - mTotal: Total de moedas
// - m200, m100, ..., m1: Contadores por tipo de moeda
// - videofile: Nome do vídeo processado
void escreverInfo(FILE* fp, int cont, int mTotal, int m200, int m100, int m50, int m20, int m10, int m5, int m2, int m1, const char* videofile) {
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
    // Nota: Para incluir área, perímetro e tipo, é necessário passar o array OVC* passou.
    // Exemplo: fprintf(fp, "MOEDA %d: AREA=%d, PERIMETRO=%d, TIPO=%d\n", i, passou[i].area, passou[i].perimeter, passou[i].tipo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Aloca memória para uma imagem IVC
// Parâmetros:
// - width: Largura da imagem
// - height: Altura da imagem
// - channels: Número de canais
// - levels: Níveis de intensidade
// Retorna: Ponteiro para a imagem alocada, ou NULL em caso de erro
IVC* vc_image_new(int width, int height, int channels, int levels) {
    IVC* image = (IVC*)malloc(sizeof(IVC));

    if (image == NULL) return NULL;
    if ((levels <= 0) || (levels > 255)) return NULL;

    image->width = width;
    image->height = height;
    image->channels = channels;
    image->levels = levels;
    image->bytesperline = image->width * image->channels;
    image->data = (unsigned char*)malloc(image->width * image->height * image->channels * sizeof(char));

    if (image->data == NULL) {
        return vc_image_free(image);
    }

    return image;
}

// Liberta memória de uma imagem IVC
// Parâmetro:
// - image: Ponteiro para a imagem
// Retorna: NULL após libertar a memória
IVC* vc_image_free(IVC* image) {
    if (image != NULL) {
        if (image->data != NULL) {
            free(image->data);
            image->data = NULL;
        }

        free(image);
        image = NULL;
    }

    return image;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Lê um token de um ficheiro NetPBM, ignorando comentários
// Parâmetros:
// - file: Ficheiro de entrada
// - tok: Buffer para armazenar o token
// - len: Tamanho máximo do buffer
// Retorna: Ponteiro para o token lido
char* netpbm_get_token(FILE* file, char* tok, int len) {
    char* t;
    int c;

    for (;;) {
        while (isspace(c = getc(file)));
        if (c != '#') break;
        do c = getc(file);
        while ((c != '\n') && (c != EOF));
        if (c == EOF) break;
    }

    t = tok;

    if (c != EOF) {
        do {
            *t++ = c;
            c = getc(file);
        } while ((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));

        if (c == '#') ungetc(c, file);
    }

    *t = 0;

    return tok;
}

// Converte dados de unsigned char para bits (PBM)
// Parâmetros:
// - datauchar: Dados de entrada
// - databit: Dados de saída em bits
// - width: Largura da imagem
// - height: Altura da imagem
// Retorna: Número de bytes escritos
long int unsigned_char_to_bit(unsigned char* datauchar, unsigned char* databit, int width, int height) {
    int x, y;
    int countbits;
    long int pos, counttotalbytes;
    unsigned char* p = databit;

    *p = 0;
    countbits = 1;
    counttotalbytes = 0;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            pos = width * y + x;

            if (countbits <= 8) {
                // Na nossa imagem: 1 = Branco, 0 = Preto
                *p |= (datauchar[pos] == 0) << (8 - countbits);

                countbits++;
            }
            if ((countbits > 8) || (x == width - 1)) {
                p++;
                *p = 0;
                countbits = 1;
                counttotalbytes++;
            }
        }
    }

    return counttotalbytes;
}

// Converte dados de bits para unsigned char (PBM)
// Parâmetros:
// - databit: Dados de entrada em bits
// - datauchar: Dados de saída
// - width: Largura da imagem
// - height: Altura da imagem
void bit_to_unsigned_char(unsigned char* databit, unsigned char* datauchar, int width, int height) {
    int x, y;
    int countbits;
    long int pos;
    unsigned char* p = databit;

    countbits = 1;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            pos = width * y + x;

            if (countbits <= 8) {
                // Na nossa imagem: 1 = Branco, 0 = Preto
                datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;

                countbits++;
            }
            if ((countbits > 8) || (x == width - 1)) {
                p++;
                countbits = 1;
            }
        }
    }
}

// Lê uma imagem PBM, PGM ou PPM de um ficheiro
// Parâmetro:
// - filename: Nome do ficheiro
// Retorna: Ponteiro para a imagem lida, ou NULL em caso de erro
IVC* vc_read_image(char* filename) {
    FILE* file = NULL;
    IVC* image = NULL;
    unsigned char* tmp;
    char tok[20];
    long int size, sizeofbinarydata;
    int width, height, channels;
    int levels = 255;
    int v;

    // Abre o ficheiro
    if ((file = fopen(filename, "rb")) != NULL) {
        // Efectua a leitura do header
        netpbm_get_token(file, tok, sizeof(tok));

        if (strcmp(tok, "P4") == 0) { channels = 1; levels = 1; }    // Se PBM (Binary [0,1])
        else if (strcmp(tok, "P5") == 0) channels = 1;                // Se PGM (Gray [0,MAX(level,255)])
        else if (strcmp(tok, "P6") == 0) channels = 3;                // Se PPM (RGB [0,MAX(level,255)])
        else {
#ifdef VC_DEBUG
            printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
#endif
            fclose(file);
            return NULL;
        }

        if (levels == 1) { // PBM
            if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
                sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1) {
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
#endif
                fclose(file);
                return NULL;
            }

            // Aloca memória para imagem
            image = vc_image_new(width, height, channels, levels);
            if (image == NULL) return NULL;

            sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
            tmp = (unsigned char*)malloc(sizeofbinarydata);
            if (tmp == NULL) return 0;

#ifdef VC_DEBUG
            printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

            if ((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata) {
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif
                vc_image_free(image);
                fclose(file);
                free(tmp);
                return NULL;
            }

            bit_to_unsigned_char(tmp, image->data, image->width, image->height);

            free(tmp);
        }
        else { // PGM ou PPM
            if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
                sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
                sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255) {
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
#endif
                fclose(file);
                return NULL;
            }

            // Aloca memória para imagem
            image = vc_image_new(width, height, channels, levels);
            if (image == NULL) return NULL;

#ifdef VC_DEBUG
            printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

            size = image->width * image->height * image->channels;

            if ((v = fread(image->data, sizeof(unsigned char), size, file)) != size) {
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif
                vc_image_free(image);
                fclose(file);
                return NULL;
            }
        }

        fclose(file);
    }
    else {
#ifdef VC_DEBUG
        printf("ERROR -> vc_read_image():\n\tFile not found.\n");
#endif
    }

    return image;
}

// Escreve uma imagem PBM, PGM ou PPM num ficheiro
// Parâmetros:
// - filename: Nome do ficheiro
// - image: Imagem a escrever
// Retorna: 1 se bem-sucedido, 0 em caso de erro
int vc_write_image(char* filename, IVC* image) {
    FILE* file = NULL;
    unsigned char* tmp;
    long int totalbytes, sizeofbinarydata;

    if (image == NULL) return 0;

    if ((file = fopen(filename, "wb")) != NULL) {
        if (image->levels == 1) {
            sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
            tmp = (unsigned char*)malloc(sizeofbinarydata);
            if (tmp == NULL) return 0;

            fprintf(file, "%s %d %d\n", "P4", image->width, image->height);

            totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
            printf("Total = %ld\n", totalbytes);
            if (fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes) {
#ifdef VC_DEBUG
                fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif
                fclose(file);
                free(tmp);
                return 0;
            }

            free(tmp);
        }
        else {
            fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);

            if (fwrite(image->data, image->bytesperline, image->height, file) != image->height) {
#ifdef VC_DEBUG
                fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif
                fclose(file);
                return 0;
            }
        }

        fclose(file);

        return 1;
    }

    return 0;
}

// Realiza a etiquetagem de blobs binários
// Parâmetros:
// - src: Imagem de entrada
// - dst: Imagem de saída com etiquetas
// - nlabels: Número de etiquetas encontradas
// Retorna: Array de blobs detectados
OVC* vc_binary_blob_labelling(IplImage* src, IplImage* dst, int* nlabels) {
    unsigned char* datasrc = (unsigned char*)src->imageData;
    unsigned char* datadst = (unsigned char*)dst->imageData;
    int width = src->width;
    int height = src->height;
    int bytesperline = src->widthStep;
    int channels = src->nChannels;
    int x, y, a, b;
    long int i, size;
    long int posX, posA, posB, posC, posD;
    int labeltable[256] = { 0 };
    int label = 1;      // Etiqueta inicial
    int num, tmplabel;
    OVC* blobs;         // Array de blobs retornado

    // Verificação de erros
    if ((src->width <= 0) || (src->height <= 0) || (src->imageData == NULL)) { return 0; }
    if ((src->width != dst->width) || (src->height != dst->height) || (src->nChannels != dst->nChannels)) { return 0; }
    if (channels != 1) { return 0; }

    // Copia dados da imagem binária para imagem grayscale
    memcpy(datadst, datasrc, bytesperline * height);

    // Todos os píxeis de plano de fundo devem ter valor 0
    // Todos os píxeis de primeiro plano devem ter valor 255
    for (i = 0, size = bytesperline * height; i < size; i++) {
        if (datadst[i] != 0) datadst[i] = 255;
    }

    // Limpa os rebordos da imagem binária
    for (y = 0; y < height; y++) {
        datadst[y * bytesperline + 0 * channels] = 0;
        datadst[y * bytesperline + (width - 1) * channels] = 0;
    }
    for (x = 0; x < width; x++) {
        datadst[0 * bytesperline + x * channels] = 0;
        datadst[(height - 1) * bytesperline + x * channels] = 0;
    }

    // Efectua a etiquetagem
    for (y = 1; y < height - 1; y++) {
        for (x = 1; x < width - 1; x++) {
            // Kernel: A B C
            //         D X
            posA = (y - 1) * bytesperline + (x - 1) * channels; // A
            posB = (y - 1) * bytesperline + x * channels;       // B
            posC = (y - 1) * bytesperline + (x + 1) * channels; // C
            posD = y * bytesperline + (x - 1) * channels;       // D
            posX = y * bytesperline + x * channels;             // X

            // Se o pixel foi marcado
            if (datadst[posX] != 0) {
                if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0)) {
                    datadst[posX] = label;
                    labeltable[label] = label;
                    label++;
                }
                else {
                    num = 255;

                    // Se A está marcado
                    if (datadst[posA] != 0) num = labeltable[datadst[posA]];
                    // Se B está marcado, e é menor que a etiqueta "num"
                    if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
                    // Se C está marcado, e é menor que a etiqueta "num"
                    if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
                    // Se D está marcado, e é menor que a etiqueta "num"
                    if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num)) num = labeltable[datadst[posD]];

                    // Atribui a etiqueta ao pixel
                    datadst[posX] = num;
                    labeltable[num] = num;

                    // Actualiza a tabela de etiquetas
                    if (datadst[posA] != 0) {
                        if (labeltable[datadst[posA]] != num) {
                            for (tmplabel = labeltable[datadst[posA]], a = 1; a < label; a++) {
                                if (labeltable[a] == tmplabel) labeltable[a] = num;
                            }
                        }
                    }
                    if (datadst[posB] != 0) {
                        if (labeltable[datadst[posB]] != num) {
                            for (tmplabel = labeltable[datadst[posB]], a = 1; a < label; a++) {
                                if (labeltable[a] == tmplabel) labeltable[a] = num;
                            }
                        }
                    }
                    if (datadst[posC] != 0) {
                        if (labeltable[datadst[posC]] != num) {
                            for (tmplabel = labeltable[datadst[posC]], a = 1; a < label; a++) {
                                if (labeltable[a] == tmplabel) labeltable[a] = num;
                            }
                        }
                    }
                    if (datadst[posD] != 0) {
                        if (labeltable[datadst[posD]] != num) {
                            for (tmplabel = labeltable[datadst[posD]], a = 1; a < label; a++) {
                                if (labeltable[a] == tmplabel) labeltable[a] = num;
                            }
                        }
                    }

                    // Atualiza o índice atual do posX com o menor valor num
                    labeltable[datadst[posX]] = num;
                    datadst[posX] = num;
                }
            }
        }
    }

    // Volta a etiquetar a imagem
    for (y = 1; y < height - 1; y++) {
        for (x = 1; x < width - 1; x++) {
            posX = y * bytesperline + x * channels; // X
            if (datadst[posX] != 0) datadst[posX] = labeltable[datadst[posX]];
        }
    }

    // Contagem do número de blobs
    // Passo 1: Eliminar etiquetas repetidas
    for (a = 1; a < label - 1; a++) {
        for (b = a + 1; b < label; b++) {
            if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
        }
    }

    // Passo 2: Conta etiquetas e organiza a tabela
    *nlabels = 0;
    for (a = 1; a < label; a++) {
        if (labeltable[a] != 0) {
            labeltable[*nlabels] = labeltable[a];
            (*nlabels)++;
        }
    }

    // Se não há blobs
    if (*nlabels == 0) return NULL;

    // Cria lista de blobs e preenche a etiqueta
    blobs = (OVC*)calloc((*nlabels), sizeof(OVC));
    if (blobs != NULL) {
        for (a = 0; a < (*nlabels); a++) blobs[a].label = labeltable[a];
    }
    else {
        return NULL;
    }

    return blobs;
    // Nota: Esta função usa IplImage. Para usar cv::Mat, substitua por cv::findContours no main.cpp.
}

// Extrai informações de blobs (área, perímetro, bounding box, centro de gravidade)
// Parâmetros:
// - src: Imagem de entrada
// - blobs: Array de blobs
// - nblobs: Número de blobs
// Retorna: 1 se bem-sucedido, 0 em caso de erro
int vc_binary_blob_info(IplImage* src, OVC* blobs, int nblobs) {
    unsigned char* data = (unsigned char*)src->imageData;
    int width = src->width;
    int height = src->height;
    int bytesperline = src->widthStep;
    int channels = src->nChannels;
    int x, y, i;
    long int pos;
    int xmin, ymin, xmax, ymax;
    long int sumx, sumy;

    // Verificação de erros
    if ((src->width <= 0) || (src->height <= 0) || (src->imageData == NULL)) return 0;
    if (channels != 1) return 0;

    // Conta área de cada blob
    for (i = 0; i < nblobs; i++) {
        xmin = width - 1;
        ymin = height - 1;
        xmax = 0;
        ymax = 0;

        sumx = 0;
        sumy = 0;

        blobs[i].area = 0;

        for (y = 1; y < height - 1; y++) {
            for (x = 1; x < width - 1; x++) {
                pos = y * bytesperline + x * channels;

                if (data[pos] == blobs[i].label) {
                    // Área
                    blobs[i].area++;

                    // Centro de Gravidade
                    sumx += x;
                    sumy += y;

                    // Bounding Box
                    if (xmin > x) xmin = x;
                    if (ymin > y) ymin = y;
                    if (xmax < x) xmax = x;
                    if (ymax < y) ymax = y;

                    // Perímetro
                    if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) ||
                        (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label)) {
                        blobs[i].perimeter++;
                    }
                }
            }
        }

        // Bounding Box
        blobs[i].x = xmin;
        blobs[i].y = ymin;
        blobs[i].width = (xmax - xmin) + 1;
        blobs[i].height = (ymax - ymin) + 1;

        // Centro de Gravidade
        blobs[i].xc = sumx / MAX(blobs[i].area, 1);
        blobs[i].yc = sumy / MAX(blobs[i].area, 1);
    }
    return 1;
    // Nota: Esta função usa IplImage. Para usar cv::Mat, substitua por cv::contourArea e cv::boundingRect no main.cpp.
}

// Desenha a bounding box e o centro de gravidade de um blob
// Parâmetros:
// - src: Imagem de entrada
// - blobs: Blob a desenhar
// Retorna: 1 se bem-sucedido, 0 em caso de erro
int vc_desenha_bounding_box(IplImage* src, OVC blobs) {
    unsigned char* datasrc = (unsigned char*)src->imageData;
    int bytesperline_src = src->width * src->nChannels;
    int channels_src = src->nChannels;
    int yy, xx;
    long int posk;

    // Verificação de erros
    if ((src->width <= 0) || (src->height <= 0) || (src->imageData == NULL)) return 0;
    if ((src->nChannels != 3)) return 0;

    // Desenhar a bounding box
    for (yy = blobs.y; yy <= blobs.y + blobs.height; yy++) {
        for (xx = blobs.x; xx <= blobs.x + blobs.width; xx++) {
            posk = yy * bytesperline_src + xx * channels_src;
            if (yy == blobs.y || yy == blobs.y + blobs.height || xx == blobs.x || xx == blobs.x + blobs.width) {
                datasrc[posk] = 0;       // B
                datasrc[posk + 1] = 8;   // G
                datasrc[posk + 2] = 255; // R
            }
        }
    }

    // Desenhar o centro de massa
    for (yy = blobs.yc - 10; yy <= blobs.yc + 10; yy++) {
        for (xx = blobs.xc - 10; xx < blobs.xc + 10; xx++) {
            posk = yy * bytesperline_src + xx * channels_src;
            if (yy == blobs.yc || xx == blobs.xc) {
                datasrc[posk] = 0;
                datasrc[posk + 1] = 8;
                datasrc[posk + 2] = 255;
            }
        }
    }
    return 1;
    // Nota: Esta função usa IplImage. Para usar cv::Mat, substitua por cv::rectangle no main.cpp.
}

// Desenha uma linha verde de referência no frame
// Parâmetro:
// - frame: Imagem de entrada
// Retorna: 1 se bem-sucedido, 0 em caso de erro
int desenha_linhaVerde(IplImage* frame) {
    unsigned char* imageData = (unsigned char*)frame->imageData;
    int width = frame->width;
    int height = frame->height;
    int bytesperline = frame->widthStep;
    int channels = frame->nChannels;
    long int pos;

    // Verificação de erros
    if ((frame->width <= 0) || (frame->height <= 0) || (frame->imageData == NULL)) { return 0; }
    if ((frame->nChannels != 3)) { return 0; }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pos = y * bytesperline + x * channels;
            if (y == height / 4 + 20 || y == (height / 4) - 15) {
                imageData[pos] = 0;
                imageData[pos + 1] = 255;
                imageData[pos + 2] = 0;
            }
        }
    }
    return 1;
    // Nota: Esta função usa IplImage. Para usar cv::Mat, substitua por cv::line no main.cpp.
}

// Desenha uma linha vermelha de referência no frame
// Parâmetro:
// - frame: Imagem de entrada
// Retorna: 1 se bem-sucedido, 0 em caso de erro
int desenha_linhaVermelha(IplImage* frame) {
    unsigned char* imageData = (unsigned char*)frame->imageData;
    int width = frame->width;
    int height = frame->height;
    int bytesperline = frame->widthStep;
    int channels = frame->nChannels;
    long int pos;

    // Verificação de erros
    if ((frame->width <= 0) || (frame->height <= 0) || (frame->imageData == NULL)) { return 0; }
    if ((frame->nChannels != 3)) { return 0; }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pos = y * bytesperline + x * channels;
            if (y == height / 4 + 20 || y == (height / 4) - 15) {
                imageData[pos] = 0;
                imageData[pos + 1] = 0;
                imageData[pos + 2] = 255;
            }
        }
    }
    return 1;
    // Nota: Esta função usa IplImage. Para usar cv::Mat, substitua por cv::line no main.cpp.
}