#ifndef LOCALIZATION_MANAGER
#define LOCALIZATION_MANAGER

#include "Types.h"
#include "LargeObject.h"
#include "TranslatedTextData.h"

class CLanguageManager : public CObjectManager {

public:
	CLanguageManager();

	static ulong _commands[16];
	static ulong ULONG_ARRAY_00434970[5];

	static CLanguageManager* GetLanguageManager();

	// Begin Manager
	virtual void Game_Init();
	virtual void Game_Term();

	virtual void Level_Init();
	virtual void Level_Term();
	virtual void Level_AddAll(struct ByteCode* pByteCode);
	virtual void Level_ClearAll();

	virtual void Level_Manage();
	virtual void Level_ManagePaused();
	virtual void Level_Draw();

	virtual void Level_Reset();

	virtual void Level_CheckpointReset();

	virtual void Level_SectorChange(int oldSectorId, int newSectorId);

	virtual void Level_PauseChange(bool bPaused);

	virtual void Level_SaveContext();
	virtual void Level_LoadContext();
	// End Manager

	char* GetLevelString(ulong key);
	char* GetActionString(int index);
	char* GetOtherActionString(int index);

	CMessageFile trcText;
	CMessageFile levelText;
	CMessageFile userInterfaceText;
};

extern CLanguageManager* g_LocalizationManager_00449744;

#endif //LOCALIZATION_MANAGER
