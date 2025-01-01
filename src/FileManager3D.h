#ifndef FILE_MANAGER_3D_H
#define FILE_MANAGER_3D_H

#include "Types.h"
#include "LargeObject.h"
#include "ed3D.h"

#define NAME_MESH 1

struct Mesh {
	char* pFileData;
	int fileLength;
	ed_g3d_manager meshInfo;
	TextureInfo* pTextureInfo;
#if NAME_MESH
	std::string name;
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

	edDList_material* GetMaterialFromId(int materialId, int internalId);

	int InstanciateG2D(int index);
	struct TextureInfo* GetCommonSectorG2D();
	void Level_ClearInternalData();
	void Level_Create(ByteCode* pMemoryStream);

	void ShowCommonBackground();
	void HideCommonBackground();
	void SetupBackground(edNODE* pNode);
	void ManageBackground(edNODE* pNode, uint flags);

	ed_g3d_manager* GetG3DManager(int meshIndex, int textureIndex);

	ParticleInfo* GetG2DInfo(int index);

	ed_g2d_manager* GetActorsCommonMaterial(int index);
	ed_g2d_manager* GetActorsCommonMeshMaterial(int index);

	ed_g2d_manager * LoadDefaultTexture_001a65d0();
	ed_g3d_manager* GetInfoForCommonLevelMesh(char* pFileData);

	struct ed_g3d_manager* pMeshInfo;
	struct edNODE* pBackgroundNode;
	struct edNODE* pLastMeshTransformParent;
	ed_3d_hierarchy_setup backgroundHierarchySetup;
	float field_0x30;
	int meshCount;
	int meshLoadedCount;
	struct Mesh* aCommonLevelMeshes;
	int levelSectorTextureIndex;
	int textureCount;
	int textureLoadedCount;
	struct TextureInfo* aCommonLevelTextures;
	struct ParticleInfo* pParticleInfoArray_0x50;
};

#endif // FILE_MANAGER_3D_H
