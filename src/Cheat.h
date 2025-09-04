#ifndef CHEAT_H
#define CHEAT_H

#include "Types.h"

typedef void(*CheatExecuteFunc)(int);

struct CheatCodeSequenceEntry
{
	uint inputFlag;
	float maxDuration;
	CheatExecuteFunc pFunc;
};

struct CheatCodeSequence
{
	CheatCodeSequence(CheatCodeSequenceEntry* aEntries);
	bool Update();

	int currentIndex;
	float duration;
	CheatCodeSequenceEntry* aEntries;
};

extern CheatCodeSequence gToggleDebugLevelCheatCode_004a5700;
extern CheatCodeSequence gToggleShrinkJamgutCheatCode_004a5720;
extern CheatCodeSequence gRefillLifeCheatCode_004a5740;
extern CheatCodeSequence gShowGalleryCheatCode_004a5760;

extern int INT_0044982c;

#endif // CHEAT_H