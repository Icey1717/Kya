#include "PauseManager.h"
#include "TimeController.h"
#include "CinematicManager.h"
#include "LevelScheduleManager.h"
#include "Rendering/DisplayList.h"
#include "Rendering/Font.h"
#include "edText.h"

PauseManager* g_PauseManager_00451688;

PauseManager* PauseManager::SetupPauseManager_001b0eb0()
{
	pSubObj108_0x28 = (PauseScreenData*)0x0;
	pTexture_0x2c = (G2DObj_PauseManager*)0x0;
	field_0xd = 0;
	field_0xe = 0;
	field_0xf = 0;
	field_0x10 = 0;
	field_0x11 = 0;
	field_0x12 = 0;
	field_0x13 = 0;
	field_0x14 = 0;
	currentLevelID_0x18 = 0x10;
	field_0x1c = 0;
	field_0x20 = 0;
	field_0x24 = 0;
	currentAreaIndex = 0;
	totalAreaCount = 0;
	return this;
}

void PauseManager::OnBeginGame()
{
	//SetPauseManagerSubObjValue_002f2d10(&g_PauseScreenData_004507f0, PTR_DAT_00449754);
	//SetPauseManagerSubObjValue_002f2d00(&g_PauseScreenData_004507f0, PTR_DAT_00449754);
	//SetPauseManagerSubObjValue_002f2cf0(&g_PauseScreenData_004507f0, PTR_DAT_00449754);
	//SetPauseManagerSubObjValue_002f2d20(&g_PauseScreenData_004507f0, 0);
	field_0xf = 0;
	field_0x10 = 0;
	field_0x11 = 0;
	//FUN_001b5d20(&PTR_PTR_00450900);
	return;
}

extern int g_ScreenWidth;
extern int g_ScreenHeight;

char* inpLaceText = "PLEASE WAIT... LOADING";

FontPacked* g_MenuFont_00449754 = NULL;

void DrawLoadingScreen_001b05e0(void)
{
	bool bVar2;
	TimeController* pTVar3;
	FontFileData* pNewFont;
	char* pcVar4;
	float fVar5;
	FontFileData FStack192;
	bool bVar1;

	if (g_CinematicManager_0048efc->pSubObj_0x1c == (CinematicObjectB*)0x0) {
		pTVar3 = GetTimeController();
		fVar5 = pTVar3->totalTime * 256.0;
		if (fVar5 < 2.147484e+09) {
			FStack192.alpha = (int)fVar5;
		}
		else {
			FStack192.alpha = (int)(fVar5 - 2.147484e+09) | 0x80000000;
		}
		FStack192.alpha = FStack192.alpha % 0x140;
		if (0xff < (uint)FStack192.alpha) {
			FStack192.alpha = (0x40 - (FStack192.alpha - 0x100U)) * 0xff >> 6;
		}
		bVar1 = g_LevelScheduleManager_00449728->nextLevelID != 0xf;
		if (!bVar1) {
			FStack192.alpha = 0xff;
		}
		bVar2 = GuiDisplayListFunc_002d6360();
		if (bVar2 != false) {
			InitFontData_0028d430(&FStack192);
			pNewFont = SetActiveFontData(&FStack192);
			FStack192.SetFontTextureData_0028d3e0(NULL, 0);

			FStack192.SetFontFlag_0028d340(0x100);
			FStack192.alpha = FStack192.alpha & 0xff;
			FStack192.rgbaColour = FStack192.alpha | 0xffffff00;
			FStack192.SetFontFlag_0028d3c0(2);
			FStack192.SetFontFlag_0028d3a0(8);
			DrawTextParams drawTextParams = DrawTextParams();
			if (bVar1) {
				//pcVar4 = FindTranslatedTextFromKey_00336970(&g_TranslatedTextTRC_00449748, 0x30803025f4c4f41);
				//DrawTextA_0028c540(&drawTextParams, pcVar4);
				drawTextParams.Setup_0028c540(inpLaceText);
				DrawTextB_0028a710((float)g_ScreenWidth / 2.0, (float)g_ScreenHeight - 36.0, &drawTextParams);
			}
			else {
				//SetFontScale_0028d2b0(1.3, 1.3, &FStack192);
				//pcVar4 = FindTranslatedTextFromKey_00336970(&g_TranslatedTextTRC_00449748, 0x180403015f544845);
				//DrawTextA_0028c540(&drawTextParams, pcVar4);
				//DrawTextB_0028a710((float)g_ScreenWidth * 0.5, (float)g_ScreenHeight * 0.5, &drawTextParams);
			}
			SetActiveFontData(pNewFont);
			//DisplayListFunc_002d6340();
			//RunInPlaceDestructors_002173e0(drawTextParams.fontData_0x850, Free_00166e40, 0xc0, 0x11);
		}
	}
	return;
}


void PauseManager::LoadLevelUpdate()
{
	//PauseManagerFunc_001b0860(pPauseManager, 0);
	DrawLoadingScreen_001b05e0();
	return;
}
