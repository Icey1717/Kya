#ifndef ACTOR_SHOOT_SERVICE_H
#define ACTOR_SHOOT_SERVICE_H

#include "Types.h"

class ByteCode;

class CConicalWaveShoot
{
public:
	CConicalWaveShoot();
	~CConicalWaveShoot();

	void Create(ByteCode* pByteCode);
	void Init(CActor* pOwner);
	void Reset();
	void Fire(edF32VECTOR4* pPosition, edF32VECTOR4* pDirection);
	void Manage();
	void Draw();
	void DrawWavePart(float angle, int part);

	float field_0x0;
	float field_0x4;
	float field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;
	float field_0x18;
	CActor* pOwner;
	float field_0x20;
	byte field_0x24;
	edF32VECTOR4 field_0x30;
	edF32VECTOR4 field_0x40;
	CActorsTable actorsTable;
};

#endif // ACTOR_SHOOT_SERVICE_H
