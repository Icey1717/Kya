#include "PauseManager.h"
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

CSimpleMenu g_PauseScreenData_004507f0;

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

void CPauseManager::Game_Init()
{
	g_PauseScreenData_004507f0.set_font_title(BootDataFont);
	g_PauseScreenData_004507f0.set_font_message(BootDataFont);
	g_PauseScreenData_004507f0.set_font_help(BootDataFont);
	g_PauseScreenData_004507f0.set_message_manager(NULL);
	field_0xf = 0;
	field_0x10 = 0;
	field_0x11 = 0;
	//FUN_001b5d20(&PTR_PTR_00450900);
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

uint UINT_00448eac;

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
	IMPLEMENTATION_GUARD_LOG(
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

void CPauseManager::Level_Draw()
{
	CCinematicManager* pCinematicManager;
	CLevelScheduler* pLevelScheduleManager;
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

	if ((this->pSimpleMenu != (CSimpleMenu*)0x0) && (this->pSplashScreen != (CSplashScreen*)0x0)) {
		bVar1 = gCompatibilityHandlingPtr->HandleDisconnectedDevices(0);

		pTVar3 = GetTimer();
		fVar9 = pTVar3->totalPlayTime;
		if ((gPlayerInput.pressedBitfield != 0) ||
			(((this->field_0x34 == 0 &&
				(EVar4 = this->pSimpleMenu->get_current_page(), EVar4 != PM_MainMenu)) ||
				(bVar2 = CScene::_pinstance->IsFadeTermActive(), bVar2 != 0)))) {
			this->totalPlayTime = fVar9;
		}
		pCinematicManager = g_CinematicManager_0048efc;
		pCVar8 = (CCinematic*)0x0;
		iVar5 = g_CinematicManager_0048efc->GetNumCutscenes_001c50b0();
		index = 0;
		if (0 < iVar5) {
			do {
				pCVar6 = pCinematicManager->GetCinematic(index);
				if ((pCVar6->state != CS_Stopped) && ((pCVar6->flags_0x4 & 8) != 0)) {
					pCVar8 = pCVar6;
				}
				index = index + 1;
			} while (index < iVar5);
		}
		pLevelScheduleManager = CLevelScheduler::gThis;
		if (60.0f < fVar9 - this->totalPlayTime) {
			this->pSplashScreen->Manage((long)this->field_0x34, false, bVar1);
			if (pCVar8 == (CCinematic*)0x0) {
				IMPLEMENTATION_GUARD(
				CLevelScheduler::FUN_002db9d0(pLevelScheduleManager, 0, 0xe, 0xffffffff, 0xffffffff, 0xffffffff, 1);
				if ((g_DebugCameraFlag_00448ea4 & 4) != 0) {
					FUN_001b51e0();
				})
			}
			else {
				if (pCVar8->totalCutsceneDelta < 0.5) {
					IMPLEMENTATION_GUARD();
					//LevelScheduleManager::FUN_002db9d0(pLevelScheduleManager, 0, 0xe, 0xffffffff, 0xffffffff, 0xffffffff, 1);
					//g_DebugCameraFlag_00448ea4 = g_DebugCameraFlag_00448ea4 | 2;
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
					EVar4 = this->pSimpleMenu->get_current_page();
					if ((EVar4 == PM_MainMenu) && (bVar1 == false)) {
						this->pSplashScreen->Manage((long)this->field_0x34, false, false);
					}
				}
				else {
					bVar1 = this->pSplashScreen->Manage(this->field_0x34, false, bVar1);
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

void CSimpleMenu::SetMode(EPauseMenu mode)
{
	reset();
	this->currentPage = mode;
	this->colorA = 0xfc9900ff;
	this->colorB = 0xffffd2ff;
	this->field_0xd4 = 0xe9e6e7ff;
	return;
}

void CSimpleMenu::set_vertical_position(int verticalPos)
{
	this->verticalPos = verticalPos;
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
		edDListBegin(0.0f, 0.0f, 0.0f, 4, 4);
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

	this->verticalPos = this->verticalPos + this->field_0x3c;
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
		IMPLEMENTATION_GUARD();
		//(*(code*)param_1->pFunc_0x40)(param_1->slotID_0x44);
		this->pFunc_0x40 = 0;
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
	this->field_0x3c = 0x28;
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
	bool bVar1;
	int iVar2;
	char* pcVar3;
	EPauseMenu EVar4;
	uint uVar5;

	bVar1 = gCompatibilityHandlingPtr->HandleDisconnectedDevices(0);
	if (bVar1 == false) {
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
								IMPLEMENTATION_GUARD_LOG(
								CheatCodeSequence::Update_00407760(&gToggleDebugLevelCheatCode_004a5700);
								CheatCodeSequence::Update_00407760(&gShowGalleryCheatCode_004a5760);
								CheatCodeSequence::Update_00407760(&gRefillLifeCheatCode_004a5740);)
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
	char cVar2;
	uint uVar3;
	Timer* pTVar4;
	long lVar5;

	this->field_0x20 = 0;
	this->counter_0x24 = 0;
	this->field_0x3c = 0x28;
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
	uVar3 = get_action();
	this->lastInput_0x14 = uVar3;
	cVar2 = pInputFunc(this, this->lastInput_0x14);
	if ((iVar1 == this->field_0x2c) && (cVar2 != '\0')) {
		update_page();
	}
	iVar1 = this->screenState_0x1c;
	if (iVar1 == 4) {
		this->screenState_0x1c = 0;
		pTVar4 = Timer::GetTimer();
		this->totalTime = pTVar4->totalTime;
	}
	else {
		if (iVar1 != 3) {
			if (iVar1 == 2) {
				lVar5 = disappear_draw();
				if (lVar5 != 0) {
					this->screenState_0x1c = 3;
					pTVar4 = Timer::GetTimer();
					this->totalTime = pTVar4->totalTime;
				}
			}
			else {
				if (iVar1 == 1) {
					lVar5 = select_draw();
					if (lVar5 != 0) {
						this->screenState_0x1c = 2;
						pTVar4 = Timer::GetTimer();
						this->totalTime = pTVar4->totalTime;
					}
				}
				else {
					if ((iVar1 == 0) && (lVar5 = appear_draw(), lVar5 != 0)) {
						this->screenState_0x1c = 1;
						pTVar4 = Timer::GetTimer();
						this->totalTime = pTVar4->totalTime;
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

float FLOAT_ARRAY_00448cd0[2] = { 1.0f, 1.0f };

CSprite::CSprite()
{
	this->field_0x68 = 0;
	this->field_0x64 = 0;
	this->field_0x60 = 0;
	this->field_0x5c = 0;
	this->field_0x78 = 0.0f;
	this->field_0x74 = 0.0f;
	this->field_0x70 = (undefined*)0x0;
	this->field_0x6c = 0;
	this->flags_0x7c = 0;

	ClearLocalData();

	return;
}

void CSprite::ClearLocalData()
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
	this->fWidth = (float)(uint)this->iWidth;
	this->fHeight = (float)(uint)this->iHeight;
	this->field_0x20 = 0;
	this->field_0x24 = 0;
	this->field_0x28 = 1.0f;
	this->field_0x2c = 1.0f;
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

void CSprite::Install(char* pFileBuffer)
{
	ed_g2d_material* pMaterial;
	ed_g2d_bitmap* pBitmap;
	int iStack4;

	ed3DInstallG2D(pFileBuffer, *(int*)(pFileBuffer + 8), &iStack4, &this->textureManager, 0);

	this->flags_0x7c = this->flags_0x7c | 2;
	this->field_0x4 = 0;

	pMaterial = ed3DG2DGetG2DMaterialFromIndex(&this->textureManager, 0);
	pBitmap = ed3DG2DGetBitmapFromMaterial(pMaterial, 0);

	edDListCreatMaterialFromIndex(&this->materialInfo, 0, &this->textureManager, 2);

	this->flags_0x7c = this->flags_0x7c | 1;
	this->iWidth = pBitmap->width;
	this->iHeight = pBitmap->height;
	this->pMaterialInfo = &this->materialInfo;
	this->fWidth = (float)(uint)this->iWidth;
	this->fHeight = (float)(uint)this->iHeight;

	return;
}

void CSprite::DrawXYXY(uint param_2, float param_3, float param_4, float param_5, float param_6, float param_7)
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
		param_6 = param_4 + (float)(uint)this->iWidth;
	}

	if (param_5 == param_7) {
		param_7 = param_5 + (float)(uint)this->iHeight;
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
		edDListBegin(1.0f, 1.0f, 1.0f, 4, 4);
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
		if (fVar4 < 2.147484e+09f) {
			r = (byte)(int)fVar4;
			bVar2 = this->field_0x30[1];
		}
		else {
			r = (byte)(int)(fVar4 - 2.147484e+09f);
			bVar2 = this->field_0x30[1];
		}
		fVar4 = (float)(uint)bVar2 * param_3;
		if (fVar4 < 2.147484e+09f) {
			g = (byte)(int)fVar4;
			bVar2 = this->field_0x30[2];
		}
		else {
			g = (byte)(int)(fVar4 - 2.147484e+09f);
			bVar2 = this->field_0x30[2];
		}
		fVar4 = (float)(uint)bVar2 * param_3;
		if (fVar4 < 2.147484e+09f) {
			b = (byte)(int)fVar4;
			bVar2 = this->field_0x30[3];
		}
		else {
			b = (byte)(int)(fVar4 - 2.147484e+09f);
			bVar2 = this->field_0x30[3];
		}
		edDListColor4u8(r, g, b, bVar2);
	}

	uVar1 = param_2 & 0x30000;
	if (uVar1 == 0) {
		iVar3 = 4;
		edDListBegin(1.0f, 1.0f, 1.0f, 4, 4);
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
			edDListBegin(1.0f, 1.0f, 1.0f, 4, 8);
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
				edDListBegin(1.0f, 1.0f, 1.0f, 4, 6);
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
				edDListBegin(1.0f, 1.0f, 1.0f, 4, 0xb);
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
		this->field_0x4 = 0;

		pMaterialSection = ed3DG2DGetG2DMaterialFromIndex(&this->textureManager, 0);
		pBitmap = ed3DG2DGetBitmapFromMaterial(pMaterialSection, 0);
		edDListCreatMaterialFromIndex(&this->materialInfo, 0, &this->textureManager, 2);

		this->flags_0x7c = this->flags_0x7c | 1;

		this->iWidth = pBitmap->width;
		this->iHeight = pBitmap->height;

		this->pMaterialInfo = &this->materialInfo;
		this->fWidth = (float)(uint)this->iWidth;
		this->fHeight = (float)(uint)this->iHeight;
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

bool CSplashScreen::Manage(ulong param_2, bool param_3, bool param_4)
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
	if (((param_2 != 0) && ((gPlayerInput.releasedBitfield & KEY_START) != 0)) && (this->field_0xcc == 0))
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
			iVar7 = (int)(fVar13 / 32.0f) + 1;
			iVar9 = (int)(fVar10 / 32.0f);
			fVar11 = y_00 + 32.0f;
			fVar10 = 1.0f / (float)(int)(fVar13 / 32.0);
			fVar13 = 1.0f / (float)iVar9;
			fVar16 = fVar13 + 0.0f;
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

			param_2 = (ulong)(this->field_0xcc != 0);
			if (param_2 != 0) {
				param_2 = (ulong)(1.0f <= fVar15);
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