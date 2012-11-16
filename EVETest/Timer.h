#ifndef TIMER_H
#define TIMER_H

class Timer {
public:
	Timer(unsigned long interval);

	void start();
	void setInterval(unsigned long millis);
	void wait();
	unsigned long elapsed();
	void printElapsed();
	bool isDone();

private:
	unsigned long _getTime();

	unsigned long startTime;
	unsigned long interval;
	unsigned long endTime;
};


#endif