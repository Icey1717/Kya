#include "TimeController.h"
#include "edSystem.h"

float g_DesiredFrameTime_00483824;

TimeController g_TimeController_004516d0;

void SetDesiredFrameTime(float inTime)
{
	g_DesiredFrameTime_00483824 = inTime;
	return;
}

TimeController::TimeController()
{
	totalPlayTime = 0.0;
	frameDelta = 0.0;
	field_0x8 = 1.0;
	timeScale = 1.0;
	lastFrameTime = 0.0;
	totalTime = 0.0;
	cutsceneDeltaTime = 0.0;
	scaledTotalTime = 0.0;
	if (g_isNTSC == 1) {
		frameTime = 0.02;
	}
	else {
		frameTime = 0.01668335;
	}
	SetDesiredFrameTime(frameTime);
}

TimeController* GetTimeController(void)
{
	return &g_TimeController_004516d0;
}

float TimeController::UpdateDeltaTime()
{
	float currentTime;

	if (g_isNTSC == 1) {
		frameTime = 0.02;
	}
	else {
		frameTime = 0.01668335;
	}
	SetDesiredFrameTime(frameTime);
	lastFrameTime = frameTime;
	cutsceneDeltaTime = frameTime * timeScale;
	totalTime = totalTime + frameTime;
	scaledTotalTime = scaledTotalTime + frameTime * timeScale;
	currentTime = 0.0f; // thunk_FUN_00291c40();
	frameDelta = currentTime - totalPlayTime;
	totalPlayTime = currentTime;
	return currentTime;
}

void TimeController::ResetTimeController_001ba6d0()
{
	field_0x8 = 1.0;
	timeScale = 1.0;
	cutsceneDeltaTime = 0.0;
	scaledTotalTime = 0.0;
	return;
}
