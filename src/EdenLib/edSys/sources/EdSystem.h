#ifndef _EDSYSTEM_H
#define _EDSYSTEM_H

#include "Types.h"

struct S_MAIN_MEMORY_HEADER;

//enum edSysHandlerType
//{
//	ESHT_Profile_1 = 0,
//	ESHT_LoadFile = 4,
//	ESHT_RenderScene = 6,
//	ESHT_Sound = 7,
//	ESHT_RenderUI = 10,
//};

extern S_MAIN_MEMORY_HEADER* edSystemDataPtr_0042df0;

void _edSystemPrintf(int type, int eventID, char* format);
void edSystemBreak(void);
bool _edSystemInit(int argc, char** argv);
void _edSystemTerm();

extern int g_isNTSC;

#endif //_EDSYSTEM_H
