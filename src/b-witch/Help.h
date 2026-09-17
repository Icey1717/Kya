#ifndef _HELP_H
#define _HELP_H

#include "Types.h"
#include "SpriteWidget.h"

extern CSprite g_aEquipmentTextures_00495090[33];

struct HelpManagerSubObj_C
{
	int field_0x0;
};

struct MysteryHelpObj
{
	edDList_material* pMaterial;
};

struct OtherMysteryHelpObj
{
	int field_0x0;
	int field_0x4;
	MysteryHelpObj* field_0x8;
	uint field_0xc;
	uint field_0x10;
};

class CHelpManager : public CObjectManager
{
public:
	virtual void Game_Init();
	virtual void Game_Term();

	virtual void Level_Init();
	virtual void Level_Term();

	virtual void Level_ClearAll();

	virtual void Level_Manage();
	virtual void Level_ManagePaused();
	virtual void Level_Draw();

	HelpManagerSubObj_C field_0x4;
	OtherMysteryHelpObj* field_0x1dc;
	undefined4 field_0x1e0;
};

#endif // _HELP_H