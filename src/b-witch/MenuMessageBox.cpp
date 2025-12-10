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
#include "LevelScheduler.h"
#include "CompatibilityHandlingPS2.h"

byte MenuMessageBoxDisplay(ulong flags, ulong msgA, ulong msgB, ulong msgC, ulong msgD)
{
	ed_viewport* pViewport;
	bool cVar1;
	bool bVar1;
	uint uVar2;
	edCTextStyle* pNewFont;
	char* pcVar4;
	int confirmKey;
	int iVar6;
	byte closeResult;
	ulong uVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float x;
	float fVar12;
	float x_00;
	float fVar13;
	edCTextStyle textStyleA;
	edCTextStyle textStyleB;
	edCTextStyle textStyleC;
	edCTextStyle textStyleD;

	uVar7 = flags & 1;
	uVar2 = GameFlags & GAME_STATE_PAUSED;
	if ((uVar7 == 0) && (gPlayerInput.pressedBitfield = gPlayerInput.pressedBitfield & 0xfffffeff, uVar2 == 0)) {
		CallPauseChange(1);
	}

	fVar10 = ((float)gVideoConfig.screenHeight * 2.0f) / 2.5f;
	x = (float)gVideoConfig.screenWidth / 2.0f;
	fVar12 = ((float)gVideoConfig.screenWidth * 2.0f) / 2.5f;
	textStyleD.Reset();
	textStyleD.SetShadow(0x100);
	textStyleD.rgbaColour = -1;
	textStyleD.alpha = 0xff;
	textStyleD.SetScale(1.0f, 1.0f);
	textStyleD.SetFont(BootDataFont, false);
	textStyleD.SetHorizontalAlignment(2);
	textStyleD.SetVerticalAlignment(0);
	textStyleD.spaceSize = 6.0f;
	textStyleC.field_0x8c = textStyleD.field_0x8c * 0.5f;

	textStyleC.m0 = textStyleD.m0;
	textStyleC.m1 = textStyleD.m1;

	textStyleC.pFunction = textStyleD.pFunction;
	textStyleC.flags_0x84 = textStyleD.flags_0x84;
	textStyleC.pPackedFont = textStyleD.pPackedFont;
	textStyleC.field_0x90 = textStyleD.field_0x90;
	textStyleC.spaceSize = 6.0f;
	textStyleC.rotation = textStyleD.rotation;
	textStyleC.xScale = textStyleD.xScale;
	textStyleC.yScale = textStyleD.yScale;
	textStyleC.horizontalSize = textStyleD.horizontalSize;
	textStyleC.verticalSize = textStyleD.verticalSize;
	textStyleC.shadowShiftX = textStyleD.shadowShiftX;
	textStyleC.shadowShiftY = textStyleD.shadowShiftY;
	textStyleC.rgbaColour = textStyleD.rgbaColour;
	textStyleC.alpha = textStyleD.alpha;
	textStyleC.altColour = textStyleD.altColour;
	textStyleD.field_0x8c = textStyleC.field_0x8c;
	textStyleC.SetEolAutomatic(0x80);
	textStyleC.SetHorizontalJustification(0x10);
	textStyleC.SetHorizontalAlignment(0);
	textStyleC.spaceSize = 6.0f;

	textStyleB.m0 = textStyleD.m0;
	textStyleB.m1 = textStyleD.m1;

	textStyleB.pFunction = textStyleD.pFunction;
	textStyleB.flags_0x84 = textStyleD.flags_0x84;
	textStyleB.pPackedFont = textStyleD.pPackedFont;
	textStyleB.field_0x8c = textStyleD.field_0x8c;
	textStyleB.field_0x90 = textStyleD.field_0x90;
	textStyleB.spaceSize = textStyleD.spaceSize;
	textStyleB.rotation = textStyleD.rotation;
	textStyleB.xScale = textStyleD.xScale;
	textStyleB.yScale = textStyleD.yScale;
	textStyleB.horizontalSize = textStyleD.horizontalSize;
	textStyleB.verticalSize = textStyleD.verticalSize;
	textStyleB.shadowShiftX = textStyleD.shadowShiftX;
	textStyleB.shadowShiftY = textStyleD.shadowShiftY;
	textStyleB.rgbaColour = textStyleD.rgbaColour;
	textStyleB.alpha = textStyleD.alpha;
	textStyleB.altColour = textStyleD.altColour;
	textStyleB.SetHorizontalAlignment(0);
	textStyleB.SetVerticalAlignment(4);

	textStyleA.m0 = textStyleD.m0;
	textStyleA.m1 = textStyleD.m1;

	textStyleA.pFunction = textStyleD.pFunction;
	textStyleA.flags_0x84 = textStyleD.flags_0x84;
	textStyleA.pPackedFont = textStyleD.pPackedFont;
	textStyleA.field_0x8c = textStyleD.field_0x8c;
	textStyleA.field_0x90 = textStyleD.field_0x90;
	textStyleA.spaceSize = textStyleD.spaceSize;
	textStyleA.rotation = textStyleD.rotation;
	textStyleA.xScale = textStyleD.xScale;
	textStyleA.yScale = textStyleD.yScale;
	textStyleA.horizontalSize = textStyleD.horizontalSize;
	textStyleA.verticalSize = textStyleD.verticalSize;
	textStyleA.shadowShiftX = textStyleD.shadowShiftX;
	textStyleA.shadowShiftY = textStyleD.shadowShiftY;
	textStyleA.rgbaColour = textStyleD.rgbaColour;
	textStyleA.alpha = textStyleD.alpha;
	textStyleA.altColour = textStyleD.altColour;
	textStyleA.SetHorizontalAlignment(1);
	textStyleA.SetVerticalAlignment(4);
	iVar6 = 0;

	do {
#ifdef PLATFORM_PS2
		scePcStop();
#endif
		closeResult = 0;
		if ((flags & 2) == 0) {
			CPlayerInput::Update(GetTimer()->cutsceneDeltaTime);
			GetTimer()->Update();
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
			edTextStyleSetCurrent(&textStyleD);

			bVar1 = gCompatibilityHandlingPtr->HandleDisconnectedDevices(0);
			if (bVar1 == false) {
				pcVar4 = gMessageManager.get_message(msgA);
				edTextDraw(x, fVar13, pcVar4);
				if (msgB != 0) {
					x_00 = x;
					if ((flags & 8) == 0) {
						x_00 = fVar11 + 20.0f;
						textStyleC.SetHorizontalSize(fVar12 - 40.0f);
						edTextStyleSetCurrent(&textStyleC);
					}
					pcVar4 = gMessageManager.get_message(msgB);
					edTextDraw(x_00, fVar13 + 40.0f, pcVar4);
				}

				if (msgC != 0) {
					edTextStyleSetCurrent(&textStyleB);
					pcVar4 = gMessageManager.get_message(msgC);
					edTextDraw(fVar11 + 20.0f, fVar9 - 20.0f, pcVar4);
				}

				if (msgD != 0) {
					edTextStyleSetCurrent(&textStyleA);
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
					closeResult = 1;
				}
			}
			else {
				confirmKey = 0x19;
				if ((flags & 4) == 0) {
					confirmKey = 0x18;
				}

				if ((msgC == 0) || ((gPlayerInput.pressedBitfield & (1 << confirmKey)) == 0)) {
					if ((msgD != 0) && (((gPlayerInput.pressedBitfield & PAD_BITMASK_TRIANGLE) != 0 &&
							(closeResult = 2, CLevelScheduler::gThis->currentLevelID != 0x10)))) {
						IMPLEMENTATION_GUARD_AUDIO(
						PlaySample(1.0f, (float*)(Scene::ptable.g_FrontendManager_00451680)->field_0x78, 3, 0);)
					}
				}
				else {
					closeResult = 1;
					if (CLevelScheduler::gThis->currentLevelID != 0x10) {
						IMPLEMENTATION_GUARD_AUDIO(
						PlaySample(1.0f, (float*)(Scene::ptable.g_FrontendManager_00451680)->field_0x78, 0, 0);)
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
#ifdef PLATFORM_WIN
			Renderer::WaitUntilReady();
#endif

		}
	} while (((closeResult == 0) && (uVar7 == 0)) && ((GameFlags & 3) == 0));

	if ((uVar7 == 0) && (gPlayerInput.pressedBitfield = gPlayerInput.pressedBitfield & 0xfffffeff, uVar2 == 0)) {
		CallPauseChange(0);
	}

	return closeResult;
}
