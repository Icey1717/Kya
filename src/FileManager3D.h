#ifndef FILE_MANAGER_3D_H
#define FILE_MANAGER_3D_H

#include "Types.h"
#include "LargeObject.h"

struct MeshInfo {
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
	struct MeshInfo meshInfo;
	struct TextureInfo* pTextureInfo;
};

struct FileManager3D : public Manager {
	FileManager3D();

	virtual void Deserialize(struct MemoryStream* pMemoryStream);



	struct TextureInfo* GetLevelSectorTextureInfo_001a6670();
	void Setup_001a7110();
	void AllocateMeshTextureMemory_001a6f10(MemoryStream* pMemoryStream);
	struct MeshInfo* pMeshInfo;
	struct MeshTransformParent* pMeshTransformParent;
	struct MeshTransformParent* pLastMeshTransformParent;
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
	undefined field_0x30;
	undefined field_0x31;
	undefined field_0x32;
	undefined field_0x33;
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
