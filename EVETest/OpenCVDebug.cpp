#include "stdafx.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <Windows.h>

using namespace std;
using namespace cv;

/// Global Variables
Mat img; Mat templ; Mat result;
char* image_window = "Source Image";
char* result_window = "Result window";

int match_method;
int max_Trackbar = 5;

/// Function Headers
void MatchingMethod( int, void* );

void getTopResults();
void tryAgain();

extern int width;
extern int height;

/** @function main */
void OpenCVTest()
{
  /// Load image and template
  img = imread( "mining.bmp", 1 );
  templ = imread( "veld_dense.bmp", 1 );

  /// Create windows
  namedWindow( image_window, CV_WINDOW_AUTOSIZE );
  namedWindow( result_window, CV_WINDOW_AUTOSIZE );

  /// Create Trackbar
  char* trackbar_label = "Method: \n 0: SQDIFF \n 1: SQDIFF NORMED \n 2: TM CCORR \n 3: TM CCORR NORMED \n 4: TM COEFF \n 5: TM COEFF NORMED";
  createTrackbar( trackbar_label, image_window, &match_method, max_Trackbar, MatchingMethod );

  MatchingMethod( 0, 0 );

  waitKey(0);
}

/**
 * @function MatchingMethod
 * @brief Trackbar callback
 */
void MatchingMethod( int, void* )
{
  /// Source image to display
  Mat img_display;
  img.copyTo( img_display );

  /// Create the result matrix
  int result_cols =  img.cols - templ.cols + 1;
  int result_rows = img.rows - templ.rows + 1;

  result.create( result_cols, result_rows, CV_32FC1 );



  // CHANGED THE FOLLOWING!!!!!!
  // CHANGED THE FOLLOWING!!!!!!
  // CHANGED THE FOLLOWING!!!!!!
  // CHANGED THE FOLLOWING!!!!!!
  match_method = 1;


  /// Do the Matching and Normalize
  matchTemplate( img, templ, result, match_method );
  //normalize( result, result, 0, 1000, NORM_MINMAX, -1, Mat() );

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


  /// Show me what you got
  rectangle( img_display, matchLoc, Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), Scalar::all(0), 2, 8, 0 );
  rectangle( result, matchLoc, Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), Scalar::all(0), 2, 8, 0 );

  int startRow = matchLoc.y - 250;
  int endRow = matchLoc.y + 250 + templ.rows;
  if(startRow < 0)
	  startRow = 0;
  if(endRow >= result.rows)
	  endRow = result.rows - 1;

  int startCol = matchLoc.x - 250;
  int endCol = matchLoc.x + 250 + templ.cols;
  if(startCol < 0)
	  startCol = 0;
  if(endCol >= result.cols)
	  endCol = result.cols - 1;

  cout << "Rows: " << startRow << ", " << endRow << endl;
  cout << "Cols: " << startCol << ", " << endCol << endl;

  img_display = img_display.rowRange(startRow, endRow);
  img_display = img_display.colRange(startCol, endCol);

  result = result.rowRange(startRow, endRow);
  result = result.colRange(startCol, endCol);
  
  cout << "Correlation: " << matchVal << endl;

  imshow( image_window, img_display );
  imshow( result_window, result );

  return;
}