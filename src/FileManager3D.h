#ifndef FILE_MANAGER_3D_H
#define FILE_MANAGER_3D_H

#include "Types.h"
#include "LargeObject.h"

struct ed_g3d_manager {
	char* fileBufferStart;
	char* field_0x4;
	int fileLengthA;
	undefined4 field_0xc;
	char* OBJA;
	char* LIA;
	char* CAMA;
	char* SPRA;
	char* HALL;
	char* CSTA;
	char* GEOM;
	char* MBNA;
	char* INFA;
	int fileLengthB;
	char* CDZA;
	char* ANMA;
};

struct Mesh {
	char* pFileData;
	int fileLength;
	struct ed_g3d_manager meshInfo;
	struct TextureInfo* pTextureInfo;
};

struct ParticleInfo {
	int ID;
	int materialCount_0x4;
	struct edDList_material* materialInfoArray_0x8;
};

struct FileManager3D : public Manager {
	FileManager3D();

	virtual void Level_AddAll(struct ByteCode* pMemoryStream);



	int InstanciateG2D(int index);
	struct TextureInfo* GetCommonSectorG2D();
	void Level_ClearInternalData();
	void Level_Create(ByteCode* pMemoryStream);
	struct ed_g3d_manager* pMeshInfo;
	struct edNODE* pMeshTransformParent;
	struct edNODE* pLastMeshTransformParent;
	char* field_0x10;
	undefined field_0x14;
	undefined field_0x15;
	undefined field_0x16;
	undefined field_0x17;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
	undefined field_0x20;
	undefined field_0x21;
	undefined field_0x22;
	undefined field_0x23;
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
