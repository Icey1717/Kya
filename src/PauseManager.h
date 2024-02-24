#ifndef _PAUSEMANAGER_H
#define _PAUSEMANAGER_H

#include "Types.h"

#ifdef PLATFORM_PS2
#include <libvu0.h>
#endif
#include "LargeObject.h"
#include "ed3D.h"
#include "edDlist.h"
#include "Settings.h"

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

struct CSimpleMenu;

typedef uint(*DrawMenuFuncPtr)(CSimpleMenu*, uint);

uint DrawGameMenu(CSimpleMenu* pMenu, uint input);

struct HistoryEntry {
	int currentPage;
	int selectedIndex;
	undefined4 field_0x8;
};

struct CSimpleMenu {
public:
	CSimpleMenu();
	void reset();
	void SetMode(EPauseMenu mode);

	void set_vertical_position(int verticalPos);

	float draw_func(float param_2);

	void draw_option(char* pMessage, uint color);
	bool draw_option_type_page(ulong helpMsgId, ulong msgId, EPauseMenu mode, int param_5, uint color);
	bool draw_option_type_page(ulong helpMsgId, char* pMessage, EPauseMenu mode, int param_5, uint color);

	void set_font_help(struct edCTextFont* pFont);
	void set_font_message(struct edCTextFont* pFont);
	void set_font_title(struct edCTextFont* pFont);
	void set_message_manager(struct CMessageFile* pTextData);
	EPauseMenu get_current_page();
	void perform_action();
	uint get_action();

	void draw_title(ulong msgHash, int offset);
	void draw_help_line(ulong msgId, int x, int y, uint color);

	void DrawMainMenu();
	void DrawInitialSaveMenuHelp(ulong helpMsgId, uint color);

	void update_page();

	void draw(DrawMenuFuncPtr pInputFunc);

	virtual bool select_draw();
	virtual bool appear_draw();
	virtual bool disappear_draw();
	virtual void on_validate();
	virtual void on_cancel();
	virtual void on_change_selection();

	struct CMessageFile* pTranslatedTextData;
	struct edCTextFont* pFontA;
	struct edCTextFont* pFontB;
	struct edCTextFont* pFontC;
	int lastInput_0x14;
	float totalTime;
	int screenState_0x1c;
	int field_0x20;
	undefined4 counter_0x24;
	undefined field_0x28;
	byte bShadow;
	undefined field_0x2a;
	undefined field_0x2b;
	int field_0x2c;
	int horizontalPos;
	int verticalPos;
	int horizontalAllignment;
	undefined4 field_0x3c;
	undefined4 pFunc_0x40;
	undefined4 slotID_0x44;
	enum EPauseMenu currentPage;
	int selectedIndex;
	int field_0x50;
	
	HistoryEntry aHistory[10];

	uint colorA;
	uint colorB;
	undefined4 field_0xd4;
	undefined4 field_0xd8;
	float field_0xdc;
	float selectedScaleX;
	float selectedScaleY;
	float scaleX;
	float scaleY;
	float field_0xf0;
	byte field_0xf4;
	undefined field_0xf5;
	undefined field_0xf6;
	undefined field_0xf7;
	undefined4 field_0xf8;
	undefined4 field_0xfc;
	undefined4 field_0x100;
	undefined4 field_0x104;
};

class CPauseManager : public CObjectManager {

public:
	CPauseManager();

	// Manager
	virtual void Game_Init();
	virtual void LevelLoading_Draw();
	virtual void Level_Init();
	virtual void Level_Draw();
	// End Manager

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
	struct CSimpleMenu* pSimpleMenu;
	struct CSplashScreen* pSplashScreen;
	float totalPlayTime;
	int field_0x34;
};

struct Sprite {
public:
	Sprite();
	void Install(char* pFileBuffer);
	void DrawXYXY(uint param_2, float param_3, float param_4, float param_5, float param_6, float param_7);
	byte field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	undefined4 field_0x8;
	struct edDList_material* pMaterialInfo;
	undefined4 field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	undefined4 field_0x20;
	undefined4 field_0x24;
	float field_0x28;
	float field_0x2c;
	byte field_0x30[4];
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
	float field_0x4c;
	float field_0x50;
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
	uint flags_0x7c;
	edDList_material materialInfo;
	ed_g2d_manager textureInfo;
};

struct CSplashScreen : public Sprite {
public:
	CSplashScreen();
	bool Init(float param_1, char* filePath);
	bool Manage(ulong param_2, bool param_3, bool param_4);
	void SetDrawLocation(float x, float y, float z, float w);
	char* pTextureFileData;
	float field_0xc4;
	float field_0xc8;
	undefined4 field_0xcc;
	undefined4 field_0xd0;
	edF32VECTOR4 drawOffsets;
};

extern struct edCTextFont* BootDataFont;

struct PauseStaticObj {
	bool field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	undefined4 languageID;
	bool field_0x8;
	byte field_0x9;
	undefined field_0xa;
	undefined field_0xb;
	int setOffsetX;
	int setOffsetY;
	undefined4 field_0x14;
	uint field_0x18;
	uint field_0x1c;
};

extern CSettings gSettings;

#endif // _PAUSEMANAGER_H
