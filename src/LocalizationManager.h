#ifndef LOCALIZATION_MANAGER
#define LOCALIZATION_MANAGER

#include "Types.h"
#include "LargeObject.h"
#include "TranslatedTextData.h"

class CLocalizationManager : public CObjectManager {

public:
	CLocalizationManager();

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

	virtual void Level_PauseChange();

	virtual void Level_SaveContext();
	virtual void Level_LoadContext();
	// End Manager

	CMessageFile trcText;
	CMessageFile levelText;
	CMessageFile userInterfaceText;
};

extern CLocalizationManager* g_LocalizationManager_00449744;

#endif //LOCALIZATION_MANAGER
