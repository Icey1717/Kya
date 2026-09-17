#include "Help.h"
#include "BootData.h"
#include "FrontEndBank.h"
#include "LargeObject.h"
#include "LocalizationManager.h"
#include "DlistManager.h"

CSimpleMenuSound gHelpMenu;

CSprite g_aEquipmentTextures_00495090[33];

char* g_aEquipmentTextureName_00425a10[33] = {
	"Equipment_silhouette_kya.g2d",
	"Equipment_boomy_basic.g2d",
	"Equipment_boomy_strong.g2d",
	"Equipment_boomy_gold.g2d",
	"Equipment_bomb_trap.g2d",
	"Equipment_bomb_elec.g2d",
	"Equipment_monster_trapped.g2d",
	"Equipment_fruit_back2life.g2d",
	"Equipment_fruit_damage.g2d",
	"Equipment_fruit_invincible.g2d",
	"Equipment_jamgut_whistle.g2d",
	"Equipment_binoculars.g2d",
	"Equipment_bounce_shoes.g2d",
	"Equipment_climbing_glove.g2d",
	"Equipment_regeneration.g2d",
	"Equipment_magic_board01.g2d",
	"Equipment_magic_board02.g2d",
	"Equipment_receptacle.g2d",
	"Equipment_rune01.g2d",
	"Equipment_rune02.g2d",
	"Equipment_rune03.g2d",
	"Equipment_rune04.g2d",
	"Equipment_rune05.g2d",
	"Equipment_rune06.g2d",
	"Equipment_rune07.g2d",
	"Equipment_white_bracelet.g2d",
	"Equipment_yellow_bracelet.g2d",
	"Equipment_green_bracelet.g2d",
	"Equipment_blue_bracelet.g2d",
	"Equipment_brown_bracelet.g2d",
	"Equipment_black_bracelet.g2d",
	"Equipment_silver_bracelet.g2d",
	"Equipment_gold_bracelet.g2d"
};

void CHelpManager::Game_Init()
{
	char* pFileBuffer;
	CSprite* pCurSprite;
	char** ppSpriteNames;
	int iVar1;
	CFrontendBank* pFrontendBank;

	gHelpMenu.set_font_title(BootDataFont);
	gHelpMenu.set_font_message(BootDataFont);
	gHelpMenu.set_font_help(BootDataFont);

	gHelpMenu.set_message_manager(&(CScene::ptable.g_LocalizationManager_00451678)->userInterfaceText);

	memset(&this->field_0x4, 0, sizeof(HelpManagerSubObj_C));
	(this->field_0x4).field_0x0 = -1;

	this->field_0x1e0 = 0;

	pFrontendBank = CScene::ptable.g_FrontEndBank_00451674;
	iVar1 = 0;
	ppSpriteNames = g_aEquipmentTextureName_00425a10;
	pCurSprite = g_aEquipmentTextures_00495090;
	do {
		NAME_NEXT_OBJECT("Help %s", *ppSpriteNames);
		pFileBuffer = pFrontendBank->GetResource(*ppSpriteNames, (edBANK_ENTRY_INFO*)0x0);
		pCurSprite->Install(pFileBuffer);
		iVar1 = iVar1 + 1;
		ppSpriteNames = ppSpriteNames + 1;
		pCurSprite = pCurSprite + 1;
	} while (iVar1 < 0x21);

	return;
}

void CHelpManager::Game_Term()
{
	CSprite* pCurSprite;
	int iVar1;

	iVar1 = 0;
	pCurSprite = g_aEquipmentTextures_00495090;
	do {
		pCurSprite->Remove();

		iVar1 = iVar1 + 1;
		pCurSprite = pCurSprite + 1;
	} while (iVar1 < 0x21);

	return;
}

int INT_ARRAY_00426520[33] = {
	0xf, 0x19, 0x1e, 0x23,
	0x6, 0xb, 0x15, 0x1a,
	0x1f, 0x24, 0x7, 0xc,
	0x11, 0x16, 0x1b, 0x20,
	0x25, 0x3, 0x8, 0xd,
	0x12, 0x17, 0x1c, 0x21,
	0x26, 0x4, 0x9, 0xe,
	0x13, 0x18, 0x1d, 0x22,
	0x27
};

OtherMysteryHelpObj OtherMysteryHelpObj_00496950;
MysteryHelpObj MysteryHelpObj_ARRAY_00425ee0[40];

void CHelpManager::Level_Init()
{
	OtherMysteryHelpObj* pOVar1;
	int local_90[33];

	this->field_0x1dc = &OtherMysteryHelpObj_00496950;
	pOVar1 = this->field_0x1dc;
	pOVar1->field_0x0 = 8;
	pOVar1->field_0x4 = 5;
	pOVar1->field_0x8 = MysteryHelpObj_ARRAY_00425ee0;
	pOVar1->field_0xc = 0x80ffffff;
	pOVar1->field_0x10 = 0x70707070;

	for (int i = 0; i < 33; i++) {
		local_90[i] = INT_ARRAY_00426520[i];
	}

	for (int i = 0; i < 33; i++) {
		assert(local_90[i] < 40);
		MysteryHelpObj_ARRAY_00425ee0[local_90[i]].pMaterial = &g_aEquipmentTextures_00495090[i].materialInfo;
	}

	return;
}

void CHelpManager::Level_Term()
{
	return;
}

void CHelpManager::Level_ClearAll()
{
	return;
}

void CHelpManager::Level_Manage()
{
	return;
}

void CHelpManager::Level_ManagePaused()
{
	return;
}

uint DrawHelpMenu(CSimpleMenu* pSimpleMenu, uint)
{

}

void CHelpManager::Level_Draw()
{
	bool bVar1;

	if (((GameFlags & 8) != 0) && ((UINT_00448eac == 1 || (UINT_00448eac == 2)))) {
		ClearDisplay();
		bVar1 = GuiDList_BeginCurrent();
		if (bVar1 != false) {
			gHelpMenu.draw(DrawHelpMenu);
			GuiDList_EndCurrent();
		}

		gHelpMenu.perform_action();
	}

	return;
}
