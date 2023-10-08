#ifndef _CIN_RES_COLLECTION_H
#define _CIN_RES_COLLECTION_H

#include "Types.h"

struct edCinGameInterface;

struct edResCollectionTag {
	int field_0x0;
};


struct edResCollection {
	enum RES_TYPE {
		COT_Animation = 2,
		COT_MeshModel = 1,
		COT_MeshTexture = 4,
		COT_Particle = 8,
		COT_Sound = 0,
		COT_Text = 9,
		COT_Scene = 3,
	};

	edResCollectionTag* pData;

	char* GetResFilename(const int offset);
	void FlushAllResources(edCinGameInterface& cinGameInterface);
	char* LoadResource(edCinGameInterface& cinGameInterface, int offset, int* outSize);
};

#endif //_CIN_RES_COLLECTION_H
