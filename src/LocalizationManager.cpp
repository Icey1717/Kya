#include "LocalizationManager.h"
#include "edMem.h"
#include "LevelScheduleManager.h"
#include "EdenLib/edText/sources/edTextResources.h"

#ifdef PLATFORM_PS2
#include <stdio.h>
#endif

LocalizationManager* g_LocalizationManager_00449744 = (LocalizationManager*)0x0;

LocalizationManager::LocalizationManager()
{
	g_LocalizationManager_00449744 = this;
}

char* sz_GAMENAME_004349a8 = "GAMENAME";

void LocalizationManager::Game_Init()
{
	char* pcVar1;

	trcText.select_language("<bnk>0:TRC_%s.bin", AUTO);
	userInterfaceText.select_language("<BNK>0:UserInterface_%s.bin", AUTO);
	pcVar1 = userInterfaceText.get_message(0x2414d455f4e414d, 0);
	edTextResources.TextAdd(sz_GAMENAME_004349a8, pcVar1);
	return;
}

char* sz_LevelTextSuffix_004349a0 = "_xx.bin";

void LocalizationManager::Level_Init()
{
	int iVar1;
	LevelScheduleManager* pLVar2;
	char filePath[64];

	pLVar2 = LevelScheduleManager::gThis;
	iVar1 = LevelScheduleManager::gThis->aLevelInfo[LevelScheduleManager::gThis->currentLevelID].field_0x30;
	if ((iVar1 != -1) && (iVar1 != 0)) {
		/* Format: %s%s LEVEL_x '_xx.bin' */
		sprintf(filePath, "%s%s", LevelScheduleManager::gThis->aLevelInfo[LevelScheduleManager::gThis->currentLevelID].levelPath, sz_LevelTextSuffix_004349a0);
		this->levelText.select_language((pLVar2->levelBank).pBankFileAccessObject, filePath, AUTO);
	}
	return;
}

void LocalizationManager::Level_Term()
{
	this->levelText.select_language((edCBankBufferEntry*)0x0, (char*)0x0, AUTO);
	return;
}
