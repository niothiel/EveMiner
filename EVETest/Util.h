#pragma once

#include <stdio.h>

using namespace std;

void imageTest(char* image, char* temp);
void fatalExit(string msg);
string formatTime(unsigned long time_millis);
void sleep(unsigned int ms, unsigned int variation = 100);
bool isMinimized(HWND window);
void ensureFocus();