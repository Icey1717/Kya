#ifndef CAMERA_SHADOW_H
#define CAMERA_SHADOW_H

#include "Types.h"
#include "camera.h"

struct CCameraShadow : public CCameraExt
{
	CCameraShadow(ByteCode* pByteCode);

	// CObject
	virtual bool InitDlistPatchable(int);

	virtual void Init();
	virtual ECameraType GetMode();
	virtual bool Manage();

	void FUN_001912c0(edF32VECTOR4* param_2);
	void FUN_00190720();

	byte field_0xd0;

	int patchRegister;

	int sceneIndex;
	uint sceneFlags;

	undefined4 field_0x16dc;

	float field_0x16e0;
	float field_0x16e4;

	edF32VECTOR4 field_0x16f0;

	int field_0x1700;
};

#endif