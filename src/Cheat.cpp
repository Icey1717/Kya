#include "Cheat.h"
#include "Pause.h"
#include "ActorHero.h"
#include "TimeController.h"
#include "InputManager.h"

int gShrinkJamgutCheatSeq_00449828 = 0;

void ToggleDebugLevelCheat_00407960(int)
{
	gDebugLevelCheatEnabled_00449824 = gDebugLevelCheatEnabled_00449824 ^ 1;
	return;
}

void ToggleShrinkJamgutCheat_00407950(int)
{
	gShrinkJamgutCheatSeq_00449828 = gShrinkJamgutCheatSeq_00449828 ^ 1;
	return;
}

void FillHealthCheatCode_00407920(int)
{
	if (CActorHero::_gThis != (CActorHero*)0x0) {
		CActorHero::_gThis->LifeRestore();
	}

	return;
}

int INT_0044982c = 0;

void FUN_004078f0(int)
{
	int iVar1;

	iVar1 = INT_0044982c + 1;

	if ((INT_0044982c < iVar1) && (iVar1 < 6)) {
		INT_0044982c = iVar1;
	}

	return;
}

CheatCodeSequence::CheatCodeSequence(CheatCodeSequenceEntry* aEntries)
	: currentIndex(0), duration(0.0f), aEntries(aEntries)
{
}

bool CheatCodeSequence::Update()
{
	CheatCodeSequenceEntry* pSequenceEntry;
	float currentTime;

	pSequenceEntry = this->aEntries + this->currentIndex;
	if ((gPlayerInput.pressedBitfield & 1 << (pSequenceEntry->inputFlag & 0x1f)) != 0) {
		this->duration = 0.0f;
		if (pSequenceEntry->pFunc != (CheatExecuteFunc)0x0) {
			pSequenceEntry->pFunc(this->currentIndex);
		}

		pSequenceEntry = pSequenceEntry + 1;

		if (pSequenceEntry->inputFlag == 0x20) {
			this->currentIndex = 0;
			return true;
		}

		this->currentIndex = this->currentIndex + 1;
	}

	currentTime = this->duration + GetTimer()->lastFrameTime;
	this->duration = currentTime;

	if (pSequenceEntry->maxDuration < currentTime) {
		this->duration = 0.0f;
		this->currentIndex = 0;
	}

	return false;
}

CheatCodeSequenceEntry gToggleDebugLevelCheatCodeSeq_00427b30[11] =
{
	{ 0x0a, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x06, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x0a, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x0a, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x07, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x1b, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x05, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x1b, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x1b, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x10, 1.0f, ToggleDebugLevelCheat_00407960 },
	{ 0x20, 1.0f, (CheatExecuteFunc)0x0 },
};

CheatCodeSequence gToggleDebugLevelCheatCode_004a5700(gToggleDebugLevelCheatCodeSeq_00427b30);

CheatCodeSequenceEntry gToggleShrinkJamgutCheatCodeSeq_00427bc0[10] =
{
	{ 0x1e, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x1e, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x19, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x1e, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x1d, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x1d, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x16, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x1d, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x1a, 1.0f, ToggleShrinkJamgutCheat_00407950 },
	{ 0x20, 1.0f, (CheatExecuteFunc)0x0 },
};

CheatCodeSequence gToggleShrinkJamgutCheatCode_004a5720(gToggleShrinkJamgutCheatCodeSeq_00427bc0);

CheatCodeSequenceEntry gRefillLifeCheatCodeSeq_00427c40[12] =
{
	{ 0x1b, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x1e, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x1d, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x1c, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x14, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x14, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x16, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x1f, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x17, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x19, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x12, 1.0f, FillHealthCheatCode_00407920 },
	{ 0x20, 1.0f, (CheatExecuteFunc)0x0 },
};

CheatCodeSequence gRefillLifeCheatCode_004a5740(gRefillLifeCheatCodeSeq_00427c40);

CheatCodeSequenceEntry gShowGalleryCheatCodeSeq_00427cd0[9] = {
	{ 0x11, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x14, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x19, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x17, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x15, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x15, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x1f, 1.0f, (CheatExecuteFunc)0x0 },
	{ 0x16, 1.0f, FUN_004078f0 },
	{ 0x20, 1.0f, (CheatExecuteFunc)0x0 },
};