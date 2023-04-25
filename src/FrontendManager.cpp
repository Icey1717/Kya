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

struct BankAccessCombined {
	edCBankBuffer* pAccessObj;
	edCBank bankData;
};

BankAccessCombined* FUN_001db780(BankAccessCombined* param_1)
{
	return param_1;
}

void LoadUIBnk_001db720(BankAccessCombined* param_1, int param_2, int size)
{
	BankFilePathContainer BStack32;

	memset(&BStack32, 0, sizeof(BankFilePathContainer));
	edCBank_Setup(&param_1->bankData, size, param_2, &BStack32);
	return;
}

bool LoadBNKCombined(BankAccessCombined* param_1, char* path, char* fileName, TypePairData* pTypePairData)
{
	bool bVar1;
	edCBankBuffer* peVar2;
	BankFilePathContainer local_a0;
	char filePath[128];

	if (pTypePairData != (TypePairData*)0x0) {
		edCBank_SetDeserializeData(&param_1->bankData, pTypePairData);
	}
	memset(&local_a0, 0, sizeof(BankFilePathContainer));
	/* +.bnk */
	FormatFilePath(filePath, path, fileName, ".bnk", 0);
	local_a0.filePath = filePath;
	peVar2 = edCBank_GetBankBuffer(&param_1->bankData);
	param_1->pAccessObj = peVar2;
	bVar1 = edCBankBuffer_file_access(param_1->pAccessObj, &local_a0);
	return bVar1;
}

char* GetFileFromBank(BankAccessCombined* bankHeaderPtr, char* fileName, BankFileData* param_3)
{
	bool bVar1;
	int fileIndex;
	char* fileBuffer;

	fileIndex = GetIndexForFileName(bankHeaderPtr->pAccessObj, fileName);
	if (fileIndex == -1) {
		/* \r\nFile: %s\r\n */
		PrintString("\r\nFile: %s\r\n", fileName);
		fileBuffer = (char*)0x0;
	}
	else {
		fileBuffer = (char*)0x0;
		if (param_3 == (BankFileData*)0x0) {
			fileBuffer = GetFilePointerFromFileIndex(bankHeaderPtr->pAccessObj, fileIndex);
		}
		else {
			bVar1 = GetFileDataForIndex(bankHeaderPtr->pAccessObj, fileIndex, param_3, (char*)0x0);
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

	bVar1 = edCBankBuffer_close(param_1->pAccessObj);
	param_1->pAccessObj = (edCBankBuffer*)0x0;
	return bVar1;
}

void FreeBankCombined_001db710(BankAccessCombined* param_1)
{
	edCBank_Free_00244e10(&param_1->bankData);
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

G2DObj g_aFrontendIcons_0048fe70[13];

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

StaticMeshMaster* g_FrontendStaticMeshMaster_00448818 = NULL;

FrontendManager::FrontendManager()
{
	this->pCamera = (CameraObj_28*)0x0;
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

void FrontendManager::OnBeginGame()
{
	Manager_170* pMVar1;
	char* soundFileBuffer;
	edVideo_Globals* peVar2;
	FrameBuffer* pVidModeDataA;
	FrameBuffer* pVidModeDataB;
	CameraObj_28* pCVar3;
	char* pFileBuffer;
	undefined8 uVar4;
	char** fileToLoad;
	G2DObj* pTexture;
	int iVar5;
	char** wolfenFileName;
	char* puVar6;
	BankAccessCombined someBankObject;
	CameraObjParams local_8;

	puVar6 = this->field_0x78;
	FUN_001db780(&someBankObject);
	LoadUIBnk_001db720(&someBankObject, 1, 0x6000);
	/* Load the frontend IOP Bnk */
	uVar4 = 0;
	LoadBNKCombined(&someBankObject, "CDEURO/Frontend/", "frntdiop", NULL);
	iVar5 = 0;
	fileToLoad = g_FrontendSoundFiles_0040ec90;
	do {
		soundFileBuffer = GetFileFromBank(&someBankObject, *fileToLoad, (BankFileData*)0x0);
		//TakesMenuSoundFileBuffer(soundFileBuffer, (StaticSoundObj_18*)(puVar6 + 4), 0);
		iVar5 = iVar5 + 1;
		fileToLoad = fileToLoad + 1;
		puVar6 = puVar6 + 0x18;
	} while (iVar5 < 5);
	CloseBankCombined_001db640(&someBankObject);
	FreeBankCombined_001db710(&someBankObject);
	peVar2 = GetVideoParams_002ba3e0();
	local_8.field_0x2 = 0;
	local_8.field_0x0 = 0;
	local_8.screenWidth = (peVar2->pActiveVidParams->params26).screenWidth;
	local_8.screenHeight = (peVar2->pActiveVidParams->params26).screenHeight;
	pVidModeDataA = GetFrameBuffer_001ba9c0();
	pVidModeDataB = GetFrameBuffer_001ba9d0();
	pCVar3 = AllocateCameraObj28_002bae70(&local_8, pVidModeDataA, pVidModeDataB, 2);
	this->pCamera = pCVar3;
	SetCameraClear_002bb960(this->pCamera, 0, 0, 0);
	g_FrontendStaticMeshMaster_00448818 = ed3D::FindFreeStaticMeshMaster_002b4600(&g_CameraObj_0044ae10, this->pCamera, 1);

	sceVu0UnitMatrix((TO_SCE_MTX)&this->field_0x10);
	pMVar1 = g_ManagerSingletonArray_00451660.g_Manager170_00451674;
	iVar5 = 0;
	wolfenFileName = g_aFrontendIconFileNames_00425940;
	pTexture = g_aFrontendIcons_0048fe70;
	do {
		//pFileBuffer = OpenOrLoadSomeFile(pMVar1, *wolfenFileName, 0);
		//LoadTextIcon(pTexture, pFileBuffer);
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
