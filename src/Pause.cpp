#include "Pause.h"
#include "TimeController.h"
#include "CinematicManager.h"
#include "LevelScheduleManager.h"
#include "Rendering/DisplayList.h"
#include "Rendering/Font.h"
#include "edText.h"
#include "TranslatedTextData.h"
#include "CompatibilityHandlingPS2.h"
#include "InputManager.h"
#include <assert.h>
#include "LargeObject.h"
#include "LocalizationManager.h"
#include "edDlist.h"
#include "CameraViewManager.h"
#include "MathOps.h"
#include <math.h>

#include "edFile.h"

#ifdef PLATFORM_WIN
#include "renderer.h"
#endif
#include "Rendering/edCTextStyle.h"
#include "Rendering/edCTextFormat.h"
#include "kya.h"
#include "ActorHero.h"
#include "SaveManagement.h"
#include "BootData.h"
#include "FrontEndDisp.h"
#include "Cheat.h"
#include "MenuMessageBox.h"

CSimpleMenuPause gPauseMenu;

uint UINT_00448eac;

CPauseManager::CPauseManager()
{
	pSimpleMenu = (CSimpleMenu*)0x0;
	pSplashScreen = (CSplashScreen*)0x0;
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
	return;
}

struct SliderParams
{
	void Init();

	edDList_material* pMaterial;
	uint field_0x4;
	undefined4 field_0x8;
	uint field_0xc;
	int field_0x10;
	int field_0x14;
	int field_0x18;
	uint field_0x1c;
};

void SliderParams::Init()
{
	this->field_0x10 = 0xd;
	this->field_0x14 = 0x20;
	this->field_0x18 = 0x20;
	this->field_0x1c = 0xfffffff2;
	this->field_0xc = 0x2a7f7f7f;
	this->field_0x8 = 0;
	this->field_0x4 = 0x7f7f7f7f;
	this->pMaterial = &MenuBitmaps[5].materialInfo;

	return;
}

SliderParams SliderParams_00450900;

void CPauseManager::Game_Init()
{
	gPauseMenu.set_font_title(BootDataFont);
	gPauseMenu.set_font_message(BootDataFont);
	gPauseMenu.set_font_help(BootDataFont);
	gPauseMenu.set_message_manager(NULL);
	field_0xf = 0;
	field_0x10 = 0;
	field_0x11 = 0;
	SliderParams_00450900.Init();
	return;
}

void CPauseManager::Game_Term()
{
	return;
}

bool CPauseManager::LevelLoading_Manage()
{
	return false;
}

edCTextFont* BootDataFont = NULL;

void ProfileDraw(int)
{
	IMPLEMENTATION_GUARD_LOG();
}

void PauseEnter(EPauseMenu mode)
{
	uint uVar1;
	CPauseManager* pCVar2;
	int iVar3;
	uint uVar4;
	int* piVar5;
	int* piVar6;

	pCVar2 = CScene::ptable.g_PauseManager_00451688;
	if ((GameFlags & 4) == 0) {
		ProfileDraw(0);

		gPauseMenu.set_mode(mode);
		gSettings.StoreGlobalSettings();

		iVar3 = CLevelScheduler::gThis->GetNbAreas(CLevelScheduler::gThis->currentLevelID);
		piVar6 = &pCVar2->currentAreaIndex;
		piVar5 = &pCVar2->totalAreaCount;
		if (iVar3 < 2) {
			*piVar5 = 0;
			*piVar6 = 0;
		}
		else {
			*piVar5 = iVar3 + -1;
			if (*piVar5 <= *piVar6) {
				*piVar6 = 0;
			}
		}

		uVar4 = GameFlags | 4;
		if ((GameFlags & 0x800) == 0) {
			if ((GameFlags & 0x40) == 0) {
				uVar1 = GameFlags & 0xc;
				UINT_00448eac = 1;
				GameFlags = uVar4;
				if ((uVar1 | 4) != 0) {
					CScene::_pinstance->SetGlobalPaused_001b8c30(1);
				}

				CScene::_pinstance->Level_PauseChange(1);
			}
			else {
				UINT_00448eac = 2;
				GameFlags = uVar4;
			}
		}
		else {
			UINT_00448eac = 2;
			GameFlags = uVar4;
			CScene::_pinstance->Level_PauseChange(1);
			CScene::_pinstance->SetGlobalPaused_001b8c30(1);
		}

		gCompatibilityHandlingPtr->ActivatePadAutoRepeat();
	}

	return;
}

void PauseLeave()
{
	gSettings.SetSettingsToGlobal();

	gSettings.field_0x0 = gSettings.field_0x0;
	gSettings.languageID = gSettings.languageID;
	gSettings.field_0x8 = gSettings.field_0x8;
	gSettings.bWidescreen = gSettings.bWidescreen;
	gSettings.setOffsetX = gSettings.setOffsetX;
	gSettings.setOffsetY = gSettings.setOffsetY;
	gSettings.field_0x14 = gSettings.field_0x14;
	gSettings.field_0x18 = gSettings.field_0x18;
	gSettings.field_0x1c = gSettings.field_0x1c;

	CScene::_pinstance->Level_PauseChange(0);
	CScene::_pinstance->SetGlobalPaused_001b8c30(0);

	if ((GameFlags & 4) == 0) {
		if ((GameFlags & 8) != 0) {
			GameFlags = GameFlags & 0xfffffff7;
		}
	}
	else {
		gCompatibilityHandlingPtr->DisactivatePadAutoRepeat();
		GameFlags = GameFlags & 0xfffffffb;
	}

	UINT_00448eac = 0;
	CScene::ptable.g_FrontendManager_00451680->SetActive(true);
	ProfileDraw(1);

	return;
}

undefined4 DAT_00448ea8;

void CallPauseChange(int param_1)
{
	CScene::_pinstance->Level_PauseChange(param_1);
	return;
}


void ResumeGame(int)
{
	if ((GameFlags & 0x800) != 0) {
		CallPauseChange(0);
	}

	DAT_00448ea8 = 0;

	if (UINT_00448eac == 2) {
		GetTimer()->Update();
		PauseLeave();
	}
	else {
		if ((GameFlags & 0xc) != 0) {
			GetTimer()->Update();
			PauseLeave();
		}

		UINT_00448eac = 0;
		CScene::_pinstance->SetGlobalPaused_001b8c30(0);
	}

	return;
}

bool UnpauseBlocked(void)
{
	return gPauseMenu.get_current_page() + ~PM_InitialSave < 2;
}

void DrawLoadingScreen_001b05e0(void)
{
	bool bVar2;
	Timer* pTVar3;
	edCTextStyle* pNewFont;
	char* pcVar4;
	float fVar5;
	edCTextStyle FStack192;
	bool bIsLoadingScreen;
	int localAlpha;

	if (g_CinematicManager_0048efc->pCinematic == (CCinematic*)0x0) {
		pTVar3 = GetTimer();
		fVar5 = pTVar3->totalTime * 256.0f;
		if (fVar5 < 2.147484e+09f) {
			localAlpha = (int)fVar5;
		}
		else {
			localAlpha = (int)(fVar5 - 2.147484e+09f) | 0x80000000;
		}
		localAlpha = localAlpha % 0x140;
		if (0xff < (uint)localAlpha) {
			localAlpha = (0x40 - (localAlpha - 0x100U)) * 0xff >> 6;
		}
		bIsLoadingScreen = CLevelScheduler::gThis->nextLevelID != 0xf;
		if (!bIsLoadingScreen) {
			localAlpha = 0xff;
		}

		bVar2 = GuiDList_BeginCurrent();
		if (bVar2 != false) {
			FStack192.Reset();
			pNewFont = edTextStyleSetCurrent(&FStack192);
			FStack192.SetFont(BootDataFont, false);

			FStack192.SetShadow(0x100);
			FStack192.alpha = localAlpha & 0xff;
			FStack192.rgbaColour = FStack192.alpha | 0xffffff00;
			FStack192.SetHorizontalAlignment(2);
			FStack192.SetVerticalAlignment(8);
			edCTextFormat drawTextParams = edCTextFormat();
			if (bIsLoadingScreen) {
				pcVar4 = gMessageManager.get_message(0x30803025f4c4f41);
				drawTextParams.FormatString(pcVar4);
				drawTextParams.Display((float)gVideoConfig.screenWidth / 2.0f, (float)gVideoConfig.screenHeight - 36.0f);
			}
			else {
				// This is the text for the final end game screen.
				FStack192.SetScale(1.3f, 1.3f);
				pcVar4 = gMessageManager.get_message(0x180403015f544845);
				drawTextParams.FormatString(pcVar4);
				drawTextParams.Display((float)gVideoConfig.screenWidth * 0.5f, (float)gVideoConfig.screenHeight * 0.5f);
			}

			edTextStyleSetCurrent(pNewFont);
			GuiDList_EndCurrent();

		}
	}
	return;
}


void CPauseManager::LevelLoading_Draw()
{
	//PauseManagerFunc_001b0860(pPauseManager, 0);
	DrawLoadingScreen_001b05e0();
	return;
}

int INT_00483804 = 0;

int FUN_00284930(void)
{
	return INT_00483804;
}

void CSettings::StoreGlobalSettings()
{
	CCinematicManager* pCVar1;
	CCameraManager* pCVar2;
	CAudioManager* pGVar3;
	undefined4 uVar4;
	LANGUAGE LVar5;
	float fVar6;
	int iVar4;

	pGVar3 = CScene::ptable.g_AudioManager_00451698;
	pCVar2 = CScene::ptable.g_CameraManager_0045167c;
	pCVar1 = g_CinematicManager_0048efc;
	IMPLEMENTATION_GUARD_AUDIO(
	fVar6 = (CScene::ptable.g_AudioManager_00451698)->field_0xbc * 12.0;
	if (fVar6 < 2.147484e+09f) {
		this->field_0x18 = (int)fVar6;
	}
	else {
		this->field_0x18 = (int)(fVar6 - 2.147484e+09f) | 0x80000000;
	}
	fVar6 = pGVar3->field_0xc0 * 12.0;
	if (fVar6 < 2.147484e+09f) {
		this->field_0x1c = (int)fVar6;
	}
	else {
		this->field_0x1c = (int)(fVar6 - 2.147484e+09f) | 0x80000000;
	}
	iVar4 = edSoundOutputModeGet();
	this->field_0x14 = iVar4;)
	this->setOffsetX = gVideoConfig.offsetX;
	this->setOffsetY = gVideoConfig.offsetY;
	if (pCVar2->aspectRatio == 1.777778) {
		this->bWidescreen = 1;
	}
	else {
		this->bWidescreen = 0;
	}
	this->field_0x8 = pCVar1->bInitialized != 0;
	LVar5 = CMessageFile::get_default_language();
	this->languageID = LVar5;
	this->field_0x0 = gPlayerInput.field_0x14 != 0;
	return;
}

void CPauseManager::Level_Init()
{
	CActorHero* pAVar1;
	CSimpleMenu* pSimpleMenu;
	CSplashScreen* pSplashScreen;
	Timer* pTVar4;
	undefined8 uVar5;

	this->currentLevelID_0x18 = CLevelScheduler::gThis->currentLevelID;
	this->field_0x1c = 0;
	this->field_0x20 = 0;

	pAVar1 = CActorHero::_gThis;
	if (CActorHero::_gThis != (CActorHero*)0x0) {
		if (this->field_0xe == 0) {
			CActorHero::_gThis->bCanUseCheats = 0;
		}
		else {
			CActorHero::_gThis->bCanUseCheats = 1;
		}
		if (this->field_0xf == 0) {
			pAVar1->field_0xaa4 = 0;
		}
		else {
			pAVar1->field_0xaa4 = 1;
		}
	}

	UINT_00448eac = 0;

	if ((GameFlags & 0x40) != 0) {
		pSimpleMenu = new CSimpleMenu();
		if (pSimpleMenu != (CSimpleMenu*)0x0) {
			this->pSimpleMenu = pSimpleMenu;
			this->pSimpleMenu->reset();
			this->pSimpleMenu->set_font_title(BootDataFont);
			this->pSimpleMenu->set_font_message(BootDataFont);
			this->pSimpleMenu->set_font_help(BootDataFont);
			this->pSimpleMenu->set_message_manager(&(CScene::ptable.g_LocalizationManager_00451678)->userInterfaceText);

			pSplashScreen = new CSplashScreen();
			this->pSplashScreen = pSplashScreen;
			NAME_NEXT_OBJECT("CDEURO/Frontend/kyatitle.g2d");
			this->pSplashScreen->Init(0.0f, "CDEURO/Frontend/kyatitle.g2d");
			this->pSplashScreen->SetDrawLocation((float)gVideoConfig.screenWidth / 2.0f - 80.0f, ((float)gVideoConfig.screenHeight * 20.0f) / 512.0f,
				(float)gVideoConfig.screenWidth / 2.0f + 80.0f, ((float)gVideoConfig.screenHeight * 220.0f) / 512.0f);

			pTVar4 = GetTimer();
			this->totalPlayTime = pTVar4->totalPlayTime;
		}
	}
	this->field_0x34 = 1;
	gSettings.StoreGlobalSettings();
	this->field_0x24 = 0;
	return;
}

void CPauseManager::Level_Term()
{
	CSimpleMenu* pCVar1;

	IMPLEMENTATION_GUARD_LOG(
	if (this->pTexture_0x2c != (CSplashScreen*)0x0) {
		this->pTexture_0x2c->Term();
		delete this->pTexture_0x2c;
		this->pTexture_0x2c = (CSplashScreen*)0x0;
	})

	pCVar1 = this->pSimpleMenu;
	if (pCVar1 != (CSimpleMenu*)0x0) {
		delete this->pSimpleMenu;
		this->pSimpleMenu = (CSimpleMenu*)0x0;
	}

	return;
}

void CPauseManager::Level_ClearAll()
{
	return;
}

void CPauseManager::Level_Manage()
{
	if (gCompatibilityHandlingPtr->TestCheatCode_IdleCamera() != false) {
		(CScene::ptable.g_CameraManager_0045167c)->cheatIdleCameraActive = 1;
	}

	return;
}

void CPauseManager::Level_ManagePaused()
{
	return;
}

struct RectVertex
{
	float x;
	float y;
	float z;
	uint field_0xc;
	uint color;
	float u;
	float v;
};

int gRectangleIndices[4] = { 7, 5, 6, 4 };
int gRectangleBorderIndices[10] = { 4, 0, 5, 1, 7, 3, 6, 2, 4, 0 };

void CPauseManager::DrawRectangleBorder(float tlx, float tly, float param_3, float param_4, float borderWidth, float borderHeight, uint color, uint borderColor, long bAlphaBlend)
{
	uint vtxSkip;
	int* pCurrentIndex;
	int iVar1;
	int aIndicesB[10];
	int aIndicesA[4];
	RectVertex aVertices[8];
	uint curColor;
	int curIndex;

	if (param_3 <= borderWidth * 2.0f) {
		borderWidth = param_3 * 0.5f;
	}
	if (param_4 <= borderHeight * 2.0f) {
		borderHeight = param_4 * 0.5f;
	}
	edDListUseMaterial((edDList_material*)0x0);
	edDListLoadIdentity();
	aVertices[0].z = 0.0f;
	aVertices[0].field_0xc = 0;
	aVertices[0].u = 0.0f;
	aVertices[0].v = 0.0f;
	aVertices[0].x = tlx - param_3 * 0.5f;

	aVertices[1].z = 0.0f;
	aVertices[1].field_0xc = 0;
	aVertices[1].u = 1.0f;
	aVertices[1].v = 0.0f;

	aVertices[2].z = 0.0f;
	aVertices[2].field_0xc = 0;
	aVertices[2].u = 0.0f;

	aVertices[1].x = tlx + param_3 * 0.5f;
	aVertices[2].v = 1.0f;
	aVertices[3].z = 0.0f;
	aVertices[3].field_0xc = 0;
	aVertices[3].u = 1.0f;
	aVertices[4].x = borderWidth + aVertices[0].x;
	aVertices[3].v = 1.0f;
	aVertices[4].z = 0.0f;
	aVertices[4].field_0xc = 0;
	aVertices[4].u = 0.25f;
	aVertices[4].v = 0.25f;
	aVertices[5].x = aVertices[1].x - borderWidth;
	aVertices[5].z = 0.0f;
	aVertices[5].field_0xc = 0;
	aVertices[5].u = 0.75f;
	aVertices[5].v = 0.25f;
	aVertices[6].u = 0.25f;
	aVertices[0].y = tly - param_4 * 0.5f;
	aVertices[6].z = 0.0f;
	aVertices[6].field_0xc = 0;
	aVertices[6].v = 0.75f;
	aVertices[7].z = 0.0f;
	aVertices[2].y = tly + param_4 * 0.5f;
	aVertices[7].field_0xc = 0;
	aVertices[7].u = 0.75f;
	aVertices[7].v = 0.75f;
	aVertices[4].y = borderHeight + aVertices[0].y;
	aVertices[6].y = aVertices[2].y - borderHeight;

	aIndicesA[0] = gRectangleIndices[0];
	aIndicesA[1] = gRectangleIndices[1];
	aIndicesA[2] = gRectangleIndices[2];
	aIndicesA[3] = gRectangleIndices[3];

	aIndicesB[0] = gRectangleBorderIndices[0];
	aIndicesB[1] = gRectangleBorderIndices[1];
	aIndicesB[2] = gRectangleBorderIndices[2];
	aIndicesB[3] = gRectangleBorderIndices[3];
	aIndicesB[4] = gRectangleBorderIndices[4];
	aIndicesB[5] = gRectangleBorderIndices[5];
	aIndicesB[6] = gRectangleBorderIndices[6];
	aIndicesB[7] = gRectangleBorderIndices[7];
	aIndicesB[8] = gRectangleBorderIndices[8];
	aIndicesB[9] = gRectangleBorderIndices[9];

	aVertices[0].color = borderColor;
	aVertices[1].y = aVertices[0].y;
	aVertices[1].color = borderColor;
	aVertices[2].x = aVertices[0].x;
	aVertices[2].color = borderColor;
	aVertices[3].x = aVertices[1].x;
	aVertices[3].y = aVertices[2].y;
	aVertices[3].color = borderColor;
	aVertices[4].color = color;
	aVertices[5].y = aVertices[4].y;
	aVertices[5].color = color;
	aVertices[6].x = aVertices[4].x;
	aVertices[6].color = color;
	aVertices[7].x = aVertices[5].x;
	aVertices[7].y = aVertices[6].y;
	aVertices[7].color = color;

	edDListBlendSet(1);

	if (bAlphaBlend == 0) {
		edDListBlendFuncNormal();
	}
	else {
		edDListAlphaBlend(1, 2, 0, 1, 0);
	}

	edDlistSetUseUV(1);

	edDListBegin(0.0f, 0.0f, 0.0f, 4, 4);
	iVar1 = 0;
	pCurrentIndex = aIndicesA;
	do {
		curIndex = *pCurrentIndex;
		curColor = aVertices[curIndex].color;
		vtxSkip = (curColor & 0xff0000) >> 0x10;
		edDListColor4u8((byte)((curColor & 0xff0000) >> 0x10), (byte)(curColor >> 8), (byte)curColor, (byte)(curColor >> 0x18));
		edDListTexCoo2f(aVertices[curIndex].u, aVertices[curIndex].v);
		edDListVertex4f(aVertices[curIndex].x, aVertices[curIndex].y, aVertices[curIndex].z, vtxSkip);
		iVar1 = iVar1 + 1;
		pCurrentIndex = pCurrentIndex + 1;
	} while (iVar1 < 4);
	edDListEnd();
	
	edDListBegin(0.0f, 0.0f, 0.0f, 4, 10);
	iVar1 = 0;
	pCurrentIndex = aIndicesB;
	do {
		curIndex = *pCurrentIndex;
		curColor = aVertices[curIndex].color;
		vtxSkip = (curColor & 0xff0000) >> 0x10;
		edDListColor4u8((byte)((curColor & 0xff0000) >> 0x10), (byte)(curColor >> 8), (byte)curColor, (byte)(curColor >> 0x18));
		edDListTexCoo2f(aVertices[curIndex].u, aVertices[curIndex].v);
		edDListVertex4f(aVertices[curIndex].x, aVertices[curIndex].y, aVertices[curIndex].z, vtxSkip);
		iVar1 = iVar1 + 1;
		pCurrentIndex = pCurrentIndex + 1;
	} while (iVar1 < 10);
	edDListEnd();

	return;
}



void ExitLevel(int)
{
	bool bVar1;
	uint uVar2;
	uint uVar3;

	bVar1 = MenuMessageBoxDisplay(4, 0x400c190c0b091d16, 0x4e5b5e041e161e02, 0x49461b10070b1e1b, 0x54120c1a0b051a07);
	if (bVar1 == true) {
		CLevelScheduler::gThis->ExitLevel(0);
		uVar2 = GameFlags & 0xfffff7ff;
		uVar3 = GameFlags & 0xc;
		GameFlags = uVar2;
		if (uVar3 != 0) {
			if (DAT_00448ea8 == 0) {
				GetTimer()->Update();
			}

			PauseLeave();
		}

		UINT_00448eac = 0;
		CScene::_pinstance->SetGlobalPaused_001b8c30(0);
	}

	return;
}

int INT_00448ed0 = 0;
int INT_00448650 = 0x3c;
bool BOOL_00448ed4 = 0;

float LevelNameRectPos_00448660 = 300.0f;
float LevelNameRectPos_00448664 = 35.0f;

float LevelNameRectPos_00448668 = 0.0f;
float LevelNameRectPos_0044866c = -90.0f;

uint levelNameRectColor = 0x407F7F7F;
uint levelNameRectBorderColor = 0x7F7F7F7F;

int GetScreenWidthSafe()
{
	int screenWidth = gVideoConfig.screenWidth;

	if (gVideoConfig.screenWidth < 0) {
		screenWidth = gVideoConfig.screenWidth + 1;
	}

	return screenWidth;
}

int GetScreenHeightSafe()
{
	int screenHeight = gVideoConfig.screenHeight;

	if (gVideoConfig.screenHeight < 0) {
		screenHeight = gVideoConfig.screenHeight + 1;
	}

	return screenHeight;
}

float FLOAT_00448670 = 0.25f;
float FLOAT_00448674 = 0.5f;

float FLOAT_00448678 = -85.0f;
float FLOAT_0044867c = 0.0f;

float FLOAT_00448680 = 85.0f;
float FLOAT_00448684 = 0.0f;

float FLOAT_00448688 = 0.0f;
float FLOAT_0044868c = 120.0f;

float FLOAT_00448690 = 0.25f;
float FLOAT_00448694 = 0.75f;
float FLOAT_00448698 = -135.0f;
float FLOAT_0044869c = 0.0f;

float FLOAT_004486a0 = 0.25f;
float FLOAT_004486a4 = 0.75f;
float FLOAT_004486a8 = 135.0f;
float FLOAT_004486ac = 0.0f;

float FLOAT_004486b0 = 320.0f;
float FLOAT_004486b4 = 130.0f;

void MenuFramePause_Draw(CSimpleMenu* pPauseMenu)
{
	CPauseManager* pCVar1;
	bool bShowLevelSwitchUI;
	int saveSlotId;
	Timer* pTVar4;

	pCVar1 = CScene::ptable.g_PauseManager_00451688;

	CPauseManager::DrawRectangleBorder((float)(GetScreenWidthSafe() >> 1) + LevelNameRectPos_00448668, (float)(GetScreenHeightSafe() >> 1) + LevelNameRectPos_0044866c,
		LevelNameRectPos_00448660, LevelNameRectPos_00448664, 4.0f, 4.0f, levelNameRectColor, levelNameRectBorderColor, 1);
	
	MenuBitmaps[12].rgba[0] = 0x7f;
	MenuBitmaps[12].rgba[1] = 0x69;
	MenuBitmaps[12].rgba[2] = 0x5f;
	MenuBitmaps[12].rgba[3] = 0x4d;
	
	MenuBitmaps[12].Draw(FLOAT_00448670, FLOAT_00448674, (float)(GetScreenWidthSafe() >> 1) + FLOAT_00448678, (float)(GetScreenHeightSafe() >> 1) + FLOAT_0044867c, 0xc012);
	MenuBitmaps[12].Draw(FLOAT_00448670, FLOAT_00448674, (float)(GetScreenWidthSafe() >> 1) + FLOAT_00448680, (float)(GetScreenHeightSafe() >> 1) + FLOAT_00448684, 0x4012);

	MenuBitmaps[12].rgba[0] = 0x7f;
	MenuBitmaps[12].rgba[1] = 0x69;
	MenuBitmaps[12].rgba[2] = 0x5f;
	MenuBitmaps[12].rgba[3] = 0x4d;
	
	MenuBitmaps[12].Draw(FLOAT_00448690, FLOAT_00448694, FLOAT_00448698 + (float)(GetScreenWidthSafe() >> 1) + FLOAT_00448688, FLOAT_0044869c + (float)(GetScreenHeightSafe() >> 1) + FLOAT_0044868c, 0xc012);
	
	MenuBitmaps[12].rgba[0] = 0x7f;
	MenuBitmaps[12].rgba[1] = 0x69;
	MenuBitmaps[12].rgba[2] = 0x5f;
	MenuBitmaps[12].rgba[3] = 0x4d;
	
	MenuBitmaps[12].Draw(FLOAT_004486a0, FLOAT_004486a4, FLOAT_004486a8 + (float)(GetScreenWidthSafe() >> 1) + FLOAT_00448688, FLOAT_004486ac + (float)(GetScreenHeightSafe() >> 1) + FLOAT_0044868c, 0x4012);
	
	CPauseManager::DrawRectangleBorder((float)(GetScreenWidthSafe() >> 1) + FLOAT_00448688, (float)(GetScreenHeightSafe() >> 1) + FLOAT_0044868c,
		FLOAT_004486b0, FLOAT_004486b4, 4.0f, 4.0f, levelNameRectColor, 0x007f7f7f, 1);

	pPauseMenu->draw_title(CLevelScheduler::gThis->aLevelInfo[CLevelScheduler::gThis->currentLevelID].titleMsgHash, -0x5a);
	pPauseMenu->set_vertical_spacing(0);

	if (BOOL_00448ed4 == false) {
		BOOL_00448ed4 = true;
		INT_00448ed0 = (GetScreenWidthSafe() >> 1) + INT_00448650;
	}
	pPauseMenu->set_vertical_position(INT_00448ed0);

	//iVar2 = pCVar1->totalAreaCount;
	//bShowLevelSwitchUI = false;
	//if (0 < iVar2) {
	//	bShowLevelSwitchUI = pPauseMenu->draw_option_type_wheel(&pCVar1->currentAreaIndex, 0, iVar2);
	//}

	/*Resume Button*/
	pPauseMenu->draw_action(0x31a1c100c180c0c, ResumeGame, 0, -2);
	pPauseMenu->set_vertical_spacing(0x18);

	saveSlotId = gSaveManagement.slotID_0x28;
	if (gSaveManagement.slotID_0x28 == -1) {
		saveSlotId = 0;
	}
	pPauseMenu->draw_option_type_page(0, 0x5e470f181a0c1217, PM_SaveMenu, saveSlotId, 0);
	pPauseMenu->draw_option_type_page(0, 0x49460f181a0c0d19, PM_LoadMenu, saveSlotId, 0);

	pPauseMenu->draw_option_type_page(0, 0x40a00065f4f5054, PM_OptionsMenu, 0, 0);

	pPauseMenu->draw_action(0x31a0b0d16194149, ExitLevel, 0, -3);

	gSettings.SetSettingsToGlobal();
	gSettings.field_0x0 = gSettings.field_0x0;
	gSettings.languageID = gSettings.languageID;
	gSettings.field_0x8 = gSettings.field_0x8;
	gSettings.bWidescreen = gSettings.bWidescreen;
	gSettings.setOffsetX = gSettings.setOffsetX;
	gSettings.setOffsetY = gSettings.setOffsetY;
	gSettings.field_0x14 = gSettings.field_0x14;
	gSettings.field_0x18 = gSettings.field_0x18;
	gSettings.field_0x1c = gSettings.field_0x1c;

	//PauseDrawInterface(bShowLevelSwitchUI);

	if (pPauseMenu->lastInput_0x14 == 8) {
		ResumeGame(0);
	}

	return;
}



uint DrawPauseMenu(CSimpleMenu* pMenu, uint action)
{
	CFrontendDisplay* this_00;
	CPauseManager* pCVar1;
	bool bHandleDisconnectedDevicesResult;
	int iVar3;
	char* pMessage;
	EPauseMenu EVar4;
	uint bResult;
	float fVar5;

	bHandleDisconnectedDevicesResult = gCompatibilityHandlingPtr->HandleDisconnectedDevices(0);

	this_00 = CScene::ptable.g_FrontendManager_00451680;
	if (bHandleDisconnectedDevicesResult == false) {
		bResult = 1;
		EVar4 = pMenu->get_current_page();
		this_00->SetActive(EVar4 == PM_PauseMenu);
		EVar4 = pMenu->get_current_page();
		switch (EVar4) {
		case PM_PauseMenu:
			MenuFramePause_Draw(pMenu);
			pCVar1 = CScene::ptable.g_PauseManager_00451688;
			if ((CScene::ptable.g_PauseManager_00451688)->field_0x24 != 0) {
				IMPLEMENTATION_GUARD(
				FUN_002f1b50(pMenu, FUN_002f33e0, 0);
				pCVar1->field_0x24 = 0;)
			}

			gToggleShrinkJamgutCheatCode_004a5720.Update();
			gRefillLifeCheatCode_004a5740.Update();
			break;
		case PM_MiniGame:
			IMPLEMENTATION_GUARD(
			fVar5 = (float)gVideoConfig.screenHeight * 0.4;
			if (fVar5 < 2.147484e+09) {
				BootBitmaps[18].iHeight = (ushort)(int)fVar5;
			}
			else {
				BootBitmaps[18].iHeight = (ushort)(int)(fVar5 - 2.147484e+09);
			}

			fVar5 = (float)gVideoConfig.screenWidth * 0.55;
			if (fVar5 < 2.147484e+09) {
				BootBitmaps[18].iWidth = (ushort)(int)fVar5;
			}
			else {
				BootBitmaps[18].iWidth = (ushort)(int)(fVar5 - 2.147484e+09);
			}
			BootBitmaps[18].color[3] = 0x50;
			(*(code*)(BootBitmaps[18].pVTable)->DrawB)
				(0x3f800000, (float)gVideoConfig.screenWidth * 0.5, (float)gVideoConfig.screenHeight * 0.5, 0x4915c0, 0x12);
			fVar5 = pMenu->selectedScaleX;
			pMenu->field_0xf0 = 1.5;
			draw_title(pMenu, 0x11a1d0e0a14044d, (int)((float)gVideoConfig.screenHeight * -0.11));
			pMenu->field_0xf0 = fVar5;
			draw_action(pMenu, 0x31a1c100c180c0c, FUN_001b5110, -2, -3);
			draw_action(pMenu, 0x5e5b000c110c1f08, FUN_001b5030, -2, -3);)
			break;
		case PM_SaveMenu:
			IMPLEMENTATION_GUARD(
			bHandleDisconnectedDevicesResult = MenuFrameSave_Draw(pMenu);
			bResult = (int)bHandleDisconnectedDevicesResult & 0xff;)
			break;
		case PM_LoadMenu:
			IMPLEMENTATION_GUARD(
			bResult = DrawLoadMenu_001b2ad0(pMenu);
			bResult = bResult & 0xff;)
			break;
		case PM_OptionsMenu:
			IMPLEMENTATION_GUARD(
			MenuFrameOption_Draw(pMenu);)
			break;
		case PM_DisplayOptions:
			IMPLEMENTATION_GUARD(
			MenuFrameCentering_Draw(pMenu, action);)
		}

		if (((((GameFlags & 0x800) == 0) && (bResult != 0)) && (EVar4 = pMenu->get_current_page(), EVar4 != PM_PauseMenu)) && (EVar4 = pMenu->get_current_page(), EVar4 != PM_DisplayOptions)) {
			pMenu->DrawInitialSaveMenuHelp(0x1d1a0c141c03454c, 0xffffffff);
		}
	}
	else {
		IMPLEMENTATION_GUARD(
		set_vertical_spacing(pMenu, 0);
		iVar3 = gVideoConfig.screenHeight;
		if (gVideoConfig.screenHeight < 0) {
			iVar3 = gVideoConfig.screenHeight + 1;
		}
		set_vertical_position(pMenu, (iVar3 >> 1) + 0x3c);
		FUN_002f2c60(pMenu);
		pMessage = get_message(&gMessageManager, 0x52525f503700080c);
		draw_option(pMenu, pMessage, 0xffffffff);
		bResult = 1;)
	}

	return bResult;
}

edF32VECTOR2 gClearDispTL = { -0.005f, -0.005f };
edF32VECTOR2 gClearDispBR = { 1.005f, 1.005f };

void ClearDisplay(void)
{
	CFrontendDisplay* pFrontendManager;
	bool tlSuccess;
	bool brSuccess;
	uint vtxSkip;
	edF32VECTOR4 brScreen;
	edF32VECTOR4 tlScreen;
	edF32VECTOR2 br;
	edF32VECTOR2 tl;

	pFrontendManager = CScene::ptable.g_FrontendManager_00451680;

	tl = gClearDispTL;
	br = gClearDispBR;

	tlSuccess = CScene::ptable.g_FrontendManager_00451680->ComputeSceneCoordinate(50.0f, &tlScreen, &tl);
	brSuccess = CScene::ptable.g_FrontendManager_00451680->ComputeSceneCoordinate(50.0f, &brScreen, &br);

	if (((tlSuccess != false) && (brSuccess != false)) && (Frontend2DDList_BeginCurrent() != false)) {
		edDListLoadIdentity();
		
		edDListUseMaterial(&MenuBitmaps[0xb].materialInfo);
		edDListColor4u8(0x7f, 0x7f, 0x7f, 0x7f);
		vtxSkip = 4;
		edDListBegin(0.0f, 0.0f, 0.0f, 4, 4);

		edDListTexCoo2f(0.0f, 0.0f);
		edDListVertex4f(tlScreen.x, tlScreen.y, tlScreen.z, vtxSkip);
		edDListTexCoo2f(1.0f, 0.0f);
		edDListVertex4f(brScreen.x, tlScreen.y, tlScreen.z, vtxSkip);
		edDListTexCoo2f(0.0f, 1.0f);
		edDListVertex4f(tlScreen.x, brScreen.y, tlScreen.z, vtxSkip);
		edDListTexCoo2f(1.0f, 1.0f);
		edDListVertex4f(brScreen.x, brScreen.y, tlScreen.z, vtxSkip);

		edDListEnd();

		FrontendDList_EndCurrent();
	}

	return;
}

void CPauseManager::Level_Draw()
{
	CLevelScheduler* pLevelScheduleManager;
	bool bHandleDisconnectedDevicesResult;
	bool bVar2;
	EPauseMenu EVar4;
	int nbCinematics;
	CCinematic* pCurCinematic;
	ulong uVar7;
	int index;
	CCinematic* pActiveCinematic;
	float totalPlayTime;

	if ((this->pSimpleMenu != (CSimpleMenu*)0x0) && (this->pSplashScreen != (CSplashScreen*)0x0)) {
		bHandleDisconnectedDevicesResult = gCompatibilityHandlingPtr->HandleDisconnectedDevices(0);

		totalPlayTime = GetTimer()->totalPlayTime;
		if ((gPlayerInput.pressedBitfield != 0) || (((this->field_0x34 == 0 && (EVar4 = this->pSimpleMenu->get_current_page(), EVar4 != PM_MainMenu)) ||
				(bVar2 = CScene::_pinstance->IsFadeTermActive(), bVar2 != 0)))) {
			this->totalPlayTime = totalPlayTime;
		}

		pActiveCinematic = (CCinematic*)0x0;
		nbCinematics = g_CinematicManager_0048efc->GetNumCutscenes_001c50b0();
		index = 0;
		if (0 < nbCinematics) {
			do {
				pCurCinematic = g_CinematicManager_0048efc->GetCinematic(index);
				if ((pCurCinematic->state != CS_Stopped) && ((pCurCinematic->flags_0x4 & 8) != 0)) {
					pActiveCinematic = pCurCinematic;
				}

				index = index + 1;
			} while (index < nbCinematics);
		}

		pLevelScheduleManager = CLevelScheduler::gThis;
		if (60.0f < totalPlayTime - this->totalPlayTime) {
			this->pSplashScreen->Manage(this->field_0x34, false, bHandleDisconnectedDevicesResult);
			if (pActiveCinematic == (CCinematic*)0x0) {
				pLevelScheduleManager->Level_Run(0, 0xe, -1, -1, -1, 1);
				if ((GameFlags & 4) != 0) {
					PauseLeave();
				}
			}
			else {
				if (pActiveCinematic->totalCutsceneDelta < 0.5) {
					IMPLEMENTATION_GUARD();
					pLevelScheduleManager->Level_Run(0, 0xe, -1, -1, -1, 1);
					GameFlags = GameFlags | 2;
				}
			}
		}
		else {
			if ((GameFlags & 0x80) == 0) {
				if (this->field_0x34 == 0) {
					bVar2 = GuiDList_BeginCurrent();
					if (bVar2 != false) {
						this->pSimpleMenu->draw(DrawGameMenu);
						GuiDList_EndCurrent();
					}

					this->pSimpleMenu->perform_action();

					if ((this->pSimpleMenu->get_current_page() == PM_MainMenu) && (bHandleDisconnectedDevicesResult == false)) {
						this->pSplashScreen->Manage(this->field_0x34, false, false);
					}
				}
				else {
					this->field_0x34 = this->pSplashScreen->Manage(this->field_0x34, false, bHandleDisconnectedDevicesResult);

					if (this->field_0x34 == 0) {
						gPauseMenu.set_mode(PM_MainMenu);
						UINT_00448eac = 2;
					}
				}
			}
		}
	}
	if ((GameFlags & 4) != 0) {
		if ((UINT_00448eac == 1) || (UINT_00448eac == 2)) {
			if ((GameFlags & 0x800) == 0) {
				ClearDisplay();
			}

			if (GuiDList_BeginCurrent() != false) {
				gPauseMenu.draw(DrawPauseMenu);
				GuiDList_EndCurrent();
			}
		}

		gPauseMenu.perform_action();
	}

	FUN_001b0860(0);

	return;
}

void CPauseManager::FUN_001b0860(int param_2)
{
	bool bVar1;
	Timer* pTVar2;
	ulong uVar3;
	uint uVar4;
	float fVar5;
	float fVar6;

	IMPLEMENTATION_GUARD_LOG(
	uVar3 = CSaveManagement::FUN_002f39c0(&gSaveManagement);
	if ((uVar3 != 0) || (param_2 != 0)) {
		pTVar2 = Timer::GetTimer();
		fVar5 = pTVar2->totalTime * 256.0;
		if (fVar5 < 2.147484e+09) {
			uVar4 = (uint)fVar5;
		}
		else {
			uVar4 = (int)(fVar5 - 2.147484e+09) | 0x80000000;
		}
		uVar4 = (int)uVar4 % 0x140;
		if (0xff < uVar4) {
			uVar4 = (0x40 - (uVar4 - 0x100)) * 0xff >> 6;
		}
		if (param_2 == 0) {
			fVar5 = 64.0;
			fVar6 = ((float)gVideoConfig.screenHeight * 320.0) / 512.0;
		}
		else {
			fVar5 = (float)gVideoConfig.screenWidth / 2.0;
			fVar6 = (float)gVideoConfig.screenHeight / 2.0 - ((float)gVideoConfig.screenHeight * 32.0) / 512.0;
		}
		bVar1 = GuiDList_BeginCurrent();
		if (bVar1 != false) {
			BootBitmaps[22].color[3] = (byte)(uVar4 >> 1);
			CSprite::Draw((float)&DAT_3f333333, fVar5, fVar6, BootBitmaps + 0x16, 0x12);
			BootBitmaps[22].color[3] = 0x7f;
			GuiDList_EndCurrent();
		}
	})

	return;
}

CSimpleMenu::CSimpleMenu()
{
	reset();
}

void CSimpleMenu::reset()
{
	Timer* pTVar1;

	this->field_0x2c = 0;
	this->currentPage = PM_MainMenu;
	this->selectedIndex = 0;
	this->field_0x50 = 3;
	this->screenState_0x1c = 0;
	pTVar1 = GetTimer();
	this->totalTime = pTVar1->totalTime;
	this->colorA = 0xfff609ff;
	this->colorB = 0xff9000ff;
	this->field_0xd4 = 0xffffffff;
	this->field_0xd8 = 0x14;
	this->field_0xf0 = 1.0f;
	this->field_0xf4 = 0;
	this->field_0xf8 = 0;
	this->field_0xfc = 0;
	this->field_0x100 = 0;
	this->field_0x104 = 0;
	return;
}

void CSimpleMenu::set_vertical_position(int verticalPos)
{
	this->verticalPos = verticalPos;
	return;
}

void CSimpleMenu::set_vertical_spacing(int verticalSpacing)
{
	this->verticalSpacing = verticalSpacing;
	return;
}

float CSimpleMenu::draw_func(float param_2)
{
	edDList_material* pMaterialInfo;
	bool bVar1;
	int iVar2;
	int iVar3;
	int iVar4;
	float s;
	float s_00;
	float in_f23;
	float unaff_f22;
	float xPos;

	iVar2 = this->horizontalAllignment;
	xPos = 0.0f;
	bVar1 = false;
	if (iVar2 == 2) {
		iVar2 = this->field_0xfc;
		if (iVar2 < 0) {
			iVar2 = iVar2 + 1;
		}
		unaff_f22 = (float)(this->verticalPos + (iVar2 >> 1));
		xPos = (float)(gVideoConfig.screenWidth - this->horizontalPos);
		if (this->field_0xf4 != 0) {
			xPos = xPos - (float)this->field_0xf8;
		}
		iVar2 = this->field_0xf8;
		if (iVar2 < 0) {
			iVar2 = iVar2 + 1;
		}
		bVar1 = true;
		in_f23 = xPos + (float)(iVar2 >> 1);
	}
	else {
		if (iVar2 == 1) {
			iVar2 = this->field_0xfc;
			if (iVar2 < 0) {
				iVar2 = iVar2 + 1;
			}
			iVar4 = this->field_0xf8;
			unaff_f22 = (float)(this->verticalPos + (iVar2 >> 1));
			iVar2 = iVar4;
			if (iVar4 < 0) {
				iVar2 = iVar4 + 1;
			}
			in_f23 = (float)(this->horizontalPos + (iVar2 >> 1));
			xPos = (float)this->horizontalPos;
			if (this->field_0xf4 != 0) {
				xPos = xPos + (float)iVar4;
			}
			bVar1 = true;
		}
		else {
			if (iVar2 == 0) {
				iVar2 = this->field_0xfc;
				if (iVar2 < 0) {
					iVar2 = iVar2 + 1;
				}
				unaff_f22 = (float)(this->verticalPos + (iVar2 >> 1));
				iVar2 = gVideoConfig.screenWidth;
				if (gVideoConfig.screenWidth < 0) {
					iVar2 = gVideoConfig.screenWidth + 1;
				}
				xPos = (float)(iVar2 >> 1);
				if (this->field_0xf4 != 0) {
					iVar2 = this->field_0xf8;
					if (iVar2 < 0) {
						iVar2 = iVar2 + 1;
					}
					xPos = xPos + (float)(iVar2 >> 1);
				}
				iVar2 = this->field_0xf8;
				if (iVar2 < 0) {
					iVar2 = iVar2 + 1;
				}
				bVar1 = true;
				in_f23 = (xPos - param_2 / 2.0f) - (float)(iVar2 >> 1);
			}
		}
	}
	if ((this->field_0xf4 != 0) && (bVar1)) {
		if (this->field_0x20 == this->selectedIndex) {
			pMaterialInfo = (edDList_material*)this->field_0x100;
		}
		else {
			pMaterialInfo = (edDList_material*)this->field_0x104;
		}
		edDListLoadIdentity();
		edDListColor4u8(0x7f, 0x7f, 0x7f, 0x7f);
		edDListUseMaterial(pMaterialInfo);
		s = 0.0f;
		s_00 = 1.0f;
		if (this->horizontalAllignment == 2) {
			s_00 = 0.0f;
			s = 1.0f;
		}
		iVar4 = 4;
		edDListBegin(0.0f, 0.0f, 0.0f, DISPLAY_LIST_DATA_TYPE_TRIANGLE_LIST, 4);
		edDListTexCoo2f(s, s);
		iVar2 = this->field_0xf8;
		if (iVar2 < 0) {
			iVar2 = iVar2 + 1;
		}
		iVar3 = this->field_0xfc;
		if (iVar3 < 0) {
			iVar3 = iVar3 + 1;
		}
		edDListVertex4f(in_f23 - (float)(iVar2 >> 1), unaff_f22 - (float)(iVar3 >> 1), 0.0f, iVar4);
		edDListTexCoo2f(s_00, s);
		iVar2 = this->field_0xf8;
		if (iVar2 < 0) {
			iVar2 = iVar2 + 1;
		}
		iVar3 = this->field_0xfc;
		if (iVar3 < 0) {
			iVar3 = iVar3 + 1;
		}
		edDListVertex4f(in_f23 + (float)(iVar2 >> 1), unaff_f22 - (float)(iVar3 >> 1), 0.0f, iVar4);
		edDListTexCoo2f(s, s_00);
		iVar2 = this->field_0xf8;
		if (iVar2 < 0) {
			iVar2 = iVar2 + 1;
		}
		iVar3 = this->field_0xfc;
		if (iVar3 < 0) {
			iVar3 = iVar3 + 1;
		}
		edDListVertex4f(in_f23 - (float)(iVar2 >> 1), unaff_f22 + (float)(iVar3 >> 1), 0.0f, iVar4);
		edDListTexCoo2f(s_00, s_00);
		iVar2 = this->field_0xf8;
		if (iVar2 < 0) {
			iVar2 = iVar2 + 1;
		}
		iVar3 = this->field_0xfc;
		if (iVar3 < 0) {
			iVar3 = iVar3 + 1;
		}
		edDListVertex4f(in_f23 + (float)(iVar2 >> 1), unaff_f22 + (float)(iVar3 >> 1), 0.0f, iVar4);
		edDListEnd();
	}
	return xPos;
}

void CSimpleMenu::draw_option(char* pMessage, uint color)
{
	int iVar1;
	Timer* pTVar2;
	edCTextStyle* pNewFont;
	float fVar3;
	edCTextStyle textStyle;

	textStyle.Reset();

	if (this->bShadow == 0) {
		textStyle.SetShadow(0);
	}
	else {
		textStyle.SetShadow(0x100);
		textStyle.alpha = 0xff;
	}

	textStyle.spaceSize = 6.0f;
	textStyle.field_0x8c = this->field_0xdc;

	if (this->field_0x20 == this->selectedIndex) {
		if (this->screenState_0x1c == 1) {
			textStyle.rgbaColour = this->colorB;
		}
		else {
			if (this->screenState_0x1c == 2) {
				pTVar2 = Timer::GetTimer();
				fVar3 = fmodf(pTVar2->totalTime, 0.1f);
				if (fVar3 < 0.05f) {
					textStyle.rgbaColour = this->colorB;
				}
				else {
					textStyle.rgbaColour = this->colorA;
				}
			}
			else {
				textStyle.rgbaColour = this->colorA;
			}
		}
		textStyle.SetScale(this->selectedScaleX, this->selectedScaleY);
	}
	else {
		textStyle.rgbaColour = this->colorA;
		textStyle.SetScale(this->scaleX, this->scaleY);
	}
	if (color != 0) {
		textStyle.rgbaColour = color;
	}

	iVar1 = this->horizontalAllignment;
	if (iVar1 == 2) {
		textStyle.SetHorizontalAlignment(1);
	}
	else {
		if (iVar1 == 1) {
			textStyle.SetHorizontalAlignment(0);
		}
		else {
			if (iVar1 == 0) {
				textStyle.SetHorizontalAlignment(2);
			}
		}
	}

	this->verticalPos = this->verticalPos + this->verticalSpacing;
	textStyle.SetFont(this->pFontB, false);
	pNewFont = edTextStyleSetCurrent(&textStyle);

	edCTextFormat textFormat = edCTextFormat(&textStyle);

	textFormat.FormatString(pMessage);
	fVar3 = draw_func(textFormat.field_0x8);
	textFormat.Display(fVar3, (float)this->verticalPos);
	edTextStyleSetCurrent(pNewFont);
	this->field_0x20 = this->field_0x20 + 1;
	return;
}

bool CSimpleMenu::draw_option_type_page(ulong helpMsgId, char* pMessage, EPauseMenu mode, int param_5, uint color)
{
	bool bResult;
	Timer* pTVar1;
	int iVar2;
	//SimpleMenuVTable** ppSVar3;

	iVar2 = this->field_0x20;
	draw_option(pMessage, color);
	if (iVar2 == this->selectedIndex) {
		if (helpMsgId != 0) {
			iVar2 = 0xc2;
			if (gVideoConfig.omode == 3) {
				iVar2 = 0xe0;
			}
			IMPLEMENTATION_GUARD(
			draw_help_line(this, helpMsgId, 0, iVar2 + this->counter_0x24 * -0x20, 0xffffffff);
			this->counter_0x24 = this->counter_0x24 + 1;)
		}

		if (this->screenState_0x1c == 1) {
			if (this->lastInput_0x14 == 5) {
				this->screenState_0x1c = 2;
				pTVar1 = Timer::GetTimer();
				this->totalTime = pTVar1->totalTime;
				if (mode == PM_None) {
					on_cancel();
				}
				else {
					on_validate();
				}
			}
		}
		else {
			if (this->screenState_0x1c == 3) {
				if (mode == PM_None) {
					IMPLEMENTATION_GUARD(
					this->field_0x2c = this->field_0x2c + -1;
					ppSVar3 = &this->pVTable + this->field_0x2c * 3;
					this->currentPage = (EPauseMenu)ppSVar3[0x15];
					this->selectedIndex = (int)ppSVar3[0x16];
					this->field_0x50 = (int)ppSVar3[0x17];)
				}
				else {
					this->aHistory[this->field_0x2c].currentPage = this->currentPage;
					this->aHistory[this->field_0x2c].selectedIndex = this->selectedIndex;
					this->aHistory[this->field_0x2c].field_0x8 = this->field_0x50;
					this->field_0x2c = this->field_0x2c + 1;
					this->currentPage = mode;
					this->selectedIndex = param_5;
				}

				this->screenState_0x1c = 4;
				pTVar1 = Timer::GetTimer();
				this->totalTime = pTVar1->totalTime;
			}
		}
		bResult = true;
	}
	else {
		bResult = false;
	}

	return bResult;
}

bool CSimpleMenu::draw_option_type_page(ulong helpMsgId, ulong msgId, EPauseMenu mode, int param_5, uint color)

{
	bool bResult;
	char* pMessage;

	if (this->pTranslatedTextData == (CMessageFile*)0x0) {
		pMessage = gMessageManager.get_message(msgId);
	}
	else {
		pMessage = this->pTranslatedTextData->get_message(msgId, 0);
	}
	bResult = draw_option_type_page(helpMsgId, pMessage, mode, param_5, color);
	return bResult;
}

bool CSimpleMenu::draw_option(char* pMsg, ActionFuncPtr pFunc, int slotID, int param_5)
{
	int iVar1;
	HistoryEntry* pHVar3;

	iVar1 = this->field_0x20;
	draw_option(pMsg, 0);

	if ((iVar1 != this->selectedIndex) || (pFunc == (ActionFuncPtr)0x0)) {
		return false;
	}
	if (this->screenState_0x1c == 1) {
		if (this->lastInput_0x14 == 5) {
			this->screenState_0x1c = 2;
			this->totalTime = Timer::GetTimer()->totalTime;
			on_validate();
		}
	}
	else {
		if (this->screenState_0x1c == 3) {
			this->pFunc_0x40 = pFunc;
			this->slotID_0x44 = slotID;

			if (param_5 != -1) {
				if (param_5 == -2) {
					this->screenState_0x1c = 4;
					this->totalTime = Timer::GetTimer()->totalTime;
					reset();
					return true;
				}

				if (param_5 == -3) {
					this->screenState_0x1c = 4;
					this->totalTime = Timer::GetTimer()->totalTime;
					return true;
				}
			}

			if (param_5 == -1) {
				this->field_0x2c = this->field_0x2c + -1;
				pHVar3 = this->aHistory + this->field_0x2c + -7;
				this->currentPage = (EPauseMenu)pHVar3[7].currentPage;
				this->selectedIndex = pHVar3[7].selectedIndex;
				this->field_0x50 = pHVar3[7].field_0x8;
			}
			else {
				/* pop page? */
				pHVar3 = this->aHistory + this->field_0x2c + -7;
				pHVar3[7].currentPage = this->currentPage;
				pHVar3[7].selectedIndex = this->selectedIndex;
				pHVar3[7].field_0x8 = this->field_0x50;
				this->field_0x2c = this->field_0x2c + 1;
				this->currentPage = (EPauseMenu)param_5;
				this->selectedIndex = 0;
			}

			this->screenState_0x1c = 4;
			this->totalTime = Timer::GetTimer()->totalTime;
		}
	}

	return true;
}

bool CSimpleMenu::draw_action(ulong actionMsgId, ActionFuncPtr pFunc, int slotID, int param_5)
{
	bool bVar1;
	char* pMsg;

	if (this->pTranslatedTextData == (CMessageFile*)0x0) {
		pMsg = gMessageManager.get_message(actionMsgId);
	}
	else {
		pMsg = this->pTranslatedTextData->get_message(actionMsgId, 0);
	}

	return draw_option(pMsg, pFunc, slotID, param_5);
}

void CSimpleMenu::set_font_help(edCTextFont* pFont)
{
	pFontC = pFont;
	return;
}

void CSimpleMenu::set_font_message(edCTextFont* pFont)
{
	pFontB = pFont;
	return;
}

void CSimpleMenu::set_font_title(edCTextFont* pFont)
{
	pFontA = pFont;
	return;
}

void CSimpleMenu::set_message_manager(CMessageFile* pTextData)
{
	pTranslatedTextData = pTextData;
	return;
}

EPauseMenu CSimpleMenu::get_current_page()
{
	return currentPage;
}

void CSimpleMenu::perform_action()
{
	if (this->pFunc_0x40 != 0x0) {
		this->pFunc_0x40(this->slotID_0x44);
		this->pFunc_0x40 = (ActionFuncPtr)0x0;
		this->slotID_0x44 = 0;
	}
	return;
}

uint CSimpleMenu::get_action()
{
	uint uVar1;

	uVar1 = 0;

	if (this->screenState_0x1c == 1) {
		if (((gPlayerInput.pressedBitfield & 0x100000) == 0) && ((gPlayerInput.pressedBitfield & 4) == 0)) {
			if (((gPlayerInput.pressedBitfield & 0x200000) == 0) && ((gPlayerInput.pressedBitfield & 8) == 0)) {
				if (((gPlayerInput.pressedBitfield & 0x400000) == 0) && ((gPlayerInput.pressedBitfield & 1) == 0)) {
					if (((gPlayerInput.pressedBitfield & 0x800000) == 0) && ((gPlayerInput.pressedBitfield & 2) == 0)) {
						uVar1 = 5;

						if (((((gPlayerInput.pressedBitfield & 0x1000000) == 0) &&
							(uVar1 = 8, (gPlayerInput.pressedBitfield & 0x4000000) == 0)) &&
							(uVar1 = 6, (gPlayerInput.pressedBitfield & 0x20) == 0)) &&
							(uVar1 = 7, (gPlayerInput.pressedBitfield & 0x40) == 0)) {
							uVar1 = 0;
						}
					}
					else {
						uVar1 = 4;
					}
				}
				else {
					uVar1 = 3;
				}
			}
			else {
				uVar1 = 2;
			}
		}
		else {
			uVar1 = 1;
		}
	}
	return uVar1;
}

void CSimpleMenu::draw_title(ulong msgHash, int offset)
{
	char* text;
	int iVar1;
	edCTextStyle* pNewFont;
	edCTextStyle eStack192;

	if (this->pTranslatedTextData == (CMessageFile*)0x0) {
		text = gMessageManager.get_message(msgHash);
	}
	else {
		text = this->pTranslatedTextData->get_message(msgHash, 0);
	}

	iVar1 = gVideoConfig.screenWidth;
	if (gVideoConfig.screenWidth < 0) {
		iVar1 = gVideoConfig.screenWidth + 1;
	}

	this->horizontalPos = iVar1 >> 1;

	iVar1 = gVideoConfig.screenHeight;
	if (gVideoConfig.screenHeight < 0) {
		iVar1 = gVideoConfig.screenHeight + 1;
	}

	this->verticalPos = (iVar1 >> 1) + offset;

	eStack192.Reset();
	eStack192.SetShadow(0x100);
	eStack192.rgbaColour = this->field_0xd4;
	eStack192.alpha = 0xff;
	eStack192.SetScale(this->field_0xf0, this->field_0xf0);
	eStack192.SetHorizontalAlignment(2);
	eStack192.SetVerticalAlignment(8);
	eStack192.SetFont(this->pFontA, false);
	pNewFont = edTextStyleSetCurrent(&eStack192);
	edTextDraw((float)this->horizontalPos, (float)this->verticalPos, text);
	edTextStyleSetCurrent(pNewFont);
	this->verticalSpacing = 0x28;
	return;
}

void CSimpleMenu::draw_help_line(ulong msgId, int x, int y, uint color)
{
	char* pMsg;
	int widthOffset;
	int heightOffset;
	edCTextStyle* pTextStyle;
	edCTextStyle textStyle;

	if ((gPlayerInput.disconnectedController == 0) && (this->screenState_0x1c == 1)) {
		if (this->pTranslatedTextData == (CMessageFile*)0x0) {
			pMsg = gMessageManager.get_message(msgId);
		}
		else {
			pMsg = this->pTranslatedTextData->get_message(msgId, 0);
		}

		widthOffset = gVideoConfig.screenWidth;
		if (gVideoConfig.screenWidth < 0) {
			widthOffset = gVideoConfig.screenWidth + 1;
		}

		heightOffset = gVideoConfig.screenHeight;
		if (gVideoConfig.screenHeight < 0) {
			heightOffset = gVideoConfig.screenHeight + 1;
		}

		textStyle.Reset();
		textStyle.SetShadow(0x100);
		textStyle.alpha = 0xff;
		textStyle.rgbaColour = color;
		textStyle.SetScale(1.0f, 1.0f);
		textStyle.SetHorizontalAlignment(2);
		textStyle.SetVerticalAlignment(8);
		textStyle.SetFont(this->pFontC, false);
		pTextStyle = edTextStyleSetCurrent(&textStyle);
		edTextDraw((float)(x + (widthOffset >> 1)), (float)(y + (heightOffset >> 1)), pMsg);
		edTextStyleSetCurrent(pTextStyle);
	}
	return;
}

int gDebugLevelCheatEnabled_00449824 = 0;

void CSimpleMenu::DrawMainMenu()
{
	CPauseManager* pCVar1;
	bool bVar2;
	SaveData5* pSVar3;
	int iVar4;
	int iVar5;

	pCVar1 = CScene::ptable.g_PauseManager_00451688;
	draw_title(0, -0x8c);

	iVar5 = gSaveManagement.slotID_0x28;
	if (gSaveManagement.slotID_0x28 == -1) {
		iVar5 = 0;
	}

	IMPLEMENTATION_GUARD_LOG(
	iVar4 = 0;
	do {
		pSVar3 = FUN_002f5230(&gSaveManagement, iVar4);
		bVar2 = FUN_002f2e80(pSVar3);
		if ((bVar2 != false) && (*(short*)&pSVar3[2].field_0x0 != 0)) {
			FUN_00407850((ulong) * (ushort*)((int)&pSVar3[1].field_0x0 + 3));
		}
		iVar4 = iVar4 + 1;
	} while (iVar4 < 4);)

	iVar4 = gVideoConfig.screenHeight;
	if (gVideoConfig.screenHeight < 0) {
		iVar4 = gVideoConfig.screenHeight + 1;
	}

	
	set_vertical_position((iVar4 >> 1) + 0x14);

	if (gDebugLevelCheatEnabled_00449824 == 0) {
		draw_option_type_page(0, 0x12020f181a4e4557, PM_InitialSave, iVar5, 0);
	}
	else {
		draw_option_type_page(0, 0x12020f181a4e4557, PM_InitialSave, iVar5, 0xff0000ff);
	}

	draw_option_type_page(0, 0x91a091412094f41, PM_LoadMenu, iVar5, 0);
	draw_option_type_page(0, 0x40a00065f4f5054, PM_OptionsMenu, 0, 0);
	draw_option_type_page(0, 0x18164e555f424f4e, PM_Bonus, 0, 0);

	IMPLEMENTATION_GUARD_LOG(
	if (pCVar1->field_0x24 != 0) {
		FUN_002f1b50(FUN_002f33e0, 0);
		pCVar1->field_0x24 = 0;
	})

	gSettings.SetSettingsToGlobal();

	gSettings.field_0x0 = gSettings.field_0x0;
	gSettings.languageID = gSettings.languageID;
	gSettings.field_0x8 = gSettings.field_0x8;
	gSettings.bWidescreen = gSettings.bWidescreen;
	gSettings.setOffsetX = gSettings.setOffsetX;
	gSettings.setOffsetY = gSettings.setOffsetY;
	gSettings.field_0x14 = gSettings.field_0x14;
	gSettings.field_0x18 = gSettings.field_0x18;
	gSettings.field_0x1c = gSettings.field_0x1c;
	return;
}

void CSimpleMenu::DrawInitialSaveMenuHelp(ulong helpMsgId, uint color)
{
	int xOffset;

	if (helpMsgId != 0) {
		xOffset = 0xc2;

		if (gVideoConfig.omode == 3) {
			xOffset = 0xe0;
		}

		draw_help_line(helpMsgId, 0, xOffset + this->counter_0x24 * -0x20, color);
		this->counter_0x24 = this->counter_0x24 + 1;
	}

	return;
}

uint DrawGameMenu(CSimpleMenu* pMenu, uint input)
{
	int iVar2;
	char* pcVar3;
	EPauseMenu EVar4;
	uint uVar5;

	if (gCompatibilityHandlingPtr->HandleDisconnectedDevices(0) == false) {
		EVar4 = pMenu->get_current_page();
		uVar5 = 1;
		if (EVar4 == PM_Bonus) {
			IMPLEMENTATION_GUARD(
			DrawFunc_002f2a80(pMenu, 0x1d1a0c141c03454c, 0xffffffff);
			DrawBonusScreen_001b3c70((long)(int)pMenu);)
		}
		else {
			if (EVar4 == PM_DisplayOptions) {
				IMPLEMENTATION_GUARD(
				CSimpleMenu::MenuFrameCentering_Draw(pMenu, input);)
			}
			else {
				if (EVar4 == PM_OptionsMenu) {
					IMPLEMENTATION_GUARD(
					CSimpleMenu::MenuFrameOption_Draw(pMenu);)
				}
				else {
					if (EVar4 == PM_LoadMenu) {
						IMPLEMENTATION_GUARD(
						uVar5 = DrawLoadMenu_001b2ad0(pMenu);
						uVar5 = uVar5 & 0xff;)
					}
					else {
						if (EVar4 == PM_InitialSave) {
							IMPLEMENTATION_GUARD(
							uVar5 = DrawNewGameScreen_001b24b0(pMenu);
							uVar5 = uVar5 & 0xff;)
						}
						else {
							if (EVar4 == PM_MainMenu) {
								pMenu->DrawMainMenu();
								gToggleDebugLevelCheatCode_004a5700.Update();
								gShowGalleryCheatCode_004a5760.Update();
								gRefillLifeCheatCode_004a5740.Update();
							}
						}
					}
				}
			}
		}

		if ((((uVar5 != 0) && (EVar4 = pMenu->get_current_page(), EVar4 != PM_MainMenu)) &&
			(EVar4 = pMenu->get_current_page(), EVar4 != PM_DisplayOptions)) &&
			(EVar4 = pMenu->get_current_page(), EVar4 != PM_Bonus)) {
			pMenu->DrawInitialSaveMenuHelp(0x1d1a0c141c03454c, 0xffffffff);
		}
	}
	else {
		IMPLEMENTATION_GUARD(
		CSimpleMenu::set_vertical_spacing(pMenu, 0);
		iVar2 = gVideoConfig.screenHeight;
		if (gVideoConfig.screenHeight < 0) {
			iVar2 = gVideoConfig.screenHeight + 1;
		}
		CSimpleMenu::set_vertical_position(pMenu, (iVar2 >> 1) + 0x3c);
		FUN_002f2c60(pMenu);
		pcVar3 = get_message(&gMessageManager, 0x52525f503700080c);
		CSimpleMenu::draw_option(pMenu, pcVar3, 0xffffffff);
		uVar5 = 1;)
	}

	return uVar5;
}

void CSimpleMenu::update_page()
{
	uint uVar1;
	//SimpleMenuVTable** ppSVar2;

	this->field_0x50 = this->field_0x20;
	uVar1 = this->lastInput_0x14;
	if (uVar1 == 8) {
		if (this->field_0x2c != 0) {
			this->field_0x2c = this->field_0x2c + -1;
			IMPLEMENTATION_GUARD(
			ppSVar2 = &this->pVTable + this->field_0x2c * 3;
			this->currentPage = (EPauseMenu)ppSVar2[0x15];
			this->selectedIndex = (int)ppSVar2[0x16];
			this->field_0x50 = (int)ppSVar2[0x17];
			(*(code*)this->pVTable->on_cancel)();)
		}
	}
	else {
		if (uVar1 == 2) {
			if (this->field_0x50 != 0) {
				this->selectedIndex = this->selectedIndex + 1;
				if (this->selectedIndex == this->field_0x50) {
					this->selectedIndex = 0;
				}
				if (1 < this->field_0x50) {
					on_change_selection();
				}
			}
		}
		else {
			if ((uVar1 == 1) && (this->field_0x50 != 0)) {
				this->selectedIndex = this->selectedIndex + -1;
				if (this->selectedIndex < 0) {
					this->selectedIndex = this->field_0x50 + -1;
				}

				if (1 < this->field_0x50) {
					on_change_selection();
				}
			}
		}
	}
	return;
}

void CSimpleMenu::draw(DrawMenuFuncPtr pInputFunc)
{
	int iVar1;

	this->field_0x20 = 0;
	this->counter_0x24 = 0;
	this->verticalSpacing = 0x28;
	this->horizontalAllignment = 0;
	this->field_0xdc = 6.0f;
	this->selectedScaleX = 1.0f;
	this->selectedScaleY = 1.0f;
	this->scaleX = 1.0f;
	this->scaleY = 1.0f;
	this->bShadow = 1;
	iVar1 = this->field_0x2c;
	this->pFunc_0x40 = 0;
	this->slotID_0x44 = 0;
	this->lastInput_0x14 = get_action();
	if ((iVar1 == this->field_0x2c) && (pInputFunc(this, this->lastInput_0x14) != false)) {
		update_page();
	}

	iVar1 = this->screenState_0x1c;
	if (iVar1 == 4) {
		this->screenState_0x1c = 0;
		this->totalTime = Timer::GetTimer()->totalTime;
	}
	else {
		if (iVar1 != 3) {
			if (iVar1 == 2) {
				if (disappear_draw() != 0) {
					this->screenState_0x1c = 3;
					this->totalTime = Timer::GetTimer()->totalTime;
				}
			}
			else {
				if (iVar1 == 1) {
					if (select_draw() != 0) {
						this->screenState_0x1c = 2;
						this->totalTime = Timer::GetTimer()->totalTime;
					}
				}
				else {
					if ((iVar1 == 0) && (appear_draw() != 0)) {
						this->screenState_0x1c = 1;
						this->totalTime = Timer::GetTimer()->totalTime;
					}
				}
			}
		}
	}

	return;
}

bool CSimpleMenu::select_draw()
{
	return false;
}

bool CSimpleMenu::appear_draw()
{
	return true;
}

bool CSimpleMenu::disappear_draw()
{
	return true;
}

void CSimpleMenu::on_validate()
{

}

void CSimpleMenu::on_cancel()
{

}

void CSimpleMenu::on_change_selection()
{

}

float CSimpleMenu::get_time_in_state()
{
	return Timer::GetTimer()->totalTime - this->totalTime;
}

CSplashScreen::CSplashScreen()
{
	this->pTextureFileData = (char*)0x0;
	this->field_0xcc = 0;
	this->field_0xc4 = 0.0f;
	this->field_0xd0 = 0;
	return;
}

bool CSplashScreen::Init(float param_1, char* filePath)
{
	uint fileSize;
	edFILEH* pLoadedFile;
	int iVar2;
	char* pcVar3;
	ed_g2d_bitmap* pBitmap;
	Timer* pTVar5;
	ed_g2d_material* pMaterialSection;
	float fVar6;
	int iStack4;

	edMemSetFlags(TO_HEAP(H_MAIN), 0x100);

	pLoadedFile = edFileOpen(filePath, 9);
	if (pLoadedFile != (edFILEH*)0x0) {
		fileSize = edFileLoadSize(pLoadedFile) + 0x7ffU & 0xfffff800;
		this->pTextureFileData = (char*)edMemAllocAlign(TO_HEAP(H_MAIN), fileSize, 0x40);
		if (this->pTextureFileData != (char*)0x0) {
			edFileRead(pLoadedFile, this->pTextureFileData, fileSize);

			while (pLoadedFile->nbQueuedActions != 0) {
				edFileNoWaitStackFlush();
			}
		}

		edFileClose(pLoadedFile);
	}

	pcVar3 = this->pTextureFileData;
	if (pcVar3 != (char*)0x0) {
		ed3DInstallG2D(pcVar3, *(int*)(pcVar3 + 8), &iStack4, &this->textureManager, 0);

		this->flags_0x7c = this->flags_0x7c | 2;
		this->bValid = false;

		pMaterialSection = ed3DG2DGetG2DMaterialFromIndex(&this->textureManager, 0);
		pBitmap = ed3DG2DGetBitmapFromMaterial(pMaterialSection, 0);
		edDListCreatMaterialFromIndex(&this->materialInfo, 0, &this->textureManager, 2);

		this->flags_0x7c = this->flags_0x7c | 1;

		this->iWidth = pBitmap->width;
		this->iHeight = pBitmap->height;

		this->textBitmap.pMaterial = &this->materialInfo;
		this->textBitmap.fWidth = (float)(uint)this->iWidth;
		this->textBitmap.fHeight = (float)(uint)this->iHeight;
	}

	edMemClearFlags(TO_HEAP(H_MAIN), 0x100);

	this->field_0xc8 = param_1;
	if (param_1 <= 0.0f) {
		this->field_0xc8 = 0.0001f;
	}

	this->field_0xcc = 0;
	this->field_0xc4 = GetTimer()->totalTime;

	(this->drawOffsets).x = 0.0f;
	(this->drawOffsets).y = 0.0f;
	(this->drawOffsets).z = (float)gVideoConfig.screenWidth;
	(this->drawOffsets).w = (float)gVideoConfig.screenHeight;

	return this->pTextureFileData != (char*)0x0;
}

bool CSplashScreen::Manage(uint param_2, bool param_3, bool param_4)
{
	bool bVar1;
	Timer* pTVar2;
	uint uVar3;
	edCTextStyle* pNewFont;
	char* pcVar4;
	byte r;
	undefined8 uVar5;
	int iVar6;
	int iVar7;
	uint uVar8;
	int iVar9;
	float fVar10;
	float fVar11;
	float y;
	float fVar12;
	float fVar13;
	float y_00;
	float fVar14;
	float fVar15;
	float fVar16;
	float fVar17;
	float x;
	float fVar18;
	edCTextStyle FStack192;

	uVar8 = 0x7f;
	pTVar2 = GetTimer();
	fVar12 = pTVar2->totalTime;
	if (((param_2 != 0) && ((gPlayerInput.releasedBitfield & PAD_BITMASK_START) != 0)) && (this->field_0xcc == 0))
	{
		this->field_0xcc = 1;
		pTVar2 = GetTimer();
		this->field_0xc4 = pTVar2->totalTime;
	}

	fVar15 = (fVar12 - this->field_0xc4) / this->field_0xc8;
	if (param_3 != false) {
		if (this->field_0xcc == 0) {
			if (fVar15 < 1.0f) {
				fVar10 = fVar15 * 127.0f;
				if (2.147484e+09f <= fVar10) {
					fVar10 = fVar10 - 2.147484e+09f;
				}
				uVar8 = (int)fVar10 & 0xff;
			}
		}
		else {
			if (fVar15 < 1.0f) {
				fVar10 = (1.0f - fVar15) * 127.0f;
				if (2.147484e+09f <= fVar10) {
					fVar10 = fVar10 - 2.147484e+09f;
				}
				uVar8 = (int)fVar10 & 0xff;
			}
			else {
				uVar8 = 0;
			}
		}
	}

	bVar1 = GuiDList_BeginCurrent();
	if (bVar1 != false) {
		if ((this->pTextureFileData != (char*)0x0) && (param_4 == false)) {
			fVar13 = (this->drawOffsets).z - (this->drawOffsets).x;
			fVar10 = (this->drawOffsets).w - (this->drawOffsets).y;
			if (this->field_0xd0 != 0) {
				edDListUseMaterial((edDList_material*)0x0);
				edDListColor4u8(0, 0, 0, 0x80);
				edDListLoadIdentity();
				iVar9 = 6;
				edDListBegin(1.0f, 1.0f, 1.0f, DISPLAY_LIST_DATA_TYPE_SPRITE, 2);
				edDListVertex4f(0.0f, 0.0f, 0.0f, iVar9);
				edDListVertex4f(fVar13, fVar10, 0.0f, iVar9);
				edDListEnd();
			}

			edDListUseMaterial(&this->materialInfo);
			edDListLoadIdentity();
			edDListBlendSet(1);
			edDListBlendFuncNormal();
			r = (byte)uVar8;
			edDListColor4u8(r, r, r, r);
			y_00 = (this->drawOffsets).y;
			iVar7 = (int)(fVar13 / 32.0f) + 1;
			iVar9 = (int)(fVar10 / 32.0f);
			fVar11 = y_00 + 32.0f;
			fVar10 = 1.0f / (float)(int)(fVar13 / 32.0f);
			fVar13 = 1.0f / (float)iVar9;
			fVar16 = fVar13 + 0.0f;
			if (iVar9 != 0) {
				fVar18 = 0.0f;
				do {
					uVar5 = 4;
					edDListBegin(0.0f, 0.0f, 0.0f, DISPLAY_LIST_DATA_TYPE_TRIANGLE_LIST, iVar7 * 2);
					x = (this->drawOffsets).x;
					fVar14 = 0.0f;
					iVar6 = iVar7;
					y = fVar11;
					fVar17 = fVar16;
					while (iVar6 != 0) {
						edDListTexCoo2f(fVar14, fVar18);
						edDListVertex4f(x, y_00, 0.0f, uVar5);
						edDListTexCoo2f(fVar14, fVar17);
						edDListVertex4f(x, y, 0.0f, uVar5);
						iVar6 = iVar6 + -1;
						if (iVar6 == 1) {
							fVar14 = 1.0f;
							x = (this->drawOffsets).z;
						}
						else {
							fVar14 = fVar14 + fVar10;
							x = x + 32.0f;
						}
					}
					iVar9 = iVar9 + -1;
					if (iVar9 == 1) {
						fVar16 = 1.0f;
						fVar11 = (this->drawOffsets).w;
					}
					else {
						fVar16 = fVar17 + fVar13;
						fVar11 = y + 32.0f;
					}

					edDListEnd();
					y_00 = y;
					fVar18 = fVar17;
				} while (iVar9 != 0);
			}
		}

		if (param_2 != 0) {
			fVar12 = fmodf(fVar12, 0.4f);
			uVar3 = gVideoConfig.screenWidth;
			if ((int)gVideoConfig.screenWidth < 0) {
				uVar3 = gVideoConfig.screenWidth + 1;
			}
			FStack192.Reset();
			pNewFont = edTextStyleSetCurrent(&FStack192);
			FStack192.SetShadow(0x100);
			FStack192.alpha = (uVar8 & 0x7f) << 1;
			FStack192.rgbaColour = FStack192.alpha | 0xffffff00;
			FStack192.SetFont(BootDataFont, false);
			FStack192.SetHorizontalAlignment(2);
			if (param_4 == false) {
				if (fVar12 < 0.3f) {
					FStack192.spaceSize = 10.0f;
					FStack192.SetVerticalAlignment(8);
					fVar10 = (float)((int)uVar3 >> 1);
					fVar12 = (float)gVideoConfig.screenHeight;
					pcVar4 = gMessageManager.get_message(0x6001a010b110011);
					edTextDraw(fVar10, fVar12 - 36.0f, pcVar4);
				}
			}
			else {
				fVar12 = (float)gVideoConfig.screenHeight;
				fVar10 = (float)((int)uVar3 >> 1);
				pcVar4 = gMessageManager.get_message(0x52525f503700080c);
				edTextDraw(fVar10, fVar12 * 0.5f + 60.0f, pcVar4);
			}

			edTextStyleSetCurrent(pNewFont);

			param_2 = (uint)(this->field_0xcc != 0);
			if (param_2 != 0) {
				param_2 = (uint)(1.0f <= fVar15);
			}
		}

		GuiDList_EndCurrent();
	}

	return param_2 == 0;
}

void CSplashScreen::SetDrawLocation(float x, float y, float z, float w)
{
	(this->drawOffsets).x = x;
	(this->drawOffsets).y = y;
	(this->drawOffsets).z = z;
	(this->drawOffsets).w = w;

	return;
}

bool CSimpleMenuPause::disappear_draw()
{
	return 0.3f <= get_time_in_state();
}

void CSimpleMenuPause::set_mode(EPauseMenu mode)
{
	reset();

	this->currentPage = mode;

	this->colorB = 0xfc9900ff;
	this->colorA = 0xffffd2ff;
	this->field_0xd4 = 0xe9e6e7ff;

	return;
}

void HelpEnter()
{
	IMPLEMENTATION_GUARD();
}

void HelpLeave()
{
	IMPLEMENTATION_GUARD();
}

void MapEnter()
{
	IMPLEMENTATION_GUARD();
}

void MapLeave()
{
	IMPLEMENTATION_GUARD();
}
