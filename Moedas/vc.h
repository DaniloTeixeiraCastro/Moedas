#ifndef VC_DEBUG
#define VC_DEBUG
#endif

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                        MACROS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef MAX
#define MAX(a,b) (a > b ? a : b)
#endif
#ifndef MIN
#define MIN(a,b) (a < b ? a : b)
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
    double circularity;       // Circularidade (4π*área/perímetro²)
} OVC;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROTÓTIPOS DE FUNÇÕES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IVC* vc_image_new(int width, int height, int channels, int levels);
IVC* vc_image_free(IVC* image);

IVC* vc_read_image(char* filename);
int vc_write_image(char* filename, IVC* image);

OVC* vc_binary_blob_labelling(cv::Mat src, cv::Mat dst, int* nlabels);
int vc_binary_blob_info(cv::Mat src, OVC* blobs, int nblobs);
int vc_desenha_bounding_box(cv::Mat src, OVC blobs);

int desenha_linhaVermelha(cv::Mat frame);
int desenha_linhaVerde(cv::Mat frame);

int idBlobs(cv::Mat frameIn, cv::Mat frameOut, int hueMin, int hueMax, float satMin, float satMax, int valueMin, int valueMax);

// Funções para lógica de detecção e classificação de moedas
int verificaPassouAntes(OVC* passou, OVC moedas, int cont);
int idMoeda(int area, int perimeter, float circularity, cv::Vec3b meanColor);
void escreverInfo(FILE* fp, int cont, int mTotal, int m200, int m100, int m50, int m20, int m10, int m5, int m2, int m1, const char* videofile);

char* netpbm_get_token(FILE* file, char* tok, int len);
long int unsigned_char_to_bit(unsigned char* datauchar, unsigned char* databit, int width, int height);
void bit_to_unsigned_char(unsigned char* databit, unsigned char* datauchar, int width, int height);