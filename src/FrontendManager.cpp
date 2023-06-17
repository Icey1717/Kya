#include "FrontendManager.h"
#include "edC/edCBankBuffer.h"
#include "edC/edCBank.h"
#include "edVideo/VideoB.h"
#include "edVideo/VideoD.h"
#include "PauseManager.h"
#include "Camera.h"

#include <string.h>
#include "edVideo/VideoA.h"
#include "ed3D.h"
#include "CameraViewManager.h"
#include "edStr.h"

struct BankAccessCombined {
	edCBankBufferEntry* pAccessObj;
	edCBankBuffer bankData;
};

BankAccessCombined* FUN_001db780(BankAccessCombined* param_1)
{
	return param_1;
}

void LoadUIBnk_001db720(BankAccessCombined* param_1, int param_2, int size)
{
	edCBankInstall BStack32;

	memset(&BStack32, 0, sizeof(edCBankInstall));
	param_1->bankData.initialize(size, param_2, &BStack32);
	return;
}

bool LoadBNKCombined(BankAccessCombined* param_1, char* path, char* fileName, TypePairData* pTypePairData)
{
	bool bVar1;
	edCBankBufferEntry* peVar2;
	edCBankInstall local_a0;
	char filePath[128];

	if (pTypePairData != (TypePairData*)0x0) {
		param_1->bankData.bank_buffer_setcb(pTypePairData);
	}
	memset(&local_a0, 0, sizeof(edCBankInstall));
	/* +.bnk */
	edStrCatMulti(filePath, path, fileName, ".bnk", 0);
	local_a0.filePath = filePath;
	peVar2 = param_1->bankData.get_free_entry();
	param_1->pAccessObj = peVar2;
	bVar1 = param_1->pAccessObj->load(&local_a0);
	return bVar1;
}

char* GetFileFromBank(BankAccessCombined* bankHeaderPtr, char* fileName, edBANK_ENTRY_INFO* param_3)
{
	bool bVar1;
	int fileIndex;
	char* fileBuffer;

	fileIndex = get_index(bankHeaderPtr->pAccessObj, fileName);
	if (fileIndex == -1) {
		/* \r\nFile: %s\r\n */
		edDebugPrintf("\r\nFile: %s\r\n", fileName);
		fileBuffer = (char*)0x0;
	}
	else {
		fileBuffer = (char*)0x0;
		if (param_3 == (edBANK_ENTRY_INFO*)0x0) {
			fileBuffer = get_element(bankHeaderPtr->pAccessObj, fileIndex);
		}
		else {
			bVar1 = get_info(bankHeaderPtr->pAccessObj, fileIndex, param_3, (char*)0x0);
			if (bVar1 != false) {
				fileBuffer = param_3->fileBufferStart;
			}
		}
	}
	return fileBuffer;
}

bool CloseBankCombined_001db640(BankAccessCombined* param_1)
{
	bool bVar1;

	bVar1 = param_1->pAccessObj->close();
	param_1->pAccessObj = (edCBankBufferEntry*)0x0;
	return bVar1;
}

void FreeBankCombined_001db710(BankAccessCombined* param_1)
{
	param_1->bankData.terminate();
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

Sprite g_aFrontendIcons_0048fe70[13];

char* g_aFrontendIconFileNames_00425940[13] = {
	"i_wolfen.g2d",
	"i_native.g2d",
	"i_wolfen_exorcised.g2d",
	"inv_case_01_p0.g2d",
	"inv_select_01_p0.g2d",
	"sound_slider_cursor.g2d",
	"selector_on.g2d",
	"selector_off.g2d",
	"support.g2d",
	"arrow.g2d",
	"arrow_pressed.g2d",
	"backlayer.g2d",
	"tatoo.g2d"
};

namespace Frontend {
	ed_3D_Scene* _scene_handle = NULL;
}

FrontendManager::FrontendManager()
{
	this->pCamera = (ed_viewport*)0x0;
	//FUN_001dcc10((long)&this->field_0x54);
	this->bHideHUD = 0;
	//this->pHealthBar = (undefined*)&PTR_DAT_00451710;
	//this->pMagicOrbs = &HMagicOrbs::Singleton_00452320;
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
	BankAccessCombined someBankObject;
	CameraObjParams local_8;

	puVar6 = this->field_0x78;
	FUN_001db780(&someBankObject);
	LoadUIBnk_001db720(&someBankObject, 1, 0x6000);
	/* Init the frontend IOP Bnk */
	uVar4 = 0;
	LoadBNKCombined(&someBankObject, "CDEURO/Frontend/", "frntdiop", NULL);
	iVar5 = 0;
	fileToLoad = g_FrontendSoundFiles_0040ec90;
	do {
		soundFileBuffer = GetFileFromBank(&someBankObject, *fileToLoad, (edBANK_ENTRY_INFO*)0x0);
		//TakesMenuSoundFileBuffer(soundFileBuffer, (StaticSoundObj_18*)(puVar6 + 4), 0);
		iVar5 = iVar5 + 1;
		fileToLoad = fileToLoad + 1;
		puVar6 = puVar6 + 0x18;
	} while (iVar5 < 5);
	CloseBankCombined_001db640(&someBankObject);
	FreeBankCombined_001db710(&someBankObject);
	peVar2 = GetVideoParams_002ba3e0();
	local_8.posY = 0;
	local_8.posX = 0;
	local_8.screenWidth = (peVar2->pActiveVidParams->params26).screenWidth;
	local_8.screenHeight = (peVar2->pActiveVidParams->params26).screenHeight;
	pVidModeDataA = GetFrameBuffer_001ba9c0();
	pVidModeDataB = GetFrameBuffer_001ba9d0();
	pCVar3 = edViewportNew(&local_8, pVidModeDataA, pVidModeDataB, 2);
	this->pCamera = pCVar3;
	edViewportSetBackgroundColor(this->pCamera, 0, 0, 0);
	Frontend::_scene_handle = ed3DSceneCreate(&g_CameraObj_0044ae10, this->pCamera, 1);

	sceVu0UnitMatrix((TO_SCE_MTX)&this->field_0x10);
	pMVar1 = g_ManagerSingletonArray_00451660.g_Manager170_00451674;
	iVar5 = 0;
	wolfenFileName = g_aFrontendIconFileNames_00425940;
	pTexture = g_aFrontendIcons_0048fe70;
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
		//MenuObjBaseData::SetVisible_001d8080((MenuObjBase*)this->pMagicOrbs, '\0');
	}
	return;
}
