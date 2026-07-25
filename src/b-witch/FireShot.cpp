#include "FireShot.h"
#include "MathOps.h"
#include "MemoryStream.h"
#include "ActorProjectile.h"
#include "ActorDCA.h"
#include "TimeController.h"

CFireShot::CFireShot()
{
	this->field_0x290 = 0;
	this->field_0x294 = 0;

	return;
}

void CFireShot::Create(ByteCode* pByteCode)
{
	this->pActorStreamRef = S_ACTOR_STREAM_REF::Create(pByteCode);

	return;
}

void CFireShot::Init()
{
	for (int i = 0; i < this->pActorStreamRef->entryCount; i++) {
		this->pActorStreamRef->aEntries[i].Init();
	}

	return;
}

// Should be in: D:/Projects/b-witch/FireShot.cpp
void CFireShot::Reset()
{
	bool bHasMoreEntries;
	int entryCount;
	int entryIndex;
	CActorProjectile* pProj;

	this->field_0x298 = 0;
	this->field_0x29c = 0;

	if (this->pActorStreamRef == (S_ACTOR_STREAM_REF*)0x0) {
		entryCount = 0;
	}
	else {
		entryCount = this->pActorStreamRef->entryCount;
	}

	entryIndex = entryCount + -1;

	if (entryCount != 0) {
		entryCount = entryIndex * 4;
		do {
			pProj = reinterpret_cast<CActorProjectile*>(this->pActorStreamRef->aEntries[entryIndex].Get());
			if (pProj != (CActorProjectile*)0x0) {
				pProj->GoToSleep();
			}
			entryCount = entryCount + -4;
			bHasMoreEntries = entryIndex != 0;
			entryIndex = entryIndex + -1;
		} while (bHasMoreEntries);
	}
	return;
}

void CFireShot::ManageShots()
{
	bool bShouldDie;
	Timer* pTimer;
	int entryCount;
	int offset;
	uint shotMask;
	uint index;
	S_SHOT_DATA* pShotData;
	float remainingTime;

	pShotData = this->aShots;
	index = 0;
	shotMask = this->field_0x298;
	offset = 0;
	while (true) {
		if (this->pActorStreamRef == (S_ACTOR_STREAM_REF*)0x0) {
			entryCount = 0;
		}
		else {
			entryCount = this->pActorStreamRef->entryCount;
		}

		if (entryCount <= (int)index) break;

		if ((shotMask & 1) != 0) {
			pTimer = Timer::GetTimer();
			pShotData->field_0x18 = pShotData->field_0x18 + pTimer->cutsceneDeltaTime;
			bShouldDie = false;

			if (this->pActorStreamRef->aEntries[index].Get()->actorState == 6) {
				this->field_0x298 = this->field_0x298 & ~(1 << (index & 0x1f));
			}
			else {
				if (0.0f < pShotData->timeToExplode) {
					pTimer = Timer::GetTimer();
					remainingTime = pShotData->timeToExplode - pTimer->cutsceneDeltaTime;
					pShotData->timeToExplode = remainingTime;

					if ((remainingTime <= 0.0f) && (this->pActorStreamRef->aEntries[index].Get()->actorState != 0xe)) {
						bShouldDie = true;
					}
				}

				if (bShouldDie) {
					reinterpret_cast<CActorProjectile*>(this->pActorStreamRef->aEntries[index].Get())->Die();
				}
			}
		}

		shotMask = shotMask >> 1;
		offset = offset + 4;
		index = index + 1;
		pShotData = pShotData + 1;
	}

	return;
}

bool CFireShot::Project(float velocity, edF32VECTOR4* pPosition, edF32VECTOR4* pDirection, CActor* pFiringActor)
{
	CActorProjectile* pProjectile;
	edF32VECTOR4 direction;
	S_SHOT_DATA* pShotData;

		pProjectile = static_cast<CActorProjectile*>(_ComputeNewShotNoRelease(0.1f, pPosition, pDirection, &pShotData, pFiringActor, false));
	if (pProjectile != (CActorProjectile*)0x0) {
		edF32Vector4NormalizeHard(&direction, pDirection);
		pProjectile->Project(velocity, &direction, true, pFiringActor);
	}

	return pProjectile != (CActorProjectile*)0x0;
}

bool CFireShot::ProjectDirected(float velocity, edF32VECTOR4* pSource, edF32VECTOR4* pTarget, CActor* pFiringActor)
{
	CActorProjectile* pProjectile;
	edF32VECTOR4 direction;
	S_SHOT_DATA* pShotData;

	edF32Vector4SubHard(&direction, pTarget, pSource);

	pProjectile = static_cast<CActorProjectile*>(_ComputeNewShotNoRelease(0.1f, pSource, &direction, &pShotData, pFiringActor, false));
	if (pProjectile != (CActorProjectile*)0x0) {
		pProjectile->ProjectDirected(velocity, pSource, pTarget, 1, (CActor*)pFiringActor);
	}

	return pProjectile != (CActorProjectile*)0x0;
}

uint CFireShot::GetProjectileIndex(edF32VECTOR4* pLocation, bool param_3)
{
	S_ACTOR_STREAM_REF* pActorStream;
	bool bIsLooping;
	uint entryCount;
	uint usedEntries;
	int iEntryIndex;
	int iEntryCount;
	float distance;
	float maxDistance;
	edF32VECTOR4 locationDiff;
	uint puVar3;

	if (this->field_0x290 == 0) {
		pActorStream = this->pActorStreamRef;
		entryCount = 0;

		if (pActorStream != (S_ACTOR_STREAM_REF*)0x0) {
			entryCount = pActorStream->entryCount;
		}

		usedEntries = this->field_0x298;

		if (usedEntries != (1 << (entryCount & 0x1f)) - 1U) {
			entryCount = 0;
			while (true) {
				if (pActorStream == (S_ACTOR_STREAM_REF*)0x0) {
					bIsLooping = (int)entryCount < 0;
				}
				else {
					bIsLooping = (int)entryCount < pActorStream->entryCount;
				}

				if ((!bIsLooping) || ((usedEntries & 1) == 0)) break;

				usedEntries = usedEntries >> 1;
				entryCount = entryCount + 1;
			}

			this->field_0x298 = this->field_0x298 | 1 << (entryCount & 0x1f);
			return entryCount;
		}

		if (param_3 != false) {
			iEntryCount = 0;
			if (pActorStream != (S_ACTOR_STREAM_REF*)0x0) {
				iEntryCount = pActorStream->entryCount;
			}

			iEntryIndex = iEntryCount + -1;

			if (iEntryCount != 0) {
				iEntryCount = iEntryIndex * 4;
				maxDistance = 0.0;
				do {
					edF32Vector4SubHard(&locationDiff, &this->pActorStreamRef->aEntries[iEntryIndex].Get()->currentLocation, pLocation);

					distance = edF32Vector4GetLengthSoft(&locationDiff);
					if (distance < maxDistance) {
						distance = maxDistance;
					}

					iEntryCount = iEntryCount + -4;
					bIsLooping = iEntryIndex != 0;
					iEntryIndex = iEntryIndex + -1;
					maxDistance = distance;
				} while (bIsLooping);
			}
		}
	}
	else {
		entryCount = 1 << (this->field_0x29c & 0x1f);

		if ((this->field_0x298 & entryCount) == 0) {
			this->field_0x298 = this->field_0x298 | entryCount;

			entryCount = this->field_0x29c;
			this->field_0x29c = entryCount + 1;

			if (this->pActorStreamRef == (S_ACTOR_STREAM_REF*)0x0) {
				iEntryCount = 0;
			}
			else {
				iEntryCount = this->pActorStreamRef->entryCount;
			}

			if (iEntryCount == 0) {
				trap(7);
			}

			this->field_0x29c = (int)this->field_0x29c % iEntryCount;
			return entryCount;
		}
	}

	return 0xffffffff;
}

CActorProjectile* CFireShot::_ComputeNewShotNoRelease(float velocity, edF32VECTOR4* pLocation, edF32VECTOR4* pDirection, S_SHOT_DATA** pShotData, CActor* pFiringActor, bool param_7)
{
	uint projectileIndex;
	CActorProjectile* pRef;

	projectileIndex = GetProjectileIndex(pLocation, param_7);

	if (projectileIndex == 0xffffffff) {
		pRef = (CActorProjectile*)0x0;
	}
	else {
		pRef = reinterpret_cast<CActorProjectile*>(this->pActorStreamRef->aEntries[projectileIndex].Get());
		*pShotData = this->aShots + projectileIndex;
		edF32Vector4SubHard(&(*pShotData)->field_0x0, pDirection, pLocation);
		edF32Vector4NormalizeHard(&(*pShotData)->field_0x0, &(*pShotData)->field_0x0);

		pRef->pCollisionData->Reset();

		if (pFiringActor != (CActorDCA*)0x0) {
			pRef->pCollisionData->actorFieldA = pFiringActor;
			pFiringActor->pCollisionData->actorFieldA = pRef;
		}

		pRef->UpdatePosition(pLocation, true);

		(*pShotData)->field_0x10 = velocity;
		(*pShotData)->timeToExplode = pRef->aProjectileSubObjs->timeToExplode;
		(*pShotData)->field_0x18 = 0.0f;
	}

	return pRef;
}


bool CFireShot::FireNewShot(float param_1, edF32VECTOR4* pPosition, edF32VECTOR4* pDirection, CActor* pActor)
{
	bool bSuccess;
	CActorProjectile* pProjectile;
	S_SHOT_DATA* pShotData;

	pProjectile = _ComputeNewShotNoRelease(param_1, pPosition, pDirection, &pShotData, pActor, true);

	if ((pProjectile == (CActorProjectile*)0x0) || (pActor == (CActor*)0x0)) {
		bSuccess = false;
	}
	else {
		pProjectile->Project(param_1 / Timer::GetTimer()->cutsceneDeltaTime, &pShotData->field_0x0, true, pActor);
		bSuccess = true;
	}

	return bSuccess;
}

bool CFireShot::FireNewShotStraight(edF32VECTOR4* pPosition, edF32VECTOR4* pDirection, CActor* pActor)
{
	CActorProjectile* pProjectile;
	S_SHOT_DATA* pShotData;

	pProjectile = _ComputeNewShotNoRelease(0.1f, pPosition, pDirection, &pShotData, pActor, false);
	if (pProjectile != (CActorProjectile*)0x0) {
		pProjectile->ProjectToGoStraight(pDirection, true, pActor);
	}

	return pProjectile != (CActorProjectile*)0x0;
}



void CFireShot::KillAllProjectiles()
{
	uint projectileBitmask;
	int curProjectileIndex;

	if (this->pActorStreamRef == (S_ACTOR_STREAM_REF*)0x0) {
		curProjectileIndex = 0;
	}
	else {
		curProjectileIndex = this->pActorStreamRef->entryCount;
	}

	projectileBitmask = 1;
	while (curProjectileIndex != 0) {
		if ((this->field_0x298 & projectileBitmask) != 0) {
			static_cast<CActorProjectile*>(this->pActorStreamRef->aEntries[curProjectileIndex + -1].Get())->Die();
		}

		projectileBitmask = projectileBitmask << 1;
		curProjectileIndex = curProjectileIndex + -1;
	}

	return;
}


