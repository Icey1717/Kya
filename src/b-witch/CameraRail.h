#ifndef CAMERA_RAIL_H
#define CAMERA_RAIL_H

#include "Types.h"
#include "camera.h"
#include "edMathTCBSpline.h"
#include "Dynamic.h"

struct ByteCode;

class CCameraRailVirtual : public CCameraExt
{
public:
	CCameraRailVirtual(ByteCode* pByteCode);

	virtual void Init();

	int pathFollowIndex;
	CedMathTCBSpline* pSpline;
};

struct CameraRailSegment
{
	edF32VECTOR4 position;
	edF32VECTOR4 tangent;
	float alpha;
};

class CCameraRail : public CCameraRailVirtual
{
public:
	CCameraRail(ByteCode* pByteCode);

	virtual ECameraType GetMode();
	virtual void Init();
	virtual bool Manage();
	virtual bool AlertCamera(int alertType, int param_3, CCamera* param_4);

	void Alert();
	void _UpdateTargetPosWithPlane();

	int pathFollowIndex;
	uint currentSegmentIndex;
	float distanceToPlaneA;
	float distanceToPlaneB;
	int nbSegments;
	CameraRailSegment* aRailSegments;

	edF32VECTOR4 railTargetPosition;
};

class CCameraRailSimple : public CCameraRailVirtual
{
public:
	CCameraRailSimple(ByteCode* pByteCode);

	virtual ECameraType GetMode();
	virtual bool Manage();
	virtual bool AlertCamera(int alertType, int param_3, CCamera* param_4);

	void OnAlertCamera();

	SPEED_DYN speedDyn;

	edF32VECTOR4 field_0xe0;

	float splineAlpha;
};

#endif // !CAMERA_RAIL_H