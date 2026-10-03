#ifndef CIRCULAR_WAVE_SHOOT_H
#define CIRCULAR_WAVE_SHOOT_H

#include "Actor.h"
#include "Fx.h"
#include "Fx_Spark.h"

class CCircularWaveShoot : public CObject
{
public:
	CCircularWaveShoot();
	virtual bool InitDlistPatchable(int patchId);
	void Create(ByteCode* pByteCode);
	void Init(CActor* pOwner);
	void Reset();
	void Fire(edF32VECTOR4* pPosition);
	void UpdateWaveLife();
	void SparkPosOnCircle();
	void Draw();
	void DrawWavePart(float angle, int part);

	float field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	int field_0x20;
	int field_0x24;
	CActor* pOwner;
	float field_0x2c;
	byte field_0x30;
	edF32VECTOR4 field_0x40;
	CFxSparkNoAlloc<2, 10> aFxSparks[4];
	CActorsTable actorsTable;
	CFxHandle field_0xb14;
};

#endif // CIRCULAR_WAVE_SHOOT_H
