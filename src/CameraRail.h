#ifndef CAMERA_RAIL_H
#define CAMERA_RAIL_H

#include "Types.h"
#include "camera.h"
#include "edMathTCBSpline.h"

struct ByteCode;

class CCameraRailVirtual : public CCameraExt
{
public:
	CCameraRailVirtual(ByteCode* pByteCode);

	virtual void Init();

	int pathFollowIndex;
	CedMathTCBSpline* pSpline;
};

struct CameraRailSubObj
{
	edF32VECTOR4 field_0x0;
	edF32VECTOR4 field_0x10;
	float field_0x20;
};

class CCameraRail : public CCameraRailVirtual
{
public:
	CCameraRail(ByteCode* pByteCode);

	virtual ECameraType GetMode();
	virtual void Init();
	virtual bool Manage();
	virtual bool AlertCamera(int alertType, int param_3, CCamera* param_4);

	int field_0xe0;
	int field_0xf0;
	CameraRailSubObj* field_0xf4;
};

class CCameraRailSimple : public CCameraRailVirtual
{
public:
	CCameraRailSimple(ByteCode* pByteCode);

	virtual ECameraType GetMode();
	virtual bool Manage();
	virtual bool AlertCamera(int alertType, int param_3, CCamera* param_4);

	SPEED_DYN speedDyn;

	edF32VECTOR4 field_0xe0;

	float field_0xf8;
};

#endif // !CAMERA_RAIL_H