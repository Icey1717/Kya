#ifndef FILE_MANAGER_3D_H
#define FILE_MANAGER_3D_H

#include "Types.h"
#include "LargeObject.h"
#include "ed3D.h"

struct ed_Chunck;

struct ed_g3d_manager {
	char* fileBufferStart;
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

#define NAME_MESH 1

struct Mesh {
	char* pFileData;
	int fileLength;
	struct ed_g3d_manager meshInfo;
	struct TextureInfo* pTextureInfo;
#if NAME_MESH
	char name[256];
#endif
};

struct ParticleInfo {
	int ID;
	int materialCount_0x4;
	struct edDList_material* materialInfoArray_0x8;
};

struct C3DFileManager : public CObjectManager {
	C3DFileManager();

	virtual void Level_AddAll(struct ByteCode* pMemoryStream);



	int InstanciateG2D(int index);
	struct TextureInfo* GetCommonSectorG2D();
	void Level_ClearInternalData();
	void Level_Create(ByteCode* pMemoryStream);

	void HideCommonBackground();
	void SetupBackground(edNODE* pNode);
	void ManageBackground(edNODE* pNode, uint flags);

	ed_g3d_manager* GetG3DManager(int meshIndex, int textureIndex);

	ed_g2d_manager* GetActorsCommonMaterial(int index);
	ed_g2d_manager* GetActorsCommonMeshMaterial(int index);

	ed_g2d_manager * LoadDefaultTexture_001a65d0();

	struct ed_g3d_manager* pMeshInfo;
	struct edNODE* pBackgroundNode;
	struct edNODE* pLastMeshTransformParent;
	ed_3d_hierarchy_setup backgroundHierarchySetup;
	float field_0x30;
	int meshCount;
	int meshLoadedCount;
	struct Mesh* pMeshDataArray;
	int levelSectorTextureIndex;
	int textureCount;
	int textureLoadedCount;
	struct TextureInfo* pTextureInfoArray;
	struct ParticleInfo* pParticleInfoArray_0x50;
};

#endif // FILE_MANAGER_3D_H
