#include "ActorElectrolla.h"
#include "ActorHero.h"
#include "MemoryStream.h"
#include "MathOps.h"

void CActorElectrolla::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}

float GetActorsNearWithCriterion(float radius, CActor* pActor, CActorsTable* pTable, ColCallbackFuncPtr* pFunc)
{
	CActorHero* pCVar1;
	bool bVar2;
	float* pfVar3;
	float* pfVar4;
	float* pfVar5;
	int iVar6;
	int iVar7;
	float fVar8;
	float fVar9;
	edF32VECTOR4 local_30;
	_criterion_near_params local_20;

	pfVar3 = reinterpret_cast<float*>(gSP_Manager.GetFreeBuffer(0x100));
	pCVar1 = CActorHero::_gThis;
	local_20.field_0x10 = radius * radius;
	local_20.nearestDistance = 1e+20f;
	local_30.xyz = pActor->currentLocation.xyz;
	local_30.w = radius;
	local_20.pTable = pTable;
	local_20.aDistances = pfVar3;
	local_20.pActor = pActor;

	if ((pActor->actorFieldS & 8) == 0) {
		CScene::ptable.g_ActorManager_004516a4->cluster.ApplyCallbackToActorsIntersectingSphere(&local_30, pFunc, &local_20);
	}
	else {
		bVar2 = CActorHero::_gThis->SV_IsWorldBoundingSphereIntersectingSphere(&local_30);
		if (bVar2 != false) {
			pFunc(pCVar1, &local_20);
		}
	}

	iVar6 = 0;
	iVar7 = 0;
	fVar9 = sqrtf(local_20.nearestDistance);
	pfVar4 = pfVar3;
	pfVar5 = pfVar3;
	if (0 < pTable->nbEntries) {
		do {
			if (*pfVar4 < *pfVar5) {
				pTable->Swap(iVar7, iVar6);
				fVar8 = *pfVar4;
				iVar6 = iVar6 + 1;
				*pfVar4 = *pfVar5;
				*pfVar5 = fVar8;
				pfVar5 = pfVar5 + 1;
			}
			iVar7 = iVar7 + 1;
			pfVar4 = pfVar4 + 1;
		} while (iVar7 < pTable->nbEntries);
	}

	gSP_Manager.ReleaseBuffer(pfVar3);

	return fVar9;
}
