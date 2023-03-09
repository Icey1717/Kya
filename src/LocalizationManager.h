#ifndef LOCALIZATION_MANAGER
#define LOCALIZATION_MANAGER

#include "Types.h"
#include "LargeObject.h"
#include "TranslatedTextData.h"

class LocalizationManager : public Manager {

public:
	LocalizationManager();

	virtual void OnBeginGame();
	//virtual void LoadLevelUpdate();

	TranslatedTextData trcText;
	TranslatedTextData levelText;
	TranslatedTextData userInterfaceText;
};

extern LocalizationManager* g_LocalizationManager_00449744;

#endif //LOCALIZATION_MANAGER
