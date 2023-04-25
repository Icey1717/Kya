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

struct LightingMatrixSubSubObj {
	struct Vector* field_0x0;
	struct Matrix* field_0x4;
	struct Matrix* field_0x8;
};

struct LightingMatrixFuncObj {
	char* field_0x0;
	struct Vector* pVector_0x4;
	struct LightingMatrixSubSubObj* field_0x8;
	char* field_0xc;
	float* field_0x10;
};

struct MeshTransformDataBase {
	struct Matrix transformA;
	struct Matrix transformB;
	Hash_8 hash;
	byte field_0x88;
	undefined field_0x89;
	ushort bRenderShadow;
	struct Matrix* pShadowAnimMatrix;
	struct MeshTransformData* pLinkTransformData;
	undefined* field_0x94;
	undefined* pTextureInfo;
	ushort count_0x9c;
	ushort flags_0x9e;
	struct LightingMatrixFuncObj* pLightingMatrixFuncObj_0xa0;
	Matrix* field_0xa4;
	struct Matrix* pAnimMatrix;
	short subMeshParentCount_0xac;
	byte size_0xae;
	char field_0xaf;
};

struct MeshTransformObjData {
	char* pObj;
	short field_0x4;
	short field_0x6;
};

struct MeshTransformData {
	MeshTransformDataBase base;
	MeshTransformObjData aSubArray[4];
};

struct TextureInfo {
	TextureInfoSmall field_0x0;
	char* pFileBuffer;
};

struct ed3D_Params {
	ed3D_Params();

	int meshHeaderCountB;
	int staticMeshMasterCount;
	uint meshDisplayListInternalCount;
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

PACK(struct MeshData_HASH {
	Hash_4 header;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	undefined field_0x8;
	undefined field_0x9;
	undefined field_0xa;
	undefined field_0xb;
	int field_0xc;
});

PACK( struct LayerHeaderPacked {
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
});

struct CameraObj_28;
struct CameraObj_130;

struct StaticMeshMaster;

struct MeshInfo;
struct DisplayListInternal;
struct DisplayListInternalMesh;
struct MeshTransformParent;

extern Matrix SomeIdentityMatrix;

namespace ed3D
{
	void Init(void);
	TextureInfoSmall* LoadTextureFromBuffer(char* fileBufferStart, int fileLength, int* outInt, TextureInfoSmall* pTextureInfo, ulong param_5);
	MaterialInfo* GetMaterialInfoFromTexture(MaterialInfo* outObj, int count, TextureInfoSmall* textureInfoObj, int mode);
	char* GetMAT_Section_0029eae0(TextureInfoSmall* pTextureInfo, int param_2);
	char* GetT2D_Section_0029ea60(char* pMaterialSection, int param_2);
	StaticMeshMaster* FindFreeStaticMeshMaster_002b4600(CameraObj_130* param_1, CameraObj_28* param_2, long mode);
	MeshInfo* LoadMeshFromBuffer(char* pFileData, int fileLength, ulong flags, int* outInt, TextureInfo* textureObj, int unknown, MeshInfo* pMeshInfo);
	MeshTransformParent* SetupMeshTransform_001fffe0(StaticMeshMaster* pStaticMeshMaster, MeshInfo* pMeshInfo, ulong hash);

	void CheckHashesProcessCSTA_002a4ec0(MeshInfo* pMeshInfo, TextureInfo* pTextureInfo);
	void SetupANHR_001fd180(struct MeshData_ANHR** ppOutANHR, struct MeshData_ANHR* pInANHR, int length, MeshInfo* pMeshInfo,
		StaticMeshMaster* pStaticMeshMaster);

	void SetupHIER_002abc00(MeshInfo* pMeshInfo);

	void Func_002a5540(StaticMeshMaster* pStaticMeshMaster, MeshInfo* pMeshInfo);

	uint GetElementCount_002947b0(char* pStart, char* pEnd);
	char* FindSectionHash(Hash_8 meshHashValue, struct MeshData_HASH* textureObjMatBuffer);

	extern int g_FrameBufferPtr_004491f0;
	extern byte BYTE_00448a5c;
	extern int INT_0044935c;
	extern DisplayListInternalMesh* g_MeshDisplayListInternal_00449380;
	extern MeshTransformParent* PTR_MeshTransformParent_ARRAY_00449434;
	extern MeshTransformData* PTR_MeshTransformData_ARRAY_0044942c;
}

#endif //_ED3D_H
