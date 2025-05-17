// Adiciona um #if correspondente para corrigir o erro E0036  
#ifndef VC_DEBUG  
#define VC_DEBUG  
#endif  

// Inclui bibliotecas necessárias do OpenCV  
#include <opencv2/core/types_c.h>  
#include <opencv2/opencv.hpp>  
#include <opencv2/core.hpp>  
#include <opencv2/highgui.hpp>  
#include <opencv2/imgproc.hpp>  

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
//                        MACROS  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
// Macros para cálculo de máximo e mínimo entre dois valores  
#define MAX(a,b) (a > b ? a : b)  
#define MIN(a,b) (a < b ? a : b)  

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
//                   ESTRUTURA DE UMA IMAGEM  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
// Estrutura para representar uma imagem em formato personalizado  
typedef struct {  
   unsigned char* data;      // Dados da imagem  
   int width, height;        // Dimensões da imagem  
   int channels;             // Número de canais (1 para binário/cinzentos, 3 para RGB)  
   int levels;               // Níveis de intensidade (1 para binário, [1,255] para cinzentos/RGB)  
   int bytesperline;         // Bytes por linha (width * channels)  
} IVC;  

// Estrutura para representar uma moeda detectada  
typedef struct {  
   int x, y, width, height;  // Coordenadas e dimensões da bounding box  
   int area;                 // Área em píxeis  
   int perimeter;            // Perímetro em píxeis  
   int xc, yc;               // Centro de massa  
   int label;                // Etiqueta do blob  
   int tipo;                 // Tipo de moeda (1, 2, 5, 10, 20, 50 cêntimos; 100, 200 euros)  
   float circularity;        // Circularidade (4π*área/perímetro²)  
} OVC;  

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
//                    PROTÓTIPOS DE FUNÇÕES  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
// Funções para alocação e libertação de imagens IVC  
IVC* vc_image_new(int width, int height, int channels, int levels); // Aloca memória para uma nova imagem  
IVC* vc_image_free(IVC* image); // Liberta memória de uma imagem  

// Funções para leitura e escrita de imagens PBM, PGM e PPM  
IVC* vc_read_image(char* filename); // Lê uma imagem de um ficheiro  
int vc_write_image(char* filename, IVC* image); // Escreve uma imagem num ficheiro  

// Funções para processamento de blobs (moedas)  
OVC* vc_binary_blob_labelling(IplImage* src, IplImage* dst, int* nlabels); // Realiza a etiquetagem de blobs binários  
int vc_binary_blob_info(IplImage* src, OVC* blobs, int nblobs); // Extrai informações de blobs (área, perímetro, etc.)  
int vc_desenha_bounding_box(IplImage* src, OVC blobs); // Desenha a bounding box de um blob  

// Funções para desenho de linhas de referência  
int desenha_linhaVermelha(IplImage* frame); // Desenha uma linha vermelha de referência  
int desenha_linhaVerde(IplImage* frame); // Desenha uma linha verde de referência  

// Função para segmentação por tonalidade HSV  
int idBlobs(IplImage* frameIn, IplImage* frameOut, int hueMin, int hueMax, float satMin, float satMax, int valueMin, int valueMax); // Segmenta moedas por tonalidade HSV  

// Funções para lógica de detecção e classificação de moedas  
int verificaPassouAntes(OVC* passou, OVC moedas, int cont); // Verifica se a moeda já foi contada  
int idMoeda(int area, int perimeter); // Classifica o tipo de moeda com base em área e perímetro  
void escreverInfo(FILE* fp, int cont, int mTotal, int m200, int m100, int m50, int m20, int m10, int m5, int m2, int m1, const char* videofile); // Salva resultados no ficheiro  
