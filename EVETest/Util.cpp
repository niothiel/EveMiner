#include "stdafx.h"

void sleep(unsigned int ms, unsigned int variation) {
	Sleep(ms + rand() % variation);
}