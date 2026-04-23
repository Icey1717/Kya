#include "ActorMovingPlatform.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "CollisionManager.h"
#include "ActorCheckpointManager.h"
#include "CameraViewManager.h"
#include "ActorHero.h"
#include "PathManager.h"
#include "PoolAllocators.h"
#include "TimeController.h"

#include <math.h>
#include "FileManager3D.h"
#include "EventManager.h"

template<typename PlatformBehaviourType>
struct PlatformHeader
{
	PlatformBehaviourType* aPlatforms;
	PlatformHeader* pPrev;
	int totalEntries;
	int inUseEntries;

	inline static PlatformBehaviourType* Get(PlatformHeader** pObj) {
		PlatformBehaviourType* pBehaviour;

		bool bRequiresNewHeader = true;

		// Create the header if we don't already have one.
		if (((*pObj) != (PlatformHeader*)0x0) &&
			((*pObj)->inUseEntries + 1 <= (*pObj)->totalEntries)) {
			bRequiresNewHeader = false;
		}

		PlatformHeader* pPrevHeader;
		PlatformHeader* pCurHeader = (*pObj);

		// If the header is new or needs extending, create some new behaviours.
		if ((bRequiresNewHeader) && (pCurHeader = new PlatformHeader, pPrevHeader = (*pObj), pCurHeader != (PlatformHeader*)0x0)) {
			pCurHeader->totalEntries = 0x20;
			pCurHeader->aPlatforms = new PlatformBehaviourType[pCurHeader->totalEntries];
			pCurHeader->pPrev = pPrevHeader;
			pCurHeader->inUseEntries = 0;
		}

		(*pObj) = pCurHeader;

		if ((*pObj)->totalEntries < (*pObj)->inUseEntries + 1) {
			pBehaviour = (PlatformBehaviourType*)0x0;
		}
		else {
			pBehaviour = &(*pObj)->aPlatforms[(*pObj)->inUseEntries];
			(*pObj)->inUseEntries++;
		}

		return pBehaviour;
	}

	inline static void Release(PlatformHeader** pObj) {
		do {
			if ((*pObj) == (PlatformHeader*)0x0) {
				return;
			}

			PlatformHeader* pPrevHeader = (*pObj)->pPrev;

			if ((*pObj)->aPlatforms) {
				delete[](*pObj)->aPlatforms;
			}

			delete (*pObj);
			(*pObj) = pPrevHeader;
		} while (true);
	}
};


PlatformHeader<CBehaviourPlatformTrajectory>* gPlatform_00448e14;
PlatformHeader<CBehaviourPlatformSlab>* gPlatform_00448e18;
PlatformHeader<CBehaviourPlatformDestroyed>* gPlatform_00448e1c;
PlatformHeader<CBehaviourPlatformStand>* gPlatform_00448e20;
PlatformHeader<CBehaviourSelectorNew>* gPlatform_00448e24;
PlatformHeader<CBehaviourSelectorMaster>* gPlatform_00448e28;
PlatformHeader<S_TILT_DATA>* gTiltDataAllocator;

void CActorMovingPlatform::Create(ByteCode* pByteCode)
{
	char* pcVar1;
	uint* puVar2;
	int iVar3;
	uint uVar4;
	int* piVar5;
	S_STREAM_MPF_NO_FRICTION_ZONE* pSVar6;
	int iVar7;
	CBehaviour* pCVar8;
	ulong uVar9;
	int iVar10;
	int iVar11;
	float fVar12;

	CActorMovable::Create(pByteCode);

	piVar5 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar5 + 1);
	if (*piVar5 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar5 * sizeof(S_BRIDGE_ACTOR_STREAM_ENTRY);
	}
	this->pActorStream = (S_BRIDGE_ACTOR_STREAM*)piVar5;

	piVar5 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar5 + 1);
	if (*piVar5 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar5 * sizeof(S_ZONE_STREAM_REF);
	}
	this->pZoneStream = (S_ZONE_STREAM_REF*)piVar5;

	this->noFrictionZoneCount = pByteCode->GetS32();
	this->aNoFrictionZones = (S_STREAM_MPF_NO_FRICTION_ZONE*)0x0;

	const int nbNoFric = this->noFrictionZoneCount;
	if (nbNoFric != 0) {
		this->aNoFrictionZones = new S_STREAM_MPF_NO_FRICTION_ZONE[nbNoFric];
		pSVar6 = this->aNoFrictionZones;
		iVar11 = 0;
		if (0 < this->noFrictionZoneCount) {
			do {
				pSVar6->zoneRef.index = pByteCode->GetS32();
				piVar5 = (int*)pByteCode->currentSeekPos;
				pByteCode->currentSeekPos = (char*)(piVar5 + 1);
				if (*piVar5 != 0) {
					pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar5 * 4;
				}
				pSVar6->pActorStreamRef = (S_ACTOR_STREAM_REF*)piVar5;
				iVar11 = iVar11 + 1;
				pSVar6 = pSVar6 + 1;
			} while (iVar11 < (int)this->noFrictionZoneCount);
		}
	}

	piVar5 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar5 + 1);
	if (*piVar5 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar5 * 0x48;
	}
	this->pCameraStream = (S_BRIDGE_CAMERA_STREAM*)piVar5;

	pcVar1 = pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pcVar1 + sizeof(CActorMovingPlatform_SubObj);
	this->pProperties = reinterpret_cast<CActorMovingPlatform_SubObj*>(pcVar1);
	this->movingPlatformFlags = 0;
	this->field_0x1ec.Reset();
	this->field_0x1f4.Reset();
	
	piVar5 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar5 + 1);
	if (*piVar5 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar5 * 0x10;
	}

	IMPLEMENTATION_GUARD_AUDIO(
	this->field_0x1dc = piVar5;)

	piVar5 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar5 + 1);
	if (*piVar5 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar5 * 0x10;
	}

	IMPLEMENTATION_GUARD_AUDIO(
	this->field_0x1e0 = piVar5;
	)

	CActorMovingPlatform_SubObj* pCVar3 = this->pProperties;
	float fVar17 = pCVar3->field_0x14;
	pCVar3->field_0x14 = pCVar3->field_0x18;
	this->pProperties->field_0x18 = -fVar17;

	CCollision* pCVar4 = this->pCollisionData;
	if (pCVar4 != (CCollision*)0x0) {
		pCVar4->flags_0x0 = pCVar4->flags_0x0 & 0xffffffc4;
		pCVar4->flags_0x0 = pCVar4->flags_0x0 | 0x1800480;

		if ((this->pProperties->flags_0x24 & 0x40) != 0) {
			pCVar4->flags_0x0 = pCVar4->flags_0x0 | 0x100;
		}

		if ((this->pProperties->flags_0x24 & 0x400000) != 0) {
			pCVar4->flags_0x0 = pCVar4->flags_0x0 | 0x200;
		}

		if ((this->pProperties->flags_0x24 & 0x4000) != 0) {
			pCVar4->flags_0x0 = pCVar4->flags_0x0 & 0xffefffff;
		}

		uint uVar6 = this->pProperties->flags_0x24;
		uint uVar12 = 0;
		if ((uVar6 & 0x80) != 0) {
			uVar12 = 0x400;
		}
		if ((uVar6 & 0x100) != 0) {
			uVar12 = uVar12 | 0x800;
		}
		if (uVar12 != 0) {
			CCollision::PatchObbTreeFlagsRecurse(pCVar4->pObbTree, uVar12, -1, 0);
		}
	}

	int iVar15;
	if (this->pCameraStream == (S_BRIDGE_CAMERA_STREAM*)0x0) {
		iVar15 = 0;
	}
	else {
		iVar15 = this->pCameraStream->entryCount;
	}

	int iVar9 = 0;
	if (0 < iVar15) {
		do {
			S_BRIDGE_CAMERA_STREAM_ENTRY* pEntry = &pCameraStream->aEntries[iVar9];

			if (pEntry->field_0x0 == 0xe) {
				this->movingPlatformFlags = this->movingPlatformFlags | 8;
			}
			else {
				if ((pEntry->field_0x0 == 0xc) || (pEntry->field_0x0 == 0xb)) {
					this->movingPlatformFlags = this->movingPlatformFlags | 4;
				}
			}

			iVar9 = iVar9 + 1;
		} while (iVar9 < iVar15);
	}

	if ((this->pProperties->flags_0x24 & 0x1000) != 0) {
		this->movingPlatformFlags = this->movingPlatformFlags | 4;
	}
	if ((this->pProperties->flags_0x24 & 0x400) != 0) {
		this->movingPlatformFlags = this->movingPlatformFlags | 8;
	}

	uint uVar16 = 0;
	iVar9 = 0;
	if (0 < iVar15) {
		S_BRIDGE_CAMERA_STREAM* pSVar13 = this->pCameraStream;
		do {
			if (pSVar13->aEntries[iVar9].field_0x44 != -1) {
				uVar16 = uVar16 + 1;
			}

			iVar9 = iVar9 + 1;
		} while (iVar9 < iVar15);
	}

	if (3 < uVar16) {
		uVar16 = 3;
	}

	pCVar3 = this->pProperties;
	iVar15 = (pCVar3->field_0x0).field_0x0;
	if (iVar15 == -1) {
		iVar15 = (pCVar3->field_0x0).field_0x4;
	}
	if (iVar15 == -1) {
		iVar15 = (pCVar3->field_0x8).field_0x0;
		if (iVar15 == -1) {
			iVar15 = (pCVar3->field_0x8).field_0x4;
		}
		if (iVar15 == -1) goto LAB_0015d240;
	}

	this->movingPlatformFlags = this->movingPlatformFlags | 1;

LAB_0015d240:
	CBehaviour* pCVar10 = GetBehaviour(MOVING_PLATFORM_BEHAVIOUR_STAND);
	CBehaviourPlatformStand* pStand;
	if ((pCVar10 != (CBehaviour*)0x0) &&
		(pStand = (CBehaviourPlatformStand*)GetBehaviour(MOVING_PLATFORM_BEHAVIOUR_STAND), pStand->field_0x8 != -1)) {
		this->movingPlatformFlags = this->movingPlatformFlags | 1;
	}

	IMPLEMENTATION_GUARD_AUDIO(
	CBehaviour* pCVar10 = CActor::GetBehaviour(MOVING_PLATFORM_BEHAVIOUR_SLAB);
	CBehaviourPlatformSlab* pSlab;
	if ((pCVar10 != (CBehaviour*)0x0) &&
		(pSlab = CActor::GetBehaviour(MOVING_PLATFORM_BEHAVIOUR_SLAB), pSlab->field_0x20.Get()-> != (CBehaviourVtable*)0xffffffff)) {
		this->movingPlatformFlags = this->movingPlatformFlags | 2;
	})

	IMPLEMENTATION_GUARD_AUDIO(
	CBehaviour* pCVar10 = CActor::GetBehaviour(MOVING_PLATFORM_BEHAVIOUR_SELECTOR_MASTER);
	if ((pCVar10 != (CBehaviour*)0x0) || (pCVar10 = CActor::GetBehaviour(MOVING_PLATFORM_BEHAVIOUR_SELECTOR_NEW), pCVar10 != (CBehaviour*)0x0)
		) {
		pCVar10 = CActor::GetBehaviour(MOVING_PLATFORM_BEHAVIOUR_SELECTOR_MASTER);
		if (pCVar10 == (CBehaviour*)0x0) {
			pCVar10 = CActor::GetBehaviour(MOVING_PLATFORM_BEHAVIOUR_SELECTOR_NEW);
		}
		if (pCVar10[5].pVTable != (CBehaviourVtable*)0xffffffff) {
			this->movingPlatformFlags = this->movingPlatformFlags | 2;
		}
	})

	if ((this->movingPlatformFlags & 2) != 0) {
		uVar16 = uVar16 + 1;
	}
	if ((this->movingPlatformFlags & 1) != 0) {
		uVar16 = uVar16 + 1;
	}

	if (uVar16 == 0) {
		this->pActorSound = (CActorSound*)0x0;
	}
	else {
		this->pActorSound = CreateActorSound(uVar16);
	}

	this->field_0x1e4 = 0;
	this->field_0x1e8 = (CSound*)0x0;

	return;
}

void CActorMovingPlatform::Init()
{
	int iVar1;
	float fVar2;
	//CActorMovingPlatform_SubObj* pCVar3;
	bool bVar4;
	undefined* puVar5;
	void** ppvVar6;
	void* pvVar7;
	//S_TILT_DATA* pSVar8;
	int iVar9;
	int iVar10;
	edF32MATRIX4* peVar11;
	edF32MATRIX4* peVar12;
	S_STREAM_REF<CActor>* pSVar13;
	S_STREAM_REF<ed_zone_3d>* pStreamRef;
	int* piVar14;
	ed_zone_3d* peVar15;
	S_STREAM_MPF_NO_FRICTION_ZONE* pNoFric;
	CActor* pCVar16;

	this->movingPlatformFlags = this->movingPlatformFlags & 0xffffff0f;
	this->pProperties->tiltStreamDef.Init();
	this->pProperties->pushStreamDef.Init();

	if (((this->pProperties->tiltStreamDef).field_0x4 == 0.0f) && (this->pProperties->pushStreamDef.field_0x0 == 0.0f)) {
		this->pTiltData = (S_TILT_DATA*)0x0;
	}
	else {
		this->pTiltData = PlatformHeader<S_TILT_DATA>::Get(&gTiltDataAllocator);
		this->pTiltData->Init(this->pProperties->field_0x2c, this, &this->pProperties->tiltStreamDef);
		this->pTiltData->pushData.Init();
	}

	this->field_0x200 = gF32Matrix4Unit;

	{
		S_BRIDGE_ACTOR_STREAM_ENTRY* pEntry = this->pActorStream->aEntries;
		for (iVar10 = this->pActorStream->entryCount; iVar10 != 0; iVar10 = iVar10 + -1) {
			pEntry->actorRef.Init();
			pEntry = pEntry + 1;
		}
	}

	{
		S_STREAM_REF<ed_zone_3d>* pEntry = this->pZoneStream->aEntries;
		for (iVar10 = this->pZoneStream->entryCount; iVar10 != 0; iVar10 = iVar10 + -1) {
			pEntry->Init();
			pEntry = pEntry + 1;
		}
	}

	pNoFric = this->aNoFrictionZones;
	iVar10 = 0;
	if (0 < this->noFrictionZoneCount) {
		do {
			pNoFric->zoneRef.Init();
			S_ACTOR_STREAM_REF* pRef = pNoFric->pActorStreamRef;
			for (int i= 0; i < pNoFric->pActorStreamRef->entryCount; i++) {
				pRef[i].aEntries->Init();
			}

			iVar10 = iVar10 + 1;
			pNoFric = pNoFric + 1;
		} while (iVar10 < this->noFrictionZoneCount);
	}

	{
		S_BRIDGE_CAMERA_STREAM_ENTRY* pEntry = this->pCameraStream->aEntries;
		for (iVar10 = this->pCameraStream->entryCount; iVar10 != 0; iVar10 = iVar10 + -1) {
			pEntry->streamTarget.Init();
			pEntry->streamCameraEvent.Init();

			IMPLEMENTATION_GUARD_AUDIO(
				pEntry->field_0x44.FUN_00181e70();
			)
				pEntry = pEntry + 1;
		}
	}

	ForceCarriedStuff();

	IMPLEMENTATION_GUARD_AUDIO(
	pCVar3 = this->pProperties;
	FUN_00181d50(&pCVar3->field_0x0);
	FUN_00181d50(&pCVar3->field_0x8);
	piVar14 = this->field_0x1dc + 1;
	for (iVar10 = *this->field_0x1dc; iVar10 != 0; iVar10 = iVar10 + -1) {
		FUN_00181d50((uint*)(piVar14 + 2));
		piVar14 = piVar14 + 4;
	}
	piVar14 = this->field_0x1e0 + 1;
	for (iVar10 = *this->field_0x1e0; iVar10 != 0; iVar10 = iVar10 + -1) {
		FUN_00181d50((uint*)(piVar14 + 2));
		piVar14 = piVar14 + 4;
	})

	CActor::Init();
	return;
}

CBehaviour* CActorMovingPlatform::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType < 0xb) {
		switch (behaviourType) {
		case 0x0:
		case 0x1:
			pBehaviour = CActor::BuildBehaviour(behaviourType);
		break;
		case MOVING_PLATFORM_BEHAVIOUR_TRAJECTORY:
		{
			pBehaviour = PlatformHeader<CBehaviourPlatformTrajectory>::Get(&gPlatform_00448e14);
		}
		break;
		case MOVING_PLATFORM_BEHAVIOUR_SLAB:
		{
			pBehaviour = PlatformHeader<CBehaviourPlatformSlab>::Get(&gPlatform_00448e18);
		}
		break;
		case MOVING_PLATFORM_BEHAVIOUR_WEIGHING_MACHINE_SLAVE:
		{
			pBehaviour = new CBehaviourWeighingMachineSlave;
		}
		break;
		case MOVING_PLATFORM_BEHAVIOUR_WEIGHING_MACHINE_MASTER:
		{
			pBehaviour = new CBehaviourWeighingMachineMaster;
		}
		break;
		case MOVING_PLATFORM_BEHAVIOUR_DESTROYED:
		{
			pBehaviour = PlatformHeader<CBehaviourPlatformDestroyed>::Get(&gPlatform_00448e1c);
		}
		break;
		case MOVING_PLATFORM_BEHAVIOUR_STAND:
		{
			pBehaviour = PlatformHeader<CBehaviourPlatformStand>::Get(&gPlatform_00448e20);
		}
		break;
		case MOVING_PLATFORM_BEHAVIOUR_SELECTOR_MASTER:
		{
			pBehaviour = PlatformHeader<CBehaviourSelectorMaster>::Get(&gPlatform_00448e28);
		}
		break;
		case MOVING_PLATFORM_BEHAVIOUR_SELECTOR_NEW:
		{
			pBehaviour = PlatformHeader<CBehaviourSelectorNew>::Get(&gPlatform_00448e24);
		}
		break;
		case MOVING_PLATFORM_BEHAVIOUR_TELEPORT_RANDOM:
		{
			pBehaviour = new CBehaviourTeleportRandom;
		}
		break;
		default:
			assert(false);
		}
		return pBehaviour;
	}
	pBehaviour = CActor::BuildBehaviour(behaviourType);
	return pBehaviour;
}

void CActorMovingPlatform::TermBehaviour(int behaviourId, CBehaviour* pBehaviour)
{
	if (behaviourId == MOVING_PLATFORM_BEHAVIOUR_SELECTOR_MASTER) {
		PlatformHeader<CBehaviourSelectorMaster>::Release(&gPlatform_00448e28);
	}
	else {
		if (behaviourId == MOVING_PLATFORM_BEHAVIOUR_SELECTOR_NEW) {
			PlatformHeader<CBehaviourSelectorNew>::Release(&gPlatform_00448e24);
		}
		else {
			if (behaviourId == MOVING_PLATFORM_BEHAVIOUR_STAND) {
				PlatformHeader<CBehaviourPlatformStand>::Release(&gPlatform_00448e20);
			}
			else {
				if (behaviourId == MOVING_PLATFORM_BEHAVIOUR_DESTROYED) {
					PlatformHeader<CBehaviourPlatformDestroyed>::Release(&gPlatform_00448e1c);
				}
				else {
					if (behaviourId == MOVING_PLATFORM_BEHAVIOUR_SLAB) {
						PlatformHeader<CBehaviourPlatformSlab>::Release(&gPlatform_00448e18);
					}
					else {
						if (behaviourId == MOVING_PLATFORM_BEHAVIOUR_TRAJECTORY) {
							PlatformHeader<CBehaviourPlatformTrajectory>::Release(&gPlatform_00448e14);
						}
						else {
							CActor::TermBehaviour(behaviourId, pBehaviour);
						}
					}
				}
			}
		}
	}
	return;
}

bool CActorMovingPlatform::Slab_MoveAndDetectCarriedObject(CBehaviourPlatformSlab* pBehaviour, int param_3)
{
	CCollision* pCVar1;
	S_TIED_ACTOR_ENTRY* pSVar2;
	CActor* pCVar3;
	CActorMovable* pCVar4;
	bool bVar5;
	bool bVar6;
	Timer* pTVar7;
	float fVar8;
	float fVar9;
	edF32VECTOR4 eStack16;

	edF32Vector4ScaleHard(-pBehaviour->field_0x24, &eStack16, &g_xVector);
	edF32Vector4AddHard(&eStack16, &eStack16, &this->baseLocation);
	Platform_UpdatePosition(&eStack16, param_3, (CActorsTable*)0x0);

	pCVar1 = this->pCollisionData;

	if (pCVar1 == (CCollision*)0x0) {
		bVar5 = false;
	}
	else {
		fVar8 = pCVar1->GetCarriedWeight();
		bVar5 = pBehaviour->field_0x10 <= fVar8;
		if (bVar5) {
			pSVar2 = this->pCollisionData->pTiedActorEntry;

			if (pSVar2 == (S_TIED_ACTOR_ENTRY*)0x0) {
				pBehaviour->field_0x2c = (CActor*)0x0;
			}
			else {
				pBehaviour->field_0x2c = pSVar2->pActor;
			}
		}
		else {
			if ((((pBehaviour->field_0x2c != (CActor*)0x0) && (pBehaviour->field_0x24 != 0.0f)) && (bVar6 = SV_AmICarrying(pBehaviour->field_0x2c), bVar6 == false)) &&
				((pCVar3 = pBehaviour->field_0x2c, (pCVar3->flags & 4) != 0 && (bVar6 = pCVar3->IsKindOfObject(2), bVar6 != false)))) {
				pCVar4 = (CActorMovable*)pBehaviour->field_0x2c;
				fVar9 = CCollision::GetObbTreeDistanceFromPosition(this->pCollisionData->pObbTree, &pCVar4->currentLocation);
				if (fVar9 <= 0.3f) {
					fVar9 = pCVar4->GetCumulatedWeight();
					bVar5 = pBehaviour->field_0x10 <= fVar8 + fVar9;
				}
			}
		}
	}

	if (bVar5) {
		pBehaviour->field_0x28 = 0.0f;
	}
	else {
		pTVar7 = Timer::GetTimer();
		pBehaviour->field_0x28 = pBehaviour->field_0x28 + pTVar7->cutsceneDeltaTime;
		pBehaviour->field_0x2c = (CActor*)0x0;
	}

	return bVar5;
}

void CActorMovingPlatform::ManageNoFrictionZones(int param_2)
{
	ed_zone_3d* pZone;
	CActor* pActor;
	ed_zone_3d* peVar1;
	CEventManager* pCVar2;
	CActor* pCVar3;
	uint uVar4;
	undefined* puVar5;
	int iVar6;
	int iVar7;
	S_STREAM_MPF_NO_FRICTION_ZONE* pNoFrictionZone;
	int iVar9;

	pCVar2 = CScene::ptable.g_EventManager_006f5080;
	iVar9 = 0;
	pNoFrictionZone = this->aNoFrictionZones;
	if (0 < this->noFrictionZoneCount) {
		do {
			pZone = pNoFrictionZone->zoneRef.Get();
			iVar7 = 0;
			while (true) {
				S_ACTOR_STREAM_REF* pActorStreamRef = pNoFrictionZone->pActorStreamRef;
				int nbActors = 0;
				if (pActorStreamRef != (S_ACTOR_STREAM_REF*)0x0) {
					nbActors = pActorStreamRef->entryCount;
				}

				if (nbActors <= iVar7) break;

				pActor = pActorStreamRef->aEntries[iVar7].Get();

				if ((((pActor != (CActor*)0x0) && ((pActor->flags & 4) != 0)) &&
					(pActor->pCollisionData != (CCollision*)0x0)) &&
					(pCVar3 = pActor->GetCollidingActor(), pCVar3 == (CActor*)0x0)) {
					uVar4 = edEventComputeZoneAgainstVertex(pCVar2->activeChunkId, pZone, &pActor->currentLocation, 0);
					if (((uVar4 & 1) == 0) || ((pActor->flags & 0x80000) == 0)) {
						if (((CActorMovingPlatform*)pActor->pTiedActor == this) && (((pActor->flags & 0x20000) != 0 && (param_2 == 0)))) {
							pActor->TieToActor((CActor*)0x0, 0, -1, (edF32MATRIX4*)0x0);
						}
					}
					else {
						pActor->FUN_00101110(this);
						if ((pActor->pTiedActor == (CActor*)0x0) && (param_2 != 0)) {
							pActor->TieToActor(this, 0, -1, (edF32MATRIX4*)0x0);
						}
					}
				}

				iVar7 = iVar7 + 1;
			}

			iVar9 = iVar9 + 1;

			pNoFrictionZone = pNoFrictionZone + 1;
		} while (iVar9 < this->noFrictionZoneCount);
	}

	return;
}

void CActorMovingPlatform::Platform_UpdatePosition(edF32VECTOR4* pPosition, int param_3, CActorsTable* pActorsTable)
{
	bool bVar1;
	bool bPush;
	bool bTilt;
	int iVar2;
	edF32MATRIX4 eStack144;
	edF32MATRIX4 eStack80;
	edF32VECTOR4 local_10;

	ACTOR_LOG(LogLevel::Verbose, "CActorMovingPlatform::Platform_UpdatePosition {} {}", this->name, pPosition->ToString());

	ManageNoFrictionZones(1);

	local_10 = gF32Vector4Zero;
	
	if (this->pTiltData == (S_TILT_DATA*)0x0) {
		bPush = false;
	}
	else {
		bPush = SV_MOV_UpdatePush(this->pProperties->field_0x2c, &this->pTiltData->pushData, &this->pProperties->pushStreamDef);
	}

	if (bPush != false) {
		param_3 = 1;
		local_10.y = -(this->pTiltData->pushData).oscValue.value;
	}

	if (this->pTiltData == (S_TILT_DATA*)0x0) {
		bTilt = false;
	}
	else {
		bTilt = SV_MOV_UpdateTilt(this->pProperties->field_0x2c, this->pTiltData, &this->pProperties->tiltStreamDef);
	}

	if (bTilt == false) {
		if ((param_3 != 0) || (this->pTiedActor != (CActor*)0x0)) {
			SV_UpdatePosition_Rel(pPosition, param_3, 1, pActorsTable, &local_10);
		}
	}
	else {
		ACTOR_LOG(LogLevel::Verbose, "CActorMovingPlatform::Platform_UpdatePosition Tilt {} {}", this->name, this->pTiltData->oscQuat.quat.ToString());

		edQuatToMatrix4Hard(&this->pTiltData->oscQuat.quat, &eStack80);
		edF32Matrix4FromEulerSoft(&eStack144, &this->pCinData->rotationEuler, "XYZ");
		eStack144.rowT = *pPosition;
		edF32Matrix4MulF32Matrix4Hard(&eStack144, &eStack80, &eStack144);

		ACTOR_LOG(LogLevel::Verbose, "CActorMovingPlatform::Platform_UpdatePosition {} {}", this->name, eStack144.ToString());

		SV_UpdateMatrix_Rel(&eStack144, 1, 1, pActorsTable, &local_10);
	}

	if (this->pZoneStream == (S_ZONE_STREAM_REF*)0x0) {
		iVar2 = 0;
	}
	else {
		iVar2 = this->pZoneStream->entryCount;
	}

	if (iVar2 == 0) {

		if (this->pActorStream == (S_BRIDGE_ACTOR_STREAM*)0x0) {
			iVar2 = 0;
		}
		else {
			iVar2 = this->pActorStream->entryCount;
		}

		if ((iVar2 == 0) && (this->noFrictionZoneCount == 0)) goto LAB_0015b1b0;
	}

	SV_ComputeDiffMatrixFromInit(&this->field_0x200);
	edF32Matrix4GetInverseOrthoHard(&this->field_0x200, &this->field_0x200);

LAB_0015b1b0:
	if ((this->pTiedActor != (CActor*)0x0) || (param_3 != 0)) {
		if (this->field_0x1ec.IsValid()) {
			this->field_0x1ec.SetPosition(&this->currentLocation);
			this->field_0x1ec.SetRotationEuler(&this->rotationEuler);
		}

		if (this->field_0x1f4.IsValid()) {
			this->field_0x1f4.SetPosition(&this->currentLocation);
			this->field_0x1f4.SetRotationEuler(&this->rotationEuler);
		}
	}

	ManageNoFrictionZones(0);
	return;
}

void CActorMovingPlatform::ForceCarriedStuff()
{
	int iVar1;
	int iVar2;
	S_STREAM_MPF_NO_FRICTION_ZONE* pSVar3;
	int iVar4;
	int iVar5;
	CActor* piVar1;

	if (this->pActorStream == (S_BRIDGE_ACTOR_STREAM*)0x0) {
		iVar2 = 0;
	}
	else {
		iVar2 = this->pActorStream->entryCount;
	}

	iVar5 = 0;
	if (0 < iVar2) {
		iVar4 = 0;
		do {
			S_BRIDGE_ACTOR_STREAM_ENTRY* pEntry = &this->pActorStream->aEntries[iVar5];
			if (pEntry->actorRef.Get() != (CActor*)0x0) {
				pEntry->actorRef.Get()->TieToActor(this, pEntry->carryMethod, 1, &this->field_0x200);
			}
			iVar5 = iVar5 + 1;
		} while (iVar5 < iVar2);
	}

	iVar2 = 0;
	if (this->pZoneStream != (S_ZONE_STREAM_REF*)0x0) {
		iVar2 = this->pZoneStream->entryCount;
	}

	iVar5 = 0;
	if (0 < iVar2) {
		iVar4 = 0;
		do {
			S_STREAM_REF<ed_zone_3d>* pEntry = &this->pZoneStream->aEntries[iVar5];
			if (pEntry->Get() != 0) {
				pEntry->Get()->pMatrix = STORE_POINTER(&this->field_0x200);
			}
			iVar5 = iVar5 + 1;
		} while (iVar5 < iVar2);
	}

	pSVar3 = this->aNoFrictionZones;
	iVar2 = 0;
	if (0 < (int)this->noFrictionZoneCount) {
		do {
			iVar2 = iVar2 + 1;
			((pSVar3->zoneRef).Get())->pMatrix = STORE_POINTER(&this->field_0x200);
			pSVar3 = pSVar3 + 1;
		} while (iVar2 < (int)this->noFrictionZoneCount);
	}

	return;
}

StateConfig CActorMovingPlatform::gStateCfg_MPF[13] =
{
	StateConfig(6, 0x104),
	StateConfig(0, 0x004),
	StateConfig(0, 0x104),
	StateConfig(0, 0x004),
	StateConfig(0, 0x004),
	StateConfig(6, 0x004),
	StateConfig(0, 0x004),
	StateConfig(6, 0x004),
	StateConfig(0, 0x004),
	StateConfig(0, 0x004),
	StateConfig(6, 0x004),
	StateConfig(0, 0x004),
	StateConfig(7, 0x021),
};

StateConfig* CActorMovingPlatform::GetStateCfg(int state)
{
	StateConfig* pAVar1;

	if (state < 5) {
		pAVar1 = CActorMovable::GetStateCfg(state);
	}
	else {
		pAVar1 = gStateCfg_MPF + state + -5;
	}
	return pAVar1;
}

void CActorMovingPlatform::GenericManage(int param_2, int param_3, int currentSegment, int prevSegment)
{
	CActor* pCVar1;
	bool bVar2;
	int iVar3;
	uint uVar4;
	CCamera* pCameraView;
	StateConfig* pAVar5;
	int* piVar6;
	int iVar7;
	uint uVar8;
	int iVar9;
	int iVar10;
	CCameraManager* pCameraManager;
	CActorHero* pHero;

	pHero = CActorHero::_gThis;

	if ((CActorHero::_gThis == (CActorHero*)0x0) || (bVar2 = SV_AmICarrying(CActorHero::_gThis), bVar2 == false)) {
		if ((this->movingPlatformFlags & 0x20) != 0) {
			this->movingPlatformFlags = this->movingPlatformFlags & 0xffffffdf;

			TriggerSwitches(6);
		}
	}
	else {
		if ((this->movingPlatformFlags & 0x20) == 0) {
			this->movingPlatformFlags = this->movingPlatformFlags | 0x20;

			if ((this->pProperties->flags_0x24 & 0x80000) != 0) {
				SetBehaviour(6, -1, -1);

				TriggerSwitches(7);
			}

			TriggerSwitches(5);
		}
	}

	iVar10 = 0;
	if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
		iVar10 = this->pCameraStream->entryCount;
	}

	iVar9 = 0;
	if (0 < iVar10) {
		do {
			this->pCameraStream->aEntries[iVar9].streamCameraEvent.Manage(this);
			iVar9 = iVar9 + 1;
		} while (iVar9 < iVar10);
	}

	pCameraManager = CCameraManager::_gThis;

	if (this->pProperties->field_0x28 != -1) {
		bVar2 = false;
		if ((param_2 != 0) && (((this->movingPlatformFlags & 0x20) != 0 || (((((this->movingPlatformFlags & 0x80) != 0 && (pHero->pTiedActor == (CActor*)0x0)) &&
					((pHero->dynamic.flags & 4) != 0)) && (uVar4 = pHero->TestState_IsGrippedOrClimbing(0xffffffff), uVar4 == 0)))))) {
			bVar2 = true;
		}

		pCameraView = pCameraManager->GetScenaricCamera(this->pProperties->field_0x28);

		pCameraView->SetOtherTarget(this);

		if (bVar2) {
			if (((this->movingPlatformFlags & 0x80) == 0) &&
				(bVar2 = pCameraManager->PushCamera(pCameraView, 0), bVar2 != false)) {
				this->movingPlatformFlags = this->movingPlatformFlags | 0x80;
			}
		}
		else {
			if ((this->movingPlatformFlags & 0x80) != 0) {
				pCameraManager->PopCamera(pCameraView);
				this->movingPlatformFlags = this->movingPlatformFlags & 0xffffff7f;
			}
		}
	}

	if ((uint)((this->movingPlatformFlags & 0x10) != 0) == param_3) {
		if (param_3 == 0) {
			IMPLEMENTATION_GUARD_AUDIO(
			piVar6 = this->field_0x1e0;
			iVar10 = 0;
			if (piVar6 != (int*)0x0) {
				iVar10 = *piVar6;
			}
			if (iVar10 != 0) {
				iVar10 = (this->pProperties->field_0x8).field_0x0;
				if (this->field_0x1e0 == (int*)0x0) {
					iVar9 = 0;
				}
				else {
					iVar9 = *this->field_0x1e0;
				}
				iVar7 = 0;
				if (0 < iVar9) {
					do {
						if (piVar6[1] <= currentSegment) {
							if (((currentSegment <= piVar6[2]) && (piVar6[1] <= prevSegment)) && (prevSegment <= piVar6[2])) {
								iVar10 = piVar6[3];
								break;
							}
						}
						iVar7 = iVar7 + 1;
						piVar6 = piVar6 + 4;
					} while (iVar7 < iVar9);
				}
				if (iVar10 != this->field_0x1e8) {
					CActorSound::FUN_0032c600(this->pActorSound, 0);
					this->field_0x1e8 = iVar10;
					if ((int*)this->field_0x1e8 != (int*)0x0) {
						CActorSound::SoundStart(this->pActorSound, this, 0, (int*)this->field_0x1e8, 1, 0, (float**)0x0);
					}
				}
			})
		}
		else {
			IMPLEMENTATION_GUARD_AUDIO(
			piVar6 = this->field_0x1dc;
			iVar10 = 0;
			if (piVar6 != (int*)0x0) {
				iVar10 = *piVar6;
			}
			if (iVar10 != 0) {
				iVar10 = (this->pProperties->field_0x0).field_0x0;
				if (this->field_0x1dc == (int*)0x0) {
					iVar9 = 0;
				}
				else {
					iVar9 = *this->field_0x1dc;
				}
				iVar7 = 0;
				if (0 < iVar9) {
					do {
						if (piVar6[1] <= currentSegment) {
							if (((currentSegment <= piVar6[2]) && (piVar6[1] <= prevSegment)) && (prevSegment <= piVar6[2])) {
								iVar10 = piVar6[3];
								break;
							}
						}
						iVar7 = iVar7 + 1;
						piVar6 = piVar6 + 4;
					} while (iVar7 < iVar9);
				}
				if (iVar10 != this->field_0x1e4) {
					CActorSound::FUN_0032c600(this->pActorSound, 0);
					this->field_0x1e4 = iVar10;
					if ((int*)this->field_0x1e4 != (int*)0x0) {
						CActorSound::SoundStart(this->pActorSound, this, 0, (int*)this->field_0x1e4, 1, 0, (float**)0x0);
					}
				}
			})
		}
	}
	else {
		if ((this->movingPlatformFlags & 1) != 0) {
			IMPLEMENTATION_GUARD_AUDIO(
			CActorSound::FUN_0032c600(this->pActorSound, 0);)
		}

		if (param_3 == 0) {
			iVar10 = this->actorState;
			if (iVar10 == -1) {
				uVar4 = 0;
			}
			else {
				pAVar5 = GetStateCfg(iVar10);
				uVar4 = pAVar5->flags_0x4 & 1;
			}

			if (uVar4 == 0) {
				CActor::PlayAnim(0);
			}

			IMPLEMENTATION_GUARD_AUDIO(
			iVar10 = (this->pProperties->field_0x8).field_0x0;

			if (this->field_0x1e0 == (int*)0x0) {
				iVar9 = 0;
			}
			else {
				iVar9 = *this->field_0x1e0;
			}

			iVar7 = 0;
			if (0 < iVar9) {
				piVar6 = this->field_0x1e0;
				do {
					if (piVar6[1] <= currentSegment) {
						if (((currentSegment <= piVar6[2]) && (piVar6[1] <= prevSegment)) && (prevSegment <= piVar6[2])) {
							iVar10 = piVar6[3];
							break;
						}
					}
					iVar7 = iVar7 + 1;
					piVar6 = piVar6 + 4;
				} while (iVar7 < iVar9);
			}

			this->field_0x1e8 = iVar10;
			if ((int*)this->field_0x1e8 != (int*)0x0) {
				CActorSound::SoundStart(this->pActorSound, this, 0, (int*)this->field_0x1e8, 1, 0, (float**)0x0);
			})

			uVar4 = this->pProperties->field_0x20;
			if (uVar4 != 0xffffffff) {
				if (!this->field_0x1f4.IsValid()) {
					CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&this->field_0x1f4, uVar4, FX_MATERIAL_SELECTOR_NONE);
				}
			}

			if (this->field_0x1ec.IsValid()) {
				this->field_0x1ec.Stop();
			}

			if (this->field_0x1f4.IsValid()) {
				this->field_0x1f4.SetPosition(&this->currentLocation);
				this->field_0x1f4.SetRotationEuler(&this->rotationEuler);
				this->field_0x1f4.Start();
			}

			TriggerSwitches(10);

			this->movingPlatformFlags = this->movingPlatformFlags & 0xffffffef;
		}
		else {
			iVar10 = this->actorState;

			if (iVar10 == -1) {
				uVar4 = 0;
			}
			else {
				pAVar5 = GetStateCfg(iVar10);
				uVar4 = pAVar5->flags_0x4 & 1;
			}

			if (uVar4 == 0) {
				PlayAnim(0x6);
			}

			IMPLEMENTATION_GUARD_AUDIO(
			iVar10 = (this->pProperties->field_0x0).field_0x0;
			if (this->field_0x1dc == (int*)0x0) {
				iVar9 = 0;
			}
			else {
				iVar9 = *this->field_0x1dc;
			}

			iVar7 = 0;
			if (0 < iVar9) {
				piVar6 = this->field_0x1dc;
				do {
					if (piVar6[1] <= currentSegment) {
						if (((currentSegment <= piVar6[2]) && (piVar6[1] <= prevSegment)) && (prevSegment <= piVar6[2])) {
							iVar10 = piVar6[3];
							break;
						}
					}
					iVar7 = iVar7 + 1;
					piVar6 = piVar6 + 4;
				} while (iVar7 < iVar9);
			}

			this->field_0x1e4 = iVar10;
			if (this->field_0x1e4 != (CSound*)0x0) {
				CActorSound::SoundStart(this->pActorSound, this, 0, this->field_0x1e4, 1, 0, (float**)0x0);
			})

			uVar4 = this->pProperties->field_0x1c;
			if (uVar4 != 0xffffffff) {
				if (!this->field_0x1ec.IsValid()) {
					CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&this->field_0x1ec, uVar4, FX_MATERIAL_SELECTOR_NONE);
				}
			}

			if (this->field_0x1f4.IsValid()) {
				this->field_0x1f4.Stop();
			}

			if (this->field_0x1ec.IsValid()) {
				this->field_0x1ec.SetPosition(&this->currentLocation);
				this->field_0x1ec.SetRotationEuler(&this->rotationEuler);
				this->field_0x1ec.Start();
			}

			TriggerSwitches(9);

			this->movingPlatformFlags = this->movingPlatformFlags | 0x10;
		}
	}

	return;
}

int CActorMovingPlatform::Platform_UpdateMatrixOnTrajectory(CPathFollowReaderAbsolute* pPathFollowerAbs, int param_3, int param_4, S_TRAJ_POS* pTrajPos, CActorsTable* pActorsTable, edF32VECTOR4* param_7)
{
	int* piVar1;
	uint uVar2;
	CActor* pCVar3;
	bool bVar4;
	bool cVar5;
	short sVar6;
	short sVar7;
	int iVar8;
	int iVar9;
	uint uVar11;
	int iVar12;
	int local_80;
	edF32MATRIX4 eStack112;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	S_PATHREADER_POS_INFO pathReaderPosInfo;

	int result = 1;

	local_20 = this->currentLocation;

	ManageNoFrictionZones(1);

	local_30 = gF32Vector4Zero;

	if (this->pTiltData == (S_TILT_DATA*)0x0) {
		cVar5 = false;
	}
	else {
		cVar5 = CActorMovable::SV_MOV_UpdatePush(this->pProperties->field_0x2c, &this->pTiltData->pushData, &this->pProperties->pushStreamDef);
	}

	local_80 = param_3;

	if (cVar5 != false) {
		local_80 = 1;
		local_30.y = -(this->pTiltData->pushData).oscValue.value;
	}

	if (param_7 != 0) {
		edF32Vector4AddHard(&local_30, &local_30, param_7);
	}

	if (this->pTiltData == (S_TILT_DATA*)0x0) {
		cVar5 = false;
	}
	else {
		cVar5 = CActorMovable::SV_MOV_UpdateTilt(this->pProperties->field_0x2c, this->pTiltData, &this->pProperties->tiltStreamDef);
	}

	if (cVar5 == false) {
		if ((local_80 == 0) && (this->pTiedActor == (CActor*)0x0)) {
			param_4 = 0;
		}
		else {
			result = SV_UpdateMatrixOnTrajectory_Rel(pTrajPos->pathPosition, pPathFollowerAbs, this->pProperties->flags_0x24 & 2, 1, pActorsTable, (edF32MATRIX4*)0x0, &local_30, &pathReaderPosInfo);
		}
	}
	else {
		edQuatToMatrix4Hard(&this->pTiltData->oscQuat.quat, &eStack112);
		result = SV_UpdateMatrixOnTrajectory_Rel(pTrajPos->pathPosition, pPathFollowerAbs, this->pProperties->flags_0x24 & 2, 1, pActorsTable, &eStack112, &local_30, &pathReaderPosInfo);
	}

	sVar6 = -1;
	sVar7 = sVar6;

	if (param_4 != 0) {
		sVar7 = pathReaderPosInfo.currentSegment;

		if (0.0f < pathReaderPosInfo.segmentFraction) {
			sVar7 = pathReaderPosInfo.prevSegment;

			if (result == 2) {
				if ((pathReaderPosInfo.segmentFraction <= 0.999f) && (sVar7 = pathReaderPosInfo.currentSegment, 0.001f <= pathReaderPosInfo.segmentFraction)) {
					sVar7 = sVar6;
				}
			}
			else {
				if ((pPathFollowerAbs->pPathFollow == (CPathFollow*)0x0) || (pPathFollowerAbs->pPathFollow->splinePointCount != 2)) {
					sVar7 = pathReaderPosInfo.currentSegment;

					if ((pTrajPos->prevSegment != pathReaderPosInfo.currentSegment) &&
						(sVar7 = sVar6, pTrajPos->currentSegment == pathReaderPosInfo.prevSegment)) {
						sVar7 = pathReaderPosInfo.prevSegment;
					}
				}
				else {
					if ((pathReaderPosInfo.segmentFraction <= 0.9f) && (sVar7 = pathReaderPosInfo.currentSegment, 0.1f <= pathReaderPosInfo.segmentFraction)) {
						sVar7 = sVar6;
					}
				}
			}
		}

		param_4 = 0;

		if ((sVar7 != -1) && (pTrajPos->lastEventSegment != sVar7)) {
			param_4 = 1;
		}
	}

	pTrajPos->currentSegment = pathReaderPosInfo.currentSegment;
	pTrajPos->prevSegment = pathReaderPosInfo.prevSegment;
	pTrajPos->lastEventSegment = sVar7;

	if (param_4 != 0) {
		iVar8 = 0;

		if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
			iVar8 = this->pCameraStream->entryCount;
		}

		iVar12 = 0;
		if (0 < iVar8) {
			do {
				S_BRIDGE_CAMERA_STREAM_ENTRY* pStreamEntry = this->pCameraStream->aEntries + iVar12;
				if ((pStreamEntry->field_0x0 == 4) && (sVar7 == pStreamEntry->field_0x4)) {
					pStreamEntry->streamTarget.Switch(this);
					pStreamEntry->streamTarget.PostSwitch(this);
					pStreamEntry->streamCameraEvent.SwitchOn(this);
		
					IMPLEMENTATION_GUARD_AUDIO(
					piVar1 = *(int**)(iVar9 + 0x48);
					if (piVar1 != (int*)0x0) {
						uVar2 = *(uint*)&this->pActorSound->field_0x4;
						uVar11 = (uint)((this->movingPlatformFlags & 1) != 0);
						if ((this->movingPlatformFlags & 2) != 0) {
							uVar11 = uVar11 + 1;
						}
						for (; uVar11 < uVar2; uVar11 = uVar11 + 1) {
							iVar9 = CActorSound::IsInstanceAlive(this->pActorSound, uVar11);
							if (iVar9 == 0) {
								CActorSound::SoundStart(this->pActorSound, this, uVar11, piVar1, 1, 0, (float**)0x0);
								break;
							}
						}
					})
				}

				iVar12 = iVar12 + 1;
			} while (iVar12 < iVar8);
		}
	}

	if (this->pZoneStream == (S_ZONE_STREAM_REF*)0x0) {
		iVar8 = 0;
	}
	else {
		iVar8 = this->pZoneStream->entryCount;
	}

	if (iVar8 == 0) {
		if (this->pActorStream == (S_BRIDGE_ACTOR_STREAM*)0x0) {
			iVar8 = 0;
		}
		else {
			iVar8 = this->pActorStream->entryCount;
		}

		if ((iVar8 == 0) && (this->noFrictionZoneCount == 0)) goto LAB_0015aa10;
	}

	SV_ComputeDiffMatrixFromInit(&this->field_0x200);
	edF32Matrix4GetInverseOrthoHard(&this->field_0x200, &this->field_0x200);
LAB_0015aa10:
	if ((this->pTiedActor != (CActor*)0x0) || (local_80 != 0)) {
		if (this->field_0x1ec.IsValid()) {
			this->field_0x1ec.SetPosition(&this->currentLocation);
			this->field_0x1ec.SetRotationEuler(&this->rotationEuler);
		}

		if (this->field_0x1f4.IsValid()) {
			this->field_0x1f4.SetPosition(&this->currentLocation);
			this->field_0x1f4.SetRotationEuler(&this->rotationEuler);
		}
	}

	ManageNoFrictionZones(0);
	ComputeRealMoving(&local_20);

	return result;
}

bool CActorMovingPlatform::StateTrajectory(float currentFillAmount, CBehaviourPlatformTrajectory* pBehaviour, bool param_4)
{
	int* piVar1;
	uint uVar2;
	bool bVar3;
	bool bVar4;
	int iVar5;
	ulong uVar6;
	int iVar7;
	uint uVar8;
	int iVar9;
	int iVar10;
	float fVar11;
	float fVar12;
	bool bBarDepleted;

	GetTimer();
	bVar4 = false;

	if ((pBehaviour->pathFollowReaderAbs).pPathFollow != (CPathFollow*)0x0) {
		bBarDepleted = false;
		bVar3 = false;
		bVar4 = true;

		if ((param_4 != false) && (currentFillAmount < 0.0f)) {
			currentFillAmount = 0.0f;
			bBarDepleted = true;
		}

		if (((this->pProperties->flags_0x24 & 0x20) != 0) && ((pBehaviour->pathFollowReaderAbs).midPoint <= fmodf(currentFillAmount, (pBehaviour->pathFollowReaderAbs).field_0x4))) {
			fVar12 = fmodf((pBehaviour->trajPos).pathPosition, (pBehaviour->pathFollowReaderAbs).field_0x4);
			fVar11 = (pBehaviour->pathFollowReaderAbs).midPoint;
			if (fVar12 < fVar11) {
				bBarDepleted = true;
				bVar3 = true;
				currentFillAmount = fVar11;
			}
		}

		(pBehaviour->trajPos).pathPosition = currentFillAmount;

		int trajResult = Platform_UpdateMatrixOnTrajectory(&pBehaviour->pathFollowReaderAbs, 1, 1, &pBehaviour->trajPos, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);

		if (bBarDepleted) {
			SetState(MOVING_PLATFORM_STATE_MOVING, -1);

			TriggerSwitches(0);

			bVar4 = false;
			if (bVar3) {
				TriggerSwitches(1);
			}
		}
		else {
			if (trajResult == 1) {
				bVar4 = false;
			}
			else {
				if (trajResult == 2) {
					uVar2 = this->pProperties->flags_0x24;
					bBarDepleted = false;

					if ((uVar2 & 0x200) == 0) {
						if ((uVar2 & 0x30) != 0) {
							(pBehaviour->trajPos).pathPosition = pBehaviour->segmentStartTime;
						}

						SetState(MOVING_PLATFORM_STATE_MOVING, -1);

						bBarDepleted = true;
					}
					else {
						(pBehaviour->trajPos).pathPosition = (pBehaviour->pathFollowReaderAbs).field_0x4;
						SetState(MOVING_PLATFORM_STATE_TRANSITION_A, -1);
					}

					TriggerSwitches(2);

					bVar4 = false;

					if (bBarDepleted) {
						TriggerSwitches(0);
					}
				}
			}
		}
	}

	return bVar4;
}

void CActorMovingPlatform::BehaviourTrajectory_Switch(CBehaviourPlatformTrajectory* pBehaviour, int msg)
{
	if (msg == 0xe) {
		if ((GetStateFlags(this->actorState) & 0x100) == 0) {
			BehaviourTrajectory_Switch(pBehaviour, 0xf);
		}
		else {
			BehaviourTrajectory_Switch(pBehaviour, 0x10);
		}
	}
	else {
		if (msg == 0x10) {
			if ((this->pProperties->flags_0x24 & 0x200) == 0) {
				if (((GetStateFlags(this->actorState) & 0x100) != 0) && ((this->pProperties->flags_0x24 & 8) != 0)) {
					SetState(MOVING_PLATFORM_STATE_MOVING, -1);

					TriggerSwitches(0);
				}
			}
			else {
				SetState(MOVING_PLATFORM_STATE_AT_ENDPOINT, -1);
			}
		}
		else {
			if (msg == 0xf) {
				if ((GetStateFlags(this->actorState) & 0x100) == 0) {
					SetState(MOVING_PLATFORM_STATE_STAND, -1);
					pBehaviour->targetScaledTime = (pBehaviour->segmentStartTime + GetTimer()->scaledTotalTime) - (pBehaviour->trajPos).pathPosition;
				}
			}
		}
	}

	return;
}

void CActorMovingPlatform::BehaviourTrajectory_Manage(CBehaviourPlatformTrajectory* pBehaviour)
{
	float currentFillAmount;
	bool local_4;

	local_4 = false;
	currentFillAmount = BehaviourTrajectory_ComputeTime(pBehaviour);

	switch (this->actorState) {
	case MOVING_PLATFORM_STATE_STAND:
		local_4 = StateTrajectory(currentFillAmount, pBehaviour, false);
		break;
	case MOVING_PLATFORM_STATE_MOVING:
		Platform_UpdateMatrixOnTrajectory(&pBehaviour->pathFollowReaderAbs, 0, 0, &pBehaviour->trajPos, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
		break;
	case MOVING_PLATFORM_STATE_TRANSITION_A:
		Platform_UpdateMatrixOnTrajectory(&pBehaviour->pathFollowReaderAbs, 0, 0, &pBehaviour->trajPos, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
		break;
	case MOVING_PLATFORM_STATE_AT_ENDPOINT:
		Platform_UpdateMatrixOnTrajectory(&pBehaviour->pathFollowReaderAbs, 0, 0, &pBehaviour->trajPos, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);

		if (pBehaviour->loopDelay <= this->timeInAir) {
			SetState(MOVING_PLATFORM_STATE_WAITING, -1);
			TriggerSwitches(3);

			pBehaviour->targetScaledTime = (pBehaviour->trajPos).pathPosition + pBehaviour->segmentStartTime + GetTimer()->scaledTotalTime;
		}
		break;
	case MOVING_PLATFORM_STATE_WAITING:
		local_4 = StateTrajectory(currentFillAmount, pBehaviour, true);
		break;
	case MOVING_PLATFORM_STATE_TRANSITION_B:
		local_4 = StateTrajectory(currentFillAmount, pBehaviour, false);

		if (currentFillAmount == pBehaviour->pathLength) {
			SetState(MOVING_PLATFORM_STATE_MOVING, -1);
			TriggerSwitches(0);
			local_4 = false;
		}
	}

	GenericManage(1, local_4, (int)(pBehaviour->trajPos).currentSegment, (int)(pBehaviour->trajPos).prevSegment);

	return;
}

float CActorMovingPlatform::BehaviourTrajectory_ComputeTime(CBehaviourPlatformTrajectory* pBehaviour)
{
	float result;
	float newValue;
	int state;

	state = this->actorState;
	result = pBehaviour->trajPos.pathPosition;

	if (state == MOVING_PLATFORM_STATE_TRANSITION_B) {
		const bool bFinished = pBehaviour->pathLength < pBehaviour->trajPos.pathPosition;

		if ((this->pProperties->flags_0x24 & 1) == 0) {
			if (bFinished) {
				newValue = pBehaviour->trajPos.pathPosition - GetTimer()->cutsceneDeltaTime;
			}
			else {
				newValue = pBehaviour->trajPos.pathPosition + GetTimer()->cutsceneDeltaTime;
			}
		}
		else {
			if (bFinished) {
				newValue = pBehaviour->targetScaledTime - GetTimer()->scaledTotalTime;
			}
			else {
				newValue = GetTimer()->scaledTotalTime - pBehaviour->targetScaledTime;
			}
		}

		// Clamp the new value to the path length, depending on the direction of movement.
		if (bFinished) {
			result = pBehaviour->pathLength;
			if (pBehaviour->pathLength < newValue) {
				result = newValue;
			}
		}
		else {
			result = pBehaviour->pathLength;
			if (newValue < pBehaviour->pathLength) {
				result = newValue;
			}
		}
	}
	else {
		if (state == MOVING_PLATFORM_STATE_WAITING) {
			if ((this->pProperties->flags_0x24 & 1) == 0) {
				result = pBehaviour->trajPos.pathPosition - GetTimer()->cutsceneDeltaTime;
			}
			else {
				result = pBehaviour->targetScaledTime - GetTimer()->scaledTotalTime;
			}
		}
		else {
			if (state == MOVING_PLATFORM_STATE_STAND) {
				if ((this->pProperties->flags_0x24 & 1) == 0) {
					result = pBehaviour->trajPos.pathPosition + GetTimer()->cutsceneDeltaTime;
				}
				else {
					result = (pBehaviour->segmentStartTime + GetTimer()->scaledTotalTime) - pBehaviour->targetScaledTime;
				}
			}
		}
	}

	return result;
}

void CActorMovingPlatform::SaveContext(void* pData, uint mode, uint maxSize)
{
	CBehaviour* pCVar1;

	S_SAVE_CLASS_MOVING_PLATFORM* pSaveData = reinterpret_cast<S_SAVE_CLASS_MOVING_PLATFORM*>(pData);

	pSaveData->flags = 0;
	pSaveData->behaviourId = this->curBehaviourId;

	if ((this->movingPlatformFlags & 0x40) != 0) {
		pSaveData->flags = pSaveData->flags | 1;
	}

	if (this->curBehaviourId == MOVING_PLATFORM_BEHAVIOUR_DESTROYED) {
		pSaveData->flags = pSaveData->flags | 2;
	}

	if ((this->flags & 0x400000) != 0) {
		pSaveData->flags = pSaveData->flags | 0x20;
	}

	switch (this->curBehaviourId) {
	case MOVING_PLATFORM_BEHAVIOUR_TRAJECTORY:
	case MOVING_PLATFORM_BEHAVIOUR_SLAB:
	case MOVING_PLATFORM_BEHAVIOUR_WEIGHING_MACHINE_SLAVE:
	case MOVING_PLATFORM_BEHAVIOUR_WEIGHING_MACHINE_MASTER:
	case MOVING_PLATFORM_BEHAVIOUR_DESTROYED:
	case MOVING_PLATFORM_BEHAVIOUR_STAND:
	case MOVING_PLATFORM_BEHAVIOUR_SELECTOR_MASTER:
	case MOVING_PLATFORM_BEHAVIOUR_SELECTOR_NEW:
	case MOVING_PLATFORM_BEHAVIOUR_TELEPORT_RANDOM:
		pCVar1 = GetBehaviour(this->curBehaviourId);
		if (pCVar1 != (CBehaviour*)0x0) {
			CBehaviourPlatform* pBehaviourMovingPlatform = static_cast<CBehaviourPlatform*>(pCVar1);
			pBehaviourMovingPlatform->SaveContext(pSaveData);
		}
	}
	return;
}

void CActorMovingPlatform::LoadContext(void* pData, uint mode, uint maxSize)
{
	int bhvrId;
	CBehaviourPlatform* pBehaviourMovingPlatform;

	S_SAVE_CLASS_MOVING_PLATFORM* pSaveData = reinterpret_cast<S_SAVE_CLASS_MOVING_PLATFORM*>(pData);

	if (mode == 0x20004) {
		if ((pSaveData->flags & 1) == 0) {
			if ((pSaveData->flags & 2) != 0) {
				SetBehaviour(MOVING_PLATFORM_BEHAVIOUR_DESTROYED, -1, -1);
			}
		}
		else {
			Die(0);
		}

		if ((pSaveData->flags & 0x20) != 0) {
			this->flags = this->flags | 0x400000;
		}

		bhvrId = this->curBehaviourId;
		if (pSaveData->behaviourId == bhvrId) {
			switch (bhvrId) {
			case MOVING_PLATFORM_BEHAVIOUR_TRAJECTORY:
			case MOVING_PLATFORM_BEHAVIOUR_SLAB:
			case MOVING_PLATFORM_BEHAVIOUR_WEIGHING_MACHINE_SLAVE:
			case MOVING_PLATFORM_BEHAVIOUR_WEIGHING_MACHINE_MASTER:
			case MOVING_PLATFORM_BEHAVIOUR_DESTROYED:
			case MOVING_PLATFORM_BEHAVIOUR_STAND:
			case MOVING_PLATFORM_BEHAVIOUR_SELECTOR_MASTER:
			case MOVING_PLATFORM_BEHAVIOUR_SELECTOR_NEW:
			case MOVING_PLATFORM_BEHAVIOUR_TELEPORT_RANDOM:
				pBehaviourMovingPlatform = static_cast<CBehaviourPlatform*>(GetBehaviour(bhvrId));
				if (pBehaviourMovingPlatform != (CBehaviour*)0x0) {
					pBehaviourMovingPlatform->LoadContext(pSaveData);
				}
			}
		}
	}
	return;
}

void CActorMovingPlatform::BehaviourSlab_Manage(CBehaviourPlatformSlab* pBehaviour)
{
	bool bVar1;
	Timer* pTVar2;
	S_TARGET_ON_OFF_STREAM_REF* pSVar3;
	int iVar4;
	int iVar5;
	int iVar6;
	float fVar7;

	iVar6 = this->actorState;
	iVar5 = 0;

	ACTOR_LOG(LogLevel::Verbose, "CActorMovingPlatform::BehaviourSlab_Manage: state: {}", iVar6);

	if (iVar6 == MOVING_PLATFORM_STATE_IDLE_TRIGGERED) {
		bVar1 = Slab_MoveAndDetectCarriedObject(pBehaviour, 0);

		if (bVar1 == false) {
			if (pBehaviour->field_0x1c <= pBehaviour->field_0x28) {
				SetState(MOVING_PLATFORM_STATE_SLAB_OFF_TO_ON, -1);
			}
		}
		else {
			SetState(MOVING_PLATFORM_STATE_AT_WAYPOINT, -1);
		}
	}
	else {
		if (iVar6 == MOVING_PLATFORM_STATE_SLAB_OFF_TO_ON) {
			pTVar2 = GetTimer();

			fVar7 = pBehaviour->field_0x24 - pBehaviour->field_0x14 * pTVar2->cutsceneDeltaTime;
			pBehaviour->field_0x24 = fVar7;

			if (fVar7 <= 0.0f) {
				pBehaviour->field_0x24 = 0.0f;
			}

			bVar1 = Slab_MoveAndDetectCarriedObject(pBehaviour, 1);
			if (bVar1 == false) {
				if (fVar7 <= 0.0f) {
					if ((this->pProperties->flags_0x24 & 0x1000000) == 0) {
						pBehaviour->switchOnOff.pTargetStreamRef->SwitchOff(this);
					}

					SetState(MOVING_PLATFORM_STATE_TRAJ_TO_B, -1);
				}
			}
			else {
				SetState(MOVING_PLATFORM_STATE_AT_WAYPOINT, -1);
			}

			iVar5 = 1;
		}
		else {
			if (iVar6 == MOVING_PLATFORM_STATE_TRAJ_TO_A) {
				bVar1 = Slab_MoveAndDetectCarriedObject(pBehaviour, 0);

				if (bVar1 == false) {
					if ((this->pProperties->flags_0x24 & 0x1000000) != 0) {
						pBehaviour->switchOnOff.pTargetStreamRef->SwitchOff(this);
					}

					SetState(MOVING_PLATFORM_STATE_IDLE_TRIGGERED, -1);
				}
			}
			else {
				if (iVar6 == MOVING_PLATFORM_STATE_AT_WAYPOINT) {
					StateSwitchSlabOff2On(pBehaviour);
					iVar5 = 1;
				}
				else {
					if ((iVar6 == MOVING_PLATFORM_STATE_TRAJ_TO_B) && (bVar1 = Slab_MoveAndDetectCarriedObject(pBehaviour, 0), bVar1 != false)) {
						SetState(MOVING_PLATFORM_STATE_AT_WAYPOINT, -1);
					}
				}
			}
		}
	}

	GenericManage(1, iVar5, -1, -1);

	return;
}

void CActorMovingPlatform::StateSwitchSlabOff2On(CBehaviourPlatformSlab* pBehaviour)
{
	//CSound* pSound;
	bool bVar1;
	bool bVar2;
	Timer* pTVar3;
	S_TARGET_ON_OFF_STREAM_REF* pSVar4;
	int iVar5;
	int iVar6;
	float fVar7;

	pTVar3 = GetTimer();
	pBehaviour->field_0x24 = pBehaviour->field_0x24 + pBehaviour->field_0x14 * pTVar3->cutsceneDeltaTime;
	fVar7 = pBehaviour->field_0x18;
	bVar1 = fVar7 <= pBehaviour->field_0x24;
	if (bVar1) {
		pBehaviour->field_0x24 = fVar7;
	}

	bVar2 = Slab_MoveAndDetectCarriedObject(pBehaviour, 1);
	if (bVar2 == false) {
		SetState(MOVING_PLATFORM_STATE_IDLE_TRIGGERED, -1);
	}
	else {
		if (bVar1) {
			IMPLEMENTATION_GUARD_AUDIO(
				pSound = (CSound*)(pBehaviour->field_0x20).field_0x0;
			if (pSound != (CSound*)0x0) {
				CActorSound::SoundStart
				(this->pActorSound, this, (uint)((this->movingPlatformFlags & 1) != 0), pSound, 1, 0,
					(float**)0x0);
			})

			pBehaviour->switchOnOff.SwitchOn(this);

			SetState(MOVING_PLATFORM_STATE_TRAJ_TO_A, -1);

			if ((this->pProperties->flags_0x24 & 0x40000) != 0) {
				DoMessage(this, (ACTOR_MESSAGE)0x36, (MSG_PARAM)0);
			}
		}
	}
	return;
}

int CActorMovingPlatform::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActor* pCVar1;
	S_BRIDGE_ACTOR_STREAM* pSVar2;
	CSound* pCVar3;
	int iVar4;
	uint uVar5;
	int hitVariant;
	int iVar7;
	int iVar8;
	uint uVar9;
	undefined4 uVar10;

	if (msg == 3) {
		SetBehaviour(MOVING_PLATFORM_BEHAVIOUR_DESTROYED, -1, -1);
		TriggerSwitches(7);
		uVar5 = 1;
	}
	else {
		if (msg == MESSAGE_KICKED) {
			_msg_hit_param* pHitMessageParams = reinterpret_cast<_msg_hit_param*>(pMsgParam);
			switch (pHitMessageParams->projectileType) {
			case HIT_TYPE_BOOMY:
				hitVariant = pHitMessageParams->hitVariant;
				if (hitVariant == HIT_VARIANT_BOOMY_CONTROL) {
					uVar10 = 0;

					if ((this->pProperties->flags_0x24 & 0x1000) != 0) {
						SetBehaviour(MOVING_PLATFORM_BEHAVIOUR_DESTROYED, -1, -1);
						TriggerSwitches(7);
						uVar10 = 1;
					}

					TriggerSwitches(0xd);
					pHitMessageParams->field_0x74 = uVar10;
				}
				else {
					if ((hitVariant == HIT_VARIANT_BOOMY_SNIPE) || (hitVariant == HIT_VARIANT_BOOMY_MELEE)) {
						uVar10 = 0;
						if ((this->pProperties->flags_0x24 & 0x1000) != 0) {
							SetBehaviour(MOVING_PLATFORM_BEHAVIOUR_DESTROYED, -1, -1);
							TriggerSwitches(7);
							uVar10 = 1;
						}

						TriggerSwitches(0xc);
						pHitMessageParams->field_0x74 = uVar10;
					}
					else {
						if (hitVariant == HIT_VARIANT_BOOMY_DEFAULT) {
							uVar10 = 0;
							if ((this->pProperties->flags_0x24 & 0x1000) != 0) {
								SetBehaviour(MOVING_PLATFORM_BEHAVIOUR_DESTROYED, -1, -1);
								TriggerSwitches(7);
								uVar10 = 1;
							}

							TriggerSwitches(0xb);
							pHitMessageParams->field_0x74 = uVar10;
						}
					}
				}
				break;
			case 5:
				uVar10 = 0;
				if ((this->pProperties->flags_0x24 & 0x400) != 0) {
					SetBehaviour(MOVING_PLATFORM_BEHAVIOUR_DESTROYED, -1, -1);
					TriggerSwitches(7);
					uVar10 = 1;
				}

				TriggerSwitches(0xe);
				pHitMessageParams->field_0x74 = uVar10;
				break;
			default:
				pHitMessageParams->field_0x74 = uVar10;
				break;
			case 8:
				uVar10 = 0;
				if ((this->pProperties->flags_0x24 & 0x20000) != 0) {
					SetBehaviour(MOVING_PLATFORM_BEHAVIOUR_DESTROYED, -1, -1);
					TriggerSwitches(7);
					uVar10 = 1;
				}

				TriggerSwitches(0x13);
				pHitMessageParams->field_0x74 = uVar10;
				break;
			case 9:
				if (pSender->typeID == WEAPON) {
					uVar10 = 0;
					if ((this->pProperties->flags_0x24 & MOVING_PLATFORM_FLAG_CAN_BE_DESTROYED) != 0) {
						SetBehaviour(MOVING_PLATFORM_BEHAVIOUR_DESTROYED, -1, -1);
						TriggerSwitches(7);
						uVar10 = 1;
					}

					TriggerSwitches(0x12);
					pHitMessageParams->field_0x74 = uVar10;
				}
				break;
			case 10:
				uVar10 = 0;
				if ((this->pProperties->flags_0x24 & 0x800) != 0) {
					SetBehaviour(MOVING_PLATFORM_BEHAVIOUR_DESTROYED, -1, -1);
					TriggerSwitches(7);
					uVar10 = 1;
				}

				TriggerSwitches(0xf);
				pHitMessageParams->field_0x74 = uVar10;
				break;
			case 0xb:
				uVar10 = 0;
				if ((this->pProperties->flags_0x24 & 0x8000) != 0) {
					SetBehaviour(MOVING_PLATFORM_BEHAVIOUR_DESTROYED, -1, -1);
					TriggerSwitches(7);
					uVar10 = 1;
				}

				TriggerSwitches(0x11);
				pHitMessageParams->field_0x74 = uVar10;
			}
		}
		else {
			if (msg == 1) {
				return (uint)((this->movingPlatformFlags & 8) != 0);
			}

			if (msg == 0x3d) {
				iVar7 = 0;

				while (true) {
					pSVar2 = this->pActorStream;
					iVar8 = 0;

					if (pSVar2 != (S_BRIDGE_ACTOR_STREAM*)0x0) {
						iVar8 = pSVar2->entryCount;
					}

					if (iVar8 <= iVar7) break;

					S_BRIDGE_ACTOR_STREAM_ENTRY* pEntry = this->pActorStream->aEntries + iVar7;
					pCVar1 = pEntry->actorRef.Get();
					if (pSender == pCVar1) {
						pCVar1->TieToActor(this, pEntry->carryMethod, 1, &this->field_0x200);
					}

					iVar7 = iVar7 + 1;
				}
			}
		}

		uVar5 = CActorMovable::InterpretMessage(pSender, msg, pMsgParam);
	}

	return uVar5;
}

int CActorMovingPlatform::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* pEventData)
{
	int iVar1;

	if ((pEventData[0] == 1) && (param_3 == 2)) {
		if (pEventData[1] == 0) {
			IMPLEMENTATION_GUARD(
			Die(this, 1);)
		}
		iVar1 = 1;
	}
	else {
		iVar1 = CActor::InterpretEvent(pEventMessage, (uint)param_3, param_4, pEventData);
	}

	return iVar1;
}

bool CActorMovingPlatform::IsLockable()
{
	bool bIsLockable;

	bIsLockable = (this->movingPlatformFlags & 4) != 0;

	if (bIsLockable) {
		bIsLockable = CActor::IsLockable();
		bIsLockable = bIsLockable != false;
	}

	return bIsLockable;
}

void CActorMovingPlatform::CinematicMode_Manage()
{
	GenericManage(1, 0, -1, -1);
	return;
}

void CActorMovingPlatform::CinematicMode_UpdateMatrix(edF32MATRIX4* pPosition)
{
	Platform_UpdateMatrix(pPosition, 1, (CActorsTable*)0x0);
	return;
}

void CActorMovingPlatform::ChangeManageState(int state)
{
	uint uVar1;
	CActor* pCVar2;
	CinNamedObject30* pCVar3;
	bool bVar4;
	CBehaviourPlatform* pCVar5;

	CActorMovable::ChangeManageState(state);

	if (state == 0) {
		if (this->field_0x1ec.IsValid()) {
			this->field_0x1ec.Kill();
			this->field_0x1ec.Reset();
		}

		this->field_0x1ec.Reset();

		if (this->field_0x1f4.IsValid()) {
			this->field_0x1f4.Kill();
			this->field_0x1f4.Reset();
		}

		this->field_0x1f4.Reset();
	}
	else {
		if ((this->movingPlatformFlags & 0x10) != 0) {
			uVar1 = this->pProperties->field_0x1c;
			if (uVar1 != 0xffffffff) {
				if (!this->field_0x1ec.IsValid()) {
					CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&this->field_0x1ec, uVar1, FX_MATERIAL_SELECTOR_NONE);
				}
			}

			if (this->field_0x1ec.IsValid()) {
				this->field_0x1ec.SetPosition(&this->currentLocation);
				this->field_0x1ec.SetRotationEuler(&this->rotationEuler);
				this->field_0x1ec.Start();
			}
		}
	}

	switch (this->curBehaviourId) {
	case MOVING_PLATFORM_BEHAVIOUR_TRAJECTORY:
	case MOVING_PLATFORM_BEHAVIOUR_SLAB:
	case MOVING_PLATFORM_BEHAVIOUR_WEIGHING_MACHINE_SLAVE:
	case MOVING_PLATFORM_BEHAVIOUR_WEIGHING_MACHINE_MASTER:
	case MOVING_PLATFORM_BEHAVIOUR_DESTROYED:
	case MOVING_PLATFORM_BEHAVIOUR_STAND:
	case MOVING_PLATFORM_BEHAVIOUR_SELECTOR_MASTER:
	case MOVING_PLATFORM_BEHAVIOUR_SELECTOR_NEW:
	case MOVING_PLATFORM_BEHAVIOUR_TELEPORT_RANDOM:
		pCVar5 = reinterpret_cast<CBehaviourPlatform*>(GetBehaviour(this->curBehaviourId));
		if (pCVar5 != (CBehaviour*)0x0) {
			pCVar5->ChangeManageState(state);
		}
	}

	return;
}

void CActorMovingPlatform::Reset()
{
	S_BRIDGE_CAMERA_STREAM_ENTRY* pEntry;
	int iVar1;
	CCollision* pCol;

	this->movingPlatformFlags = this->movingPlatformFlags & 0xffffff0f;
	if (this->pTiltData != (S_TILT_DATA*)0x0) {
		this->pTiltData->Reset();
		this->pTiltData->pushData.Reset();
	}

	pEntry = this->pCameraStream->aEntries;
	for (iVar1 = this->pCameraStream->entryCount; iVar1 != 0; iVar1 = iVar1 + -1) {
		pEntry->streamTarget.Reset();
		pEntry->streamCameraEvent.Reset(this);
		pEntry = pEntry + 1;
	}

	ForceCarriedStuff();
	CActorMovable::Reset();

	pCol = this->pCollisionData;
	if (pCol != (CCollision*)0x0) {
		pCol->flags_0x0 = pCol->flags_0x0 & 0xffdfffff;
	}

	IMPLEMENTATION_GUARD_AUDIO(
	this->field_0x1e4 = 0;
	this->field_0x1e8 = (CSound*)0x0;)

	return;
}

void CActorMovingPlatform::CheckpointReset()
{
	CActorMovable::CheckpointReset();

	ForceCarriedStuff();

	return;
}

void CActorMovingPlatform::FillThisFrameExpectedDifferentialMatrix(edF32MATRIX4* pMatrix)
{
	CActor* pCVar1;
	CBehaviourPlatformTrajectory* pBehaviour;
	float fVar2;
	edF32MATRIX4 eStack272;
	edF32MATRIX4 local_d0;
	edF32MATRIX4 local_90;
	edF32MATRIX4 local_50;
	S_PATHREADER_POS_INFO SStack16;

	if (((this->curBehaviourId == 2) && (this->pCollisionData != (CCollision*)0x0)) && ((this->flags & 0x400000) == 0)) {
		pBehaviour = (CBehaviourPlatformTrajectory*)GetBehaviour(this->curBehaviourId);

		fVar2 = BehaviourTrajectory_ComputeTime(pBehaviour);
		if ((fVar2 == (pBehaviour->trajPos).pathPosition) || ((pBehaviour->pathFollowReaderAbs).pPathFollow == (CPathFollow*)0x0)) {
			CActor::FillThisFrameExpectedDifferentialMatrix(pMatrix);
		}
		else {
			if ((this->pProperties->flags_0x24 & 2) == 0) {
				edF32Matrix4FromEulerSoft(&local_50, &(this->pCinData)->rotationEuler, "XYZ");
				local_90 = local_50;
				pBehaviour->pathFollowReaderAbs.ComputePosition(fVar2, &local_50.rowT, (edF32VECTOR4*)0x0, &SStack16);
				pBehaviour->pathFollowReaderAbs.ComputePosition((pBehaviour->trajPos).pathPosition, &local_90.rowT, (edF32VECTOR4*)0x0, &SStack16);
			}
			else {
				pBehaviour->pathFollowReaderAbs.ComputeMatrix(fVar2, &local_50, (edF32VECTOR4*)0x0, &SStack16);
				pBehaviour->pathFollowReaderAbs.ComputeMatrix((pBehaviour->trajPos).pathPosition, &local_90, (edF32VECTOR4*)0x0, &SStack16);
			}

			if (local_90.cc + local_90.aa + local_90.bb == 3.0) {
				local_d0.rowX = gF32Matrix4Unit.rowX;
				local_d0.rowY = gF32Matrix4Unit.rowY;
				local_d0.rowZ = gF32Matrix4Unit.rowZ;

				local_d0.da = 0.0f - local_90.da;
				local_d0.db = 0.0f - local_90.db;
				local_d0.dc = 0.0f - local_90.dc;
				local_d0.dd = local_90.dd;
			}
			else {
				edF32Matrix4GetInverseOrthoHard(&local_d0, &local_90);
			}

			edF32Matrix4MulF32Matrix4Hard(pMatrix, &local_d0, &local_50);

			pCVar1 = this->pTiedActor;
			if (pCVar1 != (CActor*)0x0) {
				pCVar1->FillThisFrameExpectedDifferentialMatrix(&eStack272);
				edF32Matrix4MulF32Matrix4Hard(pMatrix, &eStack272, pMatrix);
			}
		}
	}
	else {
		CActor::FillThisFrameExpectedDifferentialMatrix(pMatrix);
	}
	return;
}

void CActorMovingPlatform::BehaviourSelectorMaster_Manage(CBehaviourSelectorMaster* pBehaviour)
{
	int iVar1;
	Timer* pTVar2;
	S_ACTOR_STREAM_REF* pSVar3;
	S_STREAM_NTF_TARGET_SWITCH_EX_LIST* pSVar4;
	int iVar4;
	int iVar5;
	uint uVar6;
	uint uVar7;
	int iVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	edF32VECTOR4 eStack112;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 eStack64;
	_msg_params_0x2e local_28;
	undefined4 local_1c;
	undefined4 local_18;
	undefined4 local_14;
	undefined4 local_10;
	uint local_c;
	uint local_8;
	uint* local_4;

	iVar8 = -2;
	uVar7 = 0;
	local_28.field_0x0 = 0;
	local_28.field_0x4 = pBehaviour->actorEntryIndex;
	pSVar4 = pBehaviour->switchExList.pTargetStreamRef;

	iVar5 = 0;
	if (0 < pSVar4->entryCount) {
		do {
			iVar1 = DoMessage(LOAD_POINTER_CAST(CActor*, pSVar4->aEntries[iVar5].pRef), (ACTOR_MESSAGE)0x2e, &local_28);
			if (iVar1 != 0) {
				local_28.field_0x4 = local_28.field_0x4 + -1;
				uVar7 = uVar7 | local_28.field_0x0;
			}
			iVar5 = iVar5 + 1;
		} while (iVar5 < pSVar4->entryCount);
	}

	local_8 = (uint)((uVar7 & 1) == 0);
	iVar5 = DoMessage(this, (ACTOR_MESSAGE)0x2b, (MSG_PARAM)local_8);

	if (iVar5 != 0) {
		iVar8 = -1;
	}

	iVar5 = 0;
	uVar6 = 2;
	iVar4 = 0;
	while (true) {
		pSVar3 = pBehaviour->pActorStreamRef;
		iVar1 = 0;
		if (pSVar3 != (S_ACTOR_STREAM_REF*)0x0) {
			iVar1 = pSVar3->entryCount;
		}

		if (iVar1 <= iVar5) break;

		local_c = (uint)((uVar7 & uVar6) == 0);
		iVar1 = DoMessage(pSVar3->aEntries[iVar5].Get(), (ACTOR_MESSAGE)0x2b, (MSG_PARAM)local_c);

		if (iVar1 != 0) {
			iVar8 = iVar5;
		}

		iVar4 = iVar4 + 4;
		iVar5 = iVar5 + 1;
		uVar6 = uVar6 << 1;
	}

	if (uVar7 == 0xffffffff) {
		iVar8 = -2;
	}
	else {
		if (iVar8 == -2) {
			iVar8 = local_28.field_0x4;
		}

		if (iVar8 == -2) {
			iVar8 = -1;
		}
	}

	if (-3 < iVar8) {
		iVar5 = 0;
		if (pSVar3 != (S_ACTOR_STREAM_REF*)0x0) {
			iVar5 = pSVar3->entryCount;
		}
		if (iVar8 < iVar5) goto LAB_00153528;
	}
	iVar8 = -1;
LAB_00153528:
	iVar5 = pBehaviour->actorEntryIndex;
	if (iVar8 != iVar5) {
		if (iVar5 != -2) {
			if (iVar5 == -1) {
				local_10 = 0;
				DoMessage(this, (ACTOR_MESSAGE)0x10, 0);
			}
			else {
				local_14 = 0;
				DoMessage(pSVar3->aEntries[iVar5].Get(), (ACTOR_MESSAGE)0x10, 0);
			}
		}

		pBehaviour->actorEntryIndex = iVar8;
		iVar8 = pBehaviour->actorEntryIndex;
		if (iVar8 != -2) {
			if (iVar8 == -1) {
				local_18 = 0;
				DoMessage(this, (ACTOR_MESSAGE)0xf, 0);
			}
			else {
				local_1c = 0;
				DoMessage(pBehaviour->pActorStreamRef->aEntries[iVar8].Get(), (ACTOR_MESSAGE)0xf, 0);
			}

			iVar8 = pBehaviour->actorEntryIndex;

			pBehaviour->switchExList.Switch(this, iVar8 + 1);
		}
	}

	iVar8 = this->actorState;
	if (iVar8 == MOVING_PLATFORM_STATE_TRAJ_TO_A) {
		pTVar2 = GetTimer();
		fVar11 = pBehaviour->field_0x24;
		fVar9 = pBehaviour->field_0x10;
		fVar10 = fVar11 + pBehaviour->field_0xc * pTVar2->cutsceneDeltaTime;
		if (fVar10 <= fVar9) {
			fVar9 = fVar10;
		}

		if (fVar9 == fVar11) {
			edF32Vector4ScaleHard(-fVar11, &eStack112, &g_xVector);
			edF32Vector4AddHard(&eStack112, &eStack112, &this->baseLocation);
			Platform_UpdatePosition(&eStack112, 0, (CActorsTable*)0x0);
		}
		else {
			pBehaviour->field_0x24 = fVar9;
			edF32Vector4ScaleHard(-fVar9, &eStack96, &g_xVector);
			edF32Vector4AddHard(&eStack96, &eStack96, &this->baseLocation);
			Platform_UpdatePosition(&eStack96, 1, (CActorsTable*)0x0);
		}
	}
	else {
		if (iVar8 == MOVING_PLATFORM_STATE_TRAJ_TO_B) {
			pTVar2 = GetTimer();
			fVar10 = pBehaviour->field_0x24;
			fVar9 = fVar10 - pBehaviour->field_0xc * pTVar2->cutsceneDeltaTime;
			if (fVar9 < 0.0f) {
				fVar9 = 0.0f;
			}

			if (fVar9 == fVar10) {
				edF32Vector4ScaleHard(-fVar10, &eStack80, &g_xVector);
				edF32Vector4AddHard(&eStack80, &eStack80, &this->baseLocation);
				Platform_UpdatePosition(&eStack80, 0, (CActorsTable*)0x0);
			}
			else {
				pBehaviour->field_0x24 = fVar9;
				edF32Vector4ScaleHard(-fVar9, &eStack64, &g_xVector);
				edF32Vector4AddHard(&eStack64, &eStack64, &this->baseLocation);
				Platform_UpdatePosition(&eStack64, 1, (CActorsTable*)0x0);
			}
		}
	}

	GenericManage(1, 0, -1, -1);

	return;
}

void CActorMovingPlatform::Platform_UpdateMatrix(edF32MATRIX4* pMatrix, int param_3, CActorsTable* pActorTable)
{
	float fVar1;
	CActor* pCVar2;
	bool bVar3;
	bool cVar4;
	int iVar5;
	edF32MATRIX4* peVar6;
	edF32MATRIX4 local_50;
	edF32VECTOR4 local_10;

	ManageNoFrictionZones(1);

	local_10 = gF32Vector4Zero;

	if (this->pTiltData == (S_TILT_DATA*)0x0) {
		cVar4 = false;
	}
	else {
		cVar4 = SV_MOV_UpdatePush(this->pProperties->field_0x2c, &this->pTiltData->pushData, &this->pProperties->pushStreamDef);
	}

	if (cVar4 != false) {
		param_3 = 1;
		local_10.y = -(this->pTiltData->pushData).oscValue.value;
	}

	if (this->pTiltData == (S_TILT_DATA*)0x0) {
		cVar4 = false;
	}
	else {
		cVar4 = SV_MOV_UpdateTilt(this->pProperties->field_0x2c, this->pTiltData, &this->pProperties->tiltStreamDef);
	}

	if (cVar4 == false) {
		if ((param_3 != 0) || (this->pTiedActor != (CActor*)0x0)) {
			local_50 = *pMatrix;
			SV_UpdateMatrix_Rel(&local_50, 1, 1, pActorTable, &local_10);
		}
	}
	else {
		edQuatToMatrix4Hard(&this->pTiltData->oscQuat.quat, &local_50);
		edF32Matrix4MulF32Matrix4Hard(&local_50, &local_50, pMatrix);
		SV_UpdateMatrix_Rel(&local_50, 1, 1, pActorTable, &local_10);
	}

	if (this->pZoneStream == (S_ZONE_STREAM_REF*)0x0) {
		iVar5 = 0;
	}
	else {
		iVar5 = this->pZoneStream->entryCount;
	}

	if (iVar5 == 0) {
		if (this->pActorStream == (S_BRIDGE_ACTOR_STREAM*)0x0) {
			iVar5 = 0;
		}
		else {
			iVar5 = this->pActorStream->entryCount;
		}

		if ((iVar5 == 0) && (this->noFrictionZoneCount == 0)) goto LAB_0015adf0;
	}

	SV_ComputeDiffMatrixFromInit(&this->field_0x200);
	edF32Matrix4GetInverseOrthoHard(&this->field_0x200, &this->field_0x200);

LAB_0015adf0:
	if ((this->pTiedActor != (CActor*)0x0) || (param_3 != 0)) {
		if (this->field_0x1ec.IsValid()) {
			this->field_0x1ec.SetPosition(&this->currentLocation);
			this->field_0x1ec.SetRotationEuler(&this->rotationEuler);
		}

		if (this->field_0x1f4.IsValid()) {
			this->field_0x1f4.SetPosition(&this->currentLocation);
			this->field_0x1f4.SetRotationEuler(&this->rotationEuler);
		}
	}

	ManageNoFrictionZones(0);
	return;
}

bool CActorMovingPlatform::StateWeighingMaster(CBehaviourWeighingMachineMaster* pBehaviour)
{
	CActor* pRefActor;
	bool bVar1;
	Timer* pTVar2;
	int iVar3;
	float fVar4;
	float fVar5;
	float puVar7;
	float fVar6;
	float local_10;
	undefined4 local_c;
	float* local_4;

	pTVar2 = GetTimer();

	pRefActor = (CActorMovable*)(pBehaviour->streamActorRef).Get();

	if ((pRefActor == (CActorMovable*)0x0) || (bVar1 = pRefActor->IsKindOfObject(2), bVar1 == false)) {
		fVar4 = 0.0f;
	}
	else {
		CActorMovable* pMovable = reinterpret_cast<CActorMovable*>(pRefActor);
		fVar4 = pMovable->GetCumulatedWeight();
	}

	fVar6 = pBehaviour->trajPos.pathPosition;

	fVar5 = GetCumulatedWeight();
	puVar7 = (fVar4 - fVar5) / pBehaviour->field_0x40;
	if (1.0f < fabs(puVar7)) {
		if (0.0f <= puVar7) {
			puVar7 = 1.0f;
		}
		else {
			puVar7 = -1.0f;
		}
	}

	fVar4 = pBehaviour->trajPos.pathPosition;
	fVar5 = pTVar2->cutsceneDeltaTime / pBehaviour->field_0x44;
	if (fabs(puVar7 - fVar4) < fVar5) {
		pBehaviour->trajPos.pathPosition = puVar7;
	}
	else {
		if (fVar4 < puVar7) {
			pBehaviour->trajPos.pathPosition = fVar4 + fVar5;
		}
		else {
			pBehaviour->trajPos.pathPosition = fVar4 - fVar5;
		}
	}

	local_4 = &local_10;
	local_10 = -pBehaviour->trajPos.pathPosition;
	local_c = 0;
	DoMessage(pRefActor, (ACTOR_MESSAGE)0x11, (MSG_PARAM)local_4);

	fVar4 = pBehaviour->trajPos.pathPosition;

	pBehaviour->analogSwitch.NotifyAnalog(fVar6, fVar4, this);

	fVar4 = pBehaviour->trajPos.pathPosition;
	pBehaviour->trajPos.pathPosition = pBehaviour->trajPos.pathPosition + 1.0f;

	const bool bVar4 = fVar4 != fVar6;

	Platform_UpdateMatrixOnTrajectory(&pBehaviour->pathFollowReaderAbs, bVar4, 1, &pBehaviour->trajPos, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);

	pBehaviour->trajPos.pathPosition = pBehaviour->trajPos.pathPosition - 1.0f;
	return bVar4;
}

void CActorMovingPlatform::Die(int bTriggerSwitches)
{
	int* piVar1;
	CCollision* pCVar2;
	CSound* pSound;
	uint uVar3;
	int iVar4;
	int iVar5;
	int iVar6;
	int iVar7;
	uint uVar8;

	if (this->pActorStream == (S_BRIDGE_ACTOR_STREAM*)0x0) {
		iVar7 = 0;
	}
	else {
		iVar7 = this->pActorStream->entryCount;
	}

	iVar6 = 0;
	if (0 < iVar7) {
		do {
			if (this->pActorStream->aEntries[iVar6].actorRef.Get() != (CActor*)0x0) {
				this->pActorStream->aEntries[iVar6].actorRef.Get()->TieToActor((CActor*)0x0, 0, 1, (edF32MATRIX4*)0x0);
			}

			iVar6 = iVar6 + 1;
		} while (iVar6 < iVar7);
	}

	this->movingPlatformFlags = this->movingPlatformFlags | 0x40;
	this->flags = this->flags & 0xfffffffd;
	this->flags = this->flags | 1;
	this->flags = this->flags & 0xffffff7f;
	this->flags = this->flags | 0x20;

	EvaluateDisplayState();

	pCVar2 = this->pCollisionData;
	if (pCVar2 != (CCollision*)0x0) {
		pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xfff7efff;
	}

	if (bTriggerSwitches != 0) {
		TriggerSwitches(6);
	}

	return;
}

void CActorMovingPlatform::TriggerSwitches(int conditionType)
{
	int nbEntries = 0;
	if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
		nbEntries = this->pCameraStream->entryCount;
	}

	ACTOR_LOG(LogLevel::Info, "CActorMovingPlatform::TriggerSwitches {} (0x{:x}) nbEntries: {}", this->name, conditionType, nbEntries);

	int curEntryIndex = 0;
	if (0 < nbEntries) {
		do {
			S_BRIDGE_CAMERA_STREAM_ENTRY* pEntry = &this->pCameraStream->aEntries[curEntryIndex];

			if (pEntry->field_0x0 == conditionType) {
				ACTOR_LOG(LogLevel::Info, "CActorMovingPlatform::TriggerSwitches - Triggering entry {} for condition 0x{:x}", curEntryIndex, conditionType);
				pEntry->streamTarget.Switch(this);
				pEntry->streamTarget.PostSwitch(this);
				pEntry->streamCameraEvent.SwitchOn(this);

				IMPLEMENTATION_GUARD_AUDIO(
					piVar6 = *(int**)(iVar3 + 0x48);
				if (piVar6 != (int*)0x0) {
					uVar4 = *(uint*)&this->pActorSound->field_0x4;
					uVar8 = (uint)((this->movingPlatformFlags & 1) != 0);
					if ((this->movingPlatformFlags & 2) != 0) {
						uVar8 = uVar8 + 1;
					}
					for (; uVar8 < uVar4; uVar8 = uVar8 + 1) {
						iVar3 = CActorSound::IsInstanceAlive(this->pActorSound, uVar8);
						if (iVar3 == 0) {
							CActorSound::SoundStart(this->pActorSound, this, uVar8, piVar6, 1, 0, (float**)0x0);
							break;
						}
					}
				})
			}

			curEntryIndex = curEntryIndex + 1;
		} while (curEntryIndex < nbEntries);
	}

	return;
}

void CBehaviourPlatformStand::Create(ByteCode* pByteCode)
{
	int iVar1;

	ACTOR_LOG(LogLevel::Info, "CBehaviourPlatformStand::Create");

	iVar1 = pByteCode->GetS32();
	this->field_0x8 = iVar1;
	iVar1 = pByteCode->GetS32();
	this->field_0xc = iVar1;
	this->pCinData.Reset();
	return;
}

void CBehaviourPlatformStand::Init(CActor* pOwner)
{
	this->pOwner = reinterpret_cast<CActorMovingPlatform*>(pOwner);

	IMPLEMENTATION_GUARD_AUDIO(
	FUN_00181e70((uint*)&this->field_0x8);)
	return;
}

void CBehaviourPlatformStand::Manage()
{
	CActorMovingPlatform* pCVar1;
	bool bVar3;
	edF32VECTOR4 local_10;

	pCVar1 = this->pOwner;
	local_10 = pCVar1->baseLocation;

	this->pOwner->Platform_UpdatePosition(&local_10, 0, 0);

	pCVar1 = this->pOwner;
	if (pCVar1->pTiedActor != (CActor*)0x0) {
		if (this->pCinData.IsValid()) {
			this->pCinData.SetPosition(&pCVar1->currentLocation);
			this->pCinData.SetRotationEuler(&pCVar1->rotationEuler);
		}
	}

	this->pOwner->GenericManage(1, 0, -1, -1);
	return;
}

void CBehaviourPlatformStand::ManageFrozen()
{
	if ((GameFlags & 0x20) == 0) {
		this->Manage();
	}

	return;
}

void CBehaviourPlatformStand::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	if (newState == -1) {
		this->pOwner->SetState(0, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	if ((this->pOwner->flags & 4) != 0) {
		ChangeManageState(1);
	}
}

void CBehaviourPlatformStand::End(int newBehaviourId)
{
	ChangeManageState(0);
}

// Should be in: D:/Projects/b-witch/ActorMovingPlatform.h
void CBehaviourPlatformStand::SaveContext(S_SAVE_CLASS_MOVING_PLATFORM* pData)
{
	return;
}

void CBehaviourPlatformStand::LoadContext(S_SAVE_CLASS_MOVING_PLATFORM* pData)
{
	return;
}

void CBehaviourPlatformStand::ChangeManageState(int state)
{
	CNewFx* pCVar1;
	CActorMovingPlatform* pCVar2;
	CinNamedObject30* pCVar3;
	bool bVar4;

	if (state == 0) {
		if (this->pCinData.IsValid()) {
			this->pCinData.Kill();
			this->pCinData.Reset();
		}

		this->pCinData.Reset();

		if (this->field_0x8 != 0) {
			IMPLEMENTATION_GUARD_AUDIO(
			CActorSound::FUN_0032c600(this->pOwner->pActorSound, 0);)
		}
	}
	else {
		if ((CSound*)this->field_0x8 != (CSound*)0x0) {
			IMPLEMENTATION_GUARD_AUDIO(
			CActorSound::SoundStart
			(this->pOwner->pActorSound, this->pOwner, 0, (CSound*)this->field_0x8, 1, 0,
				(SOUND_SPATIALIZATION_PARAM*)0x0);)
		}

		if (this->field_0xc != 0xffffffff) {
			if (!this->pCinData.IsValid()) {
				CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&this->pCinData, this->field_0xc, FX_MATERIAL_SELECTOR_NONE);
			}
		}

		if (this->pCinData.IsValid()) {
			pCVar2 = this->pOwner;
			this->pCinData.SetPosition(&pCVar2->currentLocation);
			this->pCinData.SetRotationEuler(&pCVar2->rotationEuler);
			this->pCinData.Start();
		}
	}

	return;
}

CBehaviourPlatformTrajectory::CBehaviourPlatformTrajectory()
	: pathFollowReaderAbs()
{

}

void CBehaviourPlatformTrajectory::Create(ByteCode* pByteCode)
{
	ACTOR_LOG(LogLevel::Info, "CBehaviourPlatformTrajectory::Create");

	this->pathFollowReaderAbs.Create(pByteCode);
	this->segmentStartTime = pByteCode->GetF32();
	this->loopDelay = pByteCode->GetF32();

	ACTOR_LOG(LogLevel::Info, "CBehaviourPlatformTrajectory::Create {} {}", this->pathFollowReaderAbs.field_0x4, this->pathFollowReaderAbs.totalTraversalTime);
}

void CBehaviourPlatformTrajectory::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	Timer* pTVar1;
	CActorMovingPlatform* pAVar1;

	pTVar1 = GetTimer();

	this->pOwner = reinterpret_cast<CActorMovingPlatform*>(pOwner);

	this->pathLength = 0.0f;
	this->trajPos.pathPosition = this->segmentStartTime;

	this->trajPos.currentSegment = 0;
	this->trajPos.prevSegment = 0;
	this->trajPos.lastEventSegment = -1;

	if (newState == -1) {
		pAVar1 = this->pOwner;

		if ((pAVar1->pProperties->flags_0x24 & 4) == 0) {
			pAVar1->SetState(MOVING_PLATFORM_STATE_STAND, -1);
			this->targetScaledTime = pTVar1->scaledTotalTime;
		}
		else {
			pAVar1->SetState(MOVING_PLATFORM_STATE_MOVING, -1);
			this->targetScaledTime = 0.0f;
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	this->pOwner->Platform_UpdateMatrixOnTrajectory(&this->pathFollowReaderAbs, 1, 0, & this->trajPos, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
	this->trajPos.lastEventSegment = this->trajPos.currentSegment;
}

void CBehaviourPlatformTrajectory::End(int newBehaviourId)
{
	CCollision* pCol;

	pCol = this->pOwner->pCollisionData;
	if (pCol != (CCollision*)0x0) {
		pCol->flags_0x0 = pCol->flags_0x0 & 0xffdfffff;
	}

	return;
}

int CBehaviourPlatformTrajectory::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CPathFollow* pCVar1;
	int bProcessed;
	Timer* pTVar3;
	float fVar4;
	float local_18;
	int local_14;
	int local_10;
	float local_c;
	int local_8;
	int local_4;

	if (msg == 0x68) {
		pCVar1 = (this->pathFollowReaderAbs).pPathFollow;
		if (pCVar1 != (CPathFollow*)0x0) {
			this->pathFollowReaderAbs.ComputeSegment((this->trajPos).pathPosition, &local_10, &local_14, &local_18);
			if (local_18 < 0.5f) {
				local_10 = local_14;
			}

			local_10 = local_10 + -1;
			if (local_10 < pCVar1->nbLeadInPoints) {
				if ((pCVar1->type == 0) || (pCVar1->mode == 1)) {
					if (local_10 < 0) {
						local_10 = 0;
					}
				}
				else {
					local_10 = pCVar1->splinePointCount + -1;
				}
			}

			pCVar1 = (this->pathFollowReaderAbs).pPathFollow;
			if (((local_10 < 0) || (pCVar1 == (CPathFollow*)0x0)) || (pCVar1->splinePointCount <= local_10)) {
				local_10 = 0;
			}

			this->pathFollowReaderAbs.GetClosestTimeToReachWaypoint((this->trajPos).pathPosition, local_10, &this->pathLength, &this->trajPos.pathPosition);
			this->pOwner->SetState(MOVING_PLATFORM_STATE_TRANSITION_B, -1);
			fVar4 = (this->trajPos).pathPosition;
			if (fVar4 <= this->pathLength) {
				this->targetScaledTime = (this->segmentStartTime + GetTimer()->scaledTotalTime) - fVar4;
			}
			else {
				this->targetScaledTime = fVar4 + this->segmentStartTime + GetTimer()->scaledTotalTime;
			}
		}

		bProcessed = 1;
	}
	else {
		if (msg == 0x67) {
			pCVar1 = (this->pathFollowReaderAbs).pPathFollow;
			if (pCVar1 != (CPathFollow*)0x0) {
				this->pathFollowReaderAbs.ComputeSegment((this->trajPos).pathPosition, &local_4, &local_8, &local_c);
				if (local_c < 0.5f) {
					local_4 = local_8;
				}

				local_4 = local_4 + 1;
				if (pCVar1->splinePointCount <= local_4) {
					if ((pCVar1->type == 0) || (pCVar1->mode == 1)) {
						local_4 = pCVar1->splinePointCount + -1;
					}
					else {
						local_4 = pCVar1->nbLeadInPoints;
					}
				}

				pCVar1 = (this->pathFollowReaderAbs).pPathFollow;
				if (((local_4 < 0) || (pCVar1 == (CPathFollow*)0x0)) || (pCVar1->splinePointCount <= local_4)) {
					local_4 = 0;
				}

				this->pathFollowReaderAbs.GetClosestTimeToReachWaypoint((this->trajPos).pathPosition, local_4, &this->pathLength, &this->trajPos.pathPosition);
				this->pOwner->SetState(MOVING_PLATFORM_STATE_TRANSITION_B, -1);
				fVar4 = (this->trajPos).pathPosition;
				if (fVar4 <= this->pathLength) {
					this->targetScaledTime = (this->segmentStartTime + GetTimer()->scaledTotalTime) - fVar4;
				}
				else {
					this->targetScaledTime = fVar4 + this->segmentStartTime + GetTimer()->scaledTotalTime;
				}
			}

			bProcessed = 1;
		}
		else {
			if (msg == 0x3b) {
				pCVar1 = (this->pathFollowReaderAbs).pPathFollow;
				if ((((int)pMsgParam < 0) || (pCVar1 == (CPathFollow*)0x0)) || (pCVar1->splinePointCount <= (int)pMsgParam)) {
					pMsgParam = (void*)0x0;
				}

				this->pathFollowReaderAbs.GetClosestTimeToReachWaypoint((this->trajPos).pathPosition, (int)pMsgParam, &this->pathLength, &this->trajPos.pathPosition);
				this->pOwner->SetState(MOVING_PLATFORM_STATE_TRANSITION_B, -1);

				fVar4 = (this->trajPos).pathPosition;
				if (fVar4 <= this->pathLength) {
					this->targetScaledTime = (this->segmentStartTime + GetTimer()->scaledTotalTime) - fVar4;
				}
				else {
					this->targetScaledTime = fVar4 + this->segmentStartTime + GetTimer()->scaledTotalTime;
				}

				bProcessed = 1;
			}
			else {
				if (msg == MESSAGE_TIED) {
					_msg_tied_params* pTiedParams = reinterpret_cast<_msg_tied_params*>(pMsgParam);
					if (pTiedParams->bTied != 0) {
						this->pOwner->BehaviourTrajectory_Switch(this, 0xf);
					}

					bProcessed = 1;
				}
				else {
					if (((msg == 0xe) || (msg == 0x10)) || (msg == 0xf)) {
						this->pOwner->BehaviourTrajectory_Switch(this, msg);
						bProcessed = 1;
					}
					else {
						bProcessed = 0;
					}
				}
			}
		}
	}
	return bProcessed;
}

void CBehaviourPlatformTrajectory::SaveContext(S_SAVE_CLASS_MOVING_PLATFORM* pData)
{
	pData->field_0x1c = (this->trajPos).pathPosition;
	pData->field_0x14 = (this->trajPos).currentSegment;
	pData->field_0x16 = (this->trajPos).prevSegment;
	pData->field_0x18 = (this->trajPos).lastEventSegment;
	pData->field_0x20 = 0.0f;

	switch (this->pOwner->actorState) {
	case MOVING_PLATFORM_STATE_STAND:
		pData->field_0x20 = this->targetScaledTime - GetTimer()->scaledTotalTime;
		break;
	case MOVING_PLATFORM_STATE_MOVING:
		pData->flags = pData->flags | 4;
		break;
	case MOVING_PLATFORM_STATE_TRANSITION_A:
		pData->field_0x20 = this->targetScaledTime - GetTimer()->scaledTotalTime;
		pData->flags = pData->flags | 0x10;
		break;
	case MOVING_PLATFORM_STATE_AT_ENDPOINT:
	case MOVING_PLATFORM_STATE_WAITING:
		pData->flags = pData->flags | 4;
		pData->field_0x1c = 0.0f;
		break;
	case MOVING_PLATFORM_STATE_TRANSITION_B:
		pData->flags = pData->flags | 4;
		pData->field_0x1c = this->pathLength;
	}

	return;
}

void CBehaviourPlatformTrajectory::LoadContext(S_SAVE_CLASS_MOVING_PLATFORM* pData)
{
	(this->trajPos).pathPosition = pData->field_0x1c;
	(this->trajPos).currentSegment = pData->field_0x14;
	(this->trajPos).prevSegment = pData->field_0x16;
	(this->trajPos).lastEventSegment = pData->field_0x18;

	this->targetScaledTime = pData->field_0x20 + GetTimer()->scaledTotalTime;

	if ((pData->flags & 4) == 0) {
		if ((pData->flags & 0x10) == 0) {
			this->pOwner->SetState(MOVING_PLATFORM_STATE_STAND, -1);
		}
		else {
			this->pOwner->SetState(MOVING_PLATFORM_STATE_TRANSITION_A, -1);
		}
	}
	else {
		this->pOwner->SetState(MOVING_PLATFORM_STATE_MOVING, -1);
	}

	this->pOwner->Platform_UpdateMatrixOnTrajectory(&this->pathFollowReaderAbs, 1, 0, &this->trajPos, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);

	return;
}

void CBehaviourPlatformTrajectory::Manage()
{
	this->pOwner->BehaviourTrajectory_Manage(this);
}

void CBehaviourPlatformTrajectory::ManageFrozen()
{
	if ((GameFlags & 0x20) == 0) {
		this->pOwner->Platform_UpdateMatrixOnTrajectory(&this->pathFollowReaderAbs, 0, 0, &this->trajPos, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
		this->pOwner->GenericManage(1, 0, (this->trajPos).currentSegment, (this->trajPos).prevSegment);
	}

	return;
}

void CBehaviourPlatformSlab::Create(ByteCode* pByteCode)
{
	S_TARGET_ON_OFF_STREAM_REF* piVar1;
	S_STREAM_EVENT_CAMERA* pcVar2;
	int iVar3;
	float pCVar2;
	float fVar4;

	ACTOR_LOG(LogLevel::Info, "CBehaviourPlatformSlab::Create");

	switchOnOff.Create(pByteCode);

	this->field_0x10 = pByteCode->GetF32();
	this->field_0x14 = pByteCode->GetF32();
	this->field_0x18 = pByteCode->GetF32();
	this->field_0x1c = pByteCode->GetF32();

	this->field_0x20.index = pByteCode->GetF32();
	return;
}

void CBehaviourPlatformSlab::Init(CActor* pOwner)
{
	S_TARGET_ON_OFF_STREAM_REF* pSVar1;
	int iVar3;

	this->pOwner = (CActorMovingPlatform*)pOwner;

	switchOnOff.Init();

	IMPLEMENTATION_GUARD_AUDIO(
	S_STREAM_REF<CSound>::Init(&this->field_0x20);)

	return;
}

void CBehaviourPlatformSlab::Manage()
{
	ACTOR_LOG(LogLevel::Verbose, "CBehaviourPlatformSlab::Manage");

	this->switchOnOff.pStreamEventCamera->Manage(this->pOwner);
	this->pOwner->BehaviourSlab_Manage(this);
}

void CBehaviourPlatformSlab::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	switchOnOff.Reset(pOwner);

	if (newState == -1) {
		this->pOwner->SetState(MOVING_PLATFORM_STATE_TRAJ_TO_B, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	this->field_0x28 = 0.0f;
	this->field_0x24 = 0.0f;
	this->field_0x2c = (CActor*)0x0;

	this->pOwner->Slab_MoveAndDetectCarriedObject(this, 1);
	return;
}

void CBehaviourPlatformSlab::SaveContext(S_SAVE_CLASS_MOVING_PLATFORM* pData)
{
	switchOnOff.SaveContext(&pData->switchCamera);

	int iVar3 = this->pOwner->actorState;
	if ((iVar3 == MOVING_PLATFORM_STATE_SLAB_OFF_TO_ON) || (iVar3 == MOVING_PLATFORM_STATE_TRAJ_TO_A)) {
		pData->flags = pData->flags | 8;
	}

	return;
}

void CBehaviourPlatformSlab::LoadContext(S_SAVE_CLASS_MOVING_PLATFORM* pData)
{
	this->field_0x2c = (CActor*)0x0;

	switchOnOff.LoadContext(&pData->switchCamera);

	if ((pData->flags & 8) != 0) {
		this->field_0x24 = this->field_0x18;
		this->pOwner->SetState(MOVING_PLATFORM_STATE_TRAJ_TO_A, -1);
		this->pOwner->Slab_MoveAndDetectCarriedObject(this, 1);
	}

	return;
}

CBehaviourPlatformDestroyed::CBehaviourPlatformDestroyed()
	: pathFollowReaderAbs()
	, alternateModel()
{

}

void CBehaviourPlatformDestroyed::Create(ByteCode* pByteCode)
{
	int iVar1;

	ACTOR_LOG(LogLevel::Info, "CBehaviourPlatformDestroyed::Create");

	this->pathFollowReaderAbs.Create(pByteCode);
	iVar1 = pByteCode->GetS32();
	this->field_0x40 = iVar1;
	iVar1 = pByteCode->GetS32();
	this->altModelId = iVar1;
	return;
}

void CBehaviourPlatformDestroyed::Init(CActor* pOwner)
{
	this->pOwner = static_cast<CActorMovingPlatform*>(pOwner);

	if (this->altModelId != -1) {
		this->pOwner->SV_SwitchToModel(&this->alternateModel, this->altModelId, this->field_0x40, 0);
	}

	return;
}

void CBehaviourPlatformDestroyed::Term()
{
	if (this->altModelId != -1) {
		this->pOwner->SV_RestoreOrgModel(&this->alternateModel);
	}

	return;
}

void CBehaviourPlatformDestroyed::Manage()
{
	uint* puVar1;
	CAnimation* pCVar2;
	edAnmLayer* peVar3;
	S_BRIDGE_CAMERA_STREAM* pSVar4;
	bool bVar5;
	int iVar7;
	S_BRIDGE_CAMERA_STREAM* pSVar8;
	int iVar9;
	uint uVar10;
	int* piVar11;
	CActorMovingPlatform* pMovingPlatform;

	pMovingPlatform = this->pOwner;

	if (pMovingPlatform->actorState == MOVING_PLATFORM_STATE_DESTROYED) {
		(this->trajPos).pathPosition = (this->trajPos).pathPosition + Timer::GetTimer()->cutsceneDeltaTime;

		iVar7 = pMovingPlatform->Platform_UpdateMatrixOnTrajectory(&this->pathFollowReaderAbs, 1, 1, &this->trajPos, (CActorsTable*)0x0, &this->field_0x50);
		if ((iVar7 == 2) && ((pMovingPlatform->pProperties->flags_0x24 & 0x2000) != 0)) {
			pCVar2 = pMovingPlatform->pAnimationController;

			if (pCVar2 != (CAnimation*)0x0) {
				if ((!pCVar2->IsCurrentLayerAnimEndReached(0)) || (pMovingPlatform->timeInAir <= 0.0f)) goto LAB_00156de0;
			}

			pSVar4 = pMovingPlatform->pCameraStream;
			uVar10 = 0;
			if (pSVar4 == (S_BRIDGE_CAMERA_STREAM*)0x0) {
				iVar7 = 0;
			}
			else {
				iVar7 = pSVar4->entryCount;
			}

			iVar9 = 0;
			if (0 < iVar7) {
				pSVar8 = pSVar4;
				if (8 < iVar7) {
					do {
						iVar9 = iVar9 + 8;
						uVar10 = uVar10 | 
							pSVar8->aEntries[0].streamCameraEvent.flags & 0x40000000U |
							pSVar8->aEntries[1].streamCameraEvent.flags & 0x40000000U |
							pSVar8->aEntries[2].streamCameraEvent.flags & 0x40000000 |
							pSVar8->aEntries[3].streamCameraEvent.flags & 0x40000000U |
							pSVar8->aEntries[4].streamCameraEvent.flags & 0x40000000 |
							pSVar8->aEntries[5].streamCameraEvent.flags & 0x40000000U |
							pSVar8->aEntries[6].streamCameraEvent.flags & 0x40000000 |
							pSVar8->aEntries[7].streamCameraEvent.flags & 0x40000000U;
						pSVar8 = pSVar8 + 8;
					} while (iVar9 < iVar7 + -8);
				}

				if (iVar9 < iVar7) {
					S_BRIDGE_CAMERA_STREAM_ENTRY* pEntry = pSVar4->aEntries;
					do {
						uVar10 = uVar10 | pEntry->streamCameraEvent.flags & 0x40000000;
						iVar9 = iVar9 + 1;
						pEntry = pEntry + 1;
					} while (iVar9 < iVar7);
				}
			}

			if (uVar10 == 0) {
				pMovingPlatform->Die(1);
			}
		}
	}

LAB_00156de0:
	this->pOwner->GenericManage(0, 0, (this->trajPos).currentSegment, (this->trajPos).prevSegment);

	return;
}

void CBehaviourPlatformDestroyed::ManageFrozen()
{
	if ((GameFlags & 0x20) == 0) {
		this->pOwner->GenericManage(0, 0, (this->trajPos).currentSegment, (this->trajPos).prevSegment);
	}

	return;
}

void CBehaviourPlatformDestroyed::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CCollision* pCVar1;
	KyaUpdateObjA* pKVar2;
	CPathFollow* pCVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	edF32VECTOR4* v2;
	edF32VECTOR4 local_20;
	float local_10;
	float local_c;
	float local_8;
	float local_4;

	if (newState == -1) {
		this->pOwner->SetState(MOVING_PLATFORM_STATE_DESTROYED, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	if (((this->pOwner->pProperties->flags_0x24 & 0x200000) == 0) &&
		(pCVar1 = this->pOwner->pCollisionData, pCVar1 != (CCollision*)0x0)) {
		pCVar1->flags_0x0 = pCVar1->flags_0x0 & 0xfff7efff;
	}

	(this->trajPos).pathPosition = 0.0f;
	(this->trajPos).currentSegment = 0;
	(this->trajPos).prevSegment = 0;
	(this->trajPos).lastEventSegment = -1;

	if (this->altModelId != -1) {
		this->pOwner->SV_RestoreOrgModel(&this->alternateModel);
		pKVar2 = this->pOwner->subObjA;
		local_20.xyz = (pKVar2->boundingSphere).xyz;
		local_4 = (pKVar2->boundingSphere).w;
		local_20.w = 0.0f;
		local_10 = local_20.x;
		local_c = local_20.y;
		local_8 = local_20.z;
		this->pOwner->SetLocalBoundingSphere(local_4 * 10.0f, &local_20);
	}

	pCVar3 = (this->pathFollowReaderAbs).pPathFollow;
	if ((pCVar3 == (CPathFollow*)0x0) || ((this->pOwner->pProperties->flags_0x24 & 0x100000) == 0)) {
		this->field_0x50 = gF32Vector4Zero;
	}
	else {
		v2 = pCVar3->aSplinePoints;
		if (v2 == (edF32VECTOR4*)0x0) {
			v2 = &gF32Vertex4Zero;
		}

		edF32Vector4SubHard(&this->field_0x50, &this->pOwner->currentLocation, v2);
	}

	return;
}

void CBehaviourPlatformDestroyed::End(int newBehaviourId)
{
	CCollision* pCVar1;
	KyaUpdateObjA* pKVar2;
	edF32VECTOR4 local_20;
	float local_10;
	float local_c;
	float local_8;
	float local_4;

	pCVar1 = this->pOwner->pCollisionData;
	if (pCVar1 != (CCollision*)0x0) {
		pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x81000;
	}

	if (this->altModelId != -1) {
		pKVar2 = this->pOwner->subObjA;
		local_20.xyz = (pKVar2->boundingSphere).xyz;
		local_4 = (pKVar2->boundingSphere).w;
		local_20.w = 0.0f;
		local_10 = local_20.x;
		local_c = local_20.y;
		local_8 = local_20.z;
		this->pOwner->SetLocalBoundingSphere(local_4 / 10.0f, &local_20);

		this->pOwner->SV_SwitchToModel(&this->alternateModel, this->altModelId, this->field_0x40, 0);
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorMovingPlatform.h
void CBehaviourPlatformDestroyed::SaveContext(S_SAVE_CLASS_MOVING_PLATFORM* pData)
{
	return;
}

void CBehaviourPlatformDestroyed::LoadContext(S_SAVE_CLASS_MOVING_PLATFORM* pData)
{
	return;
}

void CBehaviourWeighingMachine::Create(ByteCode* pByteCode)
{
	CPathManager* pPathManager;
	CPathFollow* pPathFollow;

	pPathManager = CScene::ptable.g_PathManager_004516a0;

	const int pathIndex = pByteCode->GetS32();
	if (pathIndex == -1) {
		pPathFollow = (CPathFollow*)0x0;
	}
	else {
		pPathFollow = pPathManager->aPathFollow + pathIndex;
	}

	this->pathFollowReaderAbs.Create(2.0f, 0.0f, pPathFollow, 0, 0, pByteCode->GetS32(), 0);

	return;
}


void CBehaviourWeighingMachine::Init(CActor* pOwner)
{
	this->pOwner = static_cast<CActorMovingPlatform*>(pOwner);
	return;
}

void CBehaviourWeighingMachine::ManageFrozen()
{
	CActorMovingPlatform* pPlatform;

	if ((GameFlags & 0x20) == 0) {
		pPlatform = this->pOwner;
		(this->trajPos).pathPosition = (this->trajPos).pathPosition + 1.0f;

		pPlatform->Platform_UpdateMatrixOnTrajectory(&this->pathFollowReaderAbs, 0, 0, &this->trajPos, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);

		(this->trajPos).pathPosition = (this->trajPos).pathPosition - 1.0f;

		this->pOwner->GenericManage(1, 0, (this->trajPos).currentSegment, (this->trajPos).prevSegment);
	}
	return;
}

void CBehaviourWeighingMachine::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	if (newState == -1) {
		if ((this->pathFollowReaderAbs).pPathFollow == (CPathFollow*)0x0) {
			this->pOwner->SetState(MOVING_PLATFORM_STATE_MOVING, -1);
		}
		else {
			this->pOwner->SetState(MOVING_PLATFORM_STATE_TRANSITION_B, -1);
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	(this->trajPos).pathPosition = 0.0f;
	(this->trajPos).currentSegment = 0;
	(this->trajPos).prevSegment = 0;
	(this->trajPos).lastEventSegment = -1;
}

void CBehaviourWeighingMachine::End(int newBehaviourId)
{
	CCollision* pCol;

	pCol = this->pOwner->pCollisionData;

	if (pCol != (CCollision*)0x0) {
		pCol->flags_0x0 = pCol->flags_0x0 & 0xffdfffff;
	}

	return;
}

void CBehaviourWeighingMachineMaster::Create(ByteCode* pByteCode)
{
	CPathManager* pPathManager;
	CPathFollow* pPathFollow;

	pPathManager = CScene::ptable.g_PathManager_004516a0;

	const int pathIndex = pByteCode->GetS32();
	if (pathIndex == -1) {
		pPathFollow = (CPathFollow*)0x0;
	}
	else {
		pPathFollow = pPathManager->aPathFollow + pathIndex;
	}

	this->pathFollowReaderAbs.Create(2.0f, 0.0f, pPathFollow, 0, 0, pByteCode->GetS32(), 0);

	(this->streamActorRef).index = pByteCode->GetS32();

	this->field_0x40 = pByteCode->GetF32();
	this->field_0x44 = pByteCode->GetF32();

	analogSwitch.Create(pByteCode);

	return;
}

void CBehaviourWeighingMachineMaster::Init(CActor* pOwner)
{
	int* piVar1;
	int iVar2;
	int iVar3;

	this->pOwner = static_cast<CActorMovingPlatform*>(pOwner);

	analogSwitch.Init();
	this->streamActorRef.Init();
	return;

}

void CBehaviourWeighingMachineMaster::Manage()
{
	bool uVar1;
	S_TRAJ_POS* pTrajPos;
	CActorMovingPlatform* pPlatform;
	int state;

	this->analogSwitch.pStreamEventCamera->Manage(this->pOwner);

	pPlatform = this->pOwner;
	state = pPlatform->actorState;

	uVar1 = false;
	if (state == MOVING_PLATFORM_STATE_MOVING) {
		pTrajPos = &this->trajPos;

		this->trajPos.pathPosition = this->trajPos.pathPosition + 1.0f;

		pPlatform->Platform_UpdateMatrixOnTrajectory(&this->pathFollowReaderAbs, 0, 0, pTrajPos, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);

		pTrajPos->pathPosition = pTrajPos->pathPosition - 1.0f;
	}
	else {
		if (state == MOVING_PLATFORM_STATE_SLAB_ON_SLAVE_ACTIVE) {
			uVar1 = pPlatform->StateWeighingMaster(this);
		}
	}

	pPlatform->GenericManage(1, uVar1, this->trajPos.currentSegment, this->trajPos.prevSegment);
	return;
}

void CBehaviourWeighingMachineMaster::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	int* piVar1;
	int iVar2;
	S_TRAJ_POS* pTrajPos;
	int iVar3;
	CActorMovingPlatform* pPlatform;

	if (newState == -1) {
		if (this->pathFollowReaderAbs.pPathFollow == (CPathFollow*)0x0) {
			pPlatform = this->pOwner;
			pPlatform->SetState(MOVING_PLATFORM_STATE_MOVING, -1);
		}
		else {
			pPlatform = this->pOwner;
			pPlatform->SetState(MOVING_PLATFORM_STATE_SLAB_ON_SLAVE_ACTIVE, -1);
		}
	}
	else {
		pPlatform = this->pOwner;
		pPlatform->SetState(newState, newAnimationType);
	}

	this->trajPos.pathPosition = 0.0f;
	this->trajPos.currentSegment = 0;
	this->trajPos.prevSegment = 0;
	this->trajPos.lastEventSegment = -1;

	this->analogSwitch.Reset(pOwner);

	pTrajPos = &this->trajPos;
	pPlatform = this->pOwner;
	this->trajPos.pathPosition = this->trajPos.pathPosition + 1.0f;

	pPlatform->Platform_UpdateMatrixOnTrajectory(&this->pathFollowReaderAbs, 1, 0, pTrajPos, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);

	pTrajPos->pathPosition = pTrajPos->pathPosition - 1.0f;

	this->trajPos.lastEventSegment = this->trajPos.currentSegment;
	return;
}

void CBehaviourWeighingMachineMaster::SaveContext(S_SAVE_CLASS_MOVING_PLATFORM* pData)
{
	uint uVar4;
	int iVar3;

	uVar4 = 1;

	pData->field_0x1c = (this->trajPos).pathPosition;
	pData->field_0x14 = (this->trajPos).currentSegment;
	pData->field_0x16 = (this->trajPos).prevSegment;
	pData->field_0x18 = (this->trajPos).lastEventSegment;

	this->analogSwitch.SaveContext(&pData->switchCamera);
}

void CBehaviourWeighingMachineMaster::LoadContext(S_SAVE_CLASS_MOVING_PLATFORM* pData)
{
	S_STREAM_NTF_TARGET_ANALOG_LIST* pSVar2;
	int iVar3;
	int iVar4;
	uint uVar5;
	S_TRAJ_POS* pTrajPos;
	CActorMovingPlatform* pPlatform;

	this->trajPos.pathPosition = pData->field_0x1c;
	this->trajPos.currentSegment = pData->field_0x14;
	this->trajPos.prevSegment = pData->field_0x16;
	this->trajPos.lastEventSegment = pData->field_0x18;

	this->analogSwitch.LoadContext(&pData->switchCamera);
	
	pTrajPos = &this->trajPos;
	pPlatform = this->pOwner;
	this->trajPos.pathPosition = this->trajPos.pathPosition + 1.0f;
	pPlatform->Platform_UpdateMatrixOnTrajectory(&this->pathFollowReaderAbs, 1, 0, pTrajPos, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
	pTrajPos->pathPosition = pTrajPos->pathPosition - 1.0f;

	return;
}

void CBehaviourWeighingMachineSlave::Create(ByteCode* pByteCode)
{
	CPathManager* pPathManager;
	CPathFollow* pPathFollow;

	pPathManager = CScene::ptable.g_PathManager_004516a0;

	const int pathIndex = pByteCode->GetS32();
	if (pathIndex == -1) {
		pPathFollow = (CPathFollow*)0x0;
	}
	else {
		pPathFollow = pPathManager->aPathFollow + pathIndex;
	}

	this->pathFollowReaderAbs.Create(2.0f, 0.0f, pPathFollow, 0, 0, pByteCode->GetS32(), 0);

	return;
}

void CBehaviourWeighingMachineSlave::Manage()
{
	int actorState;
	uint uVar2;
	S_TRAJ_POS* pTrajPos;
	CActorMovingPlatform* pPlatform;

	pPlatform = this->pOwner;
	actorState = pPlatform->actorState;
	uVar2 = 0;

	if (actorState == MOVING_PLATFORM_STATE_MOVING) {
		pTrajPos = &this->trajPos;
		this->trajPos.pathPosition = this->trajPos.pathPosition + 1.0f;
		pPlatform->Platform_UpdateMatrixOnTrajectory(&this->pathFollowReaderAbs, 0, 0, pTrajPos, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
		pTrajPos->pathPosition = pTrajPos->pathPosition - 1.0f;
	}
	else {
		if (actorState == MOVING_PLATFORM_STATE_SLAB_ON_SLAVE_ACTIVE) {
			uVar2 = (uint)(this->field_0x34 != this->trajPos.pathPosition);
			this->trajPos.pathPosition = this->field_0x34;
			pTrajPos = &this->trajPos;
			this->trajPos.pathPosition = this->trajPos.pathPosition + 1.0f;
			pPlatform->Platform_UpdateMatrixOnTrajectory(&this->pathFollowReaderAbs, uVar2, 1, pTrajPos, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
			pTrajPos->pathPosition = pTrajPos->pathPosition - 1.0f;
		}
	}

	pPlatform->GenericManage(1, uVar2, this->trajPos.currentSegment, this->trajPos.prevSegment);
	return; 
}

void CBehaviourWeighingMachineSlave::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	S_TRAJ_POS* pTrajPos;
	CActorMovingPlatform* pPlatform;

	if (newState == -1) {
		if (this->pathFollowReaderAbs.pPathFollow == (CPathFollow*)0x0) {
			pPlatform = this->pOwner;
			pPlatform->SetState(MOVING_PLATFORM_STATE_MOVING, -1);
		}
		else {
			pPlatform = this->pOwner;
			pPlatform->SetState(MOVING_PLATFORM_STATE_SLAB_ON_SLAVE_ACTIVE, -1);
		}
	}
	else {
		pPlatform = this->pOwner;
		pPlatform->SetState(newState, newAnimationType);
	}

	this->trajPos.pathPosition = 0.0f;
	this->trajPos.currentSegment = 0;
	this->trajPos.prevSegment = 0;
	this->trajPos.lastEventSegment = -1;

	pTrajPos = &this->trajPos;
	pPlatform = this->pOwner;
	this->trajPos.pathPosition = this->trajPos.pathPosition + 1.0f;

	pPlatform->Platform_UpdateMatrixOnTrajectory(&this->pathFollowReaderAbs, 1, 0, pTrajPos, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);

	pTrajPos->pathPosition = pTrajPos->pathPosition - 1.0f;

	this->trajPos.lastEventSegment = this->trajPos.currentSegment;
	return;
}

int CBehaviourWeighingMachineSlave::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	if (msg == 0x11) {
		this->field_0x34 = *reinterpret_cast<float*>(pMsgParam);
	}

	return msg == 0x11;
}

void CBehaviourWeighingMachineSlave::SaveContext(S_SAVE_CLASS_MOVING_PLATFORM* pData)
{
	pData->field_0x1c = this->trajPos.pathPosition;
	pData->field_0x14 = this->trajPos.currentSegment;
	pData->field_0x16 = this->trajPos.prevSegment;
	pData->field_0x18 = this->trajPos.lastEventSegment;

	return;
}

void CBehaviourWeighingMachineSlave::LoadContext(S_SAVE_CLASS_MOVING_PLATFORM* pData)
{
	S_TRAJ_POS* pTrajPos;
	CActorMovingPlatform* pPlatform;

	this->trajPos.pathPosition = pData->field_0x1c;
	this->trajPos.currentSegment = pData->field_0x14;
	this->trajPos.prevSegment = pData->field_0x16;
	this->trajPos.lastEventSegment = pData->field_0x18;

	pTrajPos = &this->trajPos;
	pPlatform = this->pOwner;
	this->trajPos.pathPosition = this->trajPos.pathPosition + 1.0f;
	pPlatform->Platform_UpdateMatrixOnTrajectory(&this->pathFollowReaderAbs, 1, 0, pTrajPos, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
	pTrajPos->pathPosition = pTrajPos->pathPosition - 1.0f;

	return;
}

void CBehaviourSelector::Create(ByteCode* pByteCode)
{
	int iVar1;
	ParticleInfo* pPVar2;
	CSound* pCVar3;
	float fVar4;

	this->field_0x18 = pByteCode->GetS32();

	iVar1 = pByteCode->GetS32();
	CActor::SV_InstallMaterialId(iVar1);
	pPVar2 = CScene::ptable.g_C3DFileManager_00451664->GetG2DInfo(iVar1);
	this->pParticleInfo = pPVar2;

	this->field_0x8 = pByteCode->GetF32();
	this->field_0xc = pByteCode->GetF32();
	this->field_0x10 = pByteCode->GetF32();

	(this->streamRefSound).index = pByteCode->GetS32();
	return;
}

void CBehaviourSelector::Draw()
{
	IMPLEMENTATION_GUARD_FX();
}

void CBehaviourSelector::InitState(int state)
{
	CSound* pSound;
	CActorMovingPlatform* pActor;

	IMPLEMENTATION_GUARD_AUDIO(
	if ((state == MOVING_PLATFORM_STATE_TRAJ_TO_A) && (pSound = (this->streamRefSound).pSound, pSound != (CSound*)0x0)) {
		pActor = this->pOwner;
		CActorSound::SoundStart
		(pActor->pActorSound, (CActor*)pActor, (uint)((pActor->movingPlatformFlags & 1) != 0), pSound, 1, 0,
			(SOUND_SPATIALIZATION_PARAM*)0x0);
	})
	return;
}

int CBehaviourSelector::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	S_TIED_ACTOR_ENTRY* pSVar1;
	bool bVar2;
	ed_g2d_manager* pTexture;
	ulong uVar3;
	float fVar4;
	CCollision* pCol;

	uVar3 = 0;
	if (msg == 0x10) {
		this->pOwner->SetState(MOVING_PLATFORM_STATE_TRAJ_TO_B, -1);
		uVar3 = 1;
	}
	else {
		if (msg == 0xf) {
			this->pOwner->SetState(MOVING_PLATFORM_STATE_TRAJ_TO_A, -1);
			uVar3 = 1;
		}
		else {
			if (msg == 0x2b) {
				if (this->field_0x28 != (int)pMsgParam) {
					if (this->field_0x18 != -1) {
						if (pMsgParam == 0) {
							pTexture = CScene::ptable.g_C3DFileManager_00451664->GetActorsCommonMaterial(this->field_0x18);
						}
						else {
							pTexture = CScene::ptable.g_C3DFileManager_00451664->GetActorsCommonMaterial(this->pOwner->pCinData->textureIndex);
						}

						if (pTexture != (ed_g2d_manager*)0x0) {
							ed3DHierarchyBankMatLinkG2D(&this->pOwner->p3DHierNode->base, pTexture);
						}
					}

					this->field_0x28 = (int)pMsgParam;
				}

				pCol = this->pOwner->pCollisionData;
				if (((pCol != (CCollision*)0x0) && (pSVar1 = pCol->pTiedActorEntry, pSVar1 != (S_TIED_ACTOR_ENTRY*)0x0)) &&
					(pSVar1->pActor->pCollisionData != (CCollision*)0x0)) {
					fVar4 = pCol->GetCarriedWeight();

					uVar3 = (ulong)(this->field_0x8 <= fVar4);
					if (uVar3 != 0) {
						bVar2 = CCollision::IsVertexAboveAndAgainstObbTree
						(&pSVar1->pActor->pCollisionData->highestVertex, pCol->pObbTree);
						uVar3 = (ulong)(bVar2 != false);
					}
				}
			}
		}
	}

	return uVar3;
}

void CBehaviourSelector::SaveContext(S_SAVE_CLASS_MOVING_PLATFORM* pData)
{
	if (this->pOwner->actorState == MOVING_PLATFORM_STATE_TRAJ_TO_A) {
		pData->flags = pData->flags | 8;
	}

	return;
}

void CBehaviourSelector::LoadContext(S_SAVE_CLASS_MOVING_PLATFORM* pData)
{
	edF32VECTOR4 position;
	CActorMovingPlatform* pMovingPlatform;

	if ((pData->flags & 8) == 0) {
		this->pOwner->SetState(MOVING_PLATFORM_STATE_TRAJ_TO_B, -1);
		this->field_0x24 = 0.0f;
	}
	else {
		this->pOwner->SetState(MOVING_PLATFORM_STATE_TRAJ_TO_A, -1);
		this->field_0x24 = this->field_0x10;
	}

	pMovingPlatform = this->pOwner;
	edF32Vector4ScaleHard(-this->field_0x24, &position, &gF32Vector4UnitY);
	edF32Vector4AddHard(&position, &position, &pMovingPlatform->baseLocation);
	pMovingPlatform->Platform_UpdatePosition(&position, 1, (CActorsTable*)0x0);

	return;
}

void CBehaviourSelectorMaster::Create(ByteCode* pByteCode)
{
	int iVar1;
	ParticleInfo* pPVar2;
	CSound* pCVar3;
	float fVar4;

	this->field_0x18 = pByteCode->GetS32();

	iVar1 = pByteCode->GetS32();
	CActor::SV_InstallMaterialId(iVar1);
	pPVar2 = CScene::ptable.g_C3DFileManager_00451664->GetG2DInfo(iVar1);
	this->pParticleInfo = pPVar2;

	this->field_0x8 = pByteCode->GetF32();
	this->field_0xc = pByteCode->GetF32();
	this->field_0x10 = pByteCode->GetF32();

	(this->streamRefSound).index = pByteCode->GetS32();

	int* piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * sizeof(S_ACTOR_STREAM_REF);
	}

	this->pActorStreamRef = (S_ACTOR_STREAM_REF*)piVar1;

	this->switchExList.Create(pByteCode);

	return;
}

void CBehaviourSelectorMaster::Init(CActor* pOwner)
{
	int iVar2;
	S_ACTOR_STREAM_REF* pSVar3;
	int iVar4;
	int iVar5;

	this->pOwner = static_cast<CActorMovingPlatform*>(pOwner);

	this->streamRefSound.Init();

	this->pOwner->SV_InstanciateMaterialBank();

	pSVar3 = this->pActorStreamRef;

	S_STREAM_REF<CActor>* pCur = pSVar3->aEntries;

	for (iVar5 = pSVar3->entryCount; iVar5 != 0; iVar5 = iVar5 + -1) {
		pCur->Init();
		pCur = pCur + 1;
	}

	this->switchExList.Init();

	iVar4 = 0;
	while (true) {
		pSVar3 = this->pActorStreamRef;
		iVar2 = 0;
		if (pSVar3 != (S_ACTOR_STREAM_REF*)0x0) {
			iVar2 = pSVar3->entryCount;
		}

		if (iVar2 <= iVar4) break;

		CActor* pActor = pSVar3->aEntries[iVar4].Get();
		if (pActor != 0) {
			pActor->actorFieldS = pActor->actorFieldS & 0xffffffef;
		}

		iVar4 = iVar4 + 1;
	}
	return;
}

void CBehaviourSelectorMaster::Manage()
{
	this->switchExList.pStreamEventCamera->Manage(this->pOwner);
	this->pOwner->BehaviourSelectorMaster_Manage(this);
}

void CBehaviourSelectorMaster::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorMovingPlatform* pCVar1;
	ed_g2d_manager* pTexture;
	int* piVar2;
	int iVar3;
	int iVar4;
	edF32VECTOR4 eStack16;

	if ((this->field_0x18 != -1) && (pTexture = CScene::ptable.g_C3DFileManager_00451664->GetActorsCommonMaterial(this->pOwner->pCinData->textureIndex),
			pTexture != (ed_g2d_manager*)0x0)) {
		ed3DHierarchyBankMatLinkG2D(&this->pOwner->p3DHierNode->base, pTexture);
	}

	this->field_0x28 = 1;
	this->field_0x20 = 0.0f;

	this->switchExList.Reset(pOwner);

	for (iVar4 = this->pActorStreamRef->entryCount; iVar4 != 0; iVar4 = iVar4 + -1) {
		this->pActorStreamRef->aEntries[iVar4].Reset();
	}

	this->actorEntryIndex = -1;

	if (newState == -1) {
		pCVar1 = this->pOwner;
		pCVar1->SetState(MOVING_PLATFORM_STATE_TRAJ_TO_A, -1);
	}
	else {
		pCVar1 = this->pOwner;
		pCVar1->SetState(newState, newAnimationType);
	}

	this->field_0x24 = this->field_0x10;
	pCVar1 = this->pOwner;
	edF32Vector4ScaleHard(-this->field_0x24, &eStack16, &g_xVector);
	edF32Vector4AddHard(&eStack16, &eStack16, &pCVar1->baseLocation);
	pCVar1->Platform_UpdatePosition(&eStack16, 1, (CActorsTable*)0x0);
	return;
}

void CBehaviourSelectorMaster::SaveContext(S_SAVE_CLASS_MOVING_PLATFORM* pData)
{
	S_STREAM_NTF_TARGET_SWITCH_EX_LIST* pSVar1;
	int iVar3;
	uint uVar4;

	if (this->pOwner->actorState == MOVING_PLATFORM_STATE_TRAJ_TO_A) {
		pData->flags = pData->flags | 8;
	}

	uVar4 = 1;
	pData->field_0x1c = (float)this->actorEntryIndex;

	this->switchExList.SaveContext(&pData->switchCamera);

	return;
}

void CBehaviourSelectorMaster::LoadContext(S_SAVE_CLASS_MOVING_PLATFORM* pData)
{
	CActorMovingPlatform* pMovingPlatform;
	int entryCount;
	edF32VECTOR4 position;
	undefined4 local_4;

	if ((pData->flags & 8) == 0) {
		pMovingPlatform = this->pOwner;
		pMovingPlatform->SetState(MOVING_PLATFORM_STATE_TRAJ_TO_B, -1);
		this->field_0x24 = 0.0f;
	}
	else {
		pMovingPlatform = this->pOwner;
		pMovingPlatform->SetState(MOVING_PLATFORM_STATE_TRAJ_TO_A, -1);
		this->field_0x24 = this->field_0x10;
	}

	pMovingPlatform = this->pOwner;
	edF32Vector4ScaleHard(-this->field_0x24, &position, &gF32Vector4UnitY);
	edF32Vector4AddHard(&position, &position, &pMovingPlatform->baseLocation);
	pMovingPlatform->Platform_UpdatePosition(&position, 1, (CActorsTable*)0x0);
	this->actorEntryIndex = (int)pData->field_0x1c;

	this->switchExList.LoadContext(&pData->switchCamera);

	if (-3 < this->actorEntryIndex) {
		if (this->pActorStreamRef == (S_ACTOR_STREAM_REF*)0x0) {
			entryCount = 0;
		}
		else {
			entryCount = this->pActorStreamRef->entryCount;
		}

		if (this->actorEntryIndex < entryCount) goto LAB_00154070;
	}

	this->actorEntryIndex = -1;

LAB_00154070:
	if (-1 < this->actorEntryIndex) {
		local_4 = 0;
		this->pOwner->DoMessage(this->pActorStreamRef->aEntries[this->actorEntryIndex].Get(), (ACTOR_MESSAGE)0xf, 0);
	}

	return;
}

void CBehaviourSelectorNew::Create(ByteCode* pByteCode)
{
	int iVar1;
	ParticleInfo* pPVar2;
	CSound* pCVar3;
	float fVar4;

	this->field_0x18 = pByteCode->GetS32();

	iVar1 = pByteCode->GetS32();
	CActor::SV_InstallMaterialId(iVar1);
	pPVar2 = CScene::ptable.g_C3DFileManager_00451664->GetG2DInfo(iVar1);
	this->pParticleInfo = pPVar2;

	this->field_0x8 = pByteCode->GetF32();
	this->field_0xc = pByteCode->GetF32();
	this->field_0x10 = pByteCode->GetF32();

	(this->streamRefSound).index = pByteCode->GetS32();
	return;
}

void CBehaviourSelectorNew::Init(CActor* pOwner)
{
	this->pOwner = static_cast<CActorMovingPlatform*>(pOwner);

	this->streamRefSound.Init();

	this->pOwner->SV_InstanciateMaterialBank();

	return;
}

void CBehaviourSelectorNew::Manage()
{
	int actorState;
	Timer* pTVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;
	CActorMovingPlatform* pPlatform;

	pPlatform = this->pOwner;

	actorState = pPlatform->actorState;
	if (actorState == MOVING_PLATFORM_STATE_TRAJ_TO_A) {
		pTVar2 = GetTimer();
		fVar5 = this->field_0x24;
		fVar3 = this->field_0x10;

		fVar4 = fVar5 + this->field_0xc * pTVar2->cutsceneDeltaTime;
		if (fVar4 <= fVar3) {
			fVar3 = fVar4;
		}

		if (fVar3 == fVar5) {
			edF32Vector4ScaleHard(-fVar5, &eStack64, &g_xVector);
			edF32Vector4AddHard(&eStack64, &eStack64, &pPlatform->baseLocation);
			pPlatform->Platform_UpdatePosition(&eStack64, 0, (CActorsTable*)0x0);
		}
		else {
			this->field_0x24 = fVar3;
			edF32Vector4ScaleHard(-fVar3, &eStack48, &g_xVector);
			edF32Vector4AddHard(&eStack48, &eStack48, &pPlatform->baseLocation);
			pPlatform->Platform_UpdatePosition(&eStack48, 1, (CActorsTable*)0x0);
		}
	}
	else {
		if (actorState == MOVING_PLATFORM_STATE_TRAJ_TO_B) {
			pTVar2 = GetTimer();
			fVar4 = this->field_0x24;
			fVar3 = fVar4 - this->field_0xc * pTVar2->cutsceneDeltaTime;
			if (fVar3 < 0.0f) {
				fVar3 = 0.0f;
			}
			if (fVar3 == fVar4) {
				edF32Vector4ScaleHard(-fVar4, &eStack32, &g_xVector);
				edF32Vector4AddHard(&eStack32, &eStack32, &pPlatform->baseLocation);
				pPlatform->Platform_UpdatePosition(&eStack32, 0, (CActorsTable*)0x0);
			}
			else {
				this->field_0x24 = fVar3;
				edF32Vector4ScaleHard(-fVar3, &eStack16, &g_xVector);
				edF32Vector4AddHard(&eStack16, &eStack16, &pPlatform->baseLocation);
				pPlatform->Platform_UpdatePosition(&eStack16, 1, (CActorsTable*)0x0);
			}
		}
	}

	pPlatform->GenericManage(1, 0, -1, -1);

	return;
}

void CBehaviourSelectorNew::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	ed_g2d_manager* pTexture;
	edF32VECTOR4 eStack16;
	CActorMovingPlatform* pPlatform;

	if ((this->field_0x18 != -1) && (pTexture = CScene::ptable.g_C3DFileManager_00451664->GetActorsCommonMaterial(this->pOwner->pCinData->textureIndex),
		pTexture != (ed_g2d_manager*)0x0)) {
		ed3DHierarchyBankMatLinkG2D(&this->pOwner->p3DHierNode->base, pTexture);
	}

	this->field_0x28 = 1;
	this->field_0x20 = 0.0f;

	if (newState == -1) {
		pPlatform = this->pOwner;
		pPlatform->SetState(MOVING_PLATFORM_STATE_TRAJ_TO_B, -1);
	}
	else {
		pPlatform = this->pOwner;
		pPlatform->SetState(newState, newAnimationType);
	}

	this->field_0x24 = 0.0f;
	pPlatform = this->pOwner;

	edF32Vector4ScaleHard(-this->field_0x24, &eStack16, &g_xVector);
	edF32Vector4AddHard(&eStack16, &eStack16, &pPlatform->baseLocation);
	pPlatform->Platform_UpdatePosition(&eStack16, 1, (CActorsTable*)0x0);

	return;
}

void CBehaviourTeleportRandom::Create(ByteCode* pByteCode)
{
	int pathIndex = pByteCode->GetS32();
	CPathFollow* pNewPathFollow = (CPathFollow*)0x0;
	if (pathIndex != -1) {
		pNewPathFollow = CScene::ptable.g_PathManager_004516a0->aPathFollow + pathIndex;
	}
	this->pPathFollow = pNewPathFollow;

	this->activeSplinePoint = 0;

	return;
}

void CBehaviourTeleportRandom::Init()
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourTeleportRandom::Manage()
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourTeleportRandom::ManageFrozen()
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourTeleportRandom::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	IMPLEMENTATION_GUARD();
}

int CBehaviourTeleportRandom::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	IMPLEMENTATION_GUARD();
	return 0;
}

void CBehaviourTeleportRandom::SaveContext(S_SAVE_CLASS_MOVING_PLATFORM* pData)
{
	pData->field_0x14 = this->activeSplinePoint;

	return;
}

void CBehaviourTeleportRandom::LoadContext(S_SAVE_CLASS_MOVING_PLATFORM* pData)
{
	edF32MATRIX4 pathMatrix;

	this->activeSplinePoint = pData->field_0x14;

	if ((this->pPathFollow != (CPathFollow*)0x0) && (this->pPathFollow->splinePointCount <= this->activeSplinePoint))
	{
		this->activeSplinePoint = 0;
	}

	this->pOwner->SetState(MOVING_PLATFORM_STATE_MOVING, -1);

	if (this->pPathFollow != (CPathFollow*)0x0) {
		this->pPathFollow->ComputeMatrix(&pathMatrix, this->activeSplinePoint);
		this->pOwner->Platform_UpdateMatrix(&pathMatrix, 1, (CActorsTable*)0x0);
	}

	return;
}