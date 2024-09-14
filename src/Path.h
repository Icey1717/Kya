#ifndef PATH_H
#define PATH_H

#include "Types.h"
#include "PathFollow.h"

struct PlaneData
{
	edF32VECTOR4 field_0x0;
	float field_0x10;
};

struct CPathPlaneOutData
{
	int field_0x0;
	float field_0x4;
	float field_0x8;
	PlaneData* aPlaneData;
};

class CPathPlane {
public:
	CPathFollowReader pathFollowReader;
	CPathPlaneOutData outData;

	void computePlanesFromKeys(PlaneData* aPlaneData, int nbPoints);
	void InitTargetPos(edF32VECTOR4* pTargetPos, CPathPlaneOutData* pOutData);
	void ExternComputeTargetPosWithPlane(edF32VECTOR4* pTargetPos, CPathPlaneOutData* pOutData);
};

class CPathPlaneArray {
public:
	CPathPlaneArray();

	void Create(ByteCode* pByteCode);

	void Init();
	void Reset();

	int GetNbPathPlane();
	CPathPlane* GetCurPathPlane();

	void NextWayPoint();
	int AtGoal();

	void InitPosition(edF32VECTOR4* pPosition);

	int nbPathPlanes;
	int curIndex;
	CPathPlane* aPathPlanes;
};

#endif // !PATH_H
