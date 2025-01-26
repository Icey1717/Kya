#ifndef FRONTEND_BANK_H
#define FRONTEND_BANK_H

#include "Types.h"
#include "LargeObject.h"
#include "EdenLib/edBank/include/edBankBuffer.h"
#include "ed3D.h"

struct FrontendTextureListEntry
{
	char* pData;
	int size;
	ed_g2d_manager manager;
};

struct FrontendTextureList
{
	FrontendTextureListEntry aEntries[4];
};

struct FrontendMeshListEntry
{
	char* pData;
	int size;
	ed_g3d_manager manager;
};

struct FrontendMeshList
{
	FrontendMeshListEntry aEntries[1];
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

	char* GetResource(char* pFileName, edBANK_ENTRY_INFO* pEntryInfo);
	ed_g2d_manager* GetG2D(int index);

	edCBankBufferEntry* pBankBufferEntry;
	edCBankBuffer bankBuffer;

	int nbTextures;
	int nbMeshes;

	FrontendTextureList texture;
	FrontendMeshList mesh;
};

#endif // FRONTEND_BANK_H