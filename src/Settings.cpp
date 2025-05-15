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
	bool bValidSettings;

	bValidSettings = false;
	char stgs[4] = { 'S', 'T', 'G', 'S' };
	if (memcmp(pData, stgs, 4)) {
		bValidSettings = false;
		if (*(int*)(pData + 4) == 3) { //Apparently this value just needs to be 3?
			this->languageID = (LANGUAGE)*(int*)(pData + 0x08);
			this->audioMode = (AUDIO_MODE)*(pData + 0x0c);
			this->musicVolume = *(uint*)(pData + 0x10);
			this->sfxVolume = *(uint*)(pData + 0x14);
			this->setOffsetX = *(int*)(pData + 0x1c);
			this->setOffsetY = *(int*)(pData + 0x20);
			this->bEnableVibration = (bool)pData[0x24];
			this->bEnableSubtitles = (bool)pData[0x25];
			//0x26 is skipped, perhaps it was used in prototype builds
			this->bWidescreen = (bool)pData[0x27];
			bValidSettings = true;
		}
	}
	return bValidSettings;
}

void CSettings::SetSettingsToGlobal()
{
	int iVar1;
	uint volume;
	CCameraManager* pCVar3;
	//CAudioManager* pGVar4;
	LANGUAGE prevLanguageID;
	CLanguageManager* pManager;
	int iVar5;
	long lVar6;
	float fVar7;
	CCinematicManager* pCVar2;
	LANGUAGE currentLanguageID;

	//pGVar4 = Scene::ptable.g_GlobalSoundPtr_00451698;
	pCVar3 = CScene::ptable.g_CameraManager_0045167c;
	//pCVar2 = g_CinematicManager_0048efc;
	currentLanguageID = this->languageID;
	lVar6 = (long)(int)CScene::ptable.g_AudioManager_00451698;
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
	if ((gVideoConfig.offsetX != this->setOffsetX) || (gVideoConfig.offsetY != this->setOffsetY)) {
		gVideoConfig.offsetY = this->setOffsetY;
		gVideoConfig.offsetX = this->setOffsetX;
		edVideoSetOffset((short)this->setOffsetX, (short)gVideoConfig.offsetY);
	}
	volume = this->musicVolume;
	// TODO: check this mess with shift and bitwise is (maybe normalizing?)
	if ((int)volume < 0) {
		fVar7 = (float)(volume >> 1 | volume & 1);
		fVar7 = fVar7 + fVar7;
	}
	else {
		fVar7 = (float)volume;
	}
	//if (fVar7 / 12.0 != pGVar4->field_0xbc) {
	//	FUN_00182000(fVar7 / 12.0, (int)pGVar4);
	//}
	volume = this->sfxVolume;
	if ((int)volume < 0) {
		fVar7 = (float)(volume >> 1 | volume & 1);
		fVar7 = fVar7 + fVar7;
	}
	else {
		fVar7 = (float)volume;
	}
	//if (fVar7 / 12.0 != pGVar4->field_0xc0) {
	//	FUN_00181f80(fVar7 / 12.0, pGVar4);
	//}
	iVar1 = this->audioMode;
	//iVar5 = thunk_FUN_00284930();
	//if ((long)iVar1 != (long)iVar5) {
	//	FUN_00181ef0(lVar6, (long)iVar1);
	//}
	//if ((ulong)this->field_0x0 != (long)gPlayerInput.field_0x14) {
	//	FUN_001b6d90((int)&gPlayerInput, (ulong)this->field_0x0);
	//}
	return;
}