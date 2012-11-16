#ifndef OPENCV_H
#define OPENCV_H

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

void findInImage(string image, string temp, Point &loc, double &correlation);

void findOnScreen(string bmpName, double &correlation, bool refreshScr = true);
void findOnScreen(string bmpName, Point &loc, double &correlation, bool refreshScreen = true /* Maybe rectangle stuff here? */);

bool isImageOnScreen(string bmpName, double minCorrelation);
bool clickImageOnScreen(string bmpName, double minCorrelation);

bool findAsteroidOnScreen(Point &loc);

void captureScreen(HWND window, string imgName);
void writeBmp(string name,int W,int H,int Bpp,int* data);

#endif