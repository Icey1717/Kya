#ifndef _BOOT_DATA_H
#define _BOOT_DATA_H

#include "Types.h"
#include "PauseManager.h"

struct edCTextStyle;
struct edBANK_ENTRY_INFO;

extern CSprite MenuBitmaps[13];
extern char* MenuElementsBitmapNames[13];

extern CSprite BootBitmaps[23];

void InstallBootData(void);

void gTextCallback_Alpha75(edCTextStyle* pStyle);
void gTextCallback_Alpha50(edCTextStyle* pStyle);
void gTextCallback_Alpha25(edCTextStyle* pStyle);
void gTextCallback_Alpha0(edCTextStyle* pStyle);
void gTextCallback_NoBlink(edCTextStyle* pStyle);
void gTextCallback_Blink(edCTextStyle* pStyle);
void gTextCallback_White(edCTextStyle* pStyle);
void gTextCallback_Black(edCTextStyle* pStyle);
void gTextCallback_Yellow(edCTextStyle* pStyle);
void gTextCallback_Blue(edCTextStyle* pStyle);
void gTextCallback_Green(edCTextStyle* pStyle);
void gTextCallback_Red(edCTextStyle* pStyle);

char* BootData_GetResource(char* fileName, edBANK_ENTRY_INFO* outIconFileData);

#endif // _BOOT_DATA_H
