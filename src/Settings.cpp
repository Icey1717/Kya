#include "Settings.h"
#include "TranslatedTextData.h"
#include "CameraViewManager.h"
#include "kya.h"
#include "edVideo/VideoA.h"

CSettings gSettings = {};

int CSettings::GetMaxSaveBufferSize()
{
	return 0x100;
}

bool CSettings::LoadFromBuffer(void* pDataV)
{
	char* pData = (char*)pDataV;
	bool bVar1;

	bVar1 = false;
	if (*(int*)pData == 0x53544753) {
		bVar1 = false;
		if (*(int*)(pData + 4) == 3) {
			this->languageID = (LANGUAGE)*(int*)(pData + 8);
			this->field_0x14 = *(undefined4*)(pData + 0xc);
			this->field_0x18 = *(uint*)(pData + 0x10);
			this->field_0x1c = *(uint*)(pData + 0x14);
			this->setOffsetX = *(int*)(pData + 0x1c);
			this->setOffsetY = *(int*)(pData + 0x20);
			if (pData[0x24] == '\0') {
				this->field_0x0 = false;
			}
			else {
				this->field_0x0 = true;
			}
			if (pData[0x25] == '\0') {
				this->field_0x8 = false;
			}
			else {
				this->field_0x8 = true;
			}
			if (pData[0x27] == '\0') {
				this->bWidescreen = 0;
			}
			else {
				this->bWidescreen = 1;
			}
			bVar1 = true;
		}
	}
	return bVar1;
}

void CSettings::SetSettingsToGlobal()
{
	int iVar1;
	uint uVar2;
	CameraManager* pCVar3;
	//GlobalSound_00451698* pGVar4;
	LANGUAGE prevLanguageID;
	LocalizationManager* pManager;
	int iVar5;
	long lVar6;
	float fVar7;
	CinematicManager* pCVar2;
	LANGUAGE currentLanguageID;

	//pGVar4 = Scene::ptable.g_GlobalSoundPtr_00451698;
	pCVar3 = Scene::ptable.g_CameraManager_0045167c;
	//pCVar2 = g_CinematicManager_0048efc;
	currentLanguageID = this->languageID;
	lVar6 = (long)(int)Scene::ptable.g_GlobalSoundPtr_00451698;
	prevLanguageID = CMessageFile::get_default_language();
	if (currentLanguageID != prevLanguageID) {
		IMPLEMENTATION_GUARD(
		CMessageFile::set_default_language(currentLanguageID);
		pManager = GetLocalizationManager();
		ApplyLanguage_0032f8e0(pManager);)
	}
	//if (this->field_0x8 != (bool)*(byte*)&pCVar2->bInitialized) {
	//	pCVar2->bInitialized = (uint)this->field_0x8;
	//}
	if (this->bWidescreen == 0) {
		if (pCVar3->aspectRatio != 1.333333f) {
			pCVar3->aspectRatio = 1.333333f;
		}
	}
	else {
		if (pCVar3->aspectRatio != 1.777778f) {
			pCVar3->aspectRatio = 1.777778f;
		}
	}
	iVar1 = this->setOffsetX;
	if ((gVideoConfig.offsetX != iVar1) || (gVideoConfig.offsetY != this->setOffsetY)) {
		gVideoConfig.offsetY = this->setOffsetY;
		gVideoConfig.offsetX = iVar1;
		edVideoSetOffset((short)iVar1, (short)gVideoConfig.offsetY);
	}
	uVar2 = this->field_0x18;
	if ((int)uVar2 < 0) {
		fVar7 = (float)(uVar2 >> 1 | uVar2 & 1);
		fVar7 = fVar7 + fVar7;
	}
	else {
		fVar7 = (float)uVar2;
	}
	//if (fVar7 / 12.0 != pGVar4->field_0xbc) {
	//	FUN_00182000(fVar7 / 12.0, (int)pGVar4);
	//}
	uVar2 = this->field_0x1c;
	if ((int)uVar2 < 0) {
		fVar7 = (float)(uVar2 >> 1 | uVar2 & 1);
		fVar7 = fVar7 + fVar7;
	}
	else {
		fVar7 = (float)uVar2;
	}
	//if (fVar7 / 12.0 != pGVar4->field_0xc0) {
	//	FUN_00181f80(fVar7 / 12.0, pGVar4);
	//}
	iVar1 = this->field_0x14;
	//iVar5 = thunk_FUN_00284930();
	//if ((long)iVar1 != (long)iVar5) {
	//	FUN_00181ef0(lVar6, (long)iVar1);
	//}
	//if ((ulong)this->field_0x0 != (long)g_InputManager_00450960.field_0x14) {
	//	FUN_001b6d90((int)&g_InputManager_00450960, (ulong)this->field_0x0);
	//}
	return;
}