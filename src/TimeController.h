#ifndef _TIMECONTROLLER_H
#define _TIMECONTROLLER_H

#include "Types.h"

float GetFrameTime_00291c40(void);

class TimeController {
public:
	TimeController();
	float UpdateDeltaTime();
	void ResetTimeController_001ba6d0();

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

TimeController* GetTimeController(void);

#endif //_TIMECONTROLLER_H
