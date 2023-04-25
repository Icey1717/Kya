#ifndef FRONTEND_MANAGER_H
#define FRONTEND_MANAGER_H

#include "Types.h"
#include "LargeObject.h"

struct FrontendManager : public Manager {
	FrontendManager();
	virtual void OnBeginGame();

	struct CameraObj_28* pCamera;
	undefined field_0x8;
	undefined field_0x9;
	undefined field_0xa;
	undefined field_0xb;
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
	Matrix field_0x10;
	byte bHideHUD;
	undefined field_0x51;
	undefined field_0x52;
	undefined field_0x53;
	undefined field_0x54;
	undefined field_0x55;
	undefined field_0x56;
	undefined field_0x57;
	char* pHealthBar;
	struct HMagicOrbs* pMagicOrbs;
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

extern struct StaticMeshMaster* g_FrontendStaticMeshMaster_00448818;

#endif // FRONTEND_MANAGER_H
