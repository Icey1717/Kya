#ifndef VISION_H
#define VISION_H

#include "Types.h"

struct ByteCode;

class CVision 
{
public:
	CVision();
	void Create(CActor* pOwner, ByteCode* pByteCode);
	void Reset();

	CActor* ScanForTarget(CActor* pTarget, int mode);

	void ScanFromClassID(int classId, CActorsTable* pTable, int mode);

	bool SV_PointIsInVision(edF32VECTOR4* v0);

	bool _PointIsDetected(edF32VECTOR4* v0, CActor* pTargetActor);

	uint flags;

	CActor* pOwner;

	edF32VECTOR4 location;
	edF32VECTOR4 rotationQuat;

	float field_0x30;
	float visionRange_0x34;
	float field_0x38;
	float field_0x3c;
	float field_0x40;
	float field_0x44;
	CActor* pActor_0x48;
	float field_0x58;
};

#endif // !VISION_H
