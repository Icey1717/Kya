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
#ifdef PLATFORM_WIN
#pragma pack(pop)
#endif

struct CameraObj_28;

struct CameraObj_130 {
	struct Matrix matrix_0x0;
	struct Matrix matrix_0x40;
	struct Matrix matrix_0x80;
	undefined field_0xc0;
	undefined field_0xc1;
	undefined field_0xc2;
	undefined field_0xc3;
	undefined field_0xc4;
	undefined field_0xc5;
	undefined field_0xc6;
	undefined field_0xc7;
	undefined field_0xc8;
	undefined field_0xc9;
	undefined field_0xca;
	undefined field_0xcb;
	undefined field_0xcc;
	undefined field_0xcd;
	undefined field_0xce;
	undefined field_0xcf;
	undefined field_0xd0;
	undefined field_0xd1;
	undefined field_0xd2;
	undefined field_0xd3;
	undefined field_0xd4;
	undefined field_0xd5;
	undefined field_0xd6;
	undefined field_0xd7;
	undefined field_0xd8;
	undefined field_0xd9;
	undefined field_0xda;
	undefined field_0xdb;
	undefined field_0xdc;
	undefined field_0xdd;
	undefined field_0xde;
	undefined field_0xdf;
	undefined field_0xe0;
	undefined field_0xe1;
	undefined field_0xe2;
	undefined field_0xe3;
	undefined field_0xe4;
	undefined field_0xe5;
	undefined field_0xe6;
	undefined field_0xe7;
	undefined field_0xe8;
	undefined field_0xe9;
	undefined field_0xea;
	undefined field_0xeb;
	undefined field_0xec;
	undefined field_0xed;
	undefined field_0xee;
	undefined field_0xef;
	undefined field_0xf0;
	undefined field_0xf1;
	undefined field_0xf2;
	undefined field_0xf3;
	undefined field_0xf4;
	undefined field_0xf5;
	undefined field_0xf6;
	undefined field_0xf7;
	undefined field_0xf8;
	undefined field_0xf9;
	undefined field_0xfa;
	undefined field_0xfb;
	undefined field_0xfc;
	undefined field_0xfd;
	undefined field_0xfe;
	undefined field_0xff;
	undefined field_0x100;
	undefined field_0x101;
	undefined field_0x102;
	undefined field_0x103;
	undefined field_0x104;
	undefined field_0x105;
	undefined field_0x106;
	undefined field_0x107;
	undefined field_0x108;
	undefined field_0x109;
	undefined field_0x10a;
	undefined field_0x10b;
	undefined field_0x10c;
	undefined field_0x10d;
	undefined field_0x10e;
	undefined field_0x10f;
	struct Vector vector_0x110;
	undefined field_0x120;
	undefined field_0x121;
	undefined field_0x122;
	undefined field_0x123;
	undefined field_0x124;
	undefined field_0x125;
	undefined field_0x126;
	undefined field_0x127;
	undefined field_0x128;
	undefined field_0x129;
	undefined field_0x12a;
	undefined field_0x12b;
	undefined field_0x12c;
	undefined field_0x12d;
	undefined field_0x12e;
	undefined field_0x12f;
};

struct StaticMeshMaster;

namespace ed3D
{
	void Init(void);
	TextureInfoSmall* LoadTextureFromBuffer(char* fileBufferStart, int fileLength, int* outInt, TextureInfoSmall* pTextureInfo, ulong param_5);
	MaterialInfo* GetMaterialInfoFromTexture(MaterialInfo* outObj, int count, TextureInfoSmall* textureInfoObj, int mode);
	StaticMeshMaster* SetupCameraPanStaticMaster_002b4600(CameraObj_130* param_1, CameraObj_28* param_2, long mode);

	extern int g_FrameBufferPtr_004491f0;
}

#endif //_ED3D_H
