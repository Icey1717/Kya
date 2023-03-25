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

#ifdef PLATFORM_PS2
uint UINT_00449178 = 0;
uint UINT_0044917c = 0;

unsigned int GetCountValue_Time(void)
{
	unsigned int i;

	__asm__ volatile (" mfc0 %0, $09" : "=r"(i));

	return i;
}

uint GetFrameTime_00291b80(void)
{
	uint uVar1;
	int iVar2;

	iVar2 = 0;
	uVar1 = GetCountValue_Time() / 0x48000;
	if (uVar1 < UINT_0044917c) {
		iVar2 = 0x38e3;
	}
	UINT_00449178 = UINT_00449178 + ((uVar1 + iVar2) - UINT_0044917c);
	UINT_0044917c = uVar1;
	return UINT_00449178;
}
#endif

float GetFrameTime_00291c40(void)
{
#ifdef PLATFORM_PS2
	uint uVar1;
	float fVar2;

	uVar1 = GetFrameTime_00291b80();
	if ((int)uVar1 < 0) {
		fVar2 = (float)(uVar1 >> 1 | uVar1 & 1);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)uVar1;
	}
	return fVar2 / 1000.0;
#else
	return 0.0f;
#endif
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
	currentTime = GetFrameTime_00291c40();
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
