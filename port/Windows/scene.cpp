#ifdef PLATFORM_WIN

#include "Scene.h"
#include "ActorManager.h"
#include "LevelScheduler.h"
#include "CinematicManager.h"
#include "DlistManager.h"
#include "PoolAllocators.h"
#include "edVideo/Viewport.h"
#include "edDlist.h"
#include "ed3D.h"
#include "port/pointer_conv.h"

void CScene::Level_Term(void)
{
	FxFogProp* pFVar2;
	int curIndex;


	g_CinematicManager_0048efc->StopAllCutscenes();

	CLevelScheduler::gThis->Level_PreTerm();

	// Actor behaviours still hold FX pool handles on Windows. Terminate them
	// while the pools exist, but retain actors and animations until FX have
	// unregistered their bones (PS2 Level_Term at 0x001b9460 frees FX first).
	if (CScene::ptable.g_ActorManager_004516a4 != (CActorManager*)0x0) {
		CScene::ptable.g_ActorManager_004516a4->Level_TermActors();
	}

	curIndex = 0;
	CObjectManager** ppManager = CScene::ptable.IterateBackwards();
	do {
		// Actor cleanup already ran; release its memory at the original position.
		if (*ppManager != (CObjectManager*)0x0 &&
			*ppManager == (CObjectManager*)CScene::ptable.g_ActorManager_004516a4) {
			CScene::ptable.g_ActorManager_004516a4->Level_FreeActors();
			curIndex = curIndex + 1;
			ppManager = ppManager - 1;
			continue;
		}
		if (*ppManager != (CObjectManager*)0x0) {
			(*ppManager)->Level_Term();
		}
		curIndex = curIndex + 1;
		ppManager = ppManager - 1;
	} while (curIndex < 0x18);

	PopFogAndClippingSettings(this->pFogClipStream);

	this->pFogClipStream = (S_STREAM_FOG_DEF*)0x0;
	this->field_0x48 = 0;

	edViewportSetClearMask(this->pViewportA, 0);
	edViewportSetClearMask(this->pViewportB, 0);

	pFVar2 = ed3DGetFxFogProp();
	pFVar2->field_0x0 = pFVar2->field_0x0 & 0xfffffffe;
	if (g_CinematicManager_0048efc->pCinematic == (CCinematic*)0x0) {
		this->fogClipSettingStackSize = -1;
	}

	edDListDeleteFrameBufferMaterial(&this->frameBufferMaterial);

	GlobalDList_AddToView();


	curIndex = 0;
	ppManager = CScene::ptable.IterateBackwards();
	do {
		if (*ppManager != (CObjectManager*)0x0) {
			(*ppManager)->Level_ClearAll();
		}
		curIndex = curIndex + 1;
		ppManager = ppManager - 1;
	} while (curIndex < 0x18);

	POINTERCONV_RESET_TRANSIENT();

	FreeAllAllocators();

	this->field_0x10c = -1;
	this->field_0x110 = -1;
	this->pElevatorCutsceneList = 0;

	return;
}

#endif
