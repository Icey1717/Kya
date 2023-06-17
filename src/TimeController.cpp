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
	totalPlayTime = 0.0f;
	frameDelta = 0.0f;
	field_0x8 = 1.0f;
	timeScale = 1.0f;
	lastFrameTime = 0.0f;
	totalTime = 0.0f;
	cutsceneDeltaTime = 0.0f;
	scaledTotalTime = 0.0f;
	if (g_isNTSC == 1) {
		frameTime = 0.02f;
	}
	else {
		frameTime = 0.01668335f;
	}
	SetDesiredFrameTime(frameTime);
}

TimeController* GetTimer(void)
{
	return &g_TimeController_004516d0;
}

#ifdef PLATFORM_PS2
uint lastTime = 0;
uint lastResult = 0;

unsigned int GetCountValue_Time(void)
{
	unsigned int i;

	__asm__ volatile (" mfc0 %0, $09" : "=r"(i));

	return i;
}

uint _edSystemTimerGetU32(void)
{
	uint uVar1;
	int iVar2;

	iVar2 = 0;
	uVar1 = GetCountValue_Time() / 0x48000;
	if (uVar1 < lastResult) {
		iVar2 = 0x38e3;
	}
	lastTime = lastTime + ((uVar1 + iVar2) - lastResult);
	lastResult = uVar1;
	return lastTime;
}
#endif

float _edSystemTimerGet(void)
{
#ifdef PLATFORM_PS2
	uint uVar1;
	float fVar2;

	uVar1 = _edSystemTimerGetU32();
	if ((int)uVar1 < 0) {
		fVar2 = (float)(uVar1 >> 1 | uVar1 & 1);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)uVar1;
	}
	return fVar2 / 1000.0;
#else
	return 0.033f;
#endif
}

float edTimerTimeGet(void)
{
	return _edSystemTimerGet();
}

float TimeController::Update()
{
	float currentTime;

	if (g_isNTSC == 1) {
		frameTime = 0.02f;
	}
	else {
		frameTime = 0.01668335f;
	}
	SetDesiredFrameTime(frameTime);
	lastFrameTime = frameTime;
	cutsceneDeltaTime = frameTime * timeScale;
	totalTime = totalTime + frameTime;
	scaledTotalTime = scaledTotalTime + frameTime * timeScale;
	currentTime = edTimerTimeGet();
	frameDelta = currentTime - totalPlayTime;
	totalPlayTime = currentTime;
	return currentTime;
}

void TimeController::ResetGameTimers()
{
	field_0x8 = 1.0;
	timeScale = 1.0;
	cutsceneDeltaTime = 0.0;
	scaledTotalTime = 0.0;
	return;
}
