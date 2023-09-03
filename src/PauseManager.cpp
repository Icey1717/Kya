#include "PauseManager.h"
#include "TimeController.h"
#include "CinematicManager.h"
#include "LevelScheduleManager.h"
#include "Rendering/DisplayList.h"
#include "Rendering/Font.h"
#include "edText.h"
#include "TranslatedTextData.h"
#include "Iop.h"
#include "InputManager.h"
#include <assert.h>
#include "LargeObject.h"
#include "LocalizationManager.h"
#include "edDlist.h"
#include "CameraViewManager.h"
#include "MathOps.h"
#include <math.h>

#ifdef PLATFORM_WIN
#include "renderer.h"
#endif
#include "Rendering/edCTextStyle.h"
#include "Rendering/edCTextFormat.h"
#include "kya.h"

SimpleMenu g_PauseScreenData_004507f0;

PauseManager::PauseManager()
{
	pSimpleMenu = (SimpleMenu*)0x0;
	pSplashScreen = (SplashScreen*)0x0;
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

void PauseManager::Game_Init()
{
	g_PauseScreenData_004507f0.SetFontValue_002f2d10(BootDataFont);
	g_PauseScreenData_004507f0.SetFontValue_002f2d00(BootDataFont);
	g_PauseScreenData_004507f0.SetFontValue_002f2cf0(BootDataFont);
	g_PauseScreenData_004507f0.SetTranslatedTextData_002f2d20(NULL);
	field_0xf = 0;
	field_0x10 = 0;
	field_0x11 = 0;
	//FUN_001b5d20(&PTR_PTR_00450900);
	return;
}

edCTextFont* BootDataFont = NULL;

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
		bIsLoadingScreen = LevelScheduleManager::gThis->nextLevelID != 0xf;
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
			//RunInPlaceDestructors_002173e0(drawTextParams.fontData_0x850, Free_00166e40, 0xc0, 0x11);
		}
	}
	return;
}


void PauseManager::LevelLoading_Draw()
{
	//PauseManagerFunc_001b0860(pPauseManager, 0);
	DrawLoadingScreen_001b05e0();
	return;
}

struct APlayer;
extern APlayer* g_PlayerActor_00448e10;

uint UINT_00448eac;

PauseStaticObj g_PauseStaticObj_0049c9d0 = { 0 };

int INT_00483804 = 0;

int FUN_00284930(void)
{
	return INT_00483804;
}

void StaticPauseObjActivate_003c8bb0(PauseStaticObj* pPauseStaticObj)
{
	CinematicManager* pCVar1;
	CameraManager* pCVar2;
	GlobalSound_00451698* pGVar3;
	undefined4 uVar4;
	float fVar5;

	pGVar3 = Scene::ptable.g_GlobalSoundPtr_00451698;
	pCVar2 = Scene::ptable.g_CameraManager_0045167c;
	pCVar1 = g_CinematicManager_0048efc;
	//fVar5 = (Scene::ptable.g_GlobalSoundPtr_00451698)->field_0xbc * 12.0;
	//if (fVar5 < 2.147484e+09) {
	//	pPauseStaticObj->field_0x18 = (int)fVar5;
	//}
	//else {
	//	pPauseStaticObj->field_0x18 = (int)(fVar5 - 2.147484e+09) | 0x80000000;
	//}
	//fVar5 = pGVar3->field_0xc0 * 12.0;
	//if (fVar5 < 2.147484e+09) {
	//	pPauseStaticObj->field_0x1c = (int)fVar5;
	//}
	//else {
	//	pPauseStaticObj->field_0x1c = (int)(fVar5 - 2.147484e+09) | 0x80000000;
	//}
	uVar4 = FUN_00284930();
	pPauseStaticObj->field_0x14 = uVar4;
	pPauseStaticObj->setOffsetX = gVideoConfig.offsetX;
	pPauseStaticObj->setOffsetY = gVideoConfig.offsetY;
	if (pCVar2->aspectRatio == 1.777778f) {
		pPauseStaticObj->field_0x9 = 1;
	}
	else {
		pPauseStaticObj->field_0x9 = 0;
	}
	pPauseStaticObj->field_0x8 = pCVar1->bInitialized != 0;
	uVar4 = CMessageFile::get_default_language();
	pPauseStaticObj->languageID = uVar4;
	pPauseStaticObj->field_0x0 = g_InputManager_00450960.field_0x14 != 0;
	return;
}

void PauseManager::Level_Init()
{
	APlayer* pAVar1;
	SimpleMenu* pPVar2;
	SplashScreen* pGVar3;
	Timer* pTVar4;
	undefined8 uVar5;

	this->currentLevelID_0x18 = LevelScheduleManager::gThis->currentLevelID;
	this->field_0x1c = 0;
	this->field_0x20 = 0;
	pAVar1 = g_PlayerActor_00448e10;
	if (g_PlayerActor_00448e10 != (APlayer*)0x0) {
		IMPLEMENTATION_GUARD();
		if (this->field_0xe == 0) {
			//g_PlayerActor_00448e10->field_0xaa0 = 0;
		}
		else {
			//g_PlayerActor_00448e10->field_0xaa0 = 1;
		}
		if (this->field_0xf == 0) {
			//pAVar1->field_0xaa4 = 0;
		}
		else {
			//pAVar1->field_0xaa4 = 1;
		}
	}
	UINT_00448eac = 0;
	if ((GameFlags & 0x40) != 0) {
		pPVar2 = new SimpleMenu();
		if (pPVar2 != (SimpleMenu*)0x0) {
			this->pSimpleMenu = pPVar2;
			this->pSimpleMenu->Reset();
			this->pSimpleMenu->SetFontValue_002f2d10(BootDataFont);
			this->pSimpleMenu->SetFontValue_002f2d00(BootDataFont);
			this->pSimpleMenu->SetFontValue_002f2cf0(BootDataFont);
			this->pSimpleMenu->SetTranslatedTextData_002f2d20(&(Scene::ptable.g_LocalizationManager_00451678)->userInterfaceText);
			pGVar3 = new SplashScreen();
			this->pSplashScreen = pGVar3;
			/* CDEURO/Frontend/kyatitle.g2d */
			this->pSplashScreen->Init(0.0f, "CDEURO/Frontend/kyatitle.g2d");
			this->pSplashScreen->SetDrawLocation((float)gVideoConfig.screenWidth / 2.0f - 80.0f, ((float)gVideoConfig.screenHeight * 20.0f) / 512.0f,
				(float)gVideoConfig.screenWidth / 2.0f + 80.0f, ((float)gVideoConfig.screenHeight * 220.0f) / 512.0f);
			pTVar4 = GetTimer();
			this->totalPlayTime = pTVar4->totalPlayTime;
		}
	}
	this->field_0x34 = 1;
	StaticPauseObjActivate_003c8bb0(&g_PauseStaticObj_0049c9d0);
	this->field_0x24 = 0;
	return;
}

void PauseManager::Level_Draw()
{
	CinematicManager* pCinematicManager;
	LevelScheduleManager* pLevelScheduleManager;
	bool bVar1;
	bool bVar2;
	Timer* pTVar3;
	EPauseMenu EVar4;
	int iVar5;
	CCinematic* pCVar6;
	ulong uVar7;
	int index;
	CCinematic* pCVar8;
	float fVar9;

	if ((this->pSimpleMenu != (SimpleMenu*)0x0) && (this->pSplashScreen != (SplashScreen*)0x0)) {
		bVar1 = gCompatibilityHandlingPtr->GetAnyControllerConnected();
		pTVar3 = GetTimer();
		fVar9 = pTVar3->totalPlayTime;
		if ((g_InputManager_00450960.pressedBitfield != 0) ||
			(((this->field_0x34 == 0 &&
				(EVar4 = this->pSimpleMenu->get_current_page(), EVar4 != PM_MainMenu)) ||
				(bVar2 = Scene::_pinstance->CheckFunc_001b9300(), bVar2 != 0)))) {
			this->totalPlayTime = fVar9;
		}
		//pCinematicManager = g_CinematicManager_0048efc;
		//pCVar8 = (CinematicObjectB*)0x0;
		//iVar5 = GetNumCutscenes_001c50b0(g_CinematicManager_0048efc);
		//index = 0;
		//if (0 < iVar5) {
		//	do {
		//		pCVar6 = GetCutsceneData_001c50c0(pCinematicManager, index);
		//		if ((pCVar6->intOrPtrField != 0) && ((pCVar6->flags_0x4 & 8) != 0)) {
		//			pCVar8 = pCVar6;
		//		}
		//		index = index + 1;
		//	} while (index < iVar5);
		//}
		pLevelScheduleManager = LevelScheduleManager::gThis;
		if (60.0f < fVar9 - this->totalPlayTime) {
			this->pSplashScreen->Manage((long)this->field_0x34, false, bVar1);
			//if (pCVar8 == (CinematicObjectB*)0x0) {
			//	LevelScheduleManager::FUN_002db9d0(pLevelScheduleManager, 0, 0xe, 0xffffffff, 0xffffffff, 0xffffffff, 1);
			//	if ((g_DebugCameraFlag_00448ea4 & 4) != 0) {
			//		FUN_001b51e0();
			//	}
			//}
			//else {
			//	if (pCVar8->totalCutsceneDelta < 0.5) {
			//		IMPLEMENTATION_GUARD();
			//		//LevelScheduleManager::FUN_002db9d0(pLevelScheduleManager, 0, 0xe, 0xffffffff, 0xffffffff, 0xffffffff, 1);
			//		//g_DebugCameraFlag_00448ea4 = g_DebugCameraFlag_00448ea4 | 2;
			//	}
			//}
		}
		else {
			if ((GameFlags & 0x80) == 0) {
				if ((long)this->field_0x34 == 0) {
					bVar2 = GuiDList_BeginCurrent();
					if (bVar2 != false) {
						IMPLEMENTATION_GUARD();
						//this->pSubObj108_0x28->draw(DrawGameMenu);
						GuiDList_EndCurrent();
					}
					this->pSimpleMenu->perform_action();
					EVar4 = this->pSimpleMenu->get_current_page();
					if ((EVar4 == PM_MainMenu) && (bVar1 == false)) {
						this->pSplashScreen->Manage((long)this->field_0x34, false, false);
					}
				}
				else {
					bVar1 = this->pSplashScreen->Manage((long)this->field_0x34, false, bVar1);
					this->field_0x34 = (int)bVar1;
					if (this->field_0x34 == 0) {
						g_PauseScreenData_004507f0.SetMode(PM_MainMenu);
						UINT_00448eac = 2;
					}
				}
			}
		}
	}
	if ((GameFlags & 4) != 0) {
		if ((UINT_00448eac == 1) || (UINT_00448eac == 2)) {
			if ((GameFlags & 0x800) == 0) {
				IMPLEMENTATION_GUARD();
				//FUN_001b48c0();
			}
			bVar1 = GuiDList_BeginCurrent();
			if (bVar1 != false) {
				IMPLEMENTATION_GUARD();
				//gPauseMenu->draw(DrawPauseMenu);
				GuiDList_EndCurrent();
			}
		}
		//gPauseMenu->perform_action();
	}
	//PauseManagerFunc_001b0860(this, 0);
	return;
}

SimpleMenu::SimpleMenu()
{
	Reset();
}

void SimpleMenu::Reset()
{
	Timer* pTVar1;

	this->field_0x2c = 0;
	this->currentPage = PM_MainMenu;
	this->selectedIndex = 0;
	this->field_0x50 = 3;
	this->screenState_0x1c = 0;
	pTVar1 = GetTimer();
	this->totalTime = pTVar1->totalTime;
	this->field_0xcc = 0xfff609ff;
	this->field_0xd0 = 0xff9000ff;
	this->field_0xd4 = 0xffffffff;
	this->field_0xd8 = 0x14;
	this->field_0xf0 = 1.0;
	this->field_0xf4 = 0;
	this->field_0xf8 = 0;
	this->field_0xfc = 0;
	this->field_0x100 = 0;
	this->field_0x104 = 0;
	return;
}

void SimpleMenu::SetMode(EPauseMenu mode)
{
	Reset();
	this->currentPage = mode;
	this->field_0xcc = 0xfc9900ff;
	this->field_0xd0 = 0xffffd2ff;
	this->field_0xd4 = 0xe9e6e7ff;
	return;
}

void SimpleMenu::SetFontValue_002f2cf0(edCTextFont* pFont)
{
	pFontC = pFont;
	return;
}

void SimpleMenu::SetFontValue_002f2d00(edCTextFont* pFont)
{
	pFontB = pFont;
	return;
}

void SimpleMenu::SetFontValue_002f2d10(edCTextFont* pFont)
{
	pFontA = pFont;
	return;
}

void SimpleMenu::SetTranslatedTextData_002f2d20(CMessageFile* pTextData)
{
	pTranslatedTextData = pTextData;
	return;
}

EPauseMenu SimpleMenu::get_current_page()
{
	return currentPage;
}

void SimpleMenu::perform_action()
{
	if (this->pFunc_0x40 != 0x0) {
		IMPLEMENTATION_GUARD();
		//(*(code*)param_1->pFunc_0x40)(param_1->slotID_0x44);
		this->pFunc_0x40 = 0;
		this->slotID_0x44 = 0;
	}
	return;
}


float FLOAT_ARRAY_00448cd0[2] = { 1.0f, 1.0f };

Sprite::Sprite()
{
	undefined4* puVar1;
	int puVar4;
	undefined4* puVar3;
	float local_10;
	float fStack12;
	undefined4 local_8;
	undefined4 local_4;

	//puVar4 = 8;
	//puVar3 = &local_8;
	//puVar1 = puVar3;
	//while (puVar1 != (undefined4*)0x0) {
	//	*(undefined*)puVar3 = 0;
	//	puVar3 = (undefined4*)((int)puVar3 + 1);
	//	puVar4 = puVar4 + -1;
	//	puVar1 = (undefined4*)puVar4;
	//}
	this->field_0x10 = 0;
	this->field_0x14 = -0.5f;
	this->field_0x18 = (float)(uint)this->field_0x38;
	this->field_0x1c = (float)(uint)this->field_0x3a;
	this->field_0x20 = 0;
	this->field_0x24 = 0;
	this->field_0x28 = 1.0;
	this->field_0x2c = 1.0;
	this->pMaterialInfo = (edDList_material*)0x0;
	this->field_0x4c = local_8;
	this->field_0x50 = local_4;
	local_10 = FLOAT_ARRAY_00448cd0[0];
	this->field_0x54 = local_10;
	fStack12 = FLOAT_ARRAY_00448cd0[1];
	this->field_0x58 = fStack12;
	this->field_0x30[0] = 0x80;
	this->field_0x30[1] = 0x80;
	this->field_0x30[2] = 0x80;
	this->field_0x30[3] = 0x80;
	this->field_0x4 = 0;
	this->field_0x8 = 0;
	this->field_0x34 = 0;
	return;
}

void Sprite::Install(char* pFileBuffer)
{
	ed_g2d_material* pMAT_Internal;
	ushort* puVar1;
	int iStack4;

	ed3DInstallG2D(pFileBuffer, *(int*)(pFileBuffer + 8), &iStack4, &this->textureInfo, 0);
	this->flags_0x7c = this->flags_0x7c | 2;
	this->field_0x4 = 0;
	pMAT_Internal = ed3DG2DGetG2DMaterialFromIndex(&this->textureInfo, 0);
	puVar1 = (ushort*)ed3DG2DGetBitmapFromMaterial(pMAT_Internal, 0);
	edDListCreatMaterialFromIndex(&this->materialInfo, 0, &this->textureInfo, 2);
	this->flags_0x7c = this->flags_0x7c | 1;
	this->field_0x38 = *puVar1;
	this->field_0x3a = puVar1[1];
	this->pMaterialInfo = &this->materialInfo;
	this->field_0x18 = (float)(uint)this->field_0x38;
	this->field_0x1c = (float)(uint)this->field_0x3a;
	return;
}

void Sprite::DrawXYXY(uint param_2, float param_3, float param_4, float param_5, float param_6, float param_7)
{
	uint uVar1;
	byte b;
	byte bVar2;
	int iVar3;
	byte r;
	byte g;
	float fVar4;
	float fVar5;
	float y;
	float x;
	float y_00;
	float local_20;
	float local_1c;
	float local_18;
	float local_14;
	float local_10;
	float local_c;
	float local_8;
	float local_4;

	if (param_4 == param_6) {
		param_6 = param_4 + (float)(uint)this->field_0x38;
	}
	if (param_5 == param_7) {
		param_7 = param_5 + (float)(uint)this->field_0x3a;
	}
	edDListUseMaterial(&this->materialInfo);
	edDListLoadIdentity();
	local_c = this->field_0x58;
	local_4 = this->field_0x50;
	if ((this->field_0x34 & 0x2000) != 0) {
		local_c = this->field_0x50;
		local_4 = this->field_0x58;
	}
	local_10 = this->field_0x54;
	local_8 = this->field_0x4c;
	if ((param_2 & 0x1000) != 0) {
		local_10 = this->field_0x4c;
		local_8 = this->field_0x54;
	}
	local_18 = local_10;
	local_14 = local_4;
	local_20 = local_8;
	local_1c = local_c;
	if ((param_2 & 0xc000) != 0) {
		IMPLEMENTATION_GUARD(
		iVar3 = 2;
		if ((param_2 & 0x8000) == 0) {
			iVar3 = 0;
		}
		iVar3 = (uint)((param_2 & 0x4000) != 0) + iVar3;
		FUN_0038c6f0(&local_8, &local_8, iVar3);
		FUN_0038c6f0(&local_10, &local_10, iVar3);
		FUN_0038c6f0(&local_18, &local_18, iVar3);
		FUN_0038c6f0(&local_20, &local_20, iVar3);)
	}
	if ((param_2 & 0x3c0) != 0) {
		fVar4 = param_3 * 3.0;
		x = param_4 + fVar4;
		y_00 = param_5 + fVar4;
		fVar5 = param_6 - fVar4;
		y = param_7 - fVar4;
		if ((param_2 & 0x40) != 0) {
			x = x - fVar4 * 2.0;
		}
		if ((param_2 & 0x80) != 0) {
			fVar5 = fVar5 + fVar4 * 2.0;
		}
		if ((param_2 & 0x100) != 0) {
			y_00 = y_00 - fVar4 * 2.0;
		}
		if ((param_2 & 0x200) != 0) {
			y = y + fVar4 * 2.0;
		}
		edDListBlendSet(1);
		edDListBlendFunc50();
		edDListColor4u8(0, 0, 0, 0x80);
		iVar3 = 4;
		edDListBegin(1.0, 1.0, 1.0, 4, 4);
		edDListTexCoo2f(local_8, local_4);
		edDListVertex4f(x, y_00, 0.0, iVar3);
		edDListTexCoo2f(local_18, local_14);
		edDListVertex4f(fVar5, y_00, 0.0, iVar3);
		edDListTexCoo2f(local_20, local_1c);
		edDListVertex4f(x, y, 0.0, iVar3);
		edDListTexCoo2f(local_10, local_c);
		edDListVertex4f(fVar5, y, 0.0, iVar3);
		edDListEnd();
	}
	if ((param_2 & 0x800) == 0) {
		edDListBlendSet(1);
	}
	else {
		edDListBlendSet(0);
	}
	edDListBlendFuncNormal();
	if ((param_2 & 0x400) == 0) {
		edDListColor4u8(this->field_0x30[0], this->field_0x30[1], this->field_0x30[2], this->field_0x30[3]);
	}
	else {
		fVar4 = (float)(uint)this->field_0x30[0] * param_3;
		if (fVar4 < 2.147484e+09) {
			r = (byte)(int)fVar4;
			bVar2 = this->field_0x30[1];
		}
		else {
			r = (byte)(int)(fVar4 - 2.147484e+09);
			bVar2 = this->field_0x30[1];
		}
		fVar4 = (float)(uint)bVar2 * param_3;
		if (fVar4 < 2.147484e+09) {
			g = (byte)(int)fVar4;
			bVar2 = this->field_0x30[2];
		}
		else {
			g = (byte)(int)(fVar4 - 2.147484e+09);
			bVar2 = this->field_0x30[2];
		}
		fVar4 = (float)(uint)bVar2 * param_3;
		if (fVar4 < 2.147484e+09) {
			b = (byte)(int)fVar4;
			bVar2 = this->field_0x30[3];
		}
		else {
			b = (byte)(int)(fVar4 - 2.147484e+09);
			bVar2 = this->field_0x30[3];
		}
		edDListColor4u8(r, g, b, bVar2);
	}
	uVar1 = param_2 & 0x30000;
	if (uVar1 == 0) {
		iVar3 = 4;
		edDListBegin(1.0, 1.0, 1.0, 4, 4);
		edDListTexCoo2f(local_8, local_4);
		edDListVertex4f(param_4, param_5, 0.0, iVar3);
		edDListTexCoo2f(local_18, local_14);
		edDListVertex4f(param_6, param_5, 0.0, iVar3);
		edDListTexCoo2f(local_20, local_1c);
		edDListVertex4f(param_4, param_7, 0.0, iVar3);
		edDListTexCoo2f(local_10, local_c);
		edDListVertex4f(param_6, param_7, 0.0, iVar3);
		edDListEnd();
	}
	else {
		if (uVar1 == 0x20000) {
			iVar3 = 4;
			edDListBegin(1.0, 1.0, 1.0, 4, 8);
			edDListTexCoo2f(local_8, local_4);
			edDListVertex4f(param_4, param_5, 0.0, iVar3);
			edDListTexCoo2f(local_18, local_14);
			edDListVertex4f(param_6, param_5, 0.0, iVar3);
			edDListTexCoo2f(local_20, local_1c);
			fVar4 = (param_5 + param_7) / 2.0;
			edDListVertex4f(param_4, fVar4, 0.0, iVar3);
			edDListTexCoo2f(local_10, local_c);
			edDListVertex4f(param_6, fVar4, 0.0, iVar3);
			edDListTexCoo2f(local_8, local_4);
			edDListVertex4f(param_4, param_7, 0.0, iVar3);
			edDListTexCoo2f(local_18, local_14);
			edDListVertex4f(param_6, param_7, 0.0, iVar3);
			edDListEnd();
		}
		else {
			if (uVar1 == 0x10000) {
				iVar3 = 4;
				edDListBegin(1.0, 1.0, 1.0, 4, 6);
				edDListTexCoo2f(local_8, local_4);
				edDListVertex4f(param_4, param_5, 0.0, iVar3);
				edDListTexCoo2f(local_20, local_1c);
				edDListVertex4f(param_4, param_7, 0.0, iVar3);
				edDListTexCoo2f(local_18, local_14);
				fVar4 = (param_4 + param_6) / 2.0;
				edDListVertex4f(fVar4, param_5, 0.0, iVar3);
				edDListTexCoo2f(local_10, local_c);
				edDListVertex4f(fVar4, param_7, 0.0, iVar3);
				edDListTexCoo2f(local_8, local_4);
				edDListVertex4f(param_6, param_5, 0.0, iVar3);
				edDListTexCoo2f(local_20, local_1c);
				edDListVertex4f(param_6, param_7, 0.0, iVar3);
				edDListEnd();
			}
			else {
				iVar3 = 4;
				edDListBegin(1.0, 1.0, 1.0, 4, 0xb);
				edDListTexCoo2f(local_8, local_4);
				edDListVertex4f(param_4, param_5, 0.0, iVar3);
				edDListTexCoo2f(local_20, local_1c);
				fVar4 = (param_5 + param_7) / 2.0;
				edDListVertex4f(param_4, fVar4, 0.0, iVar3);
				edDListTexCoo2f(local_18, local_14);
				fVar5 = (param_4 + param_6) / 2.0;
				edDListVertex4f(fVar5, param_5, 0.0, iVar3);
				edDListTexCoo2f(local_10, local_c);
				edDListVertex4f(fVar5, fVar4, 0.0, iVar3);
				edDListTexCoo2f(local_8, local_4);
				edDListVertex4f(param_6, param_5, 0.0, iVar3);
				edDListTexCoo2f(local_20, local_1c);
				edDListVertex4f(param_6, fVar4, 0.0, iVar3);
				edDListTexCoo2f(local_8, local_4);
				edDListVertex4f(param_6, param_7, 0.0, iVar3);
				edDListTexCoo2f(local_10, local_c);
				edDListVertex4f(fVar5, fVar4, 0.0, iVar3);
				edDListTexCoo2f(local_18, local_14);
				edDListVertex4f(fVar5, param_7, 0.0, iVar3);
				edDListTexCoo2f(local_20, local_1c);
				edDListVertex4f(param_4, fVar4, 0.0, iVar3);
				edDListTexCoo2f(local_8, local_4);
				edDListVertex4f(param_4, param_7, 0.0, iVar3);
				edDListEnd();
			}
		}
	}
	return;
}

SplashScreen::SplashScreen()
{
	//(this->base).pVTable = &G2DObj_VTable_004446e0;
	this->field_0x68 = 0;
	this->field_0x64 = 0;
	this->field_0x60 = 0;
	this->field_0x5c = 0;
	this->field_0x78 = 0.0;
	this->field_0x74 = 0.0;
	this->field_0x70 = (undefined*)0x0;
	this->field_0x6c = 0;
	this->flags_0x7c = 0;
	//(*((this->base).pVTable)->init)((Sprite*)this);
	this->pTextureFileData = (char*)0x0;
	this->field_0xcc = 0;
	this->field_0xc4 = 0.0;
	this->field_0xd0 = 0;
	return;
}

bool SplashScreen::Init(float param_1, char* filePath)
{
	uint uVar1;
	edFILEH* pLoadedFile;
	int iVar2;
	char* pcVar3;
	ushort* puVar4;
	Timer* pTVar5;
	ed_g2d_material* pMaterialSection;
	float fVar6;
	int iStack4;

	edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
	pLoadedFile = edFileOpen(filePath, 9);
	if (pLoadedFile != (edFILEH*)0x0) {
		iVar2 = GetFileSize_0025bd70(pLoadedFile);
		uVar1 = iVar2 + 0x7ffU & 0xfffff800;
		pcVar3 = (char*)edMemAllocAlign(TO_HEAP(H_MAIN), (long)(int)uVar1, 0x40);
		this->pTextureFileData = pcVar3;
		if (this->pTextureFileData != (char*)0x0) {
			SetRead_0025be80(pLoadedFile, this->pTextureFileData, uVar1);
			iVar2 = pLoadedFile->count_0x228;
			while (iVar2 != 0) {
				edFileNoWaitStackFlush();
				iVar2 = pLoadedFile->count_0x228;
			}
		}
		SetClose_0025bf60(pLoadedFile);
	}
	pcVar3 = this->pTextureFileData;
	if (pcVar3 != (char*)0x0) {
		ed3DInstallG2D(pcVar3, *(int*)(pcVar3 + 8), &iStack4, &this->textureInfo, 0);
		this->flags_0x7c = this->flags_0x7c | 2;
		this->field_0x4 = 0;
		pMaterialSection = ed3DG2DGetG2DMaterialFromIndex(&this->textureInfo, 0);
		puVar4 = (ushort*)ed3DG2DGetBitmapFromMaterial(pMaterialSection, 0);
		edDListCreatMaterialFromIndex(&this->materialInfo, 0, &this->textureInfo, 2);
		this->flags_0x7c = this->flags_0x7c | 1;
		this->field_0x38 = *puVar4;
		this->field_0x3a = puVar4[1];
		this->pMaterialInfo = &this->materialInfo;
		this->field_0x18 = (float)(uint)this->field_0x38;
		this->field_0x1c = (float)(uint)this->field_0x3a;
	}
	edMemClearFlags(TO_HEAP(H_MAIN), 0x100);
	this->field_0xc8 = param_1;
	if (param_1 <= 0.0f) {
		this->field_0xc8 = 0.0001f;
	}
	this->field_0xcc = 0;
	pTVar5 = GetTimer();
	this->field_0xc4 = pTVar5->totalTime;
	uVar1 = gVideoConfig.screenWidth;
	fVar6 = (float)gVideoConfig.screenHeight;
	(this->drawOffsets).x = 0.0f;
	(this->drawOffsets).y = 0.0f;
	(this->drawOffsets).z = (float)uVar1;
	(this->drawOffsets).w = fVar6;
	return this->pTextureFileData != (char*)0x0;
}

bool SplashScreen::Manage(ulong param_2, bool param_3, bool param_4)
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
	if (((param_2 != 0) && (((uint)g_InputManager_00450960.releasedBitfield & 0x40000) != 0)) && (this->field_0xcc == 0))
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
				edDListBegin(1.0f, 1.0f, 1.0f, 6, 2);
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
			iVar7 = (int)(fVar13 / 32.0) + 1;
			iVar9 = (int)(fVar10 / 32.0);
			fVar11 = y_00 + 32.0;
			fVar10 = 1.0 / (float)(int)(fVar13 / 32.0);
			fVar13 = 1.0 / (float)iVar9;
			fVar16 = fVar13 + 0.0;
			if (iVar9 != 0) {
				fVar18 = 0.0;
				do {
					uVar5 = 4;
					edDListBegin(0.0f, 0.0f, 0.0f, 4, iVar7 * 2);
					x = (this->drawOffsets).x;
					fVar14 = 0.0;
					iVar6 = iVar7;
					y = fVar11;
					fVar17 = fVar16;
					while (iVar6 != 0) {
						edDListTexCoo2f(fVar14, fVar18);
						edDListVertex4f(x, y_00, 0.0f, (int)uVar5);
						edDListTexCoo2f(fVar14, fVar17);
						edDListVertex4f(x, y, 0.0f, (int)uVar5);
						iVar6 = iVar6 + -1;
						if (iVar6 == 1) {
							fVar14 = 1.0;
							x = (this->drawOffsets).z;
						}
						else {
							fVar14 = fVar14 + fVar10;
							x = x + 32.0;
						}
					}
					iVar9 = iVar9 + -1;
					if (iVar9 == 1) {
						fVar16 = 1.0;
						fVar11 = (this->drawOffsets).w;
					}
					else {
						fVar16 = fVar17 + fVar13;
						fVar11 = y + 32.0;
					}
					edDListEnd();
					y_00 = y;
					fVar18 = fVar17;
				} while (iVar9 != 0);
#ifdef PLATFORM_WIN
				Renderer::Draw();
#endif
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
			param_2 = (ulong)(this->field_0xcc != 0);
			if (param_2 != 0) {
				param_2 = (ulong)(1.0f <= fVar15);
			}
		}
		GuiDList_EndCurrent();
	}
	return param_2 == 0;
}

void SplashScreen::SetDrawLocation(float x, float y, float z, float w)
{
	(this->drawOffsets).x = x;
	(this->drawOffsets).y = y;
	(this->drawOffsets).z = z;
	(this->drawOffsets).w = w;
	return;
}