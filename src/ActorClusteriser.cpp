#include "ActorClusteriser.h"
#include "MemoryStream.h"
#include "ActorManager.h"
#include "camera.h"
#include "CameraViewManager.h"
#include "ActorHero.h"
#include "FileManager3D.h"
#include "SectorManager.h"
#include "EventManager.h"
#include "MathOps.h"
#include "PoolAllocators.h"

void CBehaviourClusteriserZones::Create(ByteCode* pByteCode)
{
	int iVar1;
	int* piVar2;
	_S_ZONE_CLUSTER* p_Var3;
	uint uVar4;
	int iVar5;
	_S_ZONE_CLUSTER* puVar6;

	iVar1 = pByteCode->GetS32();
	this->nbZoneClusters = iVar1;

	uVar4 = this->nbZoneClusters;
	if (uVar4 == 0) {
		this->aZoneClusters = (_S_ZONE_CLUSTER*)0x0;
	}
	else {
		this->aZoneClusters = new _S_ZONE_CLUSTER[uVar4];
		iVar1 = 0;
		if (0 < this->nbZoneClusters) {
			do {
				puVar6 = this->aZoneClusters + iVar1;
				uVar4 = pByteCode->GetU32();
				puVar6->flags = uVar4;

				iVar5 = pByteCode->GetS32();
				puVar6->field_0x4.index = iVar5;

				piVar2 = (int*)pByteCode->currentSeekPos;
				pByteCode->currentSeekPos = (char*)(piVar2 + 1);
				if (*piVar2 != 0) {
					pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 4;
				}

				puVar6->field_0xc = reinterpret_cast<S_ACTOR_STREAM_REF*>(piVar2);
				piVar2 = (int*)pByteCode->currentSeekPos;
				pByteCode->currentSeekPos = (char*)(piVar2 + 1);
				if (*piVar2 != 0) {
					pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 4;
				}
				puVar6->field_0x10 = reinterpret_cast<S_ACTOR_STREAM_REF*>(piVar2);
				puVar6->nbActors = 0;
				puVar6->field_0x18 = 0;

				piVar2 = (int*)pByteCode->currentSeekPos;
				pByteCode->currentSeekPos = (char*)(piVar2 + 1);
				if (*piVar2 != 0) {
					pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 4;
				}
				puVar6->field_0x1c = reinterpret_cast<S_CAMERA_STREAM_REF*>(piVar2);;
				puVar6->nbEvents = 0;
				puVar6->field_0x24 = 0;

				piVar2 = (int*)pByteCode->currentSeekPos;
				pByteCode->currentSeekPos = (char*)(piVar2 + 1);
				if (*piVar2 != 0) {
					pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 4;
				}
				puVar6->field_0x28 = reinterpret_cast<S_LIGHT_STREAM_REF*>(piVar2);;
				puVar6->nbLights = 0;
				puVar6->field_0x30 = 0;

				piVar2 = (int*)pByteCode->currentSeekPos;
				pByteCode->currentSeekPos = (char*)(piVar2 + 1);
				if (*piVar2 != 0) {
					pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 8;
				}
				puVar6->field_0x34 = piVar2;
				iVar5 = pByteCode->GetS32();
				puVar6->field_0x8.index = iVar5;
				iVar1 = iVar1 + 1;
			} while (iVar1 < this->nbZoneClusters);
		}
	}
	return;
}

CCluster* CBehaviourClusteriserZones::_pall_cluster = (CCluster*)0x0;
CClusterNode* CBehaviourClusteriserZones::_pact_nodes = (CClusterNode*)0x0;

void CBehaviourClusteriserZones::Init(CActor* pOwner)
{
	uint nbActors;
	int iVar1;
	ed_zone_3d* pZone;
	CActorClusteriser* pCVar2;
	CCamera* pCVar3;
	CLight* pCVar4;
	CCameraManager* pCameraManager;
	CActorManager* pActorManager;
	CEventManager* pCVar7;
	CCluster* pCluster;
	int* piVar8;
	CActor** aActors;
	CCamera** aCameras;
	undefined4 uVar9;
	long lVar10;
	int pCVar11;
	int iVar12;
	_S_ZONE_CLUSTER* pZoneCluster;
	int iVar13;
	S_ACTOR_STREAM_REF* pSVar14;
	CCamera** ppCVar15;
	CLight** ppCVar16;
	CLight** ppCVar17;
	int iVar18;
	int iVar19;
	CActor** ppCVar20;
	int iVar21;
	CLight** ppCVar22;
	CLight** aLights;
	CLightManager* pLightManager;

	CActor* pActor;

	this->pOwner = reinterpret_cast<CActorClusteriser*>(pOwner);

	pLightManager = CScene::ptable.g_LightManager_004516b0;
	pActorManager = CScene::ptable.g_ActorManager_004516a4;
	pCameraManager = CScene::ptable.g_CameraManager_0045167c;
	nbActors = (CScene::ptable.g_ActorManager_004516a4)->nbActors;
	const int nbLights = (CScene::ptable.g_LightManager_004516b0)->lightCount;

	edMemSetFlags(TO_HEAP(H_MAIN), 0x100);

	if (_pall_cluster == (CCluster*)0x0) {
		pCluster = new CCluster;
		_pall_cluster = pCluster;
		pCluster->Init(0, pActorManager->aSectorBoundingBoxes, 0);

		_pact_nodes = new CClusterNode[nbActors];

		iVar18 = 0;
		if (0 < nbActors) {
			do {
				if (iVar18 == -1) {
					pActor = (CActor*)0x0;
				}
				else {
					pActor = pActorManager->aActors[iVar18];
				}
				_pact_nodes[iVar18].Insert(_pall_cluster, pActor);
				iVar18 = iVar18 + 1;
			} while (iVar18 < nbActors);
		}
	}

	aActors = new CActor*[nbActors];
	aCameras = new CCamera*[pCameraManager->loadedCameraCount_0x9f0];

	if (nbLights == 0) {
		aLights = (CLight**)0x0;
	}
	else {
		aLights = new CLight*[nbLights];
	}

	edMemClearFlags(TO_HEAP(H_MAIN), 0x100);

	iVar21 = 0;
	if (0 < this->nbZoneClusters) {
		do {
	
			pZoneCluster = this->aZoneClusters + iVar21;
			pZoneCluster->field_0x4.Init();

			pSVar14 = pZoneCluster->field_0xc;
			for (pCVar11 = pSVar14->entryCount; pCVar11 != 0x0; pCVar11 = pCVar11 - 1)
			{
				pSVar14->aEntries[pCVar11 - 1].Init();
			}

			pSVar14 = pZoneCluster->field_0x10;
			for (pCVar11 = pSVar14->entryCount; pCVar11 != 0x0;  pCVar11 = pCVar11 - 1)
			{
				pSVar14->aEntries[pCVar11 - 1].Init();
			}

			S_CAMERA_STREAM_REF* pCameraStreamRef = pZoneCluster->field_0x1c;

			for (pCVar11 = pCameraStreamRef->entryCount; pCVar11 != 0x0; pCVar11 = pCVar11 - 1)
			{
				pCameraStreamRef->aEntries[pCVar11 - 1].Init();
			}

			S_LIGHT_STREAM_REF* pLightStreamRef = pZoneCluster->field_0x28;

			for (pCVar11 = pLightStreamRef->entryCount; pCVar11 != 0x0; pCVar11 = pCVar11 - 1)
			{
				pLightStreamRef->aEntries[pCVar11 - 1].Init();
			}

			for (iVar12 = *pZoneCluster->field_0x34; iVar12 != 0; iVar12 = iVar12 + -1) {
			}


			pZoneCluster->field_0x8.Init();

			if ((pZoneCluster->flags & 0x30) != 0) {
				iVar12 = 0;

				if (pZoneCluster->field_0xc->entryCount != 0x0) {
					iVar12 = pZoneCluster->field_0xc->entryCount;
				}

				iVar13 = 0;
				if (0 < iVar12) {
					do {
						pActor = pZoneCluster->field_0xc->aEntries[iVar13].Get();

						if (pActor != (CActor*)0x0) {
							if ((pZoneCluster->flags & 0x10) != 0) {
								if ((pZoneCluster->flags & 0x100) == 0) {
									pActor->flags = pActor->flags | 8;
									pActor->flags = pActor->flags & 0xffffffef;
									pActor->EvaluateManageState();
								}
								else {
									pActor->flags = pActor->flags | 0x18;
									pActor->EvaluateManageState();
								}
								IMPLEMENTATION_GUARD_LOG(
								(*(code*)pActor->pVTable->field_0xd4)(pActor, pZoneCluster->field_0x8);)
							}
							if ((pZoneCluster->flags & 0x20) != 0) {
								pActor->flags = pActor->flags | 0x200;
								pActor->EvaluateDisplayState();
								IMPLEMENTATION_GUARD_LOG(
								(*(code*)pActor->pVTable->field_0xd8)(pActor, pZoneCluster->field_0x8);)
							}
						}

						iVar13 = iVar13 + 1;
					} while (iVar13 < iVar12);
				}
			}

			if ((pZoneCluster->flags & 0x80) != 0) {
				iVar12 = 0;
				if (pZoneCluster->field_0x28->entryCount != 0x0) {
					iVar12 = pZoneCluster->field_0x28->entryCount;
				}

				iVar13 = 0;
				if (0 < iVar12) {
					do {
						CLight* pLight = pZoneCluster->field_0x28->aEntries[iVar13].Get();

						if (pLight != (CLight*)0x0) {
							pLightManager->SetManagedByCluster(pLight);
						}

						iVar13 = iVar13 + 1;
					} while (iVar13 < iVar12);
				}
			}

			TriggerSceneries(pZoneCluster, 0);

			pZone = pZoneCluster->field_0x4.Get();
			if (pZone != (ed_zone_3d*)0x0) {
				if ((pZoneCluster->flags & 0x30) != 0) {
					uVar9 = this->pOwner->FillActorsTableFromZone(aActors, pZone, _pall_cluster, &pZoneCluster->field_0x10);
					pZoneCluster->nbActors = uVar9;

					if (pZoneCluster->nbActors != 0) {
						pZoneCluster->field_0x18 = (CActor**)NewPool_Pointer(pZoneCluster->nbActors);
						iVar12 = 0;
						if (0 < pZoneCluster->nbActors) {
							ppCVar20 = aActors;
							do {
								pActor = *ppCVar20;
								pZoneCluster->field_0x18[iVar12] = pActor;
								if ((pZoneCluster->flags & 0x10) != 0) {
									if ((pZoneCluster->flags & 0x100) == 0) {
										pActor->flags = pActor->flags | 8;
										pActor->flags = pActor->flags & 0xffffffef;
										pActor->EvaluateManageState();
									}
									else {
										pActor->flags = pActor->flags | 0x18;
										pActor->EvaluateManageState();
									}
									IMPLEMENTATION_GUARD_LOG(
									(*(code*)pActor->pVTable->field_0xd4)(pActor, pZoneCluster->field_0x8);)
								}
								if ((pZoneCluster->flags & 0x20) != 0) {
									pActor->flags = pActor->flags | 0x200;
									pActor->EvaluateDisplayState();
									IMPLEMENTATION_GUARD_LOG(
									(*(code*)pActor->pVTable->field_0xd8)(pActor, pZoneCluster->field_0x8);)
								}
								iVar12 = iVar12 + 1;
								ppCVar20 = ppCVar20 + 1;
							} while (iVar12 < (int)pZoneCluster->nbActors);
						}
					}
				}

				pCVar7 = CScene::ptable.g_EventManager_006f5080;
				if ((pZoneCluster->flags & 0x40) != 0) {
					pCVar2 = this->pOwner;
					iVar12 = 0;
					ppCVar15 = aCameras;
					for (pCVar3 = (CScene::ptable.g_CameraManager_0045167c)->pInitialView_0x4b4; pCVar3 != (CCamera*)0x0;
						pCVar3 = pCVar3->pNextCameraView_0xa4) {
						iVar13 = pCVar2->objectId;
						if (((iVar13 == -1) || (pCVar3->objectId == iVar13)) &&
							(iVar13 = edEventComputeZoneAgainstVertex(pCVar7->activeChunkId, pZone, &pCVar3->transformationMatrix.rowT, 0), iVar13 != 2)) {
							*ppCVar15 = pCVar3;
							iVar12 = iVar12 + 1;
							ppCVar15 = ppCVar15 + 1;
						}
					}

					pZoneCluster->nbEvents = iVar12;

					if (pZoneCluster->nbEvents != 0) {
						IMPLEMENTATION_GUARD(
						iVar12 = NewPool_Pointer((long)(int)pZoneCluster->nbEvents);
						pZoneCluster->field_0x24 = iVar12;
						iVar12 = 0;
						if (0 < (int)pZoneCluster->nbEvents) {
							iVar13 = 0;
							ppCVar15 = aCameras;
							do {
								iVar12 = iVar12 + 1;
								*(CCamera**)(pZoneCluster->field_0x24 + iVar13) = *ppCVar15;
								ppCVar15 = ppCVar15 + 1;
								iVar13 = iVar13 + 4;
							} while (iVar12 < (int)pZoneCluster->nbEvents);
						})
					}
				}

				if (((pZoneCluster->flags & 0x80) != 0) && (aLights != (CLight**)0x0)) {
					ppCVar17 = (CScene::ptable.g_LightManager_004516b0)->aLights;
					ppCVar22 = ppCVar17 + (CScene::ptable.g_LightManager_004516b0)->lightCount;
					iVar12 = 0;
					ppCVar16 = aLights;

					for (; ppCVar17 < ppCVar22; ppCVar17 = ppCVar17 + 1) {
						pCVar4 = *ppCVar17;
						if ((((pCVar4->colour_0x4).a & 0x40) != 0) && (lVar10 = pCVar4->IsInCluster(pZone), lVar10 != 0)) {
							iVar12 = iVar12 + 1;
							*ppCVar16 = *ppCVar17;
							ppCVar16 = ppCVar16 + 1;
						}
					}

					pZoneCluster->nbLights = iVar12;
					if (pZoneCluster->nbLights != 0) {
						IMPLEMENTATION_GUARD(
						iVar12 = NewPool_Pointer(pZoneCluster->nbLights);
						pZoneCluster->field_0x30 = iVar12;
						iVar12 = 0;
						if (0 < (int)pZoneCluster->nbLights) {
							iVar13 = 0;
							ppCVar17 = aLights;
							do {
								pCVar4 = *ppCVar17;
								*(CLight**)(pZoneCluster->field_0x30 + iVar13) = pCVar4;
								CLightManager::SetManagedByCluster(pLightManager, (int)pCVar4);
								iVar12 = iVar12 + 1;
								ppCVar17 = ppCVar17 + 1;
								iVar13 = iVar13 + 4;
							} while (iVar12 < (int)pZoneCluster->nbLights);
						})
					}
				}
			}

			iVar21 = iVar21 + 1;
		} while (iVar21 < this->nbZoneClusters);
	}

	if (aLights != (CLight**)0x0) {
		delete[] aLights;
	}

	delete[] aActors;
	delete[] aCameras;
	return;
}

void CBehaviourClusteriserZones::Manage()
{
	ed_zone_3d* pZone;
	bool bVar1;
	CActorHero* pCameraTarget;
	int iVar2;
	_S_ZONE_CLUSTER* pZoneCluster;
	int iVar3;
	edF32VECTOR4* peVar4;
	int local_20;
	CCameraManager* pCameraManager;
	CEventManager* pEventManager;
	CActorHero* pHero;

	pEventManager = CScene::ptable.g_EventManager_006f5080;
	pCameraManager = CScene::ptable.g_CameraManager_0045167c;
	pHero = CActorHero::_gThis;

	pCameraTarget = (CActorHero*)CScene::ptable.g_CameraManager_0045167c->pActiveCamera->GetTarget();
	if (pHero == pCameraTarget) {
		pCameraTarget = (CActorHero*)0x0;
	}

	iVar3 = 0;
	if (0 < this->nbZoneClusters) {
		peVar4 = (edF32VECTOR4*)&(pCameraManager->transformationMatrix).da;
		do {
			pZoneCluster = this->aZoneClusters + iVar3;
			pZone = pZoneCluster->field_0x8.Get();
			bVar1 = false;

			if (pZone != (ed_zone_3d*)0x0) {
				if (pHero != (CActorHero*)0x0) {
					if ((pZoneCluster->flags & 1) == 0) {
						if (((((pZoneCluster->flags & 4) != 0) &&
							(iVar2 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone,&pHero->currentLocation, 0), iVar2 == 2)) &&
							(bVar1 = true, pCameraTarget != (CActorHero*)0x0)) && (iVar2 = edEventComputeZoneAgainstVertex
							(pEventManager->activeChunkId, pZone, &pCameraTarget->currentLocation, 0), iVar2 != 2))
						{
							bVar1 = false;
						}
					}
					else {
						iVar2 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, &pHero->currentLocation, 0);
						bVar1 = iVar2 != 2;

						if (pCameraTarget != (CActorHero*)0x0) {
							iVar2 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, &pCameraTarget->currentLocation, 0);
							bVar1 = bVar1 || iVar2 != 2;
						}
					}
				}

				if ((pZoneCluster->flags & 2) == 0) {
					if ((pZoneCluster->flags & 8) != 0) {
						iVar2 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, peVar4, 0);
						bVar1 = (bool)(bVar1 | iVar2 == 2);
					}
				}
				else {
					iVar2 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, peVar4, 0);
					bVar1 = (bool)(bVar1 | iVar2 != 2);
				}
			}

			if (bVar1) {
				if ((pZoneCluster->flags & 0x40000000) == 0) {
					TriggerManageClusterOn(pZoneCluster);
				}
			}
			else {
				if ((pZoneCluster->flags & 0x40000000) != 0) {
					TriggerManageClusterOff(pZoneCluster);
				}
			}

			iVar3 = iVar3 + 1;
		} while (iVar3 < this->nbZoneClusters);
	}

	return;
}

void CBehaviourClusteriserZones::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	if (_pall_cluster != (CCluster*)0x0) {
		delete[] _pact_nodes;
		_pact_nodes = (CClusterNode*)0x0;

		_pall_cluster->Term();
		delete _pall_cluster;
		_pall_cluster = (CCluster*)0x0;
	}

	if (newState == -1) {
		this->pOwner->SetState(0, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}
}

void CBehaviourClusteriserZones::End(int newBehaviourId)
{
	_S_ZONE_CLUSTER* pZoneCluster;
	int iVar2;

	iVar2 = 0;
	if (0 < this->nbZoneClusters) {
		do {
			pZoneCluster = this->aZoneClusters + iVar2;

			if ((pZoneCluster->flags & 0x40000000) != 0) {
				TriggerManageClusterOff(pZoneCluster);
			}

			iVar2 = iVar2 + 1;
		} while (iVar2 < this->nbZoneClusters);
	}
	return;
}

void CBehaviourClusteriserZones::TriggerManageClusterOff(_S_ZONE_CLUSTER* pZoneCluster)
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourClusteriserZones::TriggerManageClusterOn(_S_ZONE_CLUSTER* pZoneCluster)
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourClusteriserZones::TriggerSceneries(_S_ZONE_CLUSTER* pZoneCluster, int param_3)
{
	ulong hash;
	ed_g3d_hierarchy* peVar1;
	ed_g3d_manager* peVar2;
	int iVar3;
	int iVar4;
	int iVar5;
	int iVar6;
	int iVar7;
	int iVar8;
	ed_g3d_manager* pSectorMesh;
	C3DFileManager* pFileManager;
	CSectorManager* pSectorManager;

	pSectorManager = CScene::ptable.g_SectorManager_00451670;
	pFileManager = CScene::ptable.g_C3DFileManager_00451664;

	if (pZoneCluster->field_0x34 == (int*)0x0) {
		iVar8 = 0;
	}
	else {
		iVar8 = *pZoneCluster->field_0x34;
	}

	iVar7 = 0;
	if (0 < iVar8) {
		iVar5 = 0;
		pSectorMesh = &((CScene::ptable.g_SectorManager_00451670)->baseSector).sectorMesh;
		do {
			peVar1 = (ed_g3d_hierarchy*)0x0;
			hash = *(ulong*)((int)pZoneCluster->field_0x34 + iVar5 + 4);
			if (pFileManager->pMeshInfo != (ed_g3d_manager*)0x0) {
				peVar1 = ed3DG3DHierarchyGetFromHashcode(pFileManager->pMeshInfo, hash);
			}
			if (peVar1 == (ed_g3d_hierarchy*)0x0) {
				iVar6 = (pSectorManager->baseSector).loadStage_0x8;
				peVar2 = pSectorMesh;
				if ((iVar6 != 2) && ((iVar6 != 1 || ((pSectorManager->baseSector).sectorIndex != -1)))) {
					peVar2 = (ed_g3d_manager*)0x0;
				}
				if (peVar2 != (ed_g3d_manager*)0x0) {
					peVar1 = ed3DG3DHierarchyGetFromHashcode(peVar2, hash);
				}
				if (peVar1 == (ed_g3d_hierarchy*)0x0) {
					iVar6 = pSectorManager->sectDataCount;
					iVar4 = iVar6 * 0x144;
					while ((iVar6 != 0 && (peVar1 == (ed_g3d_hierarchy*)0x0))) {
						iVar3 = iVar4 + -0x144;
						iVar4 = (int)&pSectorManager->sectorArray[-1].desiredSectorID + iVar4;
						iVar6 = iVar6 + -1;
						if ((*(int*)(iVar4 + 8) == 2) || ((*(int*)(iVar4 + 8) == 1 && (*(int*)(iVar4 + 4) == -1)))) {
							peVar2 = (ed_g3d_manager*)(iVar4 + 0x48);
						}
						else {
							peVar2 = (ed_g3d_manager*)0x0;
						}
						iVar4 = iVar3;
						if (peVar2 != (ed_g3d_manager*)0x0) {
							peVar1 = ed3DG3DHierarchyGetFromHashcode(peVar2, hash);
						}
					}
				}
			}
			if (peVar1 != (ed_g3d_hierarchy*)0x0) {
				IMPLEMENTATION_GUARD(
				if (param_3 == 0) {
					FUN_0029f630(peVar1, 0x40);
				}
				else {
					FUN_0029f620(peVar1, 0x40);
				})
			}
			iVar7 = iVar7 + 1;
			iVar5 = iVar5 + 8;
		} while (iVar7 < iVar8);
	}
	return;
}

CActorClusteriser::CActorClusteriser()
	: CActor()
{

}

void CActorClusteriser::Init()
{
	CActor::Init();

	this->flags = this->flags & 0xfffffffd;
	this->flags = this->flags | 1;
	this->flags = this->flags & 0xffffff7f;
	this->flags = this->flags | 0x20;

	CActor::EvaluateDisplayState();
}

void CActorClusteriser::Reset()
{
	CActor::Reset();

	this->flags = this->flags & 0xfffffffd;
	this->flags = this->flags | 1;
	this->flags = this->flags & 0xffffff7f;
	this->flags = this->flags | 0x20;

	CActor::EvaluateDisplayState();
}

CBehaviour* CActorClusteriser::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 2) {
		pBehaviour = &this->behaviourClusteriserZones;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}
	return pBehaviour;
}


void CActorClusteriser::Sector_Enter()
{
	CBehaviour* pCVar1;
	CBehaviourClusteriserZones::_S_ZONE_CLUSTER* p_Var2;
	int zoneClusterIndex;

	pCVar1 = CActor::GetBehaviour(2);
	if ((pCVar1 != (CBehaviour*)0x0) && (zoneClusterIndex = 0, 0 < (this->behaviourClusteriserZones).nbZoneClusters)) {
		do {
			p_Var2 = this->behaviourClusteriserZones.aZoneClusters + zoneClusterIndex;
			this->behaviourClusteriserZones.TriggerSceneries(p_Var2, p_Var2->flags & 0x40000000);
			zoneClusterIndex = zoneClusterIndex + 1;
		} while (zoneClusterIndex < (this->behaviourClusteriserZones).nbZoneClusters);
	}
	return;
}

struct ClusterizerCallbackParams {
	CEventManager* pEventManager;
	ed_zone_3d* pZone;
	CActor** ppActors;
	int count;
	S_ACTOR_STREAM_REF** field_0x10;
	uint objId;
};

void gClusterCallback_Clusteriser_All(CActor* pActor, void* pData)
{
	bool bVar1;
	int iVar2;
	S_STREAM_REF<CActor>* pCVar3;

	ClusterizerCallbackParams* pClusterizerCallbackParams = (ClusterizerCallbackParams*)pData;

	iVar2 = edEventComputeZoneAgainstVertex(pClusterizerCallbackParams->pEventManager->activeChunkId, pClusterizerCallbackParams->pZone, &pActor->currentLocation, 0);
	if (iVar2 != 2) {
		S_ACTOR_STREAM_REF* pStream = *pClusterizerCallbackParams->field_0x10;

		for (iVar2 = pStream->entryCount; iVar2 != 0; iVar2 = iVar2 + -1) {
			pCVar3 = pStream->aEntries;
			bVar1 = true;
			if (pActor == pCVar3->Get()) goto LAB_001e4e80;
		}

		bVar1 = false;
	LAB_001e4e80:
		if (!bVar1) {
			pClusterizerCallbackParams->ppActors[pClusterizerCallbackParams->count] = pActor;
			pClusterizerCallbackParams->count = pClusterizerCallbackParams->count + 1;
		}
	}
	return;
}

void gClusterCallback_Clusteriser_Sector(CActor* pActor, void* pData)
{
	bool bVar1;
	int iVar2;
	S_STREAM_REF<CActor>* pCVar3;

	ClusterizerCallbackParams* pClusterizerCallbackParams = (ClusterizerCallbackParams*)pData;

	iVar2 = edEventComputeZoneAgainstVertex(pClusterizerCallbackParams->pEventManager->activeChunkId, pClusterizerCallbackParams->pZone, &pActor->currentLocation, 0);

	if (iVar2 != 2) {
		S_ACTOR_STREAM_REF* pStream = *pClusterizerCallbackParams->field_0x10;

		for (iVar2 = pStream->entryCount; iVar2 != 0; iVar2 = iVar2 + -1) {
			pCVar3 = pStream->aEntries;
			bVar1 = true;
			if (pActor == pCVar3->Get()) goto LAB_001e4da0;
		}

		bVar1 = false;

	LAB_001e4da0:
		if ((!bVar1) && ((pActor->objectId == pClusterizerCallbackParams->objId || (pActor->objectId == -1)))) {
			pClusterizerCallbackParams->ppActors[pClusterizerCallbackParams->count] = pActor;
			pClusterizerCallbackParams->count = pClusterizerCallbackParams->count + 1;
		}
	}

	return;
}

int CActorClusteriser::FillActorsTableFromZone(CActor** param_2, ed_zone_3d* pZone, CCluster* pCluster, S_ACTOR_STREAM_REF** param_5)
{
	int iVar1;
	edF32MATRIX4* peVar2;
	edF32VECTOR4* pSphere;
	ClusterizerCallbackParams local_90;
	edF32VECTOR4 local_70;
	edF32MATRIX4 eStack96;
	edF32VECTOR4 local_20;
	e_ed_event_prim3d_type eStack4;

	local_90.pEventManager = CScene::ptable.g_EventManager_006f5080;
	pSphere = &pZone->boundSphere;

	if (((pZone->boundSphere).w == 0.0f) &&
		(iVar1 = edEventGetChunkZoneNbInclusivePrimitives((CScene::ptable.g_EventManager_006f5080)->activeChunkId, pZone), iVar1 == 1)) {
		peVar2 = edEventGetChunkZonePrimitive((local_90.pEventManager)->activeChunkId, pZone, 0, &eStack4);
		edF32Matrix4GetInverseGaussSoft(&eStack96, peVar2);

		static edF32VECTOR4 edF32VECTOR4_0040ed80 = { 1.0f, 1.0f, 1.0f, 1.0f };

		local_70 = edF32VECTOR4_0040ed80;
		edF32Matrix4MulF32Vector4Hard(&local_70, &eStack96, &local_70);
		local_70.xyz = local_70.xyz - eStack96.rowT.xyz;
		local_70.w = 0.0f;

		local_20.xyz = eStack96.rowT.xyz;
		local_20.w = edF32Vector4GetDistHard(&local_70);
		pSphere = &local_20;
	}

	if (pSphere->w == 0.0f) {
		local_90.count = 0;
	}
	else {
		local_90.count = 0;
		local_90.objId = this->objectId;
		local_90.pZone = pZone;
		local_90.ppActors = param_2;
		local_90.field_0x10 = param_5;

		if (this->objectId == -1) {
			pCluster->ApplyCallbackToActorsIntersectingSphere(pSphere, gClusterCallback_Clusteriser_All, &local_90);
		}
		else {
			pCluster->ApplyCallbackToActorsIntersectingSphere(pSphere, gClusterCallback_Clusteriser_Sector, &local_90);
		}
	}

	return local_90.count;
}
