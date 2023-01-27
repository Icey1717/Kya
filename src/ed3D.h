#ifndef _ED3D_H
#define _ED3D_H

#include "Types.h"

struct TextureInfoSmall {
	char* textureFileBufferStart;
	int textureFileLengthA;
	char* textureHeaderStart;
	char* materialBufferStart;
	char* textureBufferStart;
	char* palleteBufferStart;
	byte field_0x18;
	byte field_0x19;
	byte field_0x1a;
	byte field_0x1b;
	int textureFileLengthB;
	char* animationBufferStart;
	undefined field_0x24;
	undefined field_0x25;
	undefined field_0x26;
	undefined field_0x27;
	undefined field_0x28;
	undefined field_0x29;
	undefined field_0x2a;
	undefined field_0x2b;
	undefined field_0x2c;
	undefined field_0x2d;
	undefined field_0x2e;
	undefined field_0x2f;
};

struct TextureInfo {
	TextureInfoSmall a;
	char* pFileBuffer;
};

struct ed3D_Params {
	ed3D_Params();

	int meshHeaderCountB;
	int initialStaticMeshMasterCount;
	int field_0x8;
	int meshHeaderCountBAlt;
	int meshHeaderCountA;
	int field_0x14;
	int field_0x18;
	int meshHeaderCountAA;
	int meshHeaderCountBB;
	byte bEnableProfile;
	byte field_0x25;
	byte field_0x26;
	byte field_0x27;
	int field_0x28;
	byte field_0x2c;
	byte field_0x2d;
	byte field_0x2e;
	byte field_0x2f;
	int meshTransformDataCount;
	undefined4 field_0x34;
};

struct MaterialInfo {
	struct TextureInfoSmall* textureInfo;
	char* matSubsectionStart;
	int mode;
	int Length;
};


#ifdef PLATFORM_PS2
struct __attribute__((__packed__))
#else
#pragma pack(push,1)
struct
#endif
LayerHeaderPacked {
	undefined field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	int field_0x8;
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
	char field_0x10;
};
#ifdef PLATFORM_WINDOWS
#pragma pack(pop)
#endif

namespace ed3D
{
	void Init(void);
	TextureInfoSmall* LoadTextureFromBuffer(char* fileBufferStart, int fileLength, int* outInt, TextureInfoSmall* pTextureInfo, ulong param_5);
	MaterialInfo* GetMaterialInfoFromTexture(MaterialInfo* outObj, int count, TextureInfoSmall* textureInfoObj, int mode);
}

#endif //_ED3D_H
