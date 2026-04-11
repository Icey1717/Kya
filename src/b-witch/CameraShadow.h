#ifndef CAMERA_SHADOW_H
#define CAMERA_SHADOW_H

#include "Types.h"
#include "camera.h"
#include "CollisionManager.h"

struct CCameraShadow : public CCameraExt
{
	CCameraShadow(ByteCode* pByteCode);

	// CObject
	virtual bool InitDlistPatchable(int);

	virtual ECameraType GetMode();
	virtual void Init();
	virtual void Reset();
	virtual bool Manage();
	virtual void Draw();

	virtual void SetTarget(CActor* pActor);

	void ComputeTargetPosition(edF32VECTOR4* pOutPosition);

	bool ComputeShadowTriangles();

	void FUN_001912c0(edF32VECTOR4* param_2);
	void FUN_00190720();

	byte field_0xd0;

	int patchRegister;

	int sceneIndex;
	uint sceneFlags;

	edF32TRIANGLE4 field_0xd4[250];

	edF32VECTOR4 field_0x1080[250];

	int field_0x16c0;
	int field_0x16c4;
	int field_0x16c8;
	int field_0x16cc;

	undefined4 field_0x16dc;

	float field_0x16e0;
	float field_0x16e4;

	edF32VECTOR4 field_0x16f0;

	int field_0x1700;
};

#endif