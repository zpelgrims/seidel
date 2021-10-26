#include "precomp.h"

Timer::Timer()
{
	running = false;
	startTime = std::chrono::system_clock::now();
	stopTime = std::chrono::system_clock::now();
}

void Timer::Start()
{
	startTime = std::chrono::system_clock::now();
	running = true;
}

void Timer::Stop()
{
	stopTime = std::chrono::system_clock::now();
	running = false;
}

float Timer::GetMilliseconds()
{
	if ( running ) stopTime = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>( stopTime - startTime ).count();
}

int Timer::CurrentTime()
{
	SYSTEMTIME time;
	GetSystemTime( &time );
	LONG time_ms = ( time.wSecond * 1000 ) + time.wMilliseconds;
	return (int)time_ms;
}