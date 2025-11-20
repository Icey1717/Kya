#include "BootData.h"
#include "edBank/edBankBuffer.h"
#include "Pause.h"
#include "edText.h"
#include "Rendering/edCTextFont.h"
#include "EdenLib/edText/sources/edTextResources.h"
#include "Rendering/edCTextStyle.h"
#include "TimeController.h"
#include <string.h>
#include <math.h>

#ifdef PLATFORM_PS2
#include <eekernel.h>
#else
#include "port/pointer_conv.h"
#endif
#include "FrontendDisp.h"

edCBankBuffer BootData_BankBuffer; // = { 0 };
edCBankBufferEntry* BootData_BankBufferEntry = NULL;
char* sz_MenuDataBankName_00435610 = "CDEURO/menu/MenuData.bnk";
char* BootDataBankName = sz_MenuDataBankName_00435610;

char* sz_MediumFont_004355f8 = "medium.fon";
char* sz_MediumFontFileName_00448b60 = sz_MediumFont_004355f8;

CSprite MenuBitmaps[13];

char* MenuElementsBitmapNames[13] = {
	"i_wolfen.g2d",
	"i_native.g2d",
	"i_wolfen_exorcised.g2d",
	"inv_case_01_p0.g2d",
	"inv_select_01_p0.g2d",
	"sound_slider_cursor.g2d",
	"selector_on.g2d",
	"selector_off.g2d",
	"support.g2d",
	"arrow.g2d",
	"arrow_pressed.g2d",
	"backlayer.g2d",
	"tatoo.g2d"
};

char* BootBitmapNames[23] = {
	"but_tria.g2d",
	"but_circ.g2d",
	"but_squa.g2d",
	"but_cros.g2d",
	"but_l1.g2d",
	"but_r1.g2d",
	"but_l2.g2d",
	"but_r2.g2d",
	"but_sele.g2d",
	"but_m1_turn.g2d",
	"but_m1.g2d",
	"but_udlr.g2d",
	"but_lr.g2d",
	"fl_haut.g2d",
	"fl_droite.g2d",
	"fl_gauche.g2d",
	"fl_bas.g2d",
	"selection.g2d",
	"woodsqre.g2d",
	"icon_clock.g2d",
	"map.g2d",
	"money.g2d",
	"memory_card.g2d"
};

CSprite BootBitmaps[23] = {};
ushort USHORT_ARRAY_0048fc60[256] = {};

void InstallBootData(void)
{
	char* puVar1;
	int fileIndex;
	char* messagesFilePointer;
	CSprite* pIconTexture;
	char** ppcVar2;
	int iVar3;
	edCBankInstall bankHeader;

	/* The menu BNK contains images for all button icons, the main Medium.fon. Icon for saves, money
	   and a map. */
	memset(&bankHeader, 0, sizeof(edCBankInstall));
	BootData_BankBuffer.initialize(0x32000, 1, &bankHeader);
	/* Set the bank header to point towards 'CDEURO/menu/Messages.bnk' */
	bankHeader.filePath = BootDataBankName;
	BootData_BankBufferEntry = BootData_BankBuffer.get_free_entry();
	BootData_BankBufferEntry->load(&bankHeader);
	/* Bank will go on the heap here */
	iVar3 = 0;
	ppcVar2 = BootBitmapNames;
	pIconTexture = BootBitmaps;
	do {
		messagesFilePointer = *ppcVar2;

		NAME_NEXT_OBJECT(messagesFilePointer);

		fileIndex = BootData_BankBufferEntry->get_index(messagesFilePointer);
		if (fileIndex == -1) {
			edDebugPrintf("\r\nFile: %s\r\n", messagesFilePointer);
			messagesFilePointer = (char*)0x0;
		}
		else {
			messagesFilePointer = BootData_BankBufferEntry->get_element(fileIndex);
		}
		pIconTexture->Install(messagesFilePointer);
		puVar1 = sz_MediumFontFileName_00448b60;
		iVar3 = iVar3 + 1;
		ppcVar2 = ppcVar2 + 1;
		pIconTexture = pIconTexture + 1;
	} while (iVar3 < 0x17);
	/* Init Medium.Fon */
	iVar3 = BootData_BankBufferEntry->get_index(sz_MediumFontFileName_00448b60);
	if (iVar3 == -1) {
		edDebugPrintf("\r\nFile: %s\r\n", puVar1);
		BootDataFont = (edCTextFont*)0x0;
	}
	else {
		BootDataFont = (edCTextFont*)BootData_BankBufferEntry->get_element(iVar3);
	}

	NAME_NEXT_OBJECT(sz_MediumFontFileName_00448b60);
	edTextInstallFont(BootDataFont);
	if ((FontPacked_2C*)BootDataFont->pSubData != (FontPacked_2C*)0x0) {
		LOAD_POINTER_CAST(FontPacked_2C*, BootDataFont->pSubData)->pOverrideData = USHORT_ARRAY_0048fc60;
	}
	USHORT_ARRAY_0048fc60[156] = 0x153;
	USHORT_ARRAY_0048fc60[241] = 0xf1;
	USHORT_ARRAY_0048fc60[231] = 0xe7;
	USHORT_ARRAY_0048fc60[199] = 199;
	USHORT_ARRAY_0048fc60[220] = 0xdc;

	edTextResources.BitmapAdd("MAGIC", &BootBitmaps[0].textBitmap);
	edTextResources.BitmapAdd("CATCH", &BootBitmaps[1].textBitmap);
	edTextResources.BitmapAdd("ACTION", &BootBitmaps[2].textBitmap);
	edTextResources.BitmapAdd("JUMP", &BootBitmaps[3].textBitmap);
	edTextResources.BitmapAdd("CROUCH", &BootBitmaps[6].textBitmap);
	edTextResources.BitmapAdd("SNEAK", &BootBitmaps[6].textBitmap);
	edTextResources.BitmapAdd("VIEW", &BootBitmaps[5].textBitmap);
	edTextResources.BitmapAdd("MAP", &BootBitmaps[4].textBitmap);
	edTextResources.BitmapAdd("INVENT", &BootBitmaps[7].textBitmap);
	edTextResources.BitmapAdd("CAMERA", &BootBitmaps[10].textBitmap);
	edTextResources.BitmapAdd("CONTROL", &BootBitmaps[10].textBitmap);
	edTextResources.BitmapAdd("MONEY", &BootBitmaps[0x15].textBitmap);
	edTextResources.BitmapAdd("BACK", &BootBitmaps[0].textBitmap);
	edTextResources.BitmapAdd("VALID", &BootBitmaps[3].textBitmap);
	edTextResources.BitmapAdd("VALDSAFE", &BootBitmaps[1].textBitmap);
	edTextResources.BitmapAdd("UDLR", &BootBitmaps[0xb].textBitmap);
	edTextResources.BitmapAdd("LR", &BootBitmaps[0xc].textBitmap);
	edTextResources.BitmapAdd("STIK_ROT", &BootBitmaps[9].textBitmap);
	edTextResources.BitmapAdd("UP", &BootBitmaps[0xd].textBitmap);
	edTextResources.BitmapAdd("RIGHT", &BootBitmaps[0xe].textBitmap);
	edTextResources.BitmapAdd("LEFT", &BootBitmaps[0xf].textBitmap);
	edTextResources.BitmapAdd("DOWN", &BootBitmaps[0x10].textBitmap);
	edTextResources.BitmapAdd("HELP", &BootBitmaps[8].textBitmap);

	edTextResources.CallbackAdd("RED", gTextCallback_Red);
	edTextResources.CallbackAdd("GREEN", gTextCallback_Green);
	edTextResources.CallbackAdd("BLUE", gTextCallback_Blue);
	edTextResources.CallbackAdd("YELLOW", gTextCallback_Yellow);
	edTextResources.CallbackAdd("BLACK", gTextCallback_Black);
	edTextResources.CallbackAdd("WHITE", gTextCallback_White);
	edTextResources.CallbackAdd("BLINK", gTextCallback_Blink);
	edTextResources.CallbackAdd("NOBLINK", gTextCallback_NoBlink);
	edTextResources.CallbackAdd("ALPHA0", gTextCallback_Alpha0);
	edTextResources.CallbackAdd("ALPHA25", gTextCallback_Alpha25);
	edTextResources.CallbackAdd("ALPHA50", gTextCallback_Alpha50);
	edTextResources.CallbackAdd("ALPHA75", gTextCallback_Alpha75);
	edTextResources.CallbackAdd("RESET", NULL);

	return;
}

void gTextCallback_Alpha75(edCTextStyle* pStyle)
{
	pStyle->rgbaColour = pStyle->rgbaColour & 0xffffff00U | 0xc0;
	pStyle->altColour = pStyle->altColour & 0xffffff00U | 0xc0;
	return;
}

void gTextCallback_Alpha50(edCTextStyle* pStyle)
{
	pStyle->rgbaColour = pStyle->rgbaColour & 0xffffff00U | 0x80;
	pStyle->altColour = pStyle->altColour & 0xffffff00U | 0x80;
	return;
}

void gTextCallback_Alpha25(edCTextStyle* pStyle)
{
	pStyle->rgbaColour = pStyle->rgbaColour & 0xffffff00U | 0x40;
	pStyle->altColour = pStyle->altColour & 0xffffff00U | 0x40;
	return;
}

void gTextCallback_Alpha0(edCTextStyle* pStyle)
{
	pStyle->rgbaColour = pStyle->rgbaColour & 0xffffff00;
	pStyle->altColour = pStyle->altColour & 0xffffff00;
	return;
}

void gTextCallback_NoBlink(edCTextStyle* pStyle)
{
	pStyle->rgbaColour = pStyle->rgbaColour & 0xffffff00U | 0xff;
	pStyle->altColour = pStyle->altColour & 0xffffff00U | 0xff;
	pStyle->alpha = pStyle->alpha & 0xffffff00U | 0xff;
	return;
}

void gTextCallback_Blink(edCTextStyle* pStyle)
{
	Timer* pTVar1;
	float fVar2;
	uint uVar3;

	pTVar1 = GetTimer();
	fVar2 = fmodf(pTVar1->totalTime, 0.8f);
	if (fVar2 < 0.2f) {
		fVar2 = (fVar2 * 255.0f) / 0.2f;
		if (fVar2 < 2.147484e+09f) {
			uVar3 = (uint)fVar2;
		}
		else {
			uVar3 = (int)(fVar2 - 2.147484e+09f) | 0x80000000;
		}
	}
	else {
		uVar3 = 0xff;
		if (0.6f <= fVar2) {
			fVar2 = ((0.8f - fVar2) * 255.0f) / 0.2f;
			if (fVar2 < 2.147484e+09f) {
				uVar3 = (uint)fVar2;
			}
			else {
				uVar3 = (int)(fVar2 - 2.147484e+09f) | 0x80000000;
			}
		}
	}
	pStyle->rgbaColour = pStyle->rgbaColour & 0xffffff00U | uVar3;
	pStyle->altColour = pStyle->altColour & 0xffffff00U | uVar3;
	pStyle->alpha = pStyle->alpha & 0xffffff00U | uVar3;
	return;
}

void gTextCallback_White(edCTextStyle* pStyle)
{
	pStyle->rgbaColour = *(byte*)&pStyle->rgbaColour | 0xffffff00;
	return;
}

void gTextCallback_Black(edCTextStyle* pStyle)
{
	pStyle->rgbaColour = (uint) * (byte*)&pStyle->rgbaColour;
	return;
}

void gTextCallback_Yellow(edCTextStyle* pStyle)
{
	pStyle->rgbaColour = *(byte*)&pStyle->rgbaColour | 0xffff0000;
	return;
}

void gTextCallback_Blue(edCTextStyle* pStyle)
{
	pStyle->rgbaColour = *(byte*)&pStyle->rgbaColour | 0xff00;
	return;
}

void gTextCallback_Green(edCTextStyle* pStyle)
{
	pStyle->rgbaColour = *(byte*)&pStyle->rgbaColour | 0xff0000;
	return;
}

void gTextCallback_Red(edCTextStyle* pStyle)
{
	pStyle->rgbaColour = *(byte*)&pStyle->rgbaColour | 0xff000000;
	return;
}

char* BootData_GetResource(char* fileName, edBANK_ENTRY_INFO* outIconFileData)
{
	bool bVar1;
	int fileIndex;
	char* fileBufferStart;

	fileIndex = BootData_BankBufferEntry->get_index(fileName);
	if (fileIndex == -1) {
		edDebugPrintf("\r\nFile: %s\r\n", fileName);
		fileBufferStart = (char*)0x0;
	}
	else {
		fileBufferStart = (char*)0x0;
		if (outIconFileData == (edBANK_ENTRY_INFO*)0x0) {
			fileBufferStart = BootData_BankBufferEntry->get_element(fileIndex);
		}
		else {
			bVar1 = BootData_BankBufferEntry->get_info(fileIndex, outIconFileData, (char*)0x0);
			if (bVar1 != false) {
				fileBufferStart = outIconFileData->fileBufferStart;
			}
		}
	}
	return fileBufferStart;
}