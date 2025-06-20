#include "MenuMessageBox.h"
#include "LargeObject.h"
#include "Rendering/edCTextStyle.h"
#include "TimeController.h"
#include "edVideo/Viewport.h"
#include "kya.h"
#include "Pause.h"
#include "TranslatedTextData.h"
#include "edText.h"
#include "edVideo/VideoD.h"
#include "BootData.h"
#include "FrontendDisp.h"

#ifdef PLATFORM_PS2
#include "libpc.h"
#endif
#include "InputManager.h"
#include "LevelScheduleManager.h"
#include "CompatibilityHandlingPS2.h"

bool MenuMessageBoxDisplay(ulong flags, ulong msgA, ulong msgB, ulong msgC, ulong msgD)
{
	ed_viewport* pViewport;
	bool cVar1;
	bool bVar1;
	uint uVar2;
	Timer* pTVar3;
	edCTextStyle* pNewFont;
	char* pcVar4;
	int iVar5;
	int iVar6;
	bool bCloseResult;
	ulong uVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float x;
	float fVar12;
	float x_00;
	float fVar13;
	edCTextStyle local_300;
	edCTextStyle local_240;
	edCTextStyle local_180;
	edCTextStyle local_c0;

	uVar7 = flags & 1;
	uVar2 = GameFlags & GAME_STATE_PAUSED;
	if ((uVar7 == 0) && (gPlayerInput.pressedBitfield = gPlayerInput.pressedBitfield & 0xfffffeff, uVar2 == 0)) {
		IMPLEMENTATION_GUARD(
		FUN_001a7220(1);)
	}

	fVar10 = ((float)gVideoConfig.screenHeight * 2.0f) / 2.5f;
	x = (float)gVideoConfig.screenWidth / 2.0f;
	fVar12 = ((float)gVideoConfig.screenWidth * 2.0f) / 2.5f;
	local_c0.Reset();
	local_c0.SetShadow(0x100);
	local_c0.rgbaColour = -1;
	local_c0.alpha = 0xff;
	local_c0.SetScale(1.0f, 1.0f);
	local_c0.SetFont(BootDataFont, false);
	local_c0.SetHorizontalAlignment(2);
	local_c0.SetVerticalAlignment(0);
	local_c0.spaceSize = 6.0f;
	local_180.field_0x8c = local_c0.field_0x8c * 0.5f;

	local_180.m0 = local_c0.m0;
	local_180.m1 = local_c0.m1;

	local_180.pFunction = local_c0.pFunction;
	local_180.flags_0x84 = local_c0.flags_0x84;
	local_180.pPackedFont = local_c0.pPackedFont;
	local_180.field_0x90 = local_c0.field_0x90;
	local_180.spaceSize = 6.0f;
	local_180.rotation = local_c0.rotation;
	local_180.xScale = local_c0.xScale;
	local_180.yScale = local_c0.yScale;
	local_180.horizontalSize = local_c0.horizontalSize;
	local_180.verticalSize = local_c0.verticalSize;
	local_180.shadowShiftX = local_c0.shadowShiftX;
	local_180.shadowShiftY = local_c0.shadowShiftY;
	local_180.rgbaColour = local_c0.rgbaColour;
	local_180.alpha = local_c0.alpha;
	local_180.altColour = local_c0.altColour;
	local_c0.field_0x8c = local_180.field_0x8c;
	local_180.SetEolAutomatic(0x80);
	local_180.SetHorizontalJustification(0x10);
	local_180.SetHorizontalAlignment(0);
	local_180.spaceSize = 6.0f;

	local_240.m0 = local_c0.m0;
	local_240.m1 = local_c0.m1;

	local_240.pFunction = local_c0.pFunction;
	local_240.flags_0x84 = local_c0.flags_0x84;
	local_240.pPackedFont = local_c0.pPackedFont;
	local_240.field_0x8c = local_c0.field_0x8c;
	local_240.field_0x90 = local_c0.field_0x90;
	local_240.spaceSize = local_c0.spaceSize;
	local_240.rotation = local_c0.rotation;
	local_240.xScale = local_c0.xScale;
	local_240.yScale = local_c0.yScale;
	local_240.horizontalSize = local_c0.horizontalSize;
	local_240.verticalSize = local_c0.verticalSize;
	local_240.shadowShiftX = local_c0.shadowShiftX;
	local_240.shadowShiftY = local_c0.shadowShiftY;
	local_240.rgbaColour = local_c0.rgbaColour;
	local_240.alpha = local_c0.alpha;
	local_240.altColour = local_c0.altColour;
	local_240.SetHorizontalAlignment(0);
	local_240.SetVerticalAlignment(4);

	local_300.m0 = local_c0.m0;
	local_300.m1 = local_c0.m1;

	local_300.pFunction = local_c0.pFunction;
	local_300.flags_0x84 = local_c0.flags_0x84;
	local_300.pPackedFont = local_c0.pPackedFont;
	local_300.field_0x8c = local_c0.field_0x8c;
	local_300.field_0x90 = local_c0.field_0x90;
	local_300.spaceSize = local_c0.spaceSize;
	local_300.rotation = local_c0.rotation;
	local_300.xScale = local_c0.xScale;
	local_300.yScale = local_c0.yScale;
	local_300.horizontalSize = local_c0.horizontalSize;
	local_300.verticalSize = local_c0.verticalSize;
	local_300.shadowShiftX = local_c0.shadowShiftX;
	local_300.shadowShiftY = local_c0.shadowShiftY;
	local_300.rgbaColour = local_c0.rgbaColour;
	local_300.alpha = local_c0.alpha;
	local_300.altColour = local_c0.altColour;
	local_300.SetHorizontalAlignment(1);
	local_300.SetVerticalAlignment(4);
	iVar6 = 0;

	do {
#ifdef PLATFORM_PS2
		scePcStop();
#endif
		bCloseResult = false;
		if ((flags & 2) == 0) {
			pTVar3 = GetTimer();
			CPlayerInput::Update(pTVar3->cutsceneDeltaTime);
			pTVar3 = GetTimer();
			pTVar3->Update();
			cVar1 = GuiDList_BeginCurrent();
		}
		else {
			cVar1 = true;
		}

		if (cVar1 != false) {
			fVar11 = ((float)gVideoConfig.screenWidth - fVar12) / 2.0f;
			fVar13 = ((float)gVideoConfig.screenHeight - fVar10) / 2.0f;
			fVar8 = fVar11 + fVar12;
			fVar9 = fVar13 + fVar10;
			BootBitmaps[0x12].DrawXYXY(0, 1.0f, fVar11, fVar13, fVar8, fVar9);
			fVar13 = fVar13 + 20.0f;
			pNewFont = edTextStyleGetCurrent();
			edTextStyleSetCurrent(&local_c0);

			bVar1 = gCompatibilityHandlingPtr->HandleDisconnectedDevices(0);
			if (bVar1 == false) {
				pcVar4 = gMessageManager.get_message(msgA);
				edTextDraw(x, fVar13, pcVar4);
				if (msgB != 0) {
					x_00 = x;
					if ((flags & 8) == 0) {
						x_00 = fVar11 + 20.0f;
						local_180.SetHorizontalSize(fVar12 - 40.0f);
						edTextStyleSetCurrent(&local_180);
					}
					pcVar4 = gMessageManager.get_message(msgB);
					edTextDraw(x_00, fVar13 + 40.0f, pcVar4);
				}

				if (msgC != 0) {
					edTextStyleSetCurrent(&local_240);
					pcVar4 = gMessageManager.get_message(msgC);
					edTextDraw(fVar11 + 20.0f, fVar9 - 20.0f, pcVar4);
				}

				if (msgD != 0) {
					edTextStyleSetCurrent(&local_300);
					pcVar4 = gMessageManager.get_message(msgD);
					edTextDraw(fVar8 - 20.0f, fVar9 - 20.0f, pcVar4);
				}

				edTextStyleSetCurrent(pNewFont);
			}
			else {
				pcVar4 = gMessageManager.get_message(0x52525f503700080c);
				edTextDraw(x, fVar13 + 40.0f, pcVar4);
			}

			if ((msgC == 0) && (msgD == 0)) {
				iVar6 = iVar6 + 1;
				if (5 < iVar6) {
					bCloseResult = true;
				}
			}
			else {
				iVar5 = 0x19;
				if ((flags & 4) == 0) {
					iVar5 = 0x18;
				}

				if ((msgC == 0) || ((gPlayerInput.pressedBitfield & 1 << iVar5) == 0)) {
					if ((msgD != 0) && (((gPlayerInput.pressedBitfield & PAD_BITMASK_TRIANGLE) != 0 &&
							(bCloseResult = true, CLevelScheduler::gThis->currentLevelID != 0x10)))) {
						IMPLEMENTATION_GUARD_AUDIO(
						PlaySample(1.0, (float*)(Scene::ptable.g_FrontendManager_00451680)->field_0x78, 3, 0);)
					}
				}
				else {
					bCloseResult = true;
					if (CLevelScheduler::gThis->currentLevelID != 0x10) {
						IMPLEMENTATION_GUARD_AUDIO(
						PlaySample(1.0, (float*)(Scene::ptable.g_FrontendManager_00451680)->field_0x78, 0, 0);)
					}
				}
			}
		}
		if ((flags & 2) == 0) {
			if (cVar1 != false) {
				GuiDList_EndCurrent();
				GlobalDList_AddToView();
			}

			//edDebugMenuLoop();
			pViewport = (CScene::ptable.g_FrontendManager_00451680)->pViewport;
			edViewportSetClearMask(pViewport, 0);
			edVideoFlip();
			edViewportSetClearMask(pViewport, 0xffffffff);
		}
	} while (((bCloseResult == false) && (uVar7 == 0)) && ((GameFlags & 3) == 0));

	if ((uVar7 == 0) && (gPlayerInput.pressedBitfield = gPlayerInput.pressedBitfield & 0xfffffeff, uVar2 == 0)) {
		IMPLEMENTATION_GUARD(
		FUN_001a7220(0);)
	}

	return bCloseResult;
}
