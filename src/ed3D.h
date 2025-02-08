#ifndef _ED3D_H
#define _ED3D_H

#include "Types.h"

#ifdef PLATFORM_WIN
#include "delegate.h"
#endif

#include "edList.h"
#include "LightManager.h"

#define HASH_CODE_HASH 0x48534148
#define HASH_CODE_MBNK 0x4b4e424d

#define HASH_CODE_CDQA 0x41514443
#define HASH_CODE_CDQU 0x55514443
#define HASH_CODE_CDOA 0x414f4443
#define HASH_CODE_CDOC 0x434f4443

#define HASH_CODE_HIER 0x52454948


#define MATRIX_PACKET_START_SPR				0x70000800 // 0x06
#define CAM_NORMAL_X_SPR					0x70000800 // 0x06
#define CAM_NORMAL_Y_SPR					0x70000810 // 0x07
#define OBJ_TO_CULLING_MATRIX				0x70000820 // 0x08
#define OBJ_TO_CLIPPING_MATRIX				0x70000860 // 0x0c
#define OBJ_TO_SCREEN_MATRIX				0x700008A0 // 0x10
#define OBJECT_TO_CAMERA_MATRIX_SPR			0x700008E0 // 0x14

#define OBJ_LIGHT_DIRECTIONS_MATRIX_SPR		0x70000920 // 0x18
#define LIGHT_COLOR_MATRIX_SPR				0x70000950 // 0x1b
#define ADJUSTED_LIGHT_AMBIENT_SPR			0x700009A0 // 0x20
#define ANIM_ST_NORMAL_EXTRUDER_SPR			0x700009B0 // 0x21
#define OBJ_TO_WORLD_INVERSE_NORMAL_SPR		0x70000A00 // 0x26

#define LIGHT_DIRECTIONS_MATRIX_PTR_SPR		0x70000a40 // 0x2a
#define LIGHT_COLOR_MATRIX_PTR_SPR			0x70000a50 // 0x2b
#define LIGHT_AMBIENT_MATRIX_PTR_SPR		0x70000a60 // 0x2c

#define SCENE_FLAG_IN_USE		0x1
#define SCENE_FLAG_CAST_SHADOW	0x2
#define SCENE_FLAG_FOG_PROPERTY	0x40

#ifdef PLATFORM_PS2
#define SCRATCHPAD_ADDRESS(addr) (edpkt_data*)addr
#define SCRATCHPAD_ADDRESS_TYPE(addr, type) (type)addr
#define SCRATCHPAD_READ_ADDRESS_TYPE(addr, type) (type)addr

#define SCRATCHPAD_WRITE_ADDRESS_TYPE(addr, type, value) *((type*)addr) = value;
#else
#define SCRATCHPAD_ADDRESS(addr) (edpkt_data*)((addr - 0x70000000) + (char*)WorldToCamera_Matrix)
#define SCRATCHPAD_ADDRESS_TYPE(addr, type) (type)((addr - 0x70000000) + (char*)WorldToCamera_Matrix)
#define SCRATCHPAD_READ_ADDRESS_TYPE(addr, type) *(type*)((addr - 0x70000000) + (char*)WorldToCamera_Matrix)

#define SCRATCHPAD_WRITE_ADDRESS_TYPE(addr, type, value) do { \
	type* destPtr = (type*)((addr - 0x70000000) + (char*)WorldToCamera_Matrix); \
	*destPtr = value; \
} while(0)

#endif

union AnimScratchpad {
	struct {
		int vuFlags;
		uint flags;
		undefined4 field_0x8;
		undefined4 field_0xc;
	};
	edpkt_data pkt;
};

struct ed_hash_code {
	Hash_8 hash;
	int pData; // char*
	char _pad[4];
};

static_assert(sizeof(ed_hash_code) == 0x10, "Invalid ed_hash_code size");

struct __attribute__((aligned(16))) ed_Chunck {
	uint hash;
	short field_0x4;
	short field_0x6;
	int size;
	int nextChunckOffset;

	// Debug
	inline std::string GetHeaderString() const {
		// convert hash into chars
		char hashStr[5];
		memcpy(hashStr, &hash, 4);
		hashStr[4] = 0;
		return std::string(hashStr);
	}
};

static_assert(sizeof(ed_Chunck) == 0x10, "Invalid ed_Chunck size");

struct GXD_FileHeader
{
	undefined4 field_0x0;
	uint flags;
	undefined4 field_0x8;
	uint hash;
};

static_assert(sizeof(GXD_FileHeader) == 0x10);

struct ed_g3d_manager {
	GXD_FileHeader* fileBufferStart;
	char* field_0x4;
	int fileLengthA;
	undefined4 field_0xc;
	ed_Chunck* OBJA;
	ed_Chunck* LIA;
	ed_Chunck* CAMA;
	ed_Chunck* SPRA;
	ed_Chunck* HALL;
	ed_Chunck* CSTA;
	ed_Chunck* GEOM;
	ed_Chunck* MBNA;
	ed_Chunck* INFA;
	int fileLengthB;
	ed_Chunck* CDZA;
	ed_Chunck* ANMA;
};
struct ed_g2d_manager {
	GXD_FileHeader* pFileBuffer;
	int textureFileLengthA;
	ed_Chunck* pTextureChunk;
	ed_Chunck* pMATA_HASH;
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


struct FrustumData {
	edF32MATRIX4 frustumMatrix;
	edF32VECTOR4 field_0x40;
	edF32VECTOR4 field_0x50;
};

struct ed_3D_Shadow_Config {
	undefined4 field_0x0;
	float field_0x4;
	float field_0x8;
	float field_0xc;
	struct ed_viewport* pCamera_0x10;
	struct ed_viewport* pViewport;
	int texWidth;
	int texHeight;
	undefined2 renderMask;
	byte field_0x22;
	byte field_0x23;
	byte field_0x24;
};

struct SceneConfig {
	int field_0x0; // undefined* 
	float clipValue_0x4;
	float nearClip;
	float farClip;
	uint projectionScaleFactorA;
	uint projectionScaleFactorB;
	float clipValue_0x18;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
	FrustumData frustumA;
	FrustumData frustumB;
	edF32VECTOR4 field_0xe0;
	ByteColor3 fogCol_0xf0;
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
	ed_3D_Shadow_Config pShadowConfig;
	undefined field_0x125;
	undefined field_0x126;
	undefined field_0x127;
	int field_0x128;
	int field_0x12c;
	ed_3D_Light_Config pLightConfig;
	undefined field_0x13c;
	undefined field_0x13d;
	undefined field_0x13e;
	undefined field_0x13f;
	undefined field_0x140;
	undefined field_0x141;
	undefined field_0x142;
	undefined field_0x143;
	undefined field_0x144;
	undefined field_0x145;
	undefined field_0x146;
	undefined field_0x147;
	undefined field_0x148;
	undefined field_0x149;
	undefined field_0x14a;
	undefined field_0x14b;
	undefined field_0x14c;
	undefined field_0x14d;
	undefined field_0x14e;
	undefined field_0x14f;
};

struct ed_3d_octree {
	edF32VECTOR4 field_0x0;
	edF32VECTOR4 worldLocation;
	ed_Chunck* pCDQU;
	char* pCDQU_End;
	ushort boundingSphereTestResult;
	undefined field_0x2a;
	undefined field_0x2b;
	float field_0x2c;
	float field_0x30;
};

struct ClusterDetails {
	int field_0x20; // int*
	int field_0x24; // int*
	ushort count_0x28; // int*
	ushort clusterHierCount;
	undefined field_0x2c;
	undefined field_0x2d;
	ushort spriteCount;
};

static_assert(sizeof(ClusterDetails) == 0x10);

struct ed_g3d_cluster {
	ushort aClusterStripCounts[5];
	ushort field_0x1a;
	ushort flags_0x1c;
	undefined2 field_0x1e;
	ClusterDetails clusterDetails;
	int field_0x30; // int*
	int field_0x34; // int*
	int field_0x38; // uint*
	int field_0x3c; // uint*
	int pMBNK; // char*
	undefined field_0x44;
	undefined field_0x45;
	undefined field_0x46;
	undefined field_0x47;
	int p3DStrip; // ed_3d_strip*
	int p3DSprite; // ed_3d_sprite*
};

static_assert(sizeof(ed_g3d_cluster) == 0x40);

struct ed_g3d_Anim_def {
	uint field_0x0;
	uint field_0x4;
	uint field_0x8;
	uint field_0xc;
	float field_0x10;
	uint field_0x14;
	uint field_0x18;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
	uint field_0x20;
	float field_0x24;
	undefined field_0x28;
	undefined field_0x29;
	undefined field_0x2a;
	undefined field_0x2b;
	undefined field_0x2c;
	undefined field_0x2d;
	undefined field_0x2e;
	undefined field_0x2f;
	uint field_0x30;
	float field_0x34;
};

static_assert(sizeof(ed_g3d_Anim_def) == 0x38, "ed_g3d_Anim_def size is incorrect");

PACK(struct MeshData_CSTA {
	edF32VECTOR3 field_0x20;
	undefined field_0x2c;
	undefined field_0x2d;
	undefined field_0x2e;
	undefined field_0x2f;
	edF32VECTOR4 worldLocation;
};)

struct ed_3D_Scene {
	void ed3DSceneRemoveFlag(uint flag);
	void ed3DSceneSetFlag(uint flag);
	void ed3DSceneSetFogProperty(bool bValue);
	void SetFlag_002a5440(bool bValue);

	int bShadowScene;
	uint flags;
	struct edFCamera* pCamera;
	struct ed_viewport* pViewport;
	edLIST meshClusterShadowList;
	edLIST meshClusterList;
	SceneConfig sceneConfig;
	undefined field_0x18c;
	undefined field_0x18d;
	undefined field_0x18e;
	undefined field_0x18f;
	undefined field_0x190;
	undefined field_0x191;
	undefined field_0x192;
	undefined field_0x193;
	undefined field_0x194;
	undefined field_0x195;
	undefined field_0x196;
	undefined field_0x197;
	undefined field_0x198;
	undefined field_0x199;
	undefined field_0x19a;
	undefined field_0x19b;
	undefined field_0x19c;
	undefined field_0x19d;
	undefined field_0x19e;
	undefined field_0x19f;
	struct edLIST* field_0x1a0;
	struct edLIST* pHierListA;
	struct edLIST* pHierListB;
	int* field_0x1ac;
	uint heirarchyListCount;
	undefined field_0x1b4;
	undefined field_0x1b5;
	undefined field_0x1b6;
	undefined field_0x1b7;
	undefined field_0x1b8;
	undefined field_0x1b9;
	undefined field_0x1ba;
	undefined field_0x1bb;
	undefined field_0x1bc;
	undefined field_0x1bd;
	undefined field_0x1be;
	undefined field_0x1bf;
};

struct ed_3d_hierarchy_setup {
	float* clipping_0x0;
	union edF32VECTOR4* pBoundingSphere;
	struct ed_3D_Light_Config* pLightData;
	float* pLodBiases;
	float* field_0x10;
};


struct ed3DLod {
	int pObj; // char*
	short renderType;
	short sizeBias;
};

static_assert(sizeof(ed3DLod) == 0x8, "Invalid ed3DLod size");

struct ed_3d_hierarchy {
	edF32MATRIX4 transformA;
	edF32MATRIX4 transformB;
	Hash_8 hash;
	byte bSceneRender;
	undefined field_0x89;
	ushort bRenderShadow;
	union edF32MATRIX4* pShadowAnimMatrix;
	struct ed_3d_hierarchy* pLinkTransformData;
	undefined* field_0x94;
	char* pTextureInfo;
	ushort lodCount;
	ushort flags_0x9e;
	struct ed_3d_hierarchy_setup* pHierarchySetup;
	edpkt_data* pMatrixPkt;
	union edF32MATRIX4* pAnimMatrix;
	short linkedHierCount;
	byte desiredLod;
	char GlobalAlhaON;
};

PACK(struct MeshData_ANHR {
	uint hash;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	uint nb3dHierarchies;
	uint fileDataEntryCount;
});


union ObbFloat
{
	int pObb_Internal;
	float number;
};

PACK(struct ANHR_Internal {
	int pHierNode; // edNode*
	int pHierNodeData; // ed_3d_hierarchy_node*
	int pHierAnimStream; // S_HIERANM_ANIM
	uint nodeChunkCount;
	float field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	uint flags;
	float field_0x24;
	float field_0x28;
	float field_0x2c;
	ObbFloat pObbFloat;
	float field_0x34;
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
	int pMatrixPkt; // edpkt_data*
	int pAnimMatrix; // edF32MATRIX4*
	short subMeshParentCount_0xac;
	byte desiredLod;
	char GlobalAlhaON;
	ed3DLod aLods[];
});

struct ed_3d_hierarchy_node {
	ed_3d_hierarchy base;
	ed3DLod aLods[4];
};

struct TextureInfo {
	ed_g2d_manager manager;
	char* pFileBuffer;
};

struct ScratchPadRenderInfo {
	edF32MATRIX4* pSharedMeshTransform;
	edF32MATRIX4* pMeshTransformMatrix;
	int boundingSphereTestResult;
	ed_3d_hierarchy_setup* pHierarchySetup;
	uint flags;
	float biggerScale;
	edpkt_data* pPkt;
	ed_3d_hierarchy* pMeshTransformData;
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
	byte nbLayers;
	undefined field_0x1;
	ushort flags;
	int pDMA_Material; // ed_dma_material*
	int pCommandBufferTexture; // RenderCommand*
	int commandBufferTextureSize;
	int aLayers[];
});

struct edPSX2Header
{
	int pPkt;
	int size;
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
	byte field_0x1b;
	short bHasTexture;
	ushort paletteId;
	int pTex; // ed_Chunck*
});


PACK(struct ed_g2d_bitmap {
	ushort width;
	ushort height;
	ushort psm;
	ushort maxMipLevel;
	int pPSX2; //edpkt_data*
});

PACK(
	struct ed_g2d_texture {
	ed_hash_code hashCode;
	int bHasPalette;
	int pAnimSpeedNormalExtruder; //edF32VECTOR4*
	float field_0x18;
	int pAnimChunck; //ed_Chunck*
});

struct ed_dma_material {
	ed_g2d_material* pMaterial;
	float field_0x4;
	edLIST list;
	uint flags;
	ed_g2d_bitmap* pBitmap;
};

struct ed_viewport;
struct edFCamera;

struct ed_3D_Scene;

struct ed_g3d_manager;
struct DisplayListInternal;
struct edCluster;
struct edNODE;

ed_g3d_manager* ed3DInstallG3D(char* pFileData, int fileLength, ulong flags, int* outInt, ed_g2d_manager* textureObj, int unknown, ed_g3d_manager* pMeshInfo);


void Init3D(void);
ed_g2d_manager* ed3DInstallG2D(char* pFileBuffer, int fileLength, int* outInt, ed_g2d_manager* pManager, int param_5);
ed_hash_code* ed3DG2DGetMaterialFromIndex(ed_g2d_manager* pManager, int index);
ed_g2d_material* ed3DG2DGetG2DMaterialFromIndex(ed_g2d_manager* pManager, int index);
ed_g2d_texture* ed3DG2DGetTextureFromMaterial(ed_g2d_material* pMaterial, int index);
ed_g2d_bitmap* ed3DG2DGetBitmapFromMaterial(ed_g2d_material* pMaterial, int param_2);
ed_g3d_Anim_def* ed3DG2DAnimTexGet(ed_g2d_texture* pManager);
ed_3D_Scene* ed3DSceneCreate(edFCamera* pCamera, ed_viewport* pViewport, int bInitHierList);
edNODE* ed3DHierarchyAddToScene(ed_3D_Scene* pScene, ed_g3d_manager* pG3D, char* szString);
edNODE* ed3DHierarchyAddToSceneByHashcode(ed_3D_Scene* pStaticMeshMaster, ed_g3d_manager* pMeshInfo, ulong hash);
edNODE* ed3DHierarchyRefreshSonNumbers(edNODE* pMeshTransformParent, short* outMeshCount);

void ed3DRunTimeStripBufferReset(void);

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

union DMA_Matrix {
	int pDMA_Matrix;

	struct {
		ushort flagsA;
		ushort flagsB;
	};
};

struct ed_3d_strip {
	uint flags;
	short materialIndex;
	short cachedIncPacket;
	int vifListOffset;
	int pNext; // ed_3d_strip*
	edF32VECTOR4 boundingSphere;
	int pSTBuf; // char*
	int pColorBuf; // _rgba*
	int pVertexBuf; // edVertex*
	int field_0x2c; // char*
	short shadowCastFlags;
	short shadowReceiveFlags;
	DMA_Matrix pDMA_Matrix; // ed_dma_matrix*
	byte field_0x38;
	byte primListIndex;
	short meshCount;
	int pBoundSpherePkt; // ed_Bound_Sphere_packet*
};

static_assert(sizeof(ed_3d_strip) == 0x40, "Invalid ed_3d_strip size");


struct ed_dma_matrix : public edLIST {
	edF32MATRIX4* pObjToWorld;
	ed_3d_hierarchy* pHierarchy;
	int flags_0x28;
	float normalScale;
};

void ed3DHierarchyCopyHashCode(ed_g3d_manager* pMeshInfo);
edNODE* ed3DHierarchyAddNode(edLIST* pList, ed_3d_hierarchy_node* pHierNode, edNODE* pNode, ed_g3d_hierarchy* p3DA, ed_3d_hierarchy* p3DB);

void ed3DScenePushCluster(ed_3D_Scene* pStaticMeshMaster, ed_g3d_manager* pMeshInfo);

uint edChunckGetNb(void* pStart, char* pEnd);
ed_hash_code* edHashcodeGet(Hash_8 meshHashValue, ed_Chunck* pChunck);

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
edNODE* ed3DHierarchyNodeGetByHashcodeFromList(edNODE* pNode, ulong hash);
ed_Chunck* ed3DHierarchyNodeGetSkeletonChunck(edNODE* pMeshTransformParent, bool bGetFromHierarc);
void ed3DHierarchyNodeSetSetup(edNODE* pNode, ed_3d_hierarchy_setup* pSetup);
ed_dma_matrix* ed3DListCreateDmaMatrixNode(ScratchPadRenderInfo* pRenderInfo, ed_3d_hierarchy* pHierarchy);

PACK(struct ed_g3d_object {
	undefined field_0x10;
	undefined field_0x11;
	undefined field_0x12;
	undefined field_0x13;
	int stripCount;
	edF32VECTOR4 boundingSphere;
	undefined field_0x28;
	undefined field_0x29;
	undefined field_0x2a;
	undefined field_0x2b;
	int p3DStrip; //struct ed_3d_strip*
});

ed_g3d_object* ed3DHierarchyGetObject(ed_3d_hierarchy* pHier);
ed3DLod* ed3DChooseGoodLOD(ed_3d_hierarchy* pHierarchy);
uint ed3DFlushStripGetIncPacket(ed_3d_strip* pStrip, bool param_2, bool bCachedSizeDirty);
edpkt_data* ed3DPKTCopyMatrixPacket(edpkt_data* pPkt, ed_dma_matrix* pDmaMatrix, byte param_3);
edpkt_data* ed3DPKTAddMatrixPacket(edpkt_data* pPkt, ed_dma_matrix* pDmaMatrix);
float ed3DMatrixGetBigerScale(edF32MATRIX4* m0);
int ed3DInitRenderEnvironement(ed_3D_Scene* pStaticMeshMaster, long mode);
edLIST* ed3DHierarchyListInit(void);
edNODE* ed3DHierarchyAddToList(edLIST* pList, ed_3d_hierarchy_node* pHierNode, edNODE* pNode, ed_g3d_manager* pMeshInfo, char* szString);
void ed3DHierarchyAddSonsToList(edLIST* pList, ed_3d_hierarchy_node* pHierNode, edNODE* pParentNode, ed_Chunck* pChunck, edNODE* pNewNode,
	ed_hash_code* pHashCode, uint count);
ed_g2d_material* ed3DG2DGetG2DMaterialFromIndex(ed_hash_code* pMBNK, int index);

ed_hash_code* ed3DHierarchyGetMaterialBank(ed_3d_hierarchy* pHier);

// Possibly an inlined function that doesn't exist in the compiled code.
int ed3DG2DGetG2DNbMaterials(ed_Chunck* pChunck);

int ed3DG2DGetG2DNbMaterials(ed_hash_code* pHashCode);
int ed3DG2DGetG2DNbMaterials(ed_g2d_manager* pManager);

int ed3DHierarchyBankMatGetSize(ed_3d_hierarchy* pHier);
void* ed3DHierarchyBankMatInstanciate(ed_3d_hierarchy* pHier, void* pData);
void ed3DHierarchyBankMatLinkG2D(ed_3d_hierarchy_node* pHier, ed_g2d_manager* pTexture);
ed_hash_code* ed3DG2DGetMaterial(ed_g2d_manager* pTextureInfo, ulong hash);
ed_g3d_hierarchy* ed3DG3DHierarchyGetFromHashcode(ed_g3d_manager* pG3d, ulong hash);
void ed3DG3DHierarchySetStripShadowReceiveFlag(ed_g3d_hierarchy* pHier, ushort flag);

uint ed3DTestBoundingSphereObjectNoZFar(edF32VECTOR4* pSphere);

ed3DLod* ed3DHierarcGetLOD(ed_3d_hierarchy* pHier, uint index);
ed3DLod* ed3DHierarcGetLOD(ed_g3d_hierarchy* pHier, uint index);

ed_Chunck* edChunckGetFirst(void* pBuffStart, char* pBuffEnd);

void ed3DHierarchyRemoveFromScene(ed_3D_Scene* pScene, edNODE* pNode);
void ed3DScenePopCluster(ed_3D_Scene* pScene, ed_g3d_manager* pMeshInfo);

void ed3DUnInstallG3D(ed_g3d_manager* pMeshInfo);
void ed3DUnInstallG2D(ed_g2d_manager* pTextureInfo);

void ed3DHierarchyNodeClrFlag(edNODE* pNode, ushort flag);
void ed3DHierarchyNodeSetFlag(edNODE* pNode, ushort flag);
void ed3DHierarchyNodeSetAlpha(edNODE* pNode, byte alpha);
void ed3DHierarchyNodeSetBFCulling(edNODE* pNode, byte bActive);

ed_3D_Scene* ed3DGetScene(int index);

ulong ed3DComputeHashCode(char* inString);
bool ed3DComputeSceneCoordinate(edF32VECTOR2* pOutScreenCoord, edF32VECTOR4* pPosition, ed_3D_Scene* pScene);
ed_g2d_material* ed3DG2DGetG2DMaterial(ed_g2d_manager* pManager, ulong hashCode);
ed_g2d_layer* ed3DG2DMaterialGetLayer(ed_g2d_material* pMaterial, uint index);

void ed3DUnLockLOD(ed_3d_hierarchy_node* pHier);
void ed3DLockLOD(ed_3d_hierarchy_node* pNode, byte desiredLod);

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
Multidelegate<ed_g2d_manager*, std::string>& ed3DGetTextureLoadedDelegate();
Multidelegate<ed_g3d_manager*, std::string>& ed3DGetMeshLoadedDelegate();

namespace ed3D {
	namespace DebugOptions {
		bool& GetForceHighestLod();
		bool& GetDisableClusterRendering();
	}
}
#endif

#define SHELLDMA_TAG_ID_CNT (0x10)
#define SHELLDMA_TAG_ID_REF (0x30)

#define ED_VIF1_SET_TAG_CNT(qwc) ((ulong)(qwc) | ((ulong)SHELLDMA_TAG_ID_CNT << 24) | ((ulong)0xe << 32))
#define ED_VIF1_SET_TAG_REF(qwc, addr) ((ulong)((ulong)(qwc) | ((ulong)SHELLDMA_TAG_ID_REF << 24))) | ((ulong)(addr) << 32)

extern int gNbVertexDMA;

extern ed_3D_Scene* gScene3D;
extern ed3DConfig ged3DConfig;
extern edpkt_data* gPKTMatrixCur;

#endif //_ED3D_H
