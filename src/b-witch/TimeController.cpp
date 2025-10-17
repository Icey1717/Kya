#include "TimeController.h"
#include "EdenLib/edSys/sources/EdSystem.h"
#include "LargeObject.h"

float g_DesiredFrameTime_00483824;

Timer gTimer;

void SetDesiredFrameTime(float inTime)
{
	g_DesiredFrameTime_00483824 = inTime;
	return;
}

Timer::Timer()
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

Timer* GetTimer(void)
{
	return &gTimer;
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
	// Get the current time in milliseconds on windows using chrono
	auto now = std::chrono::high_resolution_clock::now();
	auto duration = now.time_since_epoch();
	auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	return (float)millis / 1000.0f;
#endif
}

float edTimerTimeGet(void)
{
	return _edSystemTimerGet();
}

float Timer::Update()
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

void Timer::ResetGameTimers()
{
	field_0x8 = 1.0;
	timeScale = 1.0;
	cutsceneDeltaTime = 0.0;
	scaledTotalTime = 0.0;
	return;
}

Timer* Timer::GetTimer()
{
	return &gTimer;
}

void Timer::SetTimeScale_001ba6f0(int bPaused)
{
	if (bPaused == 0) {
		GameFlags = GameFlags & 0xffffffdf;
		this->timeScale = this->field_0x8;
	}
	else {
		if (this->field_0x8 != this->timeScale) {
			this->field_0x8 = this->timeScale;
		}

		this->timeScale = 0.0;
		GameFlags = GameFlags | 0x20;
	}

	return;
}
