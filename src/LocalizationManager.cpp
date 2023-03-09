#include "LocalizationManager.h"
#include "edMem.h"
#include "TextIconDictionary.h"

LocalizationManager* g_LocalizationManager_00449744 = (LocalizationManager*)0x0;

LocalizationManager::LocalizationManager()
{
	g_LocalizationManager_00449744 = this;
}

char* sz_GAMENAME_004349a8 = "GAMENAME";

void LocalizationManager::OnBeginGame()
{
	char* pcVar1;

	trcText.LoadTranslatedTextFromDisk("<bnk>0:TRC_%s.bin", AUTO);
	userInterfaceText.LoadTranslatedTextFromDisk("<BNK>0:UserInterface_%s.bin", AUTO);
	pcVar1 = userInterfaceText.GetText_00336c10(0x2414d455f4e414d, 0);
	g_TextIconDictionary.AddTextEntry(sz_GAMENAME_004349a8, pcVar1);
	return;
}
