#define VC_DEBUG

#include <opencv2/core/types_c.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                        MACROS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    unsigned char* data;
    int width, height;
    int channels;            // Binário/Cinzentos=1; RGB=3
    int levels;                // Binário=1; Cinzentos [1,255]; RGB [1,255]
    int bytesperline;        // width * channels
} IVC;


typedef struct {
    int x, y, width, height;      // Caixa Delimitadora
    int area;                     // Área
    int perimeter;                // Perimetro
    int xc, yc;                   // Centro de Massa
    int label;                    // Etiqueta
    int tipo;                     // Tipo de Moeda (1, 2, 5, 10, 20, 50 cent. 1, 2 euros.)
} OVC;




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROTÓTIPOS DE FUNÇÕES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
IVC* vc_image_new(int width, int height, int channels, int levels);
IVC* vc_image_free(IVC* image);

// FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC* vc_read_image(char* filename);
int vc_write_image(char* filename, IVC* image);

OVC* vc_binary_blob_labelling(IplImage* src, IplImage* dst, int* nlabels);

int vc_binary_blob_info(IplImage* src, OVC* blobs, int nblobs);

int vc_desenha_bounding_box(IplImage* src, OVC blobs);

int desenha_linhaVermelha(IplImage* frame);

int desenha_linhaVerde(IplImage* frame);

int idBlobs(IplImage* frameIn, IplImage* frameOut, int hueMin, int hueMax, float satMin, float satMax, int valueMin, int valueMax);

int verificaPassouAntes(OVC* passou, OVC moedas, int cont);

int idMoeda(int area, int perimeter);

void escreverInfo(FILE* fp, int cont, int mTotal, int m200, int m100, int m50, int m20, int m10, int m5, int m2, int m1, const char* videofile);




