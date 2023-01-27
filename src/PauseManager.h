#ifndef _PAUSEMANAGER_H
#define _PAUSEMANAGER_H

#include "Types.h"

#ifdef PLATFORM_PS2
#include <libvu0.h>
#endif
#include "LargeObject.h"
#include "ed3D.h"

typedef enum EPauseMenu {
	PM_Bonus = 13,
	PM_DisplayOptions = 7,
	PM_InitialSave = 5,
	PM_LoadMenu = 4,
	PM_MainMenu = 0,
	PM_MiniGame = 2,
	PM_None = 4294967295,
	PM_OptionsMenu = 6,
	PM_PauseMenu = 1,
	PM_SaveMenu = 3
} EPauseMenu;

struct PauseScreenData {
	struct PauseScreenData* field_0x4;
	undefined4 field_0x8;
	void* field_0xc;
	void* field_0x10;
	int lastInput_0x14;
	float field_0x18;
	int screenState_0x1c;
	int field_0x20;
	undefined4 counter_0x24;
	undefined field_0x28;
	byte field_0x29;
	undefined field_0x2a;
	undefined field_0x2b;
	int field_0x2c;
	int field_0x30;
	undefined4 field_0x34;
	undefined4 field_0x38;
	undefined4 field_0x3c;
	undefined4 pFunc_0x40;
	undefined4 slotID_0x44;
	enum EPauseMenu currentMode;
	int selectedIndex;
	int field_0x50;
	undefined field_0x54;
	undefined field_0x55;
	undefined field_0x56;
	undefined field_0x57;
	undefined field_0x58;
	undefined field_0x59;
	undefined field_0x5a;
	undefined field_0x5b;
	undefined field_0x5c;
	undefined field_0x5d;
	undefined field_0x5e;
	undefined field_0x5f;
	undefined field_0x60;
	undefined field_0x61;
	undefined field_0x62;
	undefined field_0x63;
	undefined field_0x64;
	undefined field_0x65;
	undefined field_0x66;
	undefined field_0x67;
	undefined field_0x68;
	undefined field_0x69;
	undefined field_0x6a;
	undefined field_0x6b;
	undefined field_0x6c;
	undefined field_0x6d;
	undefined field_0x6e;
	undefined field_0x6f;
	undefined field_0x70;
	undefined field_0x71;
	undefined field_0x72;
	undefined field_0x73;
	undefined field_0x74;
	undefined field_0x75;
	undefined field_0x76;
	undefined field_0x77;
	undefined field_0x78;
	undefined field_0x79;
	undefined field_0x7a;
	undefined field_0x7b;
	undefined field_0x7c;
	undefined field_0x7d;
	undefined field_0x7e;
	undefined field_0x7f;
	undefined field_0x80;
	undefined field_0x81;
	undefined field_0x82;
	undefined field_0x83;
	undefined field_0x84;
	undefined field_0x85;
	undefined field_0x86;
	undefined field_0x87;
	undefined field_0x88;
	undefined field_0x89;
	undefined field_0x8a;
	undefined field_0x8b;
	undefined field_0x8c;
	undefined field_0x8d;
	undefined field_0x8e;
	undefined field_0x8f;
	undefined field_0x90;
	undefined field_0x91;
	undefined field_0x92;
	undefined field_0x93;
	undefined field_0x94;
	undefined field_0x95;
	undefined field_0x96;
	undefined field_0x97;
	undefined field_0x98;
	undefined field_0x99;
	undefined field_0x9a;
	undefined field_0x9b;
	undefined field_0x9c;
	undefined field_0x9d;
	undefined field_0x9e;
	undefined field_0x9f;
	undefined field_0xa0;
	undefined field_0xa1;
	undefined field_0xa2;
	undefined field_0xa3;
	undefined field_0xa4;
	undefined field_0xa5;
	undefined field_0xa6;
	undefined field_0xa7;
	undefined field_0xa8;
	undefined field_0xa9;
	undefined field_0xaa;
	undefined field_0xab;
	undefined field_0xac;
	undefined field_0xad;
	undefined field_0xae;
	undefined field_0xaf;
	undefined field_0xb0;
	undefined field_0xb1;
	undefined field_0xb2;
	undefined field_0xb3;
	undefined field_0xb4;
	undefined field_0xb5;
	undefined field_0xb6;
	undefined field_0xb7;
	undefined field_0xb8;
	undefined field_0xb9;
	undefined field_0xba;
	undefined field_0xbb;
	undefined field_0xbc;
	undefined field_0xbd;
	undefined field_0xbe;
	undefined field_0xbf;
	undefined field_0xc0;
	undefined field_0xc1;
	undefined field_0xc2;
	undefined field_0xc3;
	undefined field_0xc4;
	undefined field_0xc5;
	undefined field_0xc6;
	undefined field_0xc7;
	undefined field_0xc8;
	undefined field_0xc9;
	undefined field_0xca;
	undefined field_0xcb;
	undefined4 field_0xcc;
	undefined4 field_0xd0;
	undefined4 field_0xd4;
	undefined4 field_0xd8;
	float field_0xdc;
	float field_0xe0;
	float field_0xe4;
	float field_0xe8;
	float field_0xec;
	float field_0xf0;
	undefined field_0xf4;
	undefined field_0xf5;
	undefined field_0xf6;
	undefined field_0xf7;
	undefined4 field_0xf8;
	undefined4 field_0xfc;
	undefined4 field_0x100;
	undefined4 field_0x104;
};

class PauseManager : public Manager {

public:
	PauseManager* SetupPauseManager_001b0eb0();

	virtual void OnBeginGame();
	virtual void LoadLevelUpdate();

	int currentAreaIndex;
	int totalAreaCount;
	byte field_0xc;
	byte field_0xd;
	byte field_0xe;
	byte field_0xf;
	byte field_0x10;
	byte field_0x11;
	byte field_0x12;
	byte field_0x13;
	byte field_0x14;
	undefined field_0x15;
	undefined field_0x16;
	undefined field_0x17;
	int currentLevelID_0x18;
	int field_0x1c;
	int field_0x20;
	byte field_0x24;
	undefined field_0x25;
	undefined field_0x26;
	undefined field_0x27;
	struct PauseScreenData* pSubObj108_0x28;
	struct G2DObj_PauseManager* pTexture_0x2c;
	float field_0x30;
	int field_0x34;
};

struct G2DObj {
	struct G2DObj_VTable* pVTable;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	undefined4 field_0x8;
	void* field_0xc;
	undefined4 field_0x10;
	undefined* field_0x14;
	float field_0x18;
	float field_0x1c;
	undefined4 field_0x20;
	undefined4 field_0x24;
	float field_0x28;
	float field_0x2c;
	undefined4 field_0x30;
	undefined4 field_0x34;
	ushort field_0x38;
	ushort field_0x3a;
	undefined field_0x3c;
	undefined field_0x3d;
	undefined field_0x3e;
	undefined field_0x3f;
	undefined field_0x40;
	undefined field_0x41;
	undefined field_0x42;
	undefined field_0x43;
	undefined field_0x44;
	undefined field_0x45;
	undefined field_0x46;
	undefined field_0x47;
	undefined field_0x48;
	undefined field_0x49;
	undefined field_0x4a;
	undefined field_0x4b;
	undefined4 field_0x4c;
	undefined4 field_0x50;
	float field_0x54;
	float field_0x58;
	undefined4 field_0x5c;
	undefined4 field_0x60;
	undefined4 field_0x64;
	undefined4 field_0x68;
	undefined4 field_0x6c;
	undefined* field_0x70;
	float field_0x74;
	float field_0x78;
	int field_0x7c;
	struct MaterialInfo materialInfo;
	struct TextureInfoSmall textureInfo;
};

struct G2DObj_VTable {
	void* field_0x0;
	void* field_0x4;
	void (*init)(struct G2DObj*);
	void* field_0xc;
	void* field_0x10;
	void* field_0x14;
	void* field_0x18;
	void* field_0x1c;
	void* field_0x20;
	void* field_0x24;
	void* field_0x28;
	void* field_0x2c;
};

struct G2DObj_PauseManager {
	struct G2DObj base;
	float field_0xc4;
	float field_0xc8;
	undefined4 field_0xcc;
	undefined4 field_0xd0;
	sceVu0FVECTOR vector_0xd4;
};

extern PauseManager* g_PauseManager_00451688;

#endif // _PAUSEMANAGER_H
