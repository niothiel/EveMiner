#include "stdafx.h"
#include "OpenCV.h"

extern HWND eveWindow;

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

bool isMinimized(HWND window) {
	return IsIconic(window);
}

void ensureFocus() {
	if(isMinimized(eveWindow)) {				// If the window is minimized, restore it
		ShowWindow(eveWindow, SW_RESTORE);
	}

	Sleep(300);

#ifdef DIRECT_IO								// And if we need active focus for IO, set it as the foreground window.
	if(GetForegroundWindow() != eveWindow) {	// This comparison might be redundant, not sure.
		SetForegroundWindow(eveWindow);
		Sleep(300);
	}
#endif
}