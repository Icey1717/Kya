#include "ActorFogManager.h"
#include "MemoryStream.h"
#include "ActorHero.h"
#include "CameraViewManager.h"
#include "EventManager.h"
#include "MathOps.h"

void CActorFogManager::Create(ByteCode* pByteCode)
{
	uint count;
	int curIndex;
	CFogHierarchy* pHier;
	float fVar4;

	CActor::Create(pByteCode);

	this->fogHierarchiesArray.nbFogHierarchies = pByteCode->GetS32();
	count = this->fogHierarchiesArray.nbFogHierarchies;

	if (count != 0) {
		this->fogHierarchiesArray.aFogHierarchies = new CFogHierarchy[this->fogHierarchiesArray.nbFogHierarchies];
		curIndex = 0;
		if (0 < this->fogHierarchiesArray.nbFogHierarchies) {
			do {
				pHier = this->fogHierarchiesArray.aFogHierarchies + curIndex;
				pHier->Create(pByteCode);

				pHier->field_0x20 = pByteCode->GetF32();
				pHier->field_0x24 = pByteCode->GetF32();

				curIndex = curIndex + 1;
			} while (curIndex < this->fogHierarchiesArray.nbFogHierarchies);
		}
	}

	return;
}

void CActorFogManager::Init()
{
	int curIndex;

	CActor::Init();

	this->flags = this->flags | 2;
	this->flags = this->flags & 0xfffffffe;
	this->flags = this->flags & 0xffffff7f;
	this->flags = this->flags | 0x20;

	EvaluateDisplayState();

	curIndex = 0;
	if (0 < this->fogHierarchiesArray.nbFogHierarchies) {
		do {
			this->fogHierarchiesArray.aFogHierarchies[curIndex].Init(this->sectorId);
			curIndex = curIndex + 1;
		} while (curIndex < this->fogHierarchiesArray.nbFogHierarchies);
	}

	return;
}

void CActorFogManager::Term()
{
	int curIndex;

	curIndex = 0;
	if (0 < this->fogHierarchiesArray.nbFogHierarchies) {
		do {
			this->fogHierarchiesArray.aFogHierarchies[curIndex].Term(this->sectorId);
			curIndex = curIndex + 1;
		} while (curIndex < this->fogHierarchiesArray.nbFogHierarchies);
	}

	CActor::Term();

	return;
}

void CActorFogManager::Reset()
{
	CActor::Reset();

	this->flags = this->flags | 2;
	this->flags = this->flags & 0xfffffffe;
	this->flags = this->flags & 0xffffff7f;
	this->flags = this->flags | 0x20;

	EvaluateDisplayState();

	return;
}

CBehaviour* CActorFogManager::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 2) {
		pBehaviour = &this->behaviourFogManagerZones;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

void CBehaviourFogManagerZones::Create(ByteCode* pByteCode)
{
	CStreamFogZoneInternal* pFogZoneArray;

	pFogZoneArray = reinterpret_cast<CStreamFogZoneInternal*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = reinterpret_cast<char*>(pFogZoneArray->aFogZones);
	if (pFogZoneArray->nbFogZones != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pFogZoneArray->nbFogZones * sizeof(CStreamFogZone);
	}

	this->streamFogZoneArray.pInternal = pFogZoneArray;

	return;
}

void CBehaviourFogManagerZones::Init(CActor* pOwner)
{
	CStreamFogZone* pFogZone;
	int curIndex;

	this->pOwner = static_cast<CActorFogManager*>(pOwner);
	pFogZone = this->streamFogZoneArray.pInternal->aFogZones;
	for (curIndex = this->streamFogZoneArray.pInternal->nbFogZones; curIndex != 0; curIndex = curIndex + -1) {
		pFogZone->zoneRef.Init();
		pFogZone = pFogZone + 1;
	}

	return;
}

void CBehaviourFogManagerZones::Term()
{
	return;
}

void CBehaviourFogManagerZones::Manage()
{
	this->streamFogZoneArray.Manage();
	this->pOwner->fogHierarchiesArray.Manage();

	return;
}

void CBehaviourFogManagerZones::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CStreamFogZone* pFogZone;
	int curIndex;

	if (newState == -1) {
		this->pOwner->SetState(0, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	pFogZone = this->streamFogZoneArray.pInternal->aFogZones;
	for (curIndex = this->streamFogZoneArray.pInternal->nbFogZones; curIndex != 0; curIndex = curIndex + -1) {
		if ((pFogZone->flags & 2) != 0) {
			pFogZone->flags = pFogZone->flags & 0xfffffffd;
			CScene::_pinstance->PopFogAndClippingSettings(&pFogZone->fogDef);
		}

		pFogZone = pFogZone + 1;
	}

	return;
}

void CBehaviourFogManagerZones::End(int newBehaviourId)
{
	CStreamFogZone* pFogZone;
	int curIndex;

	pFogZone = this->streamFogZoneArray.pInternal->aFogZones;
	for (curIndex = this->streamFogZoneArray.pInternal->nbFogZones; curIndex != 0; curIndex = curIndex + -1) {
		if ((pFogZone->flags & 2) != 0) {
			pFogZone->flags = pFogZone->flags & 0xfffffffd;
			CScene::_pinstance->PopFogAndClippingSettings(&pFogZone->fogDef);
		}

		pFogZone = pFogZone + 1;
	}

	return;
}

void CStreamFogZoneArray::Manage()
{
	ed_zone_3d* pZone;
	bool bVar1;
	CActorHero* pCVar2;
	int iVar3;
	CStreamFogZone* pFogZone;
	int curIndex;
	CCameraManager* pCameraManager;
	CEventManager* pEventManager;
	CActorHero* pHero;

	pEventManager = CScene::ptable.g_EventManager_006f5080;
	pCameraManager = CScene::ptable.g_CameraManager_0045167c;

	pHero = CActorHero::_gThis;
	pCVar2 = static_cast<CActorHero*>((CScene::ptable.g_CameraManager_0045167c)->pActiveCamera->GetTarget());
	if ((pHero != (CActorHero*)0x0) && (pHero == pCVar2)) {
		pCVar2 = (CActorHero*)0x0;
	}

	curIndex = this->pInternal->nbFogZones;
	pFogZone = this->pInternal->aFogZones;

	if (curIndex != 0) {
		do {
			pZone = (pFogZone->zoneRef).Get();
			bVar1 = false;
			if (pZone != (ed_zone_3d*)0x0) {
				if ((pFogZone->flags & 1) != 0) {
					if (pHero != (CActorHero*)0x0) {
						iVar3 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, &pHero->currentLocation, 0);
						bVar1 = iVar3 != 2;
					}

					if (pCVar2 != (CActorHero*)0x0) {
						iVar3 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, &pCVar2->currentLocation, 0);
						bVar1 = static_cast<bool>(bVar1 | iVar3 != 2);
					}
				}

				if ((pFogZone->flags & 2) != 0) {
					iVar3 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, (edF32VECTOR4*)&(pCameraManager->transformationMatrix).da, 0);
					bVar1 = static_cast<bool>(bVar1 | iVar3 != 2);
				}
			}

			if (bVar1) {
				if ((pFogZone->flags & 2) == 0) {
					CScene::_pinstance->PushFogAndClippingSettings(pFogZone->field_0x18, &pFogZone->fogDef);
					pFogZone->flags = pFogZone->flags | 2;
				}
			}
			else {
				if ((pFogZone->flags & 2) != 0) {
					pFogZone->flags = pFogZone->flags & 0xfffffffd;
					CScene::_pinstance->PopFogAndClippingSettings(&pFogZone->fogDef);
				}
			}

			curIndex = curIndex + -1;
			pFogZone = pFogZone + 1;
		} while (curIndex != 0);
	}

	return;
}

CFogHierarchiesArray::CFogHierarchiesArray()
{
	this->nbFogHierarchies = 0;
	this->aFogHierarchies = (CFogHierarchy*)0x0;

	return;
}

void CFogHierarchiesArray::Manage()
{
	edNODE* pNode;
	ed_3d_hierarchy* pHier;
	edF32MATRIX4* peVar2;
	CCameraManager* pCameraManager;
	byte alpha;
	CFogHierarchy* pFogHier;
	int iVar5;
	float fVar6;
	float fVar7;
	edF32VECTOR4 local_10;

	pCameraManager = CScene::ptable.g_CameraManager_0045167c;

	for (iVar5 = 0; iVar5 < this->nbFogHierarchies; iVar5 = iVar5 + 1) {
		pFogHier = this->aFogHierarchies + iVar5;
		pNode = pFogHier->pNode;

		if (pNode != (edNODE*)0x0) {
			pHier = reinterpret_cast<ed_3d_hierarchy*>(pNode->pData);
			edF32VECTOR4* peVar4 = ed3DGetHierarchyFirstLODSphere(pHier);
			if (peVar4 == (edF32VECTOR4*)0x0) {
				local_10 = (pHier->transformA).rowT;
			}
			else {
				local_10.x = peVar4->x;
				local_10.y = peVar4->y;
				local_10.z = peVar4->z;
				local_10.w = 1.0f;

				edF32Matrix4MulF32Vector4Hard(&local_10, &pHier->transformA, &local_10);
			}

			local_10 = local_10 - (CScene::ptable.g_CameraManager_0045167c)->transformationMatrix.rowT;

			fVar7 = edF32Vector4GetDistHard(&local_10);
			alpha = 0x80;
			if (fVar7 < pFogHier->field_0x24) {
				fVar6 = pFogHier->field_0x20;
				alpha = 0;

				if (fVar6 < fVar7) {
					fVar7 = ((fVar7 - fVar6) * 128.0f) / (pFogHier->field_0x24 - fVar6);
					if (fVar7 < 2.147484e+09f) {
						alpha = (byte)static_cast<int>(fVar7);
					}
					else {
						alpha = (byte)static_cast<int>(fVar7 - 2.147484e+09f);
					}
				}
			}

			pFogHier->SetAlpha(alpha);
		}
	}

	return;
}
