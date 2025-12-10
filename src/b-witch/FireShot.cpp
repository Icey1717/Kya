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
	S_ACTOR_STREAM_REF* pSVar1;

	pSVar1 = reinterpret_cast<S_ACTOR_STREAM_REF*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;
	if (pSVar1->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pSVar1->entryCount * sizeof(S_STREAM_REF<CActor>);
	}

	this->pActorStreamRef = pSVar1;

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
	bool bVar1;
	int iVar2;
	int iVar3;
	CActorProjectile* pProj;

	this->field_0x298 = 0;
	this->field_0x29c = 0;

	if (this->pActorStreamRef == (S_ACTOR_STREAM_REF*)0x0) {
		iVar2 = 0;
	}
	else {
		iVar2 = this->pActorStreamRef->entryCount;
	}

	iVar3 = iVar2 + -1;

	if (iVar2 != 0) {
		iVar2 = iVar3 * 4;
		do {
			pProj = reinterpret_cast<CActorProjectile*>(this->pActorStreamRef->aEntries[iVar3].Get());
			if (pProj != (CActorProjectile*)0x0) {
				pProj->GoToSleep();
			}
			iVar2 = iVar2 + -4;
			bVar1 = iVar3 != 0;
			iVar3 = iVar3 + -1;
		} while (bVar1);
	}
	return;
}

void CFireShot::ManageShots()
{
	bool bVar1;
	Timer* pTVar2;
	int iVar3;
	int iVar4;
	uint uVar5;
	uint uVar6;
	S_SHOT_DATA* pSVar7;
	float fVar7;

	pSVar7 = this->aShots;
	uVar6 = 0;
	uVar5 = this->field_0x298;
	iVar4 = 0;
	while (true) {
		if (this->pActorStreamRef == (S_ACTOR_STREAM_REF*)0x0) {
			iVar3 = 0;
		}
		else {
			iVar3 = this->pActorStreamRef->entryCount;
		}

		if (iVar3 <= (int)uVar6) break;

		if ((uVar5 & 1) != 0) {
			pTVar2 = Timer::GetTimer();
			pSVar7->field_0x18 = pSVar7->field_0x18 + pTVar2->cutsceneDeltaTime;
			bVar1 = false;

			if (this->pActorStreamRef->aEntries[uVar6].Get()->actorState == 6) {
				this->field_0x298 = this->field_0x298 & ~(1 << (uVar6 & 0x1f));
			}
			else {
				if (0.0f < pSVar7->timeToExplode) {
					pTVar2 = Timer::GetTimer();
					fVar7 = pSVar7->timeToExplode - pTVar2->cutsceneDeltaTime;
					pSVar7->timeToExplode = fVar7;

					if ((fVar7 <= 0.0f) && (this->pActorStreamRef->aEntries[uVar6].Get()->actorState != 0xe)) {
						bVar1 = true;
					}
				}

				if (bVar1) {
					reinterpret_cast<CActorProjectile*>(this->pActorStreamRef->aEntries[uVar6].Get())->Die();
				}
			}
		}

		uVar5 = uVar5 >> 1;
		iVar4 = iVar4 + 4;
		uVar6 = uVar6 + 1;
		pSVar7 = pSVar7 + 1;
	}

	return;
}

bool CFireShot::Project(float velocity, edF32VECTOR4* pPosition, edF32VECTOR4* pDirection, CActor* pFiringActor)
{
	CActorProjectile* pProjectile;
	edF32VECTOR4 direction;
	S_SHOT_DATA* pSStack4;

	pProjectile = static_cast<CActorProjectile*>(_ComputeNewShotNoRelease(0.1, pPosition, pDirection, &pSStack4, pFiringActor, false));
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
	S_ACTOR_STREAM_REF* pSVar1;
	bool bVar2;
	uint uVar3;
	uint uVar4;
	int iVar5;
	int iVar6;
	float fVar7;
	float fVar8;
	edF32VECTOR4 eStack16;
	uint puVar3;

	if (this->field_0x290 == 0) {
		pSVar1 = this->pActorStreamRef;
		uVar3 = 0;

		if (pSVar1 != (S_ACTOR_STREAM_REF*)0x0) {
			uVar3 = pSVar1->entryCount;
		}

		uVar4 = this->field_0x298;

		if (uVar4 != (1 << (uVar3 & 0x1f)) - 1U) {
			uVar3 = 0;
			while (true) {
				if (pSVar1 == (S_ACTOR_STREAM_REF*)0x0) {
					bVar2 = (int)uVar3 < 0;
				}
				else {
					bVar2 = (int)uVar3 < pSVar1->entryCount;
				}

				if ((!bVar2) || ((uVar4 & 1) == 0)) break;

				uVar4 = uVar4 >> 1;
				uVar3 = uVar3 + 1;
			}

			this->field_0x298 = this->field_0x298 | 1 << (uVar3 & 0x1f);
			return uVar3;
		}

		if (param_3 != false) {
			iVar6 = 0;
			if (pSVar1 != (S_ACTOR_STREAM_REF*)0x0) {
				iVar6 = pSVar1->entryCount;
			}

			iVar5 = iVar6 + -1;

			if (iVar6 != 0) {
				iVar6 = iVar5 * 4;
				fVar8 = 0.0;
				do {
					edF32Vector4SubHard(&eStack16, &this->pActorStreamRef->aEntries[iVar5].Get()->currentLocation, pLocation);

					fVar7 = edF32Vector4GetLengthSoft(&eStack16);
					if (fVar7 < fVar8) {
						fVar7 = fVar8;
					}

					iVar6 = iVar6 + -4;
					bVar2 = iVar5 != 0;
					iVar5 = iVar5 + -1;
					fVar8 = fVar7;
				} while (bVar2);
			}
		}
	}
	else {
		uVar3 = 1 << (this->field_0x29c & 0x1f);

		if ((this->field_0x298 & uVar3) == 0) {
			this->field_0x298 = this->field_0x298 | uVar3;

			uVar3 = this->field_0x29c;
			this->field_0x29c = uVar3 + 1;

			if (this->pActorStreamRef == (S_ACTOR_STREAM_REF*)0x0) {
				iVar6 = 0;
			}
			else {
				iVar6 = this->pActorStreamRef->entryCount;
			}

			if (iVar6 == 0) {
				trap(7);
			}

			this->field_0x29c = (int)this->field_0x29c % iVar6;
			return uVar3;
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
	S_SHOT_DATA* local_4;

	pProjectile = _ComputeNewShotNoRelease(param_1, pPosition, pDirection, &local_4, pActor, true);

	if ((pProjectile == (CActorProjectile*)0x0) || (pActor == (CActor*)0x0)) {
		bSuccess = false;
	}
	else {
		pProjectile->Project(param_1 / Timer::GetTimer()->cutsceneDeltaTime, &local_4->field_0x0, true, pActor);
		bSuccess = true;
	}

	return bSuccess;
}

void CFireShot::KillAllProjectiles()
{
	uint uVar1;
	int curProjectileIndex;

	if (this->pActorStreamRef == (S_ACTOR_STREAM_REF*)0x0) {
		curProjectileIndex = 0;
	}
	else {
		curProjectileIndex = this->pActorStreamRef->entryCount;
	}

	uVar1 = 1;
	while (curProjectileIndex != 0) {
		if ((this->field_0x298 & uVar1) != 0) {
			static_cast<CActorProjectile*>(this->pActorStreamRef->aEntries[curProjectileIndex + -1].Get())->Die();
		}

		uVar1 = uVar1 << 1;
		curProjectileIndex = curProjectileIndex + -1;
	}

	return;
}


