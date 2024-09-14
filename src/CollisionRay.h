#ifndef _COLLISION_RAY_H
#define _COLLISION_RAY_H

#include "Types.h"
#include "EdenLib/edCollision/sources/edCollisions.h"

#define RAY_FLAG_SCENERY 1
#define RAY_FLAG_ACTOR 2

class CActor;

struct _ray_info_out {
	CActor* pActor_0x0;
	union {
		uint pVector_0x4;
		short type_0x4[2];
	};
	uint type_0x8;
};

class CCollisionRay : public edColRAY_OBB_IN
{
public:
	CCollisionRay() {}
	CCollisionRay(float f0, float f1, float f2, edF32VECTOR4* pLocation, edF32VECTOR4* pDirection);
	CCollisionRay(float f0, edF32VECTOR4* pLocation, edF32VECTOR4* pDirection);
	float Intersect(uint type, CActor* pActor, CActor* pOther, uint flags, edF32VECTOR4* pOutVector, _ray_info_out* pResultData);
	float IntersectActors(CActor* pActor, CActor* pOtherActor, uint flags, edF32VECTOR4* pOutVector, _ray_info_out* pOutResult);
	float IntersectActorsTable(CActorsTable* pTable, edF32VECTOR4* v0, _ray_info_out* pOutResult);
	void ComputeIntersectionNormalAndProps(float distance, void* pColObj, int colType, edF32VECTOR4* pOutNormal, uint* pOutProps);
	float IntersectScenery(edF32VECTOR4* pOutVector, _ray_info_out* pOutResult);

	void ChangeLeadVector(edF32VECTOR4* pNewLeadVector);


	undefined4 field_0xc;

	// This might be a union of float and vector4
	edF32VECTOR4 lengthB;
};

#endif
