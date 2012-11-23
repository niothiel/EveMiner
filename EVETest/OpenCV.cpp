#include "stdafx.h"

#include "IO.h"
#include "OpenCV.h"
#include "Timer.h"
#include "Util.h"

extern HWND eveWindow;		// Handle to the eve window.
extern Point overviewLoc;	// Location of the first entry in the overview, set when undocked.

#define SCR_CAP_NAME "test.bmp"

//#define OPENCV_TEST
#define MATCH_METHOD CV_TM_SQDIFF_NORMED

bool isMinimized(HWND window);

void captureScreen(HWND window, string imgName) {
	Timer t(1);

	HDC WinDC;
	HDC CopyDC;
	HBITMAP hBitmap;
	ULONG bWidth, bHeight;

	ensureFocus();
	WinDC = GetDC (window);
	CopyDC = CreateCompatibleDC (WinDC);

	RECT rect;
	GetClientRect(window, &rect);

	bWidth =  rect.right;
	bHeight = rect.bottom;

	hBitmap = CreateCompatibleBitmap(WinDC, bWidth, bHeight);
	SelectObject(CopyDC, hBitmap);
	BitBlt(CopyDC, 0, 0, bWidth, bHeight, WinDC, 0, 0, SRCCOPY);
               
	ReleaseDC(window, WinDC);
	DeleteDC(CopyDC);

	RGBQUAD *image = new RGBQUAD[bWidth*bHeight];

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

Mat safeImageRead(string fileName, int flags) {
	Mat img = imread(fileName, flags);

	if(img.data == NULL) {
		ostringstream os;
		os << "Error reading image, it most likely doesn't exist: " << fileName << endl;
		fatalExit(os.str());
	}

	return img;
}

Mat _matchTemplate(Mat img, Mat templ) {
	Mat result;

	// Create the result matrix
	int result_cols = img.cols - templ.cols + 1;
	int result_rows = img.rows - templ.rows + 1;

	// Whole bunch of error checking.
	if(result_cols <= 0 || result_rows <= 0) {
		fatalExit("Resultant matrix came back with zero or negative dimensions!");
	}

	if(img.data == NULL) {
		ostringstream os;
		os << "Error, image data is NULL: " << img << ", file probably doesn't exist!";
		fatalExit(os.str());
	}

	if(templ.data == NULL) {
		ostringstream os;
		os << "Error, template data is NULL: " << templ << ", file probably doesn't exist!" << endl;
		fatalExit(os.str());
	}

	if(img.type() != templ.type()) {
		ostringstream os;
		os << "Error with types or depths in templating: \"" << img << "\", \"" << templ << "\"" << endl;
		fatalExit(os.str());
	}

	result.create(result_cols, result_rows, CV_32FC1);

	// Do the matching
	matchTemplate(img, templ, result, MATCH_METHOD);

	// Normalize so the best value is 1, not 0 like usual.
	//result = Scalar::all(1) - result;

	return result;
}

void findOnScreen(string bmpName, double &correlation, bool refreshScr) {
	Point p;

	findOnScreen(bmpName, p, correlation, refreshScr);
}

void findOnScreen(string bmpName, Point &loc, double &correlation, bool refreshScreen) {
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
	img = safeImageRead(SCR_CAP_NAME);
	templ = safeImageRead("nav_veld.bmp");

	result = _matchTemplate(img, templ);

	loc.x = -1;
	loc.y = 50000;

	for(int x = 0; x < result.cols; x++) {
		for(int y = 0; y < result.rows; y++) {
			double corr = (double)result.at<float>(y, x);
			if(corr < 0.01 && loc.y > y) {
				loc.x = x;
				loc.y = y;
			}
		}
	}

	if(loc.x == -1) {
		return false;
	}

	loc.x += templ.cols / 2;
	loc.y += templ.rows / 2;

	cout << "Found highest asteroid in the screen location: " << loc.x << ", " << loc.y << endl;
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
		moveMouse(p.x, p.y, 1);
		return true;
	}

	return false;
}

void findInImage(string image, string temp, Point &loc, double &correlation, Dir searchDir, bool centerResult) {
	Timer t(1);

	Mat img;
	Mat templ;
	Mat result;

	/// Load image and template
	img = safeImageRead(image);
	templ = safeImageRead(temp);

	result = _matchTemplate(img, templ);

	if(searchDir == NONE) {
		/// Localizing the best match with minMaxLoc
		double minVal; double maxVal; Point minLoc; Point maxLoc;

		minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );

		/// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
		if( MATCH_METHOD  == CV_TM_SQDIFF || MATCH_METHOD == CV_TM_SQDIFF_NORMED ) {
			loc = minLoc;
			correlation = 1 - minVal;
		}
		else {
			loc = maxLoc;
			correlation = maxVal;
		}
	}
	else {
		// TODO: Directional search implementation
	}

	// Center the find
	if(centerResult) {
		loc.x += templ.cols / 2;
		loc.y += templ.rows / 2;
	}

#ifdef OPENCV_TEST
	cout << "Find Image took: " << t.elapsed() << " ms." << endl;
#endif
}

bool findFurthestToLeft(Mat img, Mat templ, Point &p) {
	Mat result;
	result = _matchTemplate(img, templ);

	p.x = 50000;
	p.y = -1;

	for(int x = 0; x < result.cols; x++) {
		for(int y = 0; y < result.rows; y++) {
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
	Mat tmp = safeImageRead(name, CV_LOAD_IMAGE_GRAYSCALE);
	Mat ret;

	adaptiveThreshold(tmp, ret, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 3, 0);
	return ret;
}

#define T_DIST_HEIGHT	19				// Height of each entry in the overview. Standard for 100% scaling.
#define T_DIST_WIDTH	65				// Width of the distance column in the overview.

// Holy shit balls I really wish I didn't have to write this shit.
double getDistance(int startX, int startY) {
	captureScreen(eveWindow, SCR_CAP_NAME);				// Grab the screen.

	Timer t(1);

	Mat img = safeImageRead(SCR_CAP_NAME);
	Mat nums = safeImageRead("nav_numbers.bmp");
	Mat nums_s = safeImageRead("nav_numbers_s.bmp");

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
	Mat kmText = safeImageRead("nav_km.bmp");
	Mat kmText_s = safeImageRead("nav_km_s.bmp");
	Mat auText = safeImageRead("nav_au.bmp");
	Mat auText_s = safeImageRead("nav_au_s.bmp");

	if(findFurthestToLeft(img, kmText, p))		// If this is true, that means "km" was found, multiply by 1000
		numMeters *= 1000;						// so now it will be in meters.
	else if(findFurthestToLeft(img, auText, p))
		numMeters *= 150000 * 100;				// If we detect AU, each one is 150km. We multiply by only 100 because AU is detected in 10ths.
	
	cout << "The number is: " << numMeters << endl;

	return numMeters;
}