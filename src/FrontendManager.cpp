#include "FrontendManager.h"
#include "edC/edCBankBuffer.h"
#include "edC/edCBank.h"
#include "edVideo/VideoB.h"
#include "edVideo/VideoD.h"
#include "PauseManager.h"
#include "edVideo/Viewport.h"

#include <string.h>
#include "edVideo/VideoA.h"
#include "ed3D.h"
#include "CameraViewManager.h"
#include "edStr.h"
#include "BootData.h"
#include "MathOps.h"

struct Bank {
	Bank() {}
	edCBankBufferEntry* pAccessObj;
	edCBankBuffer bankData;
	void Init(int param_2, int size);
	bool BankLoad(char* path, char* fileName, TypePairData* pTypePairData);
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

bool Bank::BankLoad(char* path, char* fileName, TypePairData* pTypePairData)
{
	bool bVar1;
	edCBankBufferEntry* peVar2;
	edCBankInstall local_a0;
	char filePath[128];

	if (pTypePairData != (TypePairData*)0x0) {
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
		/* \r\nFile: %s\r\n */
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

namespace Frontend {
	ed_3D_Scene* _scene_handle = NULL;
}

CFrontendMagicGauge gMagicGauge;

FrontendManager::FrontendManager()
{
	this->pViewport = (ed_viewport*)0x0;
	//FUN_001dcc10((long)&this->field_0x54);
	this->bHideHUD = 0;
	//this->pHealthBar = (undefined*)&PTR_DAT_00451710;
	this->pMagicOrbs = &gMagicGauge;
	//this->pNooties = &HNootiesWolfun_004528b0;
	//this->pFreedWolfun = &HNootiesWolfun_00452ae0;
	//this->pMenuObj_0x68 = &MenuObj_164_00452d10;
	//this->pMenuObj_0x6c = &MenuObj_Big_00452ea0;
	//this->pMenuObj_0x74 = (undefined*)&PTR_DAT_004544c0;
	//this->pMenuObj_0x70 = (undefined*)&PTR_DAT_00454b80;
	//this->field_0x78 = (char*)&FLOAT_00456220;
}

void FrontendManager::Game_Init()
{
	Manager_170* pMVar1;
	char* soundFileBuffer;
	edVideo_Globals* peVar2;
	edSurface* pVidModeDataA;
	edSurface* pVidModeDataB;
	ed_viewport* pCVar3;
	char* pFileBuffer;
	undefined8 uVar4;
	char** fileToLoad;
	Sprite* pTexture;
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
		//edSoundSampleLoad(soundFileBuffer, (ed_sound_sample*)(puVar6 + 4), 0);
		iVar5 = iVar5 + 1;
		fileToLoad = fileToLoad + 1;
		puVar6 = puVar6 + 0x18;
	} while (iVar5 < 5);
	frontendBank.BankUnload();
	frontendBank.Term();

	peVar2 = edVideoGetInfo();
	local_8.posY = 0;
	local_8.posX = 0;
	local_8.screenWidth = (peVar2->pActiveVidParams->params26).screenWidth;
	local_8.screenHeight = (peVar2->pActiveVidParams->params26).screenHeight;
	pVidModeDataA = VideoGetDisplaybuffer();
	pVidModeDataB = VideoGetZbuffer();
	pCVar3 = edViewportNew(&local_8, pVidModeDataA, pVidModeDataB, 2);
	this->pViewport = pCVar3;
	edViewportSetBackgroundColor(this->pViewport, 0, 0, 0);
	Frontend::_scene_handle = ed3DSceneCreate(&CCameraManager::_gFrontEndCamera, this->pViewport, 1);

	edF32Matrix4SetIdentityHard(&this->field_0x10);
	pMVar1 = CScene::ptable.g_Manager170_00451674;
	iVar5 = 0;
	wolfenFileName = MenuElementsBitmapNames;
	pTexture = MenuBitmaps;
	do {
		//pFileBuffer = OpenOrLoadSomeFile(pMVar1, *wolfenFileName, 0);
		//Sprite::Install(pTexture, pFileBuffer);
		iVar5 = iVar5 + 1;
		wolfenFileName = wolfenFileName + 1;
		pTexture = pTexture + 1;
	} while (iVar5 < 0xd);
	//(**(code**)(*(int*)this->pHealthBar + 0x20))();
	//(*(code*)(this->pMagicOrbs->pVTable->pVTable).pVTable.field_0x20)();
	//(*(code*)this->pNooties->pVTable->field_0x20)();
	//(*(code*)this->pFreedWolfun->pVTable->field_0x20)();
	//(*(code*)this->pMenuObj_0x68->field_0x0->field_0x20)();
	//(**(code**)(*(int*)this->pMenuObj_0x70 + 8))();
	//(*(code*)this->pMenuObj_0x6c->pVTable->field_0x20)();
	//(**(code**)(*(int*)this->pMenuObj_0x74 + 0x20))();
	if (this->bHideHUD == 0) {
		this->bHideHUD = 1;
		this->pMagicOrbs->Magic_SetDisplay(false);
	}
	return;
}

void FrontendManager::SetActive(bool bActive)
{
	if ((uint)this->bHideHUD == (bActive & 0xffU)) {
		this->bHideHUD = bActive == 0;
		this->pMagicOrbs->Magic_SetDisplay(bActive);
	}
	return;
}

void CFrontendMagicGauge::Magic_SetDisplay(unsigned char bNewVisible)
{
	if (bNewVisible != this->bVisible) {
		this->bVisible = bNewVisible;
	}
	return;
}
