#ifndef VISION_H
#define VISION_H

#include "Types.h"

#define SCAN_MODE_SYNCHRONOUS	0
#define SCAN_MODE_AMORTISED		1

struct ByteCode;

struct _vision_param
{
	float field_0x0;
	float field_0x4;
	float field_0x8;
	float field_0xc;
	int field_0x10;
	CActor* pActor_0x14;
};

class CVision 
{
public:
	CVision();
	void Create(_vision_param* pParams);
	void Create(CActor* pOwner, ByteCode* pByteCode);
	void Init(float param_1, float range, float param_3, float param_4);
	void Reset();
	void Draw() {}

	CActor* ScanForTarget(CActor* pTarget, int mode);

	void ScanFromClassID(int classId, CActorsTable* pTable, int mode);

	bool SV_PointIsInVision(edF32VECTOR4* v0);
	CActor* SV_GetNearestActor(ACTOR_CLASS typeId, int scanMode);

	bool _PointIsDetected(edF32VECTOR4* v0, CActor* pTargetActor);
	uint _ScanForTargetMultiPoint(CActor* pTargetActor, int mode);

	void ScanAccurate(float param_1, CActorsTable* pTable, int param_4);
	void Scan(CActorsTable* pTable, int param_3);

	CActor* _GetBestActorInFrontOf(CActorsTable* pTable);
	CActor* SV_GetBestActorInFrontOf(int mode);

	uint flags;

	CActor* pOwner;

	edF32VECTOR4 location;
	edF32VECTOR4 rotationQuat;

	float halfAngle;
	float visionRange;
	float field_0x38;
	float field_0x3c;
	float apexOffset;
	float field_0x44;
	CActor* pActor_0x48;
	int amortisedScanFrameIndex;
	uint field_0x50;
	int field_0x54;
	float field_0x58;
};

#endif // !VISION_H
