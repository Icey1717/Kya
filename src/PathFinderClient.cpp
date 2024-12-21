#include "PathFinderClient.h"
#include "PathManager.h"
#include "PathFinder.h"
#include "ActorMovable.h"

CPathFinderClient::CPathFinderClient()
{
	this->id = -1;
	this->pPathDynamic = (CPathDynamic*)0x0;

	return;
}

void CPathFinderClient::Init()
{
	this->id = -1;
	this->pPathDynamic = (CPathDynamic*)0x0;
	this->field_0x8 = 0;
	return;
}

void CPathFinderClient::ChangePathfindingId(CActorMovable* pActor, int newId, edF32VECTOR4* pLocation)
{
	bool cVar1;
	float fVar2;
	float fVar3;
	float fVar4;

	if (newId != this->id) {
		if (this->pPathDynamic != (CPathDynamic*)0x0) {
			CScene::ptable.g_PathManager_004516a0->pBasicPathFinder->DeletePathDynamic(this->pPathDynamic);
			this->pPathDynamic = 0;
		}

		this->id = newId;
		cVar1 = CScene::ptable.g_PathManager_004516a0->pBasicPathFinder->IsValidPosition(pLocation, this->id);

		if (cVar1 != false) {
			this->field_0x8 = 1;

			this->field_0x10 = *pLocation;
		}
	}
	return;
}

void CPathFinderClient::CleanPathDynamic()
{
	if (this->pPathDynamic != (CPathDynamic*)0x0) {
		(CScene::ptable.g_PathManager_004516a0)->pBasicPathFinder->DeletePathDynamic(this->pPathDynamic);
		this->pPathDynamic = (CPathDynamic*)0x0;
	}

	return;
}

bool CPathFinderClient::IsValidPosition(edF32VECTOR4* pPosition)
{
	return CScene::ptable.g_PathManager_004516a0->pBasicPathFinder->IsValidPosition(pPosition, this->id);
}

bool CPathFinderClient::HasPathTo(CActorMovable* pActor, edF32VECTOR4* v0)
{
	CBasicPathFinder* pBasicPathFinder;
	CPathDynamic* pCVar2;
	bool lVar3;
	edF32VECTOR4* pStart;
	bool bHasPathTo;

	bHasPathTo = false;
	pStart = (edF32VECTOR4*)0x0;
	pBasicPathFinder = (CScene::ptable.g_PathManager_004516a0)->pBasicPathFinder;

	lVar3 = pBasicPathFinder->IsValidPosition(&pActor->currentLocation, this->id);
	if (lVar3 == 0) {
		if (this->field_0x8 != 0) {
			pStart = &this->field_0x10;
		}
	}
	else {
		pStart = &pActor->currentLocation;
	}

	if ((pStart != (edF32VECTOR4*)0x0) && (pCVar2 = pBasicPathFinder->NewPathDynamic(pStart, v0), pCVar2 != (CPathDynamic*)0x0)) {
		IMPLEMENTATION_GUARD(
		(*(code*)(pBasicPathFinder->pVTable->base).FindPath)(pBasicPathFinder, pCVar2, this->id);
		lVar3 = (*(code*)pCVar2->pVTable->GetStatus)(pCVar2);
		if ((lVar3 == 0) || (lVar3 = (*(code*)pCVar2->pVTable->GetStatus)(pCVar2), bHasPathTo = false, lVar3 == 1)) {
			bHasPathTo = true;
		}
		(*(pBasicPathFinder->pVTable->base).DeletePathDynamic)(pBasicPathFinder, pCVar2);)
	}

	return bHasPathTo;
}