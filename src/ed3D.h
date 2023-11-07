#ifndef _ED3D_H
#define _ED3D_H

#include "Types.h"

#ifdef PLATFORM_WIN
#include "delegate.h"
#endif

union AnimScratchpad {
	struct {
		int field_0x0;
		uint flags;
		undefined4 field_0x8;
		undefined4 field_0xc;
	};
	edpkt_data pkt;
};

PACK(
	struct ed_hash_code {
	Hash_8 hash;
	int pData; // char*
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
};)

PACK(struct ed_Chunck {
	uint hash;
	short field_0x4;
	short field_0x6;
	int size;
	int field_0xc;
});

struct ed_g2d_manager {
	char* textureFileBufferStart;
	int textureFileLengthA;
	ed_Chunck* textureHeaderStart;
	ed_Chunck* pMAT_HASH;
	ed_Chunck* pT2DA;
	ed_Chunck* pPALL;
	byte field_0x18;
	byte field_0x19;
	byte field_0x1a;
	byte field_0x1b;
	int textureFileLengthB;
	ed_Chunck* pANMA;
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
	union edF32VECTOR4* pLightAmbient;
	union edF32MATRIX4* pLightDirections;
	union edF32MATRIX4* pLightColor;
};

struct ed_3d_hierarchy_setup {
	char* clipping_0x0;
	union edF32VECTOR4* pBoundingSphere;
	struct LightingMatrixSubSubObj* pLightData;
	ed_3d_hierarchy_setup* pNext;
	float* field_0x10;
};

struct ed_3d_hierarchy {
	edF32MATRIX4 transformA;
	edF32MATRIX4 transformB;
	Hash_8 hash;
	byte field_0x88;
	undefined field_0x89;
	ushort bRenderShadow;
	union edF32MATRIX4* pShadowAnimMatrix;
	struct ed_3d_hierarchy* pLinkTransformData;
	undefined* field_0x94;
	undefined* pTextureInfo;
	ushort lodCount;
	ushort flags_0x9e;
	struct ed_3d_hierarchy_setup* pHierarchySetup;
	edF32MATRIX4* pMatrixPkt;
	union edF32MATRIX4* pAnimMatrix;
	short subMeshParentCount_0xac;
	byte size_0xae;
	char GlobalAlhaON;
};

struct MeshTransformObjData {
	ed_hash_code* pObj;
	short field_0x4;
	short field_0x6;
};

struct ed_3d_hierarchy_node {
	ed_3d_hierarchy base;
	MeshTransformObjData aSubArray[4];
};

struct TextureInfo {
	ed_g2d_manager pManager;
	char* pFileBuffer;
};

struct ScratchPadRenderInfo {
	edF32MATRIX4* pSharedMeshTransform;
	edF32MATRIX4* pMeshTransformMatrix;
	int boundingSphereTestResult;
	ed_3d_hierarchy_setup* pHierarchySetup;
	uint flags;
	float field_0x14;
	edpkt_data* pPkt;
	ed_3d_hierarchy_node* pMeshTransformData;
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

struct edPSX2Header
{
	int pPkt;
	int field_0x4;
};

PACK(
	struct ed_g2d_layer {
	uint flags_0x0;
	uint flags_0x4;
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

ed_g3d_manager* ed3DInstallG3D(char* pFileData, int fileLength, ulong flags, int* outInt, ed_g2d_manager* textureObj, int unknown, ed_g3d_manager* pMeshInfo);


void Init3D(void);
ed_g2d_manager* ed3DInstallG2D(char* fileBufferStart, int fileLength, int* outInt, ed_g2d_manager* pTextureInfo, ulong param_5);
ed_hash_code* ed3DG2DGetMaterialFromIndex(ed_g2d_manager* pTextureInfo, int index);
ed_g2d_material* ed3DG2DGetG2DMaterialFromIndex(ed_g2d_manager* pTextureInfo, int index);
char* ed3DG2DGetBitmapFromMaterial(ed_g2d_material* pMAT_Internal, int param_2);
ed_3D_Scene* ed3DSceneCreate(edFCamera* pCamera, ed_viewport* pViewport, long mode);
edNODE* ed3DHierarchyAddToScene(ed_3D_Scene* pScene, ed_g3d_manager* pG3D, char* szString);
edNODE* ed3DHierarchyAddToSceneByHashcode(ed_3D_Scene* pStaticMeshMaster, ed_g3d_manager* pMeshInfo, ulong hash);
void ed3DHierarchyRefreshSonNumbers(edNODE* pMeshTransformParent, short* outMeshCount);

void ed3DLinkG2DToG3D(ed_g3d_manager* pMeshInfo, ed_g2d_manager* pTextureInfo);

PACK(
struct S_HIERANM_ANIM {
	int field_0x0;
	int field_0x4;
	int field_0x8;
	float field_0xc[];
});

class CHierarchyAnm {
public:
	void Install(struct MeshData_ANHR* pInANHR, int length, ed_g3d_manager* pMeshInfo, ed_3D_Scene* pStaticMeshMaster);
	bool UpdateMatrix(float param_1, edF32MATRIX4* pMatrix, S_HIERANM_ANIM* pHierAnim, int param_5);
	void Manage(float param_1, float param_2, ed_3D_Scene* pScene, int param_5);

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

PACK(struct ed3DLod {
	int pObj; // char*
	short field_0x4;
	short field_0x6;
});

PACK(struct ed_g3d_hierarchy {
	edF32MATRIX4 transformA;
	edF32MATRIX4 transformB;
	Hash_8 hash;
	byte field_0x88;
	byte field_0x89;
	ushort bRenderShadow;
	int pShadowAnimMatrix; // edF32MATRIX4*
	int pLinkTransformData; // ed_3d_hierarchy*
	int field_0x94; // undefined*
	int pTextureInfo; // undefined*
	ushort lodCount;
	ushort flags_0x9e;
	int pHierarchySetup; // ed_3d_hierarchy_setup*
	int field_0xa4; // edF32MATRIX4*
	int pAnimMatrix; // edF32MATRIX4*
	short subMeshParentCount_0xac;
	byte size_0xae;
	char GlobalAlhaON;
	ed3DLod aLods[4];
});

struct edVertex {
	float x;
	float y;
	float z;
	uint skip;
};

PACK(
	struct ed_Bound_Sphere_packet {
	edF32VECTOR3 field_0x0;
	ushort field_0xc;
	undefined field_0xe;
	undefined field_0xf;
}
);

PACK(
	struct ed_3d_strip {
	uint flags_0x0;
	short materialIndex;
	short field_0x6;
	int vifListOffset;
	int pNext; // ed_3d_strip*
	edF32VECTOR4 boundingSphere;
	int pSTBuf; // char*
	int pColorBuf; // _rgba*
	int pVertexBuf; // edVertex*
	int field_0x2c; // char*
	short shadowCastFlags;
	short shadowReceiveFlags;
	int pDMA_Matrix; // ed_dma_matrix*
	byte field_0x38;
	byte cameraPanIndex;
	short meshSectionCount_0x3a;
	int pBoundSpherePkt; // ed_Bound_Sphere_packet*
});

void ed3DHierarchyCopyHashCode(ed_g3d_manager* pMeshInfo);

void ed3DScenePushCluster(ed_3D_Scene* pStaticMeshMaster, ed_g3d_manager* pMeshInfo);

uint edChunckGetNb(char* pStart, char* pEnd);
char* edHashcodeGet(Hash_8 meshHashValue, ed_Chunck* pChunck);

edpkt_data* ed3DFlushFullAlphaTerm(edpkt_data* pRenderCommand);
edpkt_data* ed3DFlushFullAlphaInit(edpkt_data* pRenderCommand);

void ed3DSceneComputeCameraToScreenMatrix(ed_3D_Scene* pScene, edF32MATRIX4* m0);

void ed3DResetTime(void);
void ed3DSetDeltaTime(int newTime);

FxFogProp* ed3DGetFxFogProp(void);

ed_g3d_hierarchy* ed3DG3DHierarchyGetFromIndex(ed_g3d_manager* pMeshInfo, int count);
void ed3DG3DHierarchySetStripShadowCastFlag(ed_g3d_hierarchy* pHier, ushort flag);
void ed3DSetMeshTransformFlag_002abd80(edNODE* pNode, ushort flag);
void ed3DSetMeshTransformFlag_002abff0(edNODE* pNode, ushort flag);
void ed3DHierarchySetSetup(ed_3d_hierarchy* pHier, ed_3d_hierarchy_setup* pHierarchySetup);

struct SceneConfig;
struct edSurface;
struct ed_surface_desc;

SceneConfig* ed3DSceneGetConfig(ed_3D_Scene* pStaticMeshMaster);
edSurface* ed3DShadowSurfaceNew(ed_surface_desc* pVidModeData);

void ed3DHierarchyNodeSetRenderOn(ed_3D_Scene* pScene, edNODE* pNode);
void ed3DHierarchyNodeSetRenderOff(ed_3D_Scene* pScene, edNODE* pNode);

void ed3DLinkStripToViewport(ed_3d_strip* pStrip, edF32MATRIX4* pMatrix, ed_hash_code* pHash, edpkt_data* pPkt);

#ifdef PLATFORM_WIN
void ProcessTextureCommands(edpkt_data* aPkt, int size);
#endif

extern int gFXBufAddr;
extern byte BYTE_00448a5c;
extern edCluster* gCluster;
extern edNODE* gHierarchyManagerFirstFreeNode;
extern ed_3d_hierarchy_node* gHierarchyManagerBuffer;

extern edpkt_data g_stMatrixHeader;
extern edpkt_data g_stVertexGIFHeader;
extern edpkt_data g_stVertexSTHeader[2];
extern edpkt_data g_stGifTAG_Texture_NoFog[97];
extern edpkt_data g_stGifTAG_Gouraud_NoFog[97];
extern edpkt_data g_stVertexRGBAHeader;
extern edpkt_data g_stVertexXYZHeader;
extern edpkt_data g_stExecuteCode;

#ifdef PLATFORM_WIN
Multidelegate<ed_g2d_manager*>& ed3DGetTextureLoadedDelegate();
#endif

#define SHELLDMA_TAG_ID_CNT (0x10)
#define SHELLDMA_TAG_ID_REF (0x30)

#define ED_VIF1_SET_TAG_CNT(qwc) ((ulong)(qwc) | ((ulong)SHELLDMA_TAG_ID_CNT << 24) | ((ulong)0xe << 32))
#define ED_VIF1_SET_TAG_REF(qwc, addr) ((ulong)((ulong)(qwc) | ((ulong)SHELLDMA_TAG_ID_REF << 24))) | ((ulong)(addr) << 32)

extern int gNbVertexDMA;

#endif //_ED3D_H
