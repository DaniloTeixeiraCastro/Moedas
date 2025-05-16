#define _CRT_SECURE_NO_WARNINGS
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include "vc.h" // Mantendo sua estrutura original, mas adaptada para cv::Mat

int main(int argc, const char* argv[]) {
	// Vídeo
	const std::string videofile = "C:/Moedas/videos/video1.mp4";

	cv::VideoCapture capture(videofile);

	if (!capture.isOpened()) {
		std::cerr << "Erro ao abrir ficheiro!\n";
		return 1;
	}

	// Propriedades do vídeo
	int totalFrames = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_COUNT)),
		fps = static_cast<int>(capture.get(cv::CAP_PROP_FPS)),
		width = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH)),
		height = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_HEIGHT));

	cv::namedWindow("Visão por Computador - TP2", cv::WINDOW_AUTOSIZE);

	// Variáveis para contagem de moedas
	std::vector<OVC> passou(100);
	int cont = 0;
	int mTotal = 0;
	float soma = 0.0;
	int m200 = 0, m100 = 0, m50 = 0, m20 = 0, m10 = 0, m5 = 0, m2 = 0, m1 = 0;

	FILE* fp = fopen("Moedas.txt", "a");
	if (!fp) {
		std::cerr << "Erro ao abrir o ficheiro de guardar moedas!\n";
		return 1;
	}

	cv::Mat frameorig;
	while (true) {
		capture >> frameorig;
		if (frameorig.empty()) break;

		int currentFrame = static_cast<int>(capture.get(cv::CAP_PROP_POS_FRAMES));

		// Processamento de imagem
		cv::Mat framethr, frameaux;

		// Substituição da função idBlobs
		// idBlobs(frameorig, framethr, 12, 150, 35, 255, 20, 150);
		// Implementação alternativa:
		cv::Mat hsv;
		cv::cvtColor(frameorig, hsv, cv::COLOR_BGR2HSV);
		cv::inRange(hsv, cv::Scalar(12, 35, 20), cv::Scalar(150, 255, 150), framethr);

		// Operação morfológica
		cv::morphologyEx(framethr, framethr, cv::MORPH_OPEN,
			cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)),
			cv::Point(-1, -1), 5);

		// Rotulagem de blobs
		int nMoedas = 0;
		std::vector<OVC> moedas;
		// Substituição de vc_binary_blob_labelling
		// moedas = vc_binary_blob_labelling(framethr, frameaux, &nMoedas);
		// Implementação alternativa:
		std::vector<std::vector<cv::Point>> contours;
		cv::findContours(framethr.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		nMoedas = static_cast<int>(contours.size());
		moedas.resize(nMoedas);

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
			moedas[i] = blob;
		}

		// Desenhar linha vermelha
		cv::line(frameorig, cv::Point(0, frameorig.rows / 4),
			cv::Point(frameorig.cols, frameorig.rows / 4),
			cv::Scalar(0, 0, 255), 2);

		// Processar cada moeda detectada
		for (int i = 0; i < nMoedas; i++) {
			if (moedas[i].area > 8000) {
				// Mostrar informações
				std::string text = "CENTRO DE MASSA - x: " + std::to_string(moedas[i].xc) +
					", y: " + std::to_string(moedas[i].yc);
				cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc - 40),
					cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 0), 2);
				cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc - 40),
					cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 8, 255), 1);

				text = "AREA: " + std::to_string(moedas[i].area);
				cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc - 20),
					cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 0), 2);
				cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc - 20),
					cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 8, 255), 1);

				text = "PERIMETRO: " + std::to_string(moedas[i].perimeter);
				cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc),
					cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 0), 2);
				cv::putText(frameorig, text, cv::Point(moedas[i].xc + 90, moedas[i].yc),
					cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 8, 255), 1);

				// Mostrar contadores
				text = "NR TOTAL DE MOEDAS: " + std::to_string(mTotal);
				cv::putText(frameorig, text, cv::Point(20, 25),
					cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 0), 2);
				cv::putText(frameorig, text, cv::Point(20, 25),
					cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 8, 255), 1);

				// (Repetir para os outros contadores...)

				// Desenhar bounding box
				cv::rectangle(frameorig,
					cv::Point(moedas[i].x, moedas[i].y),
					cv::Point(moedas[i].x + moedas[i].width, moedas[i].y + moedas[i].height),
					cv::Scalar(255, 0, 0), 2);

				// Verificar se a moeda cruzou a linha
				if (frameorig.rows / 4 >= moedas[i].yc - 15 &&
					frameorig.rows / 4 <= moedas[i].yc + 20) {

					// Desenhar linha verde
					cv::line(frameorig, cv::Point(0, frameorig.rows / 4),
						cv::Point(frameorig.cols, frameorig.rows / 4),
						cv::Scalar(0, 255, 0), 2);

					int p = verificaPassouAntes(passou.data(), moedas[i], cont);
					if (p == 1) {
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

		cv::imshow("Visão por Computador - TP2", frameorig);

		int key = cv::waitKey(80);
		if (key == 'q') break;
	}

	cv::destroyWindow("Visão por Computador - TP2");
	capture.release();

	escreverInfo(fp, cont, mTotal, m200, m100, m50, m20, m10, m5, m2, m1, videofile.c_str());
	fclose(fp);

	return 0;
}
