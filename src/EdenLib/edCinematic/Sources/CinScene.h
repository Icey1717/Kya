#ifndef _CIN_SCENE_H
#define _CIN_SCENE_H

#include "Types.h"

struct edCinGameInterface;
struct edResCollection;

PACK(
	struct CineCreatureObject {
	int field_0x0;
	int trackDataOffset;
	undefined field_0x8;
	undefined field_0x9;
	undefined field_0xa;
	undefined field_0xb;
	int actorHashCode;
	int trackCount;
	int pCinActorInterface; // edCinActorInterface*

	char name[32];
	edF32VECTOR4 boundingSphere;
	int meshID;
	int textureID;
	edF32VECTOR3 position;
	edF32VECTOR4 heading;
	edF32VECTOR3 scale;
	undefined field_0x78;
	undefined field_0x79;
	undefined field_0x7a;
	undefined field_0x7b;
	undefined field_0x7c;
	undefined field_0x7d;
	undefined field_0x7e;
	undefined field_0x7f;
	undefined field_0x80;
	undefined field_0x81;
	undefined field_0x82;
	undefined field_0x83;
	undefined field_0x84;
	undefined field_0x85;
	undefined field_0x86;
	undefined field_0x87;
	undefined field_0x88;
	undefined field_0x89;
	undefined field_0x8a;
	undefined field_0x8b;
	undefined field_0x8c;
	undefined field_0x8d;
	undefined field_0x8e;
	undefined field_0x8f;
	undefined field_0x90;
	undefined field_0x91;
	undefined field_0x92;
	undefined field_0x93;
	undefined field_0x94;
	undefined field_0x95;
	undefined field_0x96;
	undefined field_0x97;
	undefined field_0x98;
	undefined field_0x99;
	undefined field_0x9a;
	undefined field_0x9b;
	undefined field_0x9c;
	undefined field_0x9d;
	undefined field_0x9e;
	undefined field_0x9f;
	undefined field_0xa0;
	undefined field_0xa1;
	undefined field_0xa2;
	undefined field_0xa3;
	undefined field_0xa4;
	undefined field_0xa5;
	undefined field_0xa6;
	undefined field_0xa7;
	undefined field_0xa8;
	undefined field_0xa9;
	undefined field_0xaa;
	undefined field_0xab;
	undefined field_0xac;
	undefined field_0xad;
	undefined field_0xae;
	undefined field_0xaf;
	undefined field_0xb0;
	undefined field_0xb1;
	undefined field_0xb2;
	undefined field_0xb3;
	undefined field_0xb4;
	undefined field_0xb5;
	undefined field_0xb6;
	undefined field_0xb7;
	undefined field_0xb8;
	undefined field_0xb9;
	undefined field_0xba;
	undefined field_0xbb;
	undefined field_0xbc;
	undefined field_0xbd;
	undefined field_0xbe;
	undefined field_0xbf;
	undefined field_0xc0;
	undefined field_0xc1;
	undefined field_0xc2;
	undefined field_0xc3;
	undefined field_0xc4;
	undefined field_0xc5;
	undefined field_0xc6;
	undefined field_0xc7;
	undefined field_0xc8;
	undefined field_0xc9;
	undefined field_0xca;
	undefined field_0xcb;
	undefined field_0xcc;
	undefined field_0xcd;
	undefined field_0xce;
	undefined field_0xcf;
	undefined field_0xd0;
	undefined field_0xd1;
	undefined field_0xd2;
	undefined field_0xd3;
	undefined field_0xd4;
	undefined field_0xd5;
	undefined field_0xd6;
	undefined field_0xd7;
	undefined field_0xd8;
	undefined field_0xd9;
	undefined field_0xda;
	undefined field_0xdb;
	undefined field_0xdc;
	undefined field_0xdd;
	undefined field_0xde;
	undefined field_0xdf;
});

PACK(
struct edSceneSceneryTag {
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
	undefined field_0x10;
	undefined field_0x11;
	undefined field_0x12;
	undefined field_0x13;
	char name[32];
	int meshOffset;
	int textureOffset;
});

struct edSceneScenery {
	edSceneSceneryTag* pTag;
	void Create(edCinGameInterface& loadObj, edResCollection& resStartBuffer);
};

PACK(
	struct edAnimatedPropertyTag {
	short propType;
	undefined field_0x2;
	undefined field_0x3;
	int size;
	uint type;
});

struct edAnimatedProperty {
	edAnimatedProperty() 
		: pData((edAnimatedPropertyTag*)0x0)
	{

	}

	edAnimatedProperty(edAnimatedPropertyTag* pInData) 
		: pData(pInData)
	{

	}

	edAnimatedPropertyTag* pData;
	bool GetQuaternionValue(float currentPlayTime, edF32VECTOR4* outVector);
	bool GetKeyIndexAndTime(float currentTime, int* outIndex, float* outTime);
	bool GetVector3Value(float time, float* outValue);
};

PACK(
	struct edAnmSubControlerTag {
	ushort keyCount;
	ushort field_0x2;
	float keyTimes[];
});

struct edAnmSubControler {
	edAnmSubControler()
		: pData((edAnmSubControlerTag*)0x0)
	{

	}

	edAnmSubControler(edAnmSubControlerTag* pInData)
		: pData(pInData)
	{

	}

	edAnmSubControlerTag* pData;
	float* GetClosestKeyIndex(float time, int* outIndex);
	float* GetClosestKeyIndexSafe(float time, int* outIndex);
	float GetKeyIndicesAndRatioSafe(float currentPlayTime, int* outKeyFrame, int* outNextKeyframe, uchar param_5);
};

PACK(
	struct CameraInfo {
	undefined field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	int trackDataOffset;
	undefined field_0x8;
	undefined field_0x9;
	undefined field_0xa;
	undefined field_0xb;
	uint field_0xc;
	int trackCount;
	char name[32]; /* Created by retype action */
	float fov;
	edF32VECTOR3 position;
	edF32VECTOR4 heading;
	int cinCam; // edCinCamInterface*
});

struct edSCENEtag {
	undefined field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	int size;
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
	int pCollection; //edResCollectionInternal* 
};

struct edScene {
	edScene() :pTag(0) {}
	edScene(edSCENEtag* pTag);
	edSCENEtag* pTag;
	edSCENEtag* Create(void* inFileBuffer, uint fileLength, edCinGameInterface& loadObj);
	bool Initialize();
	bool Timeslice(float currentPlayTime, uint param_3);
};


#endif //_CIN_SCENE_H
