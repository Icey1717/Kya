#ifndef FRONTEND_DISP_H
#define FRONTEND_DISP_H

#include "Types.h"
#include "Frontend.h"
#include "LargeObject.h"

class CInterface;

enum FRONTEND_INTERFACE
{
	FRONTEND_INTERFACE_LIFE,
	FRONTEND_INTERFACE_ENEMY_LIST,
	FRONTEND_INTERFACE_MAGIC,
	FRONTEND_INTERFACE_MONEY,
	FRONTEND_INTERFACE_FREED_WOLFEN,
	FRONTEND_INTERFACE_ACTION,
	FRONTEND_INTERFACE_MENU_INVENTORY,
	FRONTEND_INTERFACE_MENU_0x74,
};

class CFrontendMagicGauge
{
public:
	unsigned char bVisible;
	void Magic_SetDisplay(unsigned char bNewVisible);
};

class CFrontendLifeGauge;

struct CFrontendDisplay : public CFrontend {
	CFrontendDisplay();

	virtual void Game_Init();
	virtual void Game_Term();

	virtual void Level_Init();
	virtual void Level_Term();

	virtual void Level_Manage();
	virtual void Level_ManagePaused();
	virtual void Level_Draw();

	virtual void Level_Reset();

	virtual void Level_SectorChange(int oldSectorId, int newSectorId) {}

	virtual void Level_CheckpointReset();

	virtual void Level_PauseChange(bool bPaused);

	virtual void Level_LoadContext();

	virtual void DeclareInterface(FRONTEND_INTERFACE interfaceType, CInterface* pInterface);
	virtual void SetHeroActionA(int actionId);
	virtual void SetHeroActionB(int actionId);
	virtual void SetGlobalFunc_001cf8e0();

	void SetActive(bool bActive);

	struct ed_viewport* pViewport;
	undefined field_0x8;
	undefined field_0x9;
	undefined field_0xa;
	undefined field_0xb;
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
	edF32MATRIX4 field_0x10;
	byte bHideHUD;
	undefined field_0x51;
	undefined field_0x52;
	undefined field_0x53;
	undefined field_0x54;
	undefined field_0x55;
	undefined field_0x56;
	undefined field_0x57;
	CFrontendLifeGauge* pHealthBar;
	struct CFrontendMagicGauge* pMagicOrbs;
	struct CFrontendMoney* pMoney;
	struct CFrontendEnemy* pFreedWolfun;
	struct CFrontendAction* pFrontendAction;
	struct MenuObj_Big* pMenuObj_0x6c;
	char* pMenuObj_0x70;
	char* pMenuObj_0x74;
	char* field_0x78;
	undefined field_0x7c;
	undefined field_0x7d;
	undefined field_0x7e;
	undefined field_0x7f;
};

#endif // FRONTEND_DISP_H
