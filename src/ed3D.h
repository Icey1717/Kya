#ifndef _ED3D_H
#define _ED3D_H

#include "Types.h"

#ifdef PLATFORM_WIN
#include "delegate.h"
#endif

PACK(
	struct ed_hash_code {
	Hash_8 field_0x0;
	int pData; // char*
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
};)

struct ed_g2d_manager {
	char* textureFileBufferStart;
	int textureFileLengthA;
	char* textureHeaderStart;
	char* pMAT_HASH;
	char* pT2DA;
	char* pPALL;
	byte field_0x18;
	byte field_0x19;
	byte field_0x1a;
	byte field_0x1b;
	int textureFileLengthB;
	char* pANMA;
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
	struct edF32VECTOR4* pLightAmbient;
	union edF32MATRIX4* pLightDirections;
	union edF32MATRIX4* pLightColor;
};

struct ed_3d_hierarchy_setup {
	char* field_0x0;
	struct edF32VECTOR4* pVector_0x4;
	struct LightingMatrixSubSubObj* pLightData;
	char* field_0xc;
	float* field_0x10;
};

struct MeshTransformDataBase {
	edF32MATRIX4 transformA;
	edF32MATRIX4 transformB;
	Hash_8 hash;
	byte field_0x88;
	undefined field_0x89;
	ushort bRenderShadow;
	union edF32MATRIX4* pShadowAnimMatrix;
	struct ed_g3d_hierarchy* pLinkTransformData;
	undefined* field_0x94;
	undefined* pTextureInfo;
	ushort count_0x9c;
	ushort flags_0x9e;
	struct ed_3d_hierarchy_setup* pHierarchySetup;
	edF32MATRIX4* pMatrixPkt;
	union edF32MATRIX4* pAnimMatrix;
	short subMeshParentCount_0xac;
	byte size_0xae;
	char GlobalAlhaON;
};

struct MeshTransformObjData {
	char* pObj;
	short field_0x4;
	short field_0x6;
};

struct ed_3d_hierarchy_node {
	MeshTransformDataBase base;
	MeshTransformObjData aSubArray[4];
};

struct TextureInfo {
	ed_g2d_manager field_0x0;
	char* pFileBuffer;
};

struct ed3DConfig {
	ed3DConfig();

	int meshHeaderCountB;
	int sceneCount;
	uint maxClusterCount;
	int meshHeaderCountBAlt;
	int matrixBufferCount;
	int materialBufferCount;
	int field_0x18;
	int g3dManagerCount;
	int g2dManagerCount;
	byte bEnableProfile;
	byte field_0x25;
	byte clusterTabListSize;
	byte field_0x27;
	int field_0x28;
	byte field_0x2c;
	byte field_0x2d;
	byte field_0x2e;
	byte field_0x2f;
	int meshTransformDataCount;
	undefined4 field_0x34;
};

PACK(
	struct ed_g2d_material {
	byte count_0x0;
	undefined field_0x1;
	ushort field_0x2;
	int pDMA_Material; // ed_dma_material*
	int pCommandBufferTexture; // RenderCommand*
	int commandBufferTextureSize;
});

PACK(
	struct TextureData_MAT {
	Hash_4 header;
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
	ed_g2d_material body;
});

struct edDList_material {
	struct ed_g2d_manager* textureInfo;
	ed_g2d_material* pMAT;
	int mode;
	int Length;
};

struct edPSX2Header
{
	int pPkt;
	int field_0x4;
};

PACK(
	struct ed_g2d_layer {
	uint flags_0x0;
	uint flags;
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
	undefined field_0x17;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	short field_0x1c;
	ushort field_0x1e;
	int pTex; // TextureData_TEX*
});

PACK(
	struct ed_g2d_layer_header {
	Hash_4 header;
	uint field_0x4;
	undefined field_0x8;
	undefined field_0x9;
	undefined field_0xa;
	undefined field_0xb;
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
	ed_g2d_layer body;
});


PACK(struct ed_g2d_bitmap {
	ushort width;
	ushort height;
	ushort psm;
	ushort maxMipLevel;
	int pPSX2; //edpkt_data*
});

PACK(struct TextureData_MATA {
	Hash_4 header;
	undefined4 field_0x4;
	undefined4 size_0x8;
	int field_0xc;
});

PACK(struct TextureData_PA32 {
	Hash_4 header;
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
	ed_g2d_bitmap body;
});

PACK(
	struct TextureData_HASH_Internal_MAT {
	undefined field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	int pMAT; // TextureData_MAT*
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
});

PACK(
	struct TextureData_HASH_Internal_PA32 {
	undefined field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	int pPA32; // TextureData_PA32*
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
});

PACK(
	struct TextureData_TEX_Internal {
	ed_hash_code hashCode;
	int palette;
	int field_0x14; // edF32VECTOR4*
	float field_0x18;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
});

PACK(
	struct TextureData_TEX {
	undefined field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
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
	TextureData_TEX_Internal body;
});

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

struct ed_viewport;
struct edFCamera;

struct ed_3D_Scene;

struct ed_g3d_manager;
struct DisplayListInternal;
struct edCluster;
struct edNODE;

#ifdef PLATFORM_WIN
Multidelegate<edDList_material*>& ed3DGetMaterialLoadedDelegate();
Multidelegate<edDList_material*>& ed3DGetMaterialUnloadedDelegate();
Multidelegate<ed_g2d_manager*>& ed3DGetTextureLoadedDelegate();
#endif

ed_g3d_manager* ed3DInstallG3D(char* pFileData, int fileLength, ulong flags, int* outInt, TextureInfo* textureObj, int unknown, ed_g3d_manager* pMeshInfo);


void Init3D(void);
ed_g2d_manager* ed3DInstallG2D(char* fileBufferStart, int fileLength, int* outInt, ed_g2d_manager* pTextureInfo, ulong param_5);
TextureData_HASH_Internal_MAT* ed3DG2DGetMaterialFromIndex(ed_g2d_manager* pTextureInfo, int index);
edDList_material* edDListCreatMaterialFromIndex(edDList_material* outObj, int index, ed_g2d_manager* textureInfoObj, int mode);
ed_g2d_material* ed3DG2DGetG2DMaterialFromIndex(ed_g2d_manager* pTextureInfo, int index);
bool edDListTermMaterial(edDList_material* pMaterial);
char* ed3DG2DGetBitmapFromMaterial(ed_g2d_material* pMAT_Internal, int param_2);
ed_3D_Scene* ed3DSceneCreate(edFCamera* pCamera, ed_viewport* pViewport, long mode);
edNODE* ed3DHierarchyAddToSceneByHashcode(ed_3D_Scene* pStaticMeshMaster, ed_g3d_manager* pMeshInfo, ulong hash);

void ed3DLinkG2DToG3D(ed_g3d_manager* pMeshInfo, TextureInfo* pTextureInfo);

struct HierarchyAnm {
	void Install(struct MeshData_ANHR* pInANHR, int length, ed_g3d_manager* pMeshInfo, ed_3D_Scene* pStaticMeshMaster);
	uint UpdateMatrix(float param_1, ed_3d_hierarchy_node* param_3, int* pFileData, int param_5);

	MeshData_ANHR* pThis;

	static edF32MATRIX4 _gscale_mat;
};


struct FxFogProp {
	uint field_0x0;
	uint field_0x4;
	int field_0x8;
	int field_0xc;
	int field_0x10;
	uint field_0x14;
};

void ed3DHierarchyCopyHashCode(ed_g3d_manager* pMeshInfo);

void ed3DScenePushCluster(ed_3D_Scene* pStaticMeshMaster, ed_g3d_manager* pMeshInfo);

uint edChunckGetNb(char* pStart, char* pEnd);
char* edHashcodeGet(Hash_8 meshHashValue, struct MeshData_HASH* textureObjMatBuffer);

edpkt_data* ed3DFlushFullAlphaTerm(edpkt_data* pRenderCommand);
edpkt_data* ed3DFlushFullAlphaInit(edpkt_data* pRenderCommand);

void ed3DSceneComputeCameraToScreenMatrix(ed_3D_Scene* pScene, edF32MATRIX4* m0);

void ed3DResetTime(void);
void ed3DSetDeltaTime(int newTime);

FxFogProp* ed3DGetFxFogProp(void);

extern int gFXBufAddr;
extern byte BYTE_00448a5c;
extern edCluster* gCluster;
extern edNODE* gHierarchyManagerFirstFreeNode;
extern ed_3d_hierarchy_node* gHierarchyManagerBuffer;

extern int gNbVertexDMA;

#endif //_ED3D_H
