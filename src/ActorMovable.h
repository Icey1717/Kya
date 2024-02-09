#ifndef ACTOR_MOVABLE_H
#define ACTOR_MOVABLE_H

#include "Types.h"
#include "Actor.h"

class CDynamic
{
public:
	CDynamic();

	void Reset(CActor* pActor);

	static float gMaxSpeed_Horiz;
	static float gMaxSpeed_Vert;

	edF32VECTOR4 rotationQuat;
	edF32VECTOR4 field_0x10;
	edF32VECTOR4 currentLocation;
	edF32VECTOR4 horizontalLocation;

	uint flags;

	float field_0x40;
	float field_0x44;
	float intensity;
	undefined4 field_0x4c;

	float field_0x54;
	float field_0x58;
};

class CActorMovable : public CActor {
public:

	virtual void Create(ByteCode* pByteCode);
	virtual void SetState(int newState, int animType);

	void SV_MOV_UpdateSpeedIntensity(float param_1, float param_2);

	void ComputeRealMoving(edF32VECTOR4* delta, float param_3);

	CDynamic dynamic;
	float field_0x1c0;
};

#endif //ACTOR_MOVABLE_H