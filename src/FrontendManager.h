#ifndef FRONTEND_MANAGER_H
#define FRONTEND_MANAGER_H

#include "Types.h"
#include "LargeObject.h"

struct CFrontendMagicGauge {
	unsigned char bVisible;
	void Magic_SetDisplay(unsigned char bNewVisible);
};

class CWidget {};

class CFrontendLifeGauge : public CWidget
{
public:
	void UpdatePercent(float) { IMPLEMENTATION_GUARD_LOG(); }
};

struct CFrontendDisplay : public CObjectManager {
	CFrontendDisplay();
	virtual void Game_Init();
	virtual void Game_Term() { IMPLEMENTATION_GUARD(); }

	virtual void Level_Init() { IMPLEMENTATION_GUARD_UI(); }
	virtual void Level_Term() { IMPLEMENTATION_GUARD_UI(); }

	virtual void Level_ClearAll() { IMPLEMENTATION_GUARD_UI(); }
	virtual void Level_Manage() { IMPLEMENTATION_GUARD_UI(); }
	virtual void Level_ManagePaused() { IMPLEMENTATION_GUARD_UI(); }
	virtual void Level_Draw() { IMPLEMENTATION_GUARD_UI(); }

	virtual void Level_Reset() { IMPLEMENTATION_GUARD_UI(); }

	virtual void Level_SectorChange(int oldSectorId, int newSectorId) {}

	virtual void Level_CheckpointReset() { IMPLEMENTATION_GUARD_UI(); }

	virtual void Level_PauseChange() { IMPLEMENTATION_GUARD_UI(); }

	virtual void Level_SaveContext() { IMPLEMENTATION_GUARD_UI(); }

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
	struct HNootiesWolfun* pNooties;
	struct HNootiesWolfun* pFreedWolfun;
	struct MenuObj_164* pMenuObj_0x68;
	struct MenuObj_Big* pMenuObj_0x6c;
	char* pMenuObj_0x70;
	char* pMenuObj_0x74;
	char* field_0x78;
	undefined field_0x7c;
	undefined field_0x7d;
	undefined field_0x7e;
	undefined field_0x7f;
};

struct ed_3D_Scene;

namespace Frontend {
	extern ed_3D_Scene* _scene_handle;
}

#endif // FRONTEND_MANAGER_H
