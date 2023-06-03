#ifndef LOCALIZATION_MANAGER
#define LOCALIZATION_MANAGER

#include "Types.h"
#include "LargeObject.h"
#include "TranslatedTextData.h"

class LocalizationManager : public Manager {

public:
	LocalizationManager();

	// Begin Manager
	virtual void Game_Init();
	virtual void Level_Init();
	virtual void Level_Term();
	// End Manager

	MessageFile trcText;
	MessageFile levelText;
	MessageFile userInterfaceText;
};

extern LocalizationManager* g_LocalizationManager_00449744;

#endif //LOCALIZATION_MANAGER
