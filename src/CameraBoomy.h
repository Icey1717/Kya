#ifndef CAMERA_BOOMY_H
#define CAMERA_BOOMY_H

#include "Types.h"
#include "camera.h"
#include "Dynamic.h"

class CCameraAimBase : public CCameraExt
{
public:
	CCameraAimBase();

	virtual void Init();

	edF32VECTOR4 field_0xd0;
	SPEED_DYN field_0xe0;
	SPEED_DYN field_0xe8;
	undefined4 field_0xf0;
};

class CCameraAim : public CCameraAimBase
{
public:
	CCameraAim();

	virtual ECameraType GetMode();
	virtual void Init();

	virtual bool Manage();
	virtual void Draw();
	virtual bool AlertCamera(int param_2, int param_3, CCamera* param_4);

	float field_0x100;
	float field_0x104;

	undefined4 field_0x114;

	undefined4 field_0x178;
	float field_0x17c;
};

#endif // CAMERA_BOOMY_H