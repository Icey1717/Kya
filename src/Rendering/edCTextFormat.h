#ifndef _EDCTEXTFORMAT_H
#define _EDCTEXTFORMAT_H

#include "Types.h"
#include "edCTextStyle.h"

#include <stdarg.h>

#define TEXT_FLAG_SHADOW 0x100

struct edDList_material;

struct edTextBitmap
{
	edDList_material* pMaterial;
	float field_0x4;
	float field_0x8;
	float fWidth;
	float fHeight;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	float field_0x20;
};

struct TextLine
{
	char* pTextStart;
	char* pTextEnd;
	struct edCTextStyle* pTextStyle;
	char* pText;
	float field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	ushort field_0x20;
	ushort field_0x22;
	bool field_0x24;
	undefined field_0x25;
	undefined field_0x26;
	undefined field_0x27;
};

struct FormatSpec
{
	int switchMode;
	int field_0x4;
	char* pText;
	int characterCount;
	char* field_0x10;
	byte field_0x14;
	char field_0x15;
	undefined field_0x16;
	undefined field_0x17;
	int field_0x18;
	int numberFormatBase;
	int caseMode;
};

struct astruct_10
{
	int field_0x0;
	int field_0x4;
	int field_0x8;
	int field_0xc;
	int field_0x10;
	int field_0x14;
	int field_0x18;
	int field_0x1c;
};

struct edCTextFormat
{
	void DisplayWindow(float x, float y, Rectangle* pRect);
	void Display(float x, float y);
	void DisplayDebugInfos(float x, float y);
	void DrawString(float x, float y, bool bFlag);
	void SendTextRenderCommands_0028b0e0(struct DrawText16* pTextRenderCommands);
	void GetRect();
	bool FormatString(char* pText, va_list param_3);
	bool FormatString(char* pText, ...);

	edCTextFormat();
	edCTextFormat(edCTextStyle* pStyle);

	float offsetX_0x0;
	float offsetY_0x4;
	float field_0x8;
	float field_0xc;
	Rectangle vector_0x10;
	char* pScratchpadA;
	char* pScratchpadB;
	uint flags_0x28;
	uint lineCount;
	int field_0x30;

	int nbBitmaps;
	edTextBitmap* aBitmaps[16];

	TextLine aTextLines[50];
	undefined field_0x848;
	undefined field_0x849;
	undefined field_0x84a;
	undefined field_0x84b;
	undefined field_0x84c;
	undefined field_0x84d;
	undefined field_0x84e;
	undefined field_0x84f;
	edCTextStyle fontData_0x850[17];
};

#endif // _EDCTEXTFORMAT_H
