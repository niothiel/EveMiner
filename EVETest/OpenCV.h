#pragma once

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

enum Dir {
	UP,
	DOWN,
	LEFT,
	RIGHT,
	NONE
};

void getScreenshot();

Mat safeImageRead(string fileName, int flags = 1, bool prependImgDir = true);

void findInImage(string image, string temp, Point &loc, double &correlation, Dir searchDir = NONE, bool centerResult = true);

void findOnScreen(string bmpName, double &correlation, bool refreshScr = true);
void findOnScreen(string bmpName, Point &loc, double &correlation, bool refreshScreen = true /* Maybe rectangle stuff here? */);

bool isImageOnScreen(string bmpName, double minCorrelation);
bool clickImageOnScreen(string bmpName, double minCorrelation);

bool findAsteroidOnScreen(Point &loc);

void captureScreen(HWND window, string imgName);
void writeBmp(string name,int W,int H,int Bpp,int* data);

double getDistance(int startX, int startY, bool refreshScr = false);