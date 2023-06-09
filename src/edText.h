#ifndef _EDTEXT_H
#define _EDTEXT_H

#include "Types.h"
#include "ed3D.h"

struct PagePacked {
	char header[4];
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	undefined field_0x8;
	undefined field_0x9;
	undefined field_0xa;
	undefined field_0xb;
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
	undefined field_0x10;
	undefined field_0x11;
	undefined field_0x12;
	undefined field_0x13;
	undefined field_0x14;
	undefined field_0x15;
	undefined field_0x16;
};

bool edTextInit(void);
bool edTextInstallFont(struct edCTextFont* pFontData);
void edTextDraw(float x, float y, char* text);

#endif //_EDTEXT_H
