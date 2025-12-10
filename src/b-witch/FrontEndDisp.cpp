#include "FrontendDisp.h"
#include "edBank/edBankBuffer.h"
#include "edVideo/VideoB.h"
#include "edVideo/VideoD.h"
#include "Pause.h"
#include "edVideo/Viewport.h"

#include <string.h>
#include "edVideo/VideoA.h"
#include "ed3D.h"
#include "CameraViewManager.h"
#include "edStr.h"
#include "BootData.h"
#include "MathOps.h"
#include "FrontEndBank.h"
#include "FrontEndLife.h"
#include "FrontEndMagic.h"
#include "TimeController.h"
#include "LevelScheduler.h"
#include "FrontEndMoney.h"
#include "FrontEndEnemy.h"
#include "FrontEndInventory.h"

struct Bank {
	Bank() {}
	edCBankBufferEntry* pAccessObj;
	edCBankBuffer bankData;
	void Init(int param_2, int size);
	bool BankLoad(char* path, char* fileName, edCBankCallback* pTypePairData);
	char* GetResource(char* fileName, edBANK_ENTRY_INFO* param_3);
	bool BankUnload();
	void Term();
};

void Bank::Init(int param_2, int size)
{
	edCBankInstall BStack32;

	memset(&BStack32, 0, sizeof(edCBankInstall));
	this->bankData.initialize(size, param_2, &BStack32);
	return;
}

bool Bank::BankLoad(char* path, char* fileName, edCBankCallback* pTypePairData)
{
	bool bVar1;
	edCBankBufferEntry* peVar2;
	edCBankInstall local_a0;
	char filePath[128];

	if (pTypePairData != (edCBankCallback*)0x0) {
		this->bankData.bank_buffer_setcb(pTypePairData);
	}

	memset(&local_a0, 0, sizeof(edCBankInstall));
	/* +.bnk */
	edStrCatMulti(filePath, path, fileName, ".bnk", 0);
	local_a0.filePath = filePath;
	peVar2 = this->bankData.get_free_entry();
	this->pAccessObj = peVar2;
	bVar1 = this->pAccessObj->load(&local_a0);
	return bVar1;
}

char* Bank::GetResource(char* fileName, edBANK_ENTRY_INFO* param_3)
{
	bool bVar1;
	int fileIndex;
	char* fileBuffer;

	fileIndex = this->pAccessObj->get_index(fileName);
	if (fileIndex == -1) {
		edDebugPrintf("\r\nFile: %s\r\n", fileName);
		fileBuffer = (char*)0x0;
	}
	else {
		fileBuffer = (char*)0x0;
		if (param_3 == (edBANK_ENTRY_INFO*)0x0) {
			fileBuffer = this->pAccessObj->get_element(fileIndex);
		}
		else {
			bVar1 = this->pAccessObj->get_info(fileIndex, param_3, (char*)0x0);
			if (bVar1 != false) {
				fileBuffer = param_3->fileBufferStart;
			}
		}
	}
	return fileBuffer;
}

bool Bank::BankUnload()
{
	bool bVar1;

	bVar1 = this->pAccessObj->close();
	this->pAccessObj = (edCBankBufferEntry*)0x0;
	return bVar1;
}

void Bank::Term()
{
	this->bankData.terminate();
	return;
}

char* g_FrontendSoundFiles_0040ec90[5] =
{
	"valid.vag",
	"changeSelection.vag",
	"changeValue.vag",
	"cancel.vag",
	"takeMoney.vag",
};

CFrontendLifeGauge gFrontendLifeGauge;
CFrontendMagicGauge gMagicGauge;
CFrontendAction gFrontendAction;
CFrontendMoney gFrontendMoney;
CFrontendEnemyCount gFrontendEnemyCount;
CFrontendEnemyList gFrontendEnemyList;
CFrontendInventory gFrontendInventory;

CFrontendDisplay::CFrontendDisplay()
{
	this->bHideHUD = 0;
	this->pHealthBar = &gFrontendLifeGauge;
	this->pMagicOrbs = &gMagicGauge;
	this->pMoney = &gFrontendMoney;
	this->pFreedWolfun = &gFrontendEnemyCount;
	this->pFrontendAction = &gFrontendAction;
	this->pInventory = &gFrontendInventory;
	//this->pMenuObj_0x74 = (undefined*)&PTR_DAT_004544c0;
	this->pFrontendEnemyList = &gFrontendEnemyList;
	//this->field_0x78 = (char*)&FLOAT_00456220;
}

void CFrontendDisplay::Game_Init()
{
	CFrontendBank* pMVar1;
	char* soundFileBuffer;
	edVideo_Globals* peVar2;
	edSurface* pVidModeDataA;
	edSurface* pVidModeDataB;
	ed_viewport* pCVar3;
	char* pFileBuffer;
	undefined8 uVar4;
	char** fileToLoad;
	CSprite* pTexture;
	int iVar5;
	char** wolfenFileName;
	char* puVar6;
	Bank frontendBank;
	CameraObjParams local_8;

	puVar6 = this->field_0x78;
	frontendBank.Init(1, 0x6000);
	/* Init the frontend IOP Bnk */
	uVar4 = 0;
	frontendBank.BankLoad("CDEURO/Frontend/", "frntdiop", NULL);
	iVar5 = 0;
	fileToLoad = g_FrontendSoundFiles_0040ec90;
	do {
		soundFileBuffer = frontendBank.GetResource(*fileToLoad, (edBANK_ENTRY_INFO*)0x0);
		IMPLEMENTATION_GUARD_AUDIO(
			edSoundSampleLoad(soundFileBuffer, (ed_sound_sample*)(puVar6 + 4), 0);)
			iVar5 = iVar5 + 1;
		fileToLoad = fileToLoad + 1;
		puVar6 = puVar6 + 0x18;
	} while (iVar5 < 5);
	frontendBank.BankUnload();
	frontendBank.Term();

	peVar2 = edVideoGetInfo();
	local_8.posY = 0;
	local_8.posX = 0;
	local_8.screenWidth = peVar2->pActiveVidParams->screenWidth;
	local_8.screenHeight = peVar2->pActiveVidParams->screenHeight;
	pVidModeDataA = VideoGetDisplaybuffer();
	pVidModeDataB = VideoGetZbuffer();
	pCVar3 = edViewportNew(&local_8, pVidModeDataA, pVidModeDataB, 2);
	this->pViewport = pCVar3;
	edViewportSetBackgroundColor(this->pViewport, 0, 0, 0);
	CFrontend::_scene_handle = ed3DSceneCreate(&CCameraManager::_gFrontEndCamera, this->pViewport, 1);

	edF32Matrix4SetIdentityHard(&this->field_0x10);
	iVar5 = 0;
	wolfenFileName = MenuElementsBitmapNames;
	pTexture = MenuBitmaps;
	do {
		NAME_NEXT_OBJECT(*wolfenFileName);
		pTexture->Install(CScene::ptable.g_FrontEndBank_00451674->GetResource(*wolfenFileName, (edBANK_ENTRY_INFO*)0x0));
		iVar5 = iVar5 + 1;
		wolfenFileName = wolfenFileName + 1;
		pTexture = pTexture + 1;
	} while (iVar5 < 0xd);

	this->pHealthBar->Init();
	this->pMagicOrbs->Init();
	this->pMoney->Init();
	this->pFreedWolfun->Init();
	this->pFrontendAction->Init();
	this->pFrontendEnemyList->Init();
	this->pInventory->Init();
	//(**(code**)(*(int*)this->pMenuObj_0x74 + 0x20))();

	if (this->bHideHUD == 0) {
		this->bHideHUD = 1;
		this->pMagicOrbs->Magic_SetDisplay(false);
	}
	return;
}

void CFrontendDisplay::Game_Term()
{
	return;
}

int Global_00448814;

void CFrontendDisplay::Level_Init()
{
	if (this->bHideHUD == 1) {
		this->bHideHUD = 0;
		this->pMagicOrbs->Magic_SetDisplay(true);
	}

	if (Global_00448814 != 0) {
		this->pMagicOrbs->ShowMagic();
		this->pHealthBar->ShowLife();
		this->pMoney->ShowMoney();
		this->pFreedWolfun->ShowEnemy();
		Global_00448814 = 0;
	}

	this->pInventory->Level_Init();
	//FUN_003c9370((long)(int)this->pMenuObj_0x74);

	DeclareInterface(FRONTEND_INTERFACE_FREED_WOLFEN, &this->interfaceEnemyCount);

	ed3DSceneComputeCameraToScreenMatrix(CFrontend::_scene_handle, &this->field_0x10);

	return;
}

void CFrontendDisplay::Level_Term()
{
	DeclareInterface(FRONTEND_INTERFACE_FREED_WOLFEN, (CInterface*)0x0);

	if (this->bHideHUD == 0) {
		this->bHideHUD = 1;
		this->pMagicOrbs->Magic_SetDisplay(false);
	}

	this->pInventory->Level_Term();
	this->pFrontendEnemyList->Level_Term();
	//FUN_003c9330((int)this->pMenuObj_0x74);

	return;
}

void CFrontendDisplay::Level_Manage()
{
	float fVar2;

	if (this->bHideHUD == 0) {
		fVar2 = Timer::GetTimer()->totalTime;
		this->pMagicOrbs->Update(fVar2);
		this->pMoney->Update(fVar2);
		this->pFreedWolfun->Update(fVar2);
		this->pHealthBar->Update(fVar2);
		this->pFrontendAction->Update(fVar2);
		this->pFrontendEnemyList->Update(fVar2);
		this->pInventory->Update(fVar2);
		//(**(code**)(*(int*)this->pMenuObj_0x74 + 0x38))(fVar2);
	}

	return;
}

// Should be in: D:/Projects/b-witch/FrontendDisp.h
void CFrontendDisplay::Level_ManagePaused()
{
	this->Level_Manage();

	return;
}

void CFrontendDisplay::Level_Draw()
{
	if (this->bHideHUD == 0) {
		//(**(code**)(*(int*)this->pMenuObj_0x74 + 0x3c))();
		this->pHealthBar->Draw();
		this->pMagicOrbs->Draw();
		this->pMoney->Draw();
		this->pFreedWolfun->Draw();
		this->pFrontendAction->Draw();
		this->pFrontendEnemyList->Draw();
		this->pInventory->Draw();
	}

	return;
}

void CFrontendDisplay::Level_Reset()
{
	this->pHealthBar->Reset();
	this->pMagicOrbs->Reset();
	this->pMoney->Reset();
	this->pFreedWolfun->Reset();
	this->pFrontendAction->Reset();
	this->pFrontendEnemyList->Reset();
	this->pInventory->Reset();
	//(**(code**)(*(int*)this->pMenuObj_0x74 + 0x18))();

	return;
}

void CFrontendDisplay::Level_CheckpointReset()
{
	this->pHealthBar->Reset();
	this->pHealthBar->ShowLife();

	this->pMagicOrbs->Reset();
	this->pMagicOrbs->ShowMagic();
	this->pMoney->Reset();
	this->pMoney->ShowMoney();
	this->pFreedWolfun->Reset();
	this->pFreedWolfun->ShowEnemy();
	this->pFrontendAction->Reset();
	this->pFrontendEnemyList->Reset();
	this->pInventory->CheckpointReset();
	//(**(code**)(*(int*)this->pMenuObj_0x74 + 0x1c))();

	return;
}

void CFrontendDisplay::Level_PauseChange(bool bPaused)
{
	if (bPaused == 0) {
		this->pHealthBar->HideLifeAlways();
		this->pMagicOrbs->HideMagicAlways(false);
		this->pMoney->HideMoneyAlways();
	}
	else {
		this->pHealthBar->ShowLifeAlways();
		this->pMagicOrbs->ShowMagicAlways();
		this->pMoney->ShowMoneyAlways();
	}
	return;
}

char* CFrontendDisplay::ProfileGetName()
{
	return "Frntend";
}

void CFrontendDisplay::Level_LoadContext()
{
	if (CLevelScheduler::gThis->bShouldLoad != 0) {
		this->pMagicOrbs->ShowMagic();
		this->pHealthBar->ShowLife();
		this->pMoney->ShowMoney();
		this->pFreedWolfun->ShowEnemy();
	}

	return;
}

void CFrontendDisplay::DeclareInterface(FRONTEND_INTERFACE interfaceType, CInterface* pInterface)
{
	switch (interfaceType) {
	case FRONTEND_INTERFACE_LIFE:
		this->pHealthBar->SetInterface(pInterface);
		break;
	case FRONTEND_INTERFACE_ENEMY_LIST:
		this->pFrontendEnemyList->AddFighterInterface(pInterface);
		break;
	case FRONTEND_INTERFACE_MAGIC:
		this->pMagicOrbs->SetInterface(pInterface);
		break;
	case FRONTEND_INTERFACE_MONEY:
		this->pMoney->SetInterface(pInterface);
		break;
	case FRONTEND_INTERFACE_ACTION:
		this->pFrontendAction->SetInterface(pInterface);
		break;
	case FRONTEND_INTERFACE_FREED_WOLFEN:
		this->pFreedWolfun->SetInterface(pInterface);
	default:
		//IMPLEMENTATION_GUARD();
		break;
	}

	return;
}

void CFrontendDisplay::SetHeroActionA(int actionId)
{
	this->pFrontendAction->SetActionA(actionId);
}

void CFrontendDisplay::SetHeroActionB(int actionId)
{
	this->pFrontendAction->SetActionB(actionId);
}

void CFrontendDisplay::SetGlobalFunc_001cf8e0(void)
{
	Global_00448814 = 1;
	return;
}

void CFrontendDisplay::SetActive(bool bActive)
{
	if ((uint)this->bHideHUD == (bActive & 0xffU)) {
		this->bHideHUD = bActive == 0;
		this->pMagicOrbs->Magic_SetDisplay(bActive);
	}
	return;
}

bool CInterfaceEnemyCount::Manage()
{
	return CScene::_pinstance->IsFadeTermActive() == false;
}

float CInterfaceEnemyCount::GetValue()
{
	return CLevelScheduler::ScenVar_Get(SCN_GAME_NUM_FREED_WOLFENS);
}