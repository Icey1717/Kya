#ifndef FIRESHOT_H
#define FIRESHOT_H

#include "Types.h"
#include "Actor.h"

struct ByteCode;
class CActorProjectile;

struct S_SHOT_DATA
{
	edF32VECTOR4 field_0x0;
	float field_0x10;
	float timeToExplode;
	float field_0x18;
};

class CFireShot
{
public:
	CFireShot();
	void Create(ByteCode* pByteCode);
	void Init();
	void Reset();

	void ManageShots();

	bool Project(float velocity, edF32VECTOR4* pPosition, edF32VECTOR4* pDirection, CActor* pFiringActor);
	bool ProjectDirected(float velocity, edF32VECTOR4* pSource, edF32VECTOR4* pTarget, CActor* pFiringActor);

	uint GetProjectileIndex(edF32VECTOR4* pLocation, bool param_3);
	CActorProjectile* _ComputeNewShotNoRelease(float velocity, edF32VECTOR4* pLocation, edF32VECTOR4* pDirection, S_SHOT_DATA** pShotData, CActor* pFiringActor, bool param_7);

	bool FireNewShot(float param_1, edF32VECTOR4* pPosition, edF32VECTOR4* pDirection, CActor* pActor);

	void KillAllProjectiles();

	S_SHOT_DATA aShots[20];

	S_ACTOR_STREAM_REF* pActorStreamRef;
	int field_0x290;
	undefined4 field_0x294;
	uint field_0x298;
	uint field_0x29c;
};

#endif // FIRESHOT_H