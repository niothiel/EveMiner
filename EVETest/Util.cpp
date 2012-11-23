#include "stdafx.h"
#include "OpenCV.h"

using namespace std;

void imageTest(char* image, char* temp) {
	Point p;
	double corr;

	findInImage(image, temp, p, corr);

	cout << "Analysing " << image << " for " << temp << endl;
	cout << "Correlation: " << corr << " located at: " << p.x << ", " << p.y << endl;
}

string formatTime(unsigned long time_millis) {
	unsigned long hours = time_millis / (60 * 60 * 1000);
	unsigned long mins = time_millis / 1000 / 60 % 60;
	unsigned long seconds = time_millis / 1000 % 60;

	ostringstream oStream;

	if(hours)
		oStream << hours << "h ";
	if(mins)
		oStream << mins << "m ";
	oStream << seconds << "s";

	cout << "Timestamp currently: " << oStream.str() << endl;

	return oStream.str();
}

void fatalExit(string msg) {
	cout << "Fatal: " << msg << endl;
	cout << "Exiting in 30 seconds." << endl;
	Sleep(30000);
	exit(1);
}

void sleep(unsigned int ms, unsigned int variation) {
	Sleep(ms + rand() % variation);
}