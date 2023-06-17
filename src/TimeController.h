#ifndef _TIMECONTROLLER_H
#define _TIMECONTROLLER_H

#include "Types.h"

float edTimerTimeGet(void);

class TimeController {
public:
	TimeController();
	float Update();
	void ResetGameTimers();

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

TimeController* GetTimer(void);

#endif //_TIMECONTROLLER_H
