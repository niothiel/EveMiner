#include "stdafx.h"

#include <Windows.h>
#include <iostream>
#include "Timer.h"

Timer::Timer(unsigned long interval) {
	setInterval(interval);
	start();
}

void Timer::start() {
	startTime = _getTime();
	endTime = startTime + interval;
}

void Timer::setInterval(unsigned long millis) {
	interval = millis;
}

void Timer::wait() {
	while(!isDone())
		Sleep(10);
}

unsigned long Timer::elapsed() {
	return _getTime() - startTime;
}

void Timer::printElapsed() {
	std::cout << "Elapsed time: " << elapsed() << std::endl;
}

bool Timer::isDone() {
	return endTime < _getTime();
}

unsigned long Timer::_getTime() {
	return (unsigned long)GetTickCount();
}