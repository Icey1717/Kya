#include "LocalizationManager.h"
#include "edMem.h"
#include "LevelScheduleManager.h"
#include "EdenLib/edText/sources/edTextResources.h"

#ifdef PLATFORM_PS2
#include <stdio.h>
#endif

CLocalizationManager* g_LocalizationManager_00449744 = (CLocalizationManager*)0x0;

CLocalizationManager::CLocalizationManager()
{
	g_LocalizationManager_00449744 = this;
}

char* sz_GAMENAME_004349a8 = "GAMENAME";

void CLocalizationManager::Game_Init()
{
	char* pcVar1;

	trcText.select_language("<bnk>0:TRC_%s.bin", AUTO);
	userInterfaceText.select_language("<BNK>0:UserInterface_%s.bin", AUTO);
	pcVar1 = userInterfaceText.get_message(0x2414d455f4e414d, 0);
	edTextResources.TextAdd(sz_GAMENAME_004349a8, pcVar1);
	return;
}

void CLocalizationManager::Game_Term()
{
	IMPLEMENTATION_GUARD();
}

char* sz_LevelTextSuffix_004349a0 = "_xx.bin";

void CLocalizationManager::Level_Init()
{
	int iVar1;
	CLevelScheduler* pLVar2;
	char filePath[64];

	pLVar2 = CLevelScheduler::gThis;
	iVar1 = CLevelScheduler::gThis->aLevelInfo[CLevelScheduler::gThis->currentLevelID].field_0x30;
	if ((iVar1 != -1) && (iVar1 != 0)) {
		/* Format: %s%s LEVEL_x '_xx.bin' */
		sprintf(filePath, "%s%s", CLevelScheduler::gThis->aLevelInfo[CLevelScheduler::gThis->currentLevelID].levelPath, sz_LevelTextSuffix_004349a0);
		this->levelText.select_language((pLVar2)->pLevelBankBufferEntry, filePath, AUTO);
	}
	return;
}

void CLocalizationManager::Level_Term()
{
	this->levelText.select_language((edCBankBufferEntry*)0x0, (char*)0x0, AUTO);
	return;
}

void CLocalizationManager::Level_AddAll(struct ByteCode* pByteCode)
{
	return;
}

void CLocalizationManager::Level_ClearAll()
{
	return;
}

void CLocalizationManager::Level_Manage()
{
	return;
}

void CLocalizationManager::Level_ManagePaused()
{
	return;
}

void CLocalizationManager::Level_Draw()
{
	return;
}

void CLocalizationManager::Level_Reset()
{
	return;
}

void CLocalizationManager::Level_CheckpointReset()
{
	return;
}

void CLocalizationManager::Level_SectorChange(int oldSectorId, int newSectorId)
{
	return;
}

void CLocalizationManager::Level_PauseChange(bool bPaused)
{
	return;
}

void CLocalizationManager::Level_SaveContext()
{
	return;
}

void CLocalizationManager::Level_LoadContext()
{
	return;
}

char* CLocalizationManager::GetLevelText(ulong key)
{
	return this->levelText.get_message(key, 0);
}
