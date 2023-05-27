#include "LocalizationManager.h"
#include "edMem.h"
#include "TextIconDictionary.h"
#include "LevelScheduleManager.h"

LocalizationManager* g_LocalizationManager_00449744 = (LocalizationManager*)0x0;

LocalizationManager::LocalizationManager()
{
	g_LocalizationManager_00449744 = this;
}

char* sz_GAMENAME_004349a8 = "GAMENAME";

void LocalizationManager::Game_Init()
{
	char* pcVar1;

	trcText.LoadTranslatedTextFromDisk("<bnk>0:TRC_%s.bin", AUTO);
	userInterfaceText.LoadTranslatedTextFromDisk("<BNK>0:UserInterface_%s.bin", AUTO);
	pcVar1 = userInterfaceText.GetText_00336c10(0x2414d455f4e414d, 0);
	g_TextIconDictionary.AddTextEntry(sz_GAMENAME_004349a8, pcVar1);
	return;
}

char* sz_LevelTextSuffix_004349a0 = "_xx.bin";

void LocalizationManager::Level_Init()
{
	int iVar1;
	LevelScheduleManager* pLVar2;
	char filePath[64];

	pLVar2 = g_LevelScheduleManager_00449728;
	iVar1 = g_LevelScheduleManager_00449728->aLevelInfo[g_LevelScheduleManager_00449728->currentLevelID].field_0x30;
	if ((iVar1 != -1) && (iVar1 != 0)) {
		/* Format: %s%s LEVEL_x '_xx.bin' */
		sprintf(filePath, "%s%s", g_LevelScheduleManager_00449728->aLevelInfo[g_LevelScheduleManager_00449728->currentLevelID].levelPath, sz_LevelTextSuffix_004349a0);
		this->levelText.LoadTextTranslatedFromBank((pLVar2->levelBank).pBankFileAccessObject, filePath, AUTO);
	}
	return;
}

void LocalizationManager::Level_Term()
{
	this->levelText.LoadTextTranslatedFromBank((edCBankBufferEntry*)0x0, (char*)0x0, AUTO);
	return;
}
