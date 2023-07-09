#ifndef _TIMECONTROLLER_H
#define _TIMECONTROLLER_H

#include "Types.h"

float edTimerTimeGet(void);

class Timer {
public:
	Timer();
	float Update();
	void ResetGameTimers();
	static Timer* GetTimer();

	float totalPlayTime;
	float frameDelta;
	float field_0x8;
	float timeScale;
	float totalTime;
	float lastFrameTime;
	float scaledTotalTime;
	float cutsceneDeltaTime;
	float frameTime;
};

Timer* GetTimer(void);

#endif //_TIMECONTROLLER_H
