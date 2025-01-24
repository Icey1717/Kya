#ifndef FRONTEND_BANK_H
#define FRONTEND_BANK_H

#include "Types.h"
#include "LargeObject.h"
#include "EdenLib/edBank/include/edBankBuffer.h"

class CFrontendBankSubObjA
{
public:

};

class CFrontendBankSubObjB
{
public:
};

class CFrontendBank : public CObjectManager
{
public:
	CFrontendBank();

	// CObjectManager
	virtual void Game_Init();
	virtual void Game_Term();

	virtual void Level_Init();
	virtual void Level_Term();
	virtual void Level_ClearAll();

	virtual void Level_Manage();
	virtual void Level_ManagePaused();

	edCBankBufferEntry* pBankBufferEntry;
	edCBankBuffer bankBuffer;
	undefined4 field_0x44;

	CFrontendBankSubObjA texture;
	CFrontendBankSubObjB mesh;
};

#endif // FRONTEND_BANK_H