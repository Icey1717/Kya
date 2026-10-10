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

// Windows counterpart of PS2 CScene::Level_Term (0x001b9460). The original
// reverse manager walk terminates FX before entering CActorManager::Level_Term.
void CScene::Level_Term(void)
{
	FxFogProp* pFVar2;
	int curIndex;


	g_CinematicManager_0048efc->StopAllCutscenes();

	CLevelScheduler::gThis->Level_PreTerm();

	// Windows divergence: actor Term()/Destroy() run before the manager walk,
	// because behaviour cleanup still accesses FX pool handles. Keep the actor
	// and animation allocations alive: CNewFx::Kill (0x001a2b00) unregisters bones
	// through pActor->pAnimationController when the FX manager terminates.
	if (CScene::ptable.g_ActorManager_004516a4 != (CActorManager*)0x0) {
		CScene::ptable.g_ActorManager_004516a4->Level_TermActors();
	}

	curIndex = 0;
	CObjectManager** ppManager = CScene::ptable.IterateBackwards();
	do {
		// Skip a second Term()/Destroy() pass. Release actor/animation memory here,
		// after FX teardown, preserving the original manager order for deallocation.
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

	// PS2 calls edVideoFlip() here; the Windows renderer owns presentation.
	curIndex = 0;
	ppManager = CScene::ptable.IterateBackwards();
	do {
		if (*ppManager != (CObjectManager*)0x0) {
			(*ppManager)->Level_ClearAll();
		}
		curIndex = curIndex + 1;
		ppManager = ppManager - 1;
	} while (curIndex < 0x18);

	// Windows-only PS2 pointer handles must remain resolvable until every
	// manager has finished Level_ClearAll(); discard them before freeing pools.
	POINTERCONV_RESET_TRANSIENT();

	FreeAllAllocators();

	this->field_0x10c = -1;
	this->field_0x110 = -1;
	this->pElevatorCutsceneList = 0;

	return;
}

#endif
