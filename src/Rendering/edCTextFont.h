#ifndef _EDCTEXTFONT_H
#define _EDCTEXTFONT_H

#include "Types.h"
#include "ed3D.h"
#include "edDlist.h"

PACK(
	struct SegmentPacked {
	ushort header;
	ushort field_0x2;
});

struct FontPacked_2C {
	void* pGlyphData;
	void* field_0x4;
	SegmentPacked* pSegment;
	edDList_material materialInfo;
	void* field_0x1c;
	void* field_0x20;
	ed_g2d_manager* pTextureInfo;
	ushort* pOverrideData;
};

PACK(
	struct Segment_1C_Packed {
	float field_0x0;
	float field_0x4;
	float field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;
	float field_0x18;
});

PACK(
	struct edCTextFont {
	char header[4];
	short field_0x4;
	short field_0x6;
#ifdef _WIN64
	int pSubData;
#else
	struct FontPacked_2C* pSubData;
#endif
	ushort field_0xc;
	undefined field_0xe;
	undefined field_0xf;
	ushort field_0x10;
	undefined field_0x12;
	undefined field_0x13;
	ushort field_0x14;
	ushort field_0x16;
	ushort count_0x18;
	ushort pageOffset;
	char glyphHeader[4];
	Segment_1C_Packed* GetSymbol(uint character);
	float GetRelativeAlignment(ulong param_2, ulong param_3);
});

#endif // _EDCTEXTFONT_H
