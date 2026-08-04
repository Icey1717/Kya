#include "Settings.h"
#include "TranslatedTextData.h"
#include "LocalizationManager.h"
#include "CinematicManager.h"
#include "CameraViewManager.h"
#include "kya.h"
#include "InputManager.h"
#include "edVideo/VideoA.h"
#include "Audio.h"

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
	if (*(int*)pData == 0x53544753) { // 0x53544753 == "STGS"
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
	CCameraManager* pCameraManager;
	CAudioManager* pAudioManager;
	LANGUAGE prevLanguageID;
	CLanguageManager* pManager;
	int iVar5;
	float fVar7;
	CCinematicManager* pCinematicManager;
	LANGUAGE currentLanguageID;

	pAudioManager = CScene::ptable.g_AudioManager_00451698;
	pCameraManager = CScene::ptable.g_CameraManager_0045167c;
	pCinematicManager = g_CinematicManager_0048efc;
	currentLanguageID = this->languageID;

	prevLanguageID = CMessageFile::get_default_language();
	if (currentLanguageID != prevLanguageID) {
		CMessageFile::set_default_language(currentLanguageID);
		pManager = CLanguageManager::GetLanguageManager();
		pManager->ApplyLanguage();
	}

	if (this->bEnableSubtitles != (bool)*(byte*)&pCinematicManager->bUseSubtitles) {
		pCinematicManager->bUseSubtitles = (uint)this->bEnableSubtitles;
	}

	if (this->bWidescreen == 0) {
		if (pCameraManager->aspectRatio != 1.333333f) {
			pCameraManager->aspectRatio = 1.333333f;
		}
	}
	else {
		if (pCameraManager->aspectRatio != 1.777778f) {
			pCameraManager->aspectRatio = 1.777778f;
		}
	}

	if ((gVideoConfig.offsetX != this->setOffsetX) || (gVideoConfig.offsetY != this->setOffsetY)) {
		gVideoConfig.offsetY = this->setOffsetY;
		gVideoConfig.offsetX = this->setOffsetX;
		edVideoSetOffset((short)this->setOffsetX, (short)gVideoConfig.offsetY);
	}

	// Music Volume
	volume = this->musicVolume;
	// TODO: check this mess with shift and bitwise is (maybe normalizing?)
	if ((int)volume < 0) {
		fVar7 = (float)(volume >> 1 | volume & 1);
		fVar7 = fVar7 + fVar7;
	}
	else {
		fVar7 = (float)volume;
	}

	if (fVar7 / 12.0f != pAudioManager->musicVolume) {
		pAudioManager->SetMusicVolume(fVar7 / 12.0f);
	}

	// SFX Volume
	volume = this->sfxVolume;
	if ((int)volume < 0) {
		fVar7 = (float)(volume >> 1 | volume & 1);
		fVar7 = fVar7 + fVar7;
	}
	else {
		fVar7 = (float)volume;
	}

	if (fVar7 / 12.0f != pAudioManager->sfxVolume) {
		pAudioManager->SetSfxVolume(fVar7 / 12.0f);
	}

	AUDIO_MODE newMode = this->audioMode;
	AUDIO_MODE oldMode = edSoundOutputModeGet();
	if (newMode != oldMode) {
		pAudioManager->SetOutputMode(newMode);
	}

	if (this->bEnableVibration != gPlayerInput.bEnableVibration) {
		gPlayerInput.SetVibrationEnabled(this->bEnableVibration);
	}

	return;
}