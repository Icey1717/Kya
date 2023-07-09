#ifndef _EDCTEXTFORMAT_H
#define _EDCTEXTFORMAT_H

#include "Types.h"
#include "edCTextStyle.h"

struct TextLine {
	char* pTextStart;
	char* pTextEnd;
	struct edCTextStyle* pFontFileData;
	char* pText;
	float field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	ushort field_0x20;
	ushort field_0x22;
	char field_0x24;
	undefined field_0x25;
	undefined field_0x26;
	undefined field_0x27;
};

struct astruct_5 {
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
	int field_0x1c;
	int field_0x20;
};

struct astruct_10 {
	int field_0x0;
	int field_0x4;
	int field_0x8;
	int field_0xc;
	int field_0x10;
	int field_0x14;
	int field_0x18;
	int field_0x1c;
};

struct edCTextFormat {
	void DisplayWindow(float x, float y, edF32VECTOR4* param_4);
	void Display(float x, float y);
	void DisplayDebugInfos(float x, float y);
	void DrawString(float x, float y, bool bFlag);
	void SendTextRenderCommands_0028b0e0(struct DrawText16* pTextRenderCommands);
	void GetRect();
	bool FormatString(char* pText, char* param_3);
	bool FormatString(char* pText);
	edCTextFormat();

	float offsetX_0x0;
	float offsetY_0x4;
	float field_0x8;
	float field_0xc;
	edF32VECTOR4 vector_0x10;
	char* pScratchpadA;
	char* pScratchpadB;
	uint flags_0x28;
	uint lineCount;
	int field_0x30;
	int field_0x34;
	undefined* field_0x38[2];
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
	undefined field_0x4c;
	undefined field_0x4d;
	undefined field_0x4e;
	undefined field_0x4f;
	undefined field_0x50;
	undefined field_0x51;
	undefined field_0x52;
	undefined field_0x53;
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
