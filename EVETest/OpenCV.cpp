#include "stdafx.h"

#include <iostream>
#include <stdio.h>
#include <Windows.h>

#include "OpenCV.h"
#include "Timer.h"
#include "IO.h"

extern HWND eveWindow;		// Handle to the eve window.
extern Point overviewLoc;	// Location of the first entry in the overview, set when undocked.

#define SCR_CAP_NAME "test.bmp"

//#define OPENCV_TEST

using namespace std;

void captureScreen(HWND window, string imgName)
{
	Timer t(1);

	HDC WinDC;
	HDC CopyDC;
	HBITMAP hBitmap;
	ULONG bWidth, bHeight;

	SetForegroundWindow(eveWindow);
	Sleep(100);
	WinDC = GetDC (window);
	CopyDC = CreateCompatibleDC (WinDC);

	RECT rect;
	GetClientRect(window, &rect);

	bWidth =  abs(rect.right - rect.left);
	bHeight = abs(rect.bottom - rect.top);

	hBitmap = CreateCompatibleBitmap(WinDC, bWidth, bHeight);
	SelectObject(CopyDC, hBitmap);
	BitBlt(CopyDC, 0, 0, bWidth, bHeight, WinDC, 0, 0, SRCCOPY);
               
	ReleaseDC(window, WinDC);
	DeleteDC(CopyDC);

	RGBQUAD *image;
	try {
		image = new RGBQUAD[bWidth*bHeight];
	}
	catch(std::bad_alloc)
	{
		return;
	}

	GetBitmapBits(hBitmap, bWidth*bHeight*4, image);
	writeBmp(imgName, bWidth, bHeight, 32, (int*)image);

	delete image;

	DeleteObject(hBitmap);

#ifdef OPENCV_TEST
	cout << "Capture Screen took " << t.elapsed() << " ms" << endl;
#endif
}

void writeBmp(string name,int W,int H,int Bpp,int* data)
{
	BITMAPINFO Bmi={0};
	Bmi.bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
	Bmi.bmiHeader.biWidth		= W;
	Bmi.bmiHeader.biHeight		= H;
	Bmi.bmiHeader.biPlanes		= 1;
	Bmi.bmiHeader.biBitCount	= Bpp; 
	Bmi.bmiHeader.biCompression	= BI_RGB;
	Bmi.bmiHeader.biSizeImage	= W*H*Bpp/8; 

	FILE* image = fopen (name.c_str(),"wb");
	if(image==0)
		return;
	int h = abs(Bmi.bmiHeader.biHeight);
	int w = abs(Bmi.bmiHeader.biWidth);
	Bmi.bmiHeader.biHeight=-h;
	Bmi.bmiHeader.biWidth=w;
	int sz = Bmi.bmiHeader.biSizeImage;

	BITMAPFILEHEADER bfh;
	bfh.bfType=('M'<<8)+'B'; 
	bfh.bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER); 
	bfh.bfSize=sz+bfh.bfOffBits; 
	bfh.bfReserved1=0; 
	bfh.bfReserved2=0; 
   
	fwrite(&bfh,sizeof(bfh),1,image);
	fwrite(&Bmi.bmiHeader,sizeof(BITMAPINFOHEADER),1,image);
	fwrite(data,sz,1,image);
	fflush(image);
	fclose(image);
}

void findOnScreen(string bmpName, double &correlation, bool refreshScr) {
	Point p;

	findOnScreen(bmpName, p, correlation, refreshScr);
}

void findOnScreen(string bmpName, Point &loc, double &correlation, bool refreshScreen/* Maybe rectangle stuff here? */) {
	if(refreshScreen)
		captureScreen(eveWindow, SCR_CAP_NAME);

	findInImage(SCR_CAP_NAME, bmpName, loc, correlation);
}

bool findAsteroidOnScreen(Point &loc) {
	captureScreen(eveWindow, SCR_CAP_NAME);

	Mat img;
	Mat templ;
	Mat result;

	/// Load image and template
	img = imread(SCR_CAP_NAME, 1 );
	templ = imread("nav_veld.bmp", 1 );

	/// Source image to display
	Mat img_display;
	img.copyTo( img_display );

	/// Create the result matrix
	int result_cols = img.cols - templ.cols + 1;
	int result_rows = img.rows - templ.rows + 1;

	result.create( result_cols, result_rows, CV_32FC1 );

	/// Do the Matching and Normalize
	matchTemplate( img, templ, result, 1 );

	loc.x = -1;
	loc.y = 50000;

	for(int x = 0; x < result_cols; x++) {
		for(int y = 0; y < result_rows; y++) {
			double corr = (double)result.at<float>(y, x);
			if(corr < 0.01 && loc.y > y) {
				loc.x = x;
				loc.y = y;

				cout << "found asteroid with correlation: " << corr << " at " << loc.x << ", " << loc.y << endl;
			}
		}
	}

	if(loc.x == -1) {
		return false;
	}

	loc.x += templ.cols / 2;
	loc.y += templ.rows / 2;

	cout << "Found highest in the screen location: " << loc.x << ", " << loc.y << endl;
	return true;
}

bool isImageOnScreen(string bmpName, double minCorrelation) {
	double corr;

	findOnScreen(bmpName, corr);
	return corr > minCorrelation;
}

bool clickImageOnScreen(string bmpName, double minCorrelation) {
	Point p;
	double corr;

	findOnScreen(bmpName, p, corr);

	if(corr > minCorrelation) {
		MoveMouse(p.x, p.y, 1);
		return true;
	}

	return false;
}

void findInImage(string image, string temp, Point &loc, double &correlation) {
	Timer t(1);

	int match_method = CV_TM_SQDIFF_NORMED;

	Mat img;
	Mat templ;
	Mat result;

	/// Load image and template
	img = imread(image, 1 );
	templ = imread(temp, 1 );

	/// Source image to display
	Mat img_display;
	img.copyTo( img_display );

	/// Create the result matrix
	int result_cols = img.cols - templ.cols + 1;
	int result_rows = img.rows - templ.rows + 1;

	result.create( result_cols, result_rows, CV_32FC1 );

	/// Do the Matching and Normalize
	matchTemplate( img, templ, result, match_method );
	//normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

	/// Localizing the best match with minMaxLoc
	double minVal; double maxVal; Point minLoc; Point maxLoc;
	Point matchLoc; double matchVal;

	minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );

	/// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better

	if( match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED ) {
		matchLoc = minLoc;
		matchVal = 1 - minVal;
	}
	else {
		matchLoc = maxLoc;
		matchVal = maxVal;
	}

	// Center the find
	matchLoc.x += templ.cols / 2;
	matchLoc.y += templ.rows / 2;

	/// Show me what you got
	//rectangle( result, matchLoc, Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), Scalar::all(0), 2, 8, 0 );

	loc = matchLoc;
	correlation = matchVal;

#ifdef OPENCV_TEST
	cout << "Find Image took: " << t.elapsed() << " ms." << endl;
#endif
}

bool findFurthestToLeft(Mat img, Mat templ, Point &p) {
	/// Create the result matrix
	int result_cols = img.cols - templ.cols + 1;
	int result_rows = img.rows - templ.rows + 1;

	if(result_rows < 1 || result_cols < 1)
		return false;

	Mat result;
	result.create( result_cols, result_rows, CV_32FC1 );

	/// Do the Matching and Normalize
	matchTemplate( img, templ, result, CV_TM_SQDIFF_NORMED );

	p.x = 50000;
	p.y = -1;

	for(int x = 0; x < result_cols; x++) {
		for(int y = 0; y < result_rows; y++) {
			double corr = (double)result.at<float>(y, x);
			if(corr < 0.01 && p.x > x) {
				p.x = x;
				p.y = y;
			}
		}
	}

	return p.y != -1;
}

Mat getBWImage(string name) {
	Mat tmp = imread(name, CV_LOAD_IMAGE_GRAYSCALE);
	Mat ret;

	adaptiveThreshold(tmp, ret, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 7, 0);
	return ret;
}

#define T_DIST_HEIGHT	19				// Height of each entry in the overview. Standard for 100% scaling.
#define T_DIST_WIDTH	65				// Width of the distance column in the overview.
#define T_DIST_ENTRIES	31				// Number of entries that fit in the overview. Doesn't really matter, as long as it's like >20.

// Holy shit balls I really wish I didn't have to write this shit.
double getDistance(int startX, int startY) {
	captureScreen(eveWindow, SCR_CAP_NAME);				// Grab the screen.

	Timer t(1);

	Mat img = imread(SCR_CAP_NAME);
	Mat nums = imread("nav_numbers.bmp");
	Mat nums_s = imread("nav_numbers_s.bmp");

	int endX = startX + T_DIST_WIDTH;
	int endY = startY + T_DIST_HEIGHT;

	img = img.colRange(startX, endX);			// Narrow down the search area to just the distance cell.
	img = img.rowRange(startY, endY);

	ostringstream os;

	while(1) {
		Point furthestLeft;
		int furthestLeftNumber = -1;
		furthestLeft.x = 5000;
		furthestLeft.y = 5000;

		for(int x = 0; x < 10; x++) {
			Mat num = nums.colRange(x*5, (x+1)*5);// Grab each number, one by one.
			Mat num_s = nums_s.colRange(x*5, (x+1)*5);

			Point p;

			if(!findFurthestToLeft(img, num, p) && !findFurthestToLeft(img, num_s, p))
				continue;						// Look for that number, if it's not there, continue.

			if(furthestLeft.x > p.x) {			// Search for the leftmost instance of this number, and save it.
				furthestLeft = p;
				furthestLeftNumber = x;
			}
		}
		if(furthestLeftNumber == -1)			// If there was no number found, break the loop.
			break;

		os << furthestLeftNumber;				// Add the number to our string stream.
		img = img.colRange(furthestLeft.x + 5, img.cols);// Cut the detected part of the image off to search for the next number.
	}

	double numMeters = (double) atoi(os.str().c_str());	// Turn the concatenation of numbers into an actual number.

	Point p;									// Next we look for "km" and "au" in the rest of the image.
	Mat kmText = imread("nav_km.bmp");
	Mat kmText_s = imread("nav_km_s.bmp");
	Mat auText = imread("nav_au.bmp");
	Mat auText_s = imread("nav_au_s.bmp");

	if(findFurthestToLeft(img, kmText, p))		// If this is true, that means "km" was found, multiply by 1000
		numMeters *= 1000;						// so now it will be in meters.
	else if(findFurthestToLeft(img, auText, p))
		numMeters *= 150000 * 100;				// If we detect AU, each one is 150km. We multiply by only 100 because AU is detected in 10ths.
	
	cout << "The number is: " << numMeters << endl;

	return numMeters;
}

void getNumber() {
	//int startX = T_DIST_START_X;
	//int startY = T_DIST_START_Y;

	int startX = overviewLoc.x;
	int startY = overviewLoc.y;

	for(int x = 0; x < 30; x++) {
		getDistance(startX, startY + T_DIST_HEIGHT * x);
	}
	Sleep(50000);
}