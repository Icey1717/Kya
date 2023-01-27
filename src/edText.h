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

struct FontPacked_2C {
	void* pGlyphData;
	void* field_0x4;
	struct SegmentPacked* pSegment;
	struct MaterialInfo materialInfo;
	void* field_0x1c;
	void* field_0x20;
	struct TextureInfoSmall* pTextureInfo;
	ushort* pOverrideData;
};

#ifdef PLATFORM_PS2
struct __attribute__((__packed__)) 
#else
#pragma pack(push,1)
struct 
#endif
FontPacked {
	char header[4];
	short field_0x4;
	short field_0x6;
	struct FontPacked_2C* pSubData;
#ifdef _WIN64
	int padding[3];
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
};
#ifdef PLATFORM_WINDOWS
#pragma pack(pop)
#endif


#ifdef PLATFORM_PS2
struct __attribute__((__packed__))
#else
#pragma pack(push,1)
struct
#endif
SegmentPacked {
	ushort header;
	ushort field_0x2;
};
#ifdef PLATFORM_WINDOWS
#pragma pack(pop)
#endif

bool Init_edText(void);

#endif //_EDTEXT_H
