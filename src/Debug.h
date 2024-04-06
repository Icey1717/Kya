#ifndef _DEBUG_H
#define _DEBUG_H

#include "Types.h"

struct _eddebug_module;

struct _eddebug {
	_eddebug_module* curModule;
	uint KeyboardID_00468f24;
};

void edDebugInit(void);
bool edDebugMenuIsDisplayed(void);

void ToggleCameraHack(void);

extern _eddebug edDebugMenu;

#endif // !_DEBUG_H
