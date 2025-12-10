#include "LocalizationManager.h"
#include "edMem.h"
#include "LevelScheduler.h"
#include "EdenLib/edText/sources/edTextResources.h"

#ifdef PLATFORM_PS2
#include <stdio.h>
#endif

CLanguageManager* g_LocalizationManager_00449744 = (CLanguageManager*)0x0;

// Should be in: D:/Projects/b-witch/LanguageManager.cpp
CLanguageManager* CLanguageManager::GetLanguageManager()
{
	return g_LocalizationManager_00449744;
}

CLanguageManager::CLanguageManager()
{
	g_LocalizationManager_00449744 = this;
}

char* sz_GAMENAME_004349a8 = "GAMENAME";

// Should be in: D:/Projects/b-witch/LanguageManager.cpp
void CLanguageManager::Game_Init()
{
	char* pcVar1;

	trcText.select_language("<bnk>0:TRC_%s.bin", AUTO);
	userInterfaceText.select_language("<BNK>0:UserInterface_%s.bin", AUTO);
	pcVar1 = userInterfaceText.get_message(0x2414d455f4e414d, 0);
	edTextResources.TextAdd(sz_GAMENAME_004349a8, pcVar1);
	return;
}

// Should be in: D:/Projects/b-witch/LanguageManager.cpp
void CLanguageManager::Game_Term()
{
	IMPLEMENTATION_GUARD();
}

char* sz_LevelTextSuffix_004349a0 = "_xx.bin";

// Should be in: D:/Projects/b-witch/LanguageManager.cpp
void CLanguageManager::Level_Init()
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

// Should be in: D:/Projects/b-witch/LanguageManager.cpp
void CLanguageManager::Level_Term()
{
	this->levelText.select_language((edCBankBufferEntry*)0x0, (char*)0x0, AUTO);
	return;
}

void CLanguageManager::Level_AddAll(struct ByteCode* pByteCode)
{
	return;
}

// Should be in: D:/Projects/b-witch/LanguageManager.h
void CLanguageManager::Level_ClearAll()
{
	return;
}

// Should be in: D:/Projects/b-witch/LanguageManager.h
void CLanguageManager::Level_Manage()
{
	return;
}

// Should be in: D:/Projects/b-witch/LanguageManager.h
void CLanguageManager::Level_ManagePaused()
{
	return;
}

// Should be in: D:/Projects/b-witch/LanguageManager.h
void CLanguageManager::Level_Draw()
{
	return;
}

// Should be in: D:/Projects/b-witch/LanguageManager.h
void CLanguageManager::Level_Reset()
{
	return;
}

// Should be in: D:/Projects/b-witch/LanguageManager.h
void CLanguageManager::Level_CheckpointReset()
{
	return;
}

void CLanguageManager::Level_SectorChange(int oldSectorId, int newSectorId)
{
	return;
}

void CLanguageManager::Level_PauseChange(bool bPaused)
{
	return;
}

// Should be in: D:/Projects/b-witch/LanguageManager.h
void CLanguageManager::Level_SaveContext()
{
	return;
}

// Should be in: D:/Projects/b-witch/LanguageManager.h
void CLanguageManager::Level_LoadContext()
{
	return;
}

char* CLanguageManager::GetLevelString(ulong key)
{
	return this->levelText.get_message(key, 0);
}

char* CLanguageManager::GetHelpString(ulong key)
{
	return this->userInterfaceText.get_message(key, 0);
}

ulong CLanguageManager::_commands[16] =
{
	0x0,				0x5D504D1A1D0A001B, 0x525A4F511D0A001B, 0x0,
	0x5A5F45575F0A001B, 0x4B56405F5C59451B, 0x4A44454E5E42001B, 0x5A5645565444471B,
	0x6085F525F5F544F,	0x4B564D5E1D0A001B, 0x0,				0x0,
	0x5C4B454E4958414B, 0x4B5A485F574B4D1B, 0x5C5D585F4F0A001B, 0x4A43495B560A001B,
};

ulong CLanguageManager::ULONG_ARRAY_00434970[5] =
{
	0x0, 0x4060C404B43504B, 0x4D405B4751494C4C, 0x17054B5152544C4E, 0x0
};

char* CLanguageManager::GetActionString(int index)
{
	return this->userInterfaceText.get_message(_commands[index], 0);
}

char* CLanguageManager::GetOtherActionString(int index)
{
	return this->userInterfaceText.get_message(ULONG_ARRAY_00434970[index], 0);
}