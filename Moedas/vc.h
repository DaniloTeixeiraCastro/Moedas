#ifndef VC_DEBUG
#define VC_DEBUG
#endif

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#ifndef MAX
#define MAX(a,b) (a > b ? a : b)
#endif
#ifndef MIN
#define MIN(a,b) (a < b ? a : b)
#endif

typedef struct {
    int x, y, width, height;  
    int area;                 
    int perimeter;            
    int xc, yc;               
    int label;                
    int tipo;                 
    double circularity;       
} OVC;

OVC* vc_binary_blob_labelling(cv::Mat src, cv::Mat dst, int* nlabels);
int vc_binary_blob_info(cv::Mat src, OVC* blobs, int nblobs);
int vc_desenha_bounding_box(cv::Mat src, OVC blobs);

int desenha_linhaVermelha(cv::Mat frame);
int desenha_linhaVerde(cv::Mat frame);

int idBlobs(cv::Mat frameIn, cv::Mat frameOut, int hueMin, int hueMax, float satMin, float satMax, int valueMin, int valueMax);


int verificaPassouAntes(OVC* passou, OVC moedas, int cont);
int idMoeda(int area, int perimeter, float circularity, cv::Vec3b meanColor);
void escreverInfo(FILE* fp, int cont, int mTotal, int m200, int m100, int m50, int m20, int m10, int m5, int m2, int m1, const char* videofile);

