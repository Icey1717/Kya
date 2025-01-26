#include "Frontend.h"
#include "TimeController.h"

void CFrontend::Level_ClearAll()
{
	return;
}

float CFrontend::GetTime()
{
	return Timer::GetTimer()->totalTime;
}

