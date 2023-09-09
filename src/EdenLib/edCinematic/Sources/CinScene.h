#ifndef _CIN_SCENE_H
#define _CIN_SCENE_H

#include "Types.h"

struct edCinGameInterface;


PACK(
	struct edAnimatedPropertyTag {
	short propType;
	undefined field_0x2;
	undefined field_0x3;
	int size;
	uint type;
});

struct edAnimatedProperty {
	edAnimatedPropertyTag* pData;
	bool GetQuaternionValue(float currentPlayTime, edF32VECTOR4* outVector);
};

PACK(
	struct edAnmSubControlerTag {
	ushort keyCount;
	ushort field_0x2;
	float keyTimes[];
});

struct edAnmSubControler {
	edAnmSubControlerTag* pData;
	float* GetClosestKeyIndex(float param_1, int* param_3);
	float GetKeyIndicesAndRatioSafe(float currentPlayTime, int* outKeyFrame, int* outNextKeyframe, uchar param_5);
};

PACK(
	struct CameraInfo {
	undefined field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	int field_0x4;
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
