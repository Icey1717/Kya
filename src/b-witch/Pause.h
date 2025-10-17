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
#include "SpriteWidget.h"

#define MENU_STATE_APPEAR 0
#define MENU_STATE_SELECT 1
#define MENU_STATE_DISAPPEAR 2
#define MENU_STATE_EXIT 3
#define MENU_STATE_IDLE 4

typedef enum EPauseMenu
{
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
typedef void(*ActionFuncPtr)(int);

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

	void set_vertical_position(int verticalPos);
	void set_vertical_spacing(int verticalSpacing);

	void set_justification_left();
	void set_justification_right();

	float draw_func(float param_2);

	void draw_option(char* pMessage, uint color);
	bool draw_option_type_page(ulong helpMsgId, ulong msgId, EPauseMenu mode, int param_5, uint color);
	bool draw_option_type_page(ulong helpMsgId, char* pMessage, EPauseMenu mode, int param_5, uint color);
	bool draw_option(char* pMsg, ActionFuncPtr pFunc, int slotID, int param_5);

	bool draw_action(ulong actionMsgId, ActionFuncPtr pFunc, int slotID, int param_5);

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
	bool MenuFrameLoad_Draw();
	bool MenuFrameNewGame_Draw();
	void DrawInitialSaveMenuHelp(ulong helpMsgId, uint color);

	void update_page();

	void draw(DrawMenuFuncPtr pInputFunc);

	virtual bool select_draw();
	virtual bool appear_draw();
	virtual bool disappear_draw();
	virtual void on_validate();
	virtual void on_cancel();
	virtual void on_change_selection();

	float get_time_in_state();

	void pop_page();

	void FUN_002f1b50(ActionFuncPtr pFunc, int slot);

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
	undefined4 verticalSpacing;
	ActionFuncPtr pFunc_0x40;
	int slotID_0x44;
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
	virtual void Game_Term();

	virtual bool LevelLoading_Manage();
	virtual void LevelLoading_Draw();

	virtual void Level_Init();
	virtual void Level_Term();

	virtual void Level_ClearAll();

	virtual void Level_Manage();
	virtual void Level_ManagePaused();

	virtual void Level_Draw();

	virtual char* ProfileGetName();
	// End Manager

	void FUN_001b0860(int param_2);

	static void DrawRectangleBorder(float x, float y, float param_3, float param_4, float param_5, float param_6, uint colorA, uint colorB, long bAlphaBlend);

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

struct CSplashScreen : public CSprite {
public:
	CSplashScreen();

	bool Init(float param_1, char* filePath);
	bool Manage(uint param_2, bool param_3, bool param_4);
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

void PauseEnter(EPauseMenu mode);
void PauseLeave();
void ResumeGame(int);
bool UnpauseBlocked();

void HelpEnter();
void HelpLeave();

void MapEnter();
void MapLeave();

void CallPauseChange(int param_1);

void MemCardLoad0(int slotIndex);

extern CSettings gSettings;

class CSimpleMenuPause : public CSimpleMenu
{
public:
	virtual bool disappear_draw();

	virtual void set_mode(EPauseMenu mode);
};
extern CSimpleMenuPause gPauseMenu;
extern int gDebugLevelCheatEnabled_00449824;

#endif // _PAUSEMANAGER_H
