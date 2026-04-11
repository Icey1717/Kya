#ifndef CAMERA_BOOMY_H
#define CAMERA_BOOMY_H

#include "Types.h"
#include "camera.h"
#include "Vision.h"
#include "Dynamic.h"

class CCameraAimBase : public CCameraExt
{
public:
	CCameraAimBase();

	virtual void Init();

	edF32VECTOR4 field_0xd0;
	SPEED_DYN field_0xe0;
	SPEED_DYN field_0xe8;
	float field_0xf0;
};

class CCameraAim : public CCameraAimBase
{
public:
	CCameraAim();

	virtual ECameraType GetMode();
	virtual void Init();

	virtual bool Manage();
	virtual void Draw();
	virtual bool AlertCamera(int param_2, void* pParams, CCamera* param_4);

	void Manage_Pad();

	float field_0x100;
	float field_0x104;

	CVision vision;

	float field_0x170;
	CActor* field_0x174;
	CActor* field_0x178;
	float field_0x17c;
};

#endif // CAMERA_BOOMY_H