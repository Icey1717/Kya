#ifndef _CIN_RES_COLLECTION_H
#define _CIN_RES_COLLECTION_H

#include "Types.h"

struct edCinGameInterface;

PACK(
struct edResCollectionTag {
	uint flags;
	int pData;
	int size;
});

PACK(
	struct edResCollectionHeader {
	int resCount;
	edResCollectionTag aTags[];
});

struct edResCollection {
	enum RES_TYPE {
		COT_Animation = 2,
		COT_MeshModel = 1,
		COT_MeshTexture = 4,
		COT_LipTrack = 6,
		COT_Particle = 8,
		COT_Sound = 0,
		COT_Text = 9,
		COT_Scene = 3,
	};

	edResCollectionHeader* pData;

	char* GetResFilename(const int offset);
	void FlushAllResources(edCinGameInterface& cinGameInterface);
	char* LoadResource(edCinGameInterface& cinGameInterface, int offset, int* outSize);
};

#endif //_CIN_RES_COLLECTION_H
