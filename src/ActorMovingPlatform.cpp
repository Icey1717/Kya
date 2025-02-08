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

CActorMovingPlatform::CActorMovingPlatform()
{
	CActor();
	this->field_0x1ec = (CinNamedObject30*)0x0;
	this->field_0x1f0 = (CActor*)0x0;
	this->field_0x1f4 = (CinNamedObject30*)0x0;
	this->field_0x1f8 = (CActor*)0x0;
}

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
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar5 * sizeof(S_BRIDGE_ZONE_STREAM);
	}
	this->pZoneStream = (S_BRIDGE_ZONE_STREAM*)piVar5;

	uVar4 = pByteCode->GetS32();
	this->noFrictionZoneCount = uVar4;
	this->aNoFrictionZones = (S_STREAM_MPF_NO_FRICTION_ZONE*)0x0;
	uVar4 = this->noFrictionZoneCount;
	if (uVar4 != 0) {
		this->aNoFrictionZones = new S_STREAM_MPF_NO_FRICTION_ZONE[uVar4];
		pSVar6 = this->aNoFrictionZones;
		iVar11 = 0;
		if (0 < (int)this->noFrictionZoneCount) {
			do {
				iVar7 = pByteCode->GetS32();
				pSVar6->field_0x0 = iVar7;
				piVar5 = (int*)pByteCode->currentSeekPos;
				pByteCode->currentSeekPos = (char*)(piVar5 + 1);
				if (*piVar5 != 0) {
					pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar5 * 4;
				}
				pSVar6->field_0x4 = (char*)piVar5;
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
	this->field_0x1ec = (CinNamedObject30*)0x0;
	this->field_0x1f0 = (CActor*)0x0;
	this->field_0x1f4 = (CinNamedObject30*)0x0;
	this->field_0x1f8 = (CActor*)0x0;

	
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
	CBehaviour* pCVar10 = CActor::GetBehaviour(7);
	CBehaviourPlatformStand* pStand;
	if ((pCVar10 != (CBehaviour*)0x0) &&
		(pStand = (CBehaviourPlatformStand*)CActor::GetBehaviour(7), pStand->field_0x8 != -1)) {
		this->movingPlatformFlags = this->movingPlatformFlags | 1;
	}

	IMPLEMENTATION_GUARD_AUDIO(
	CBehaviour* pCVar10 = CActor::GetBehaviour(3);
	CBehaviourPlatformSlab* pSlab;
	if ((pCVar10 != (CBehaviour*)0x0) &&
		(pSlab = CActor::GetBehaviour(3), pSlab->field_0x20.Get()-> != (CBehaviourVtable*)0xffffffff)) {
		this->movingPlatformFlags = this->movingPlatformFlags | 2;
	})

	IMPLEMENTATION_GUARD_AUDIO(
	CBehaviour* pCVar10 = CActor::GetBehaviour(8);
	if ((pCVar10 != (CBehaviour*)0x0) || (pCVar10 = CActor::GetBehaviour(9), pCVar10 != (CBehaviour*)0x0)
		) {
		pCVar10 = CActor::GetBehaviour(8);
		if (pCVar10 == (CBehaviour*)0x0) {
			pCVar10 = CActor::GetBehaviour(9);
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
	if (0 < (int)this->noFrictionZoneCount) {
		IMPLEMENTATION_GUARD(
		do {
			S_STREAM_REF<ed_zone_3d>::Init((S_STREAM_REF<ed_zone_3d> *)pNoFric);
			pSVar13 = (S_STREAM_REF<CActor> *)pNoFric->field_0x4;
			for (pCVar16 = pSVar13->pActor; pCVar16 != (CActor*)0x0; pCVar16 = (CActor*)((int)&pCVar16[-1].field_0x15c + 3))
			{
				pSVar13 = pSVar13 + 1;
				S_STREAM_REF<CActor>::Init(pSVar13);
			}
			iVar10 = iVar10 + 1;
			pNoFric = pNoFric + 1;
		} while (iVar10 < (int)this->noFrictionZoneCount);)
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
	CActor* this_00;
	ed_zone_3d* peVar1;
	CEventManager* pCVar2;
	CActor* pCVar3;
	uint uVar4;
	undefined* puVar5;
	int iVar6;
	int iVar7;
	ed_zone_3d* peVar8;
	int iVar9;

	pCVar2 = CScene::ptable.g_EventManager_006f5080;
	iVar9 = 0;
	peVar8 = (ed_zone_3d*)this->aNoFrictionZones;
	if (0 < (int)this->noFrictionZoneCount) {
		IMPLEMENTATION_GUARD(
		do {
			pZone = *(ed_zone_3d**)peVar8;
			iVar7 = 0;
			iVar6 = 0;
			while (true) {
				peVar1 = *(ed_zone_3d**)((int)peVar8 + 4);
				puVar5 = (undefined*)0x0;
				if (peVar1 != (ed_zone_3d*)0x0) {
					puVar5 = peVar1->field_0x0[0];
				}
				if ((int)puVar5 <= iVar7) break;
				this_00 = *(CActor**)((int)peVar1->field_0x0 + iVar6 + 4);
				if ((((this_00 != (CActor*)0x0) && ((this_00->flags & 4) != 0)) &&
					(this_00->pCollisionData != (CCollision*)0x0)) &&
					(pCVar3 = CActor::GetCollidingActor(this_00), pCVar3 == (CActor*)0x0)) {
					uVar4 = edEventComputeZoneAgainstVertex(pCVar2->activeChunkId, pZone, &this_00->currentLocation, 0);
					if (((uVar4 & 1) == 0) || ((this_00->flags & 0x80000) == 0)) {
						if (((CActorMovingPlatform*)this_00->pTiedActor == this) &&
							(((this_00->flags & 0x20000) != 0 && (param_2 == 0)))) {
							(*this_00->pVTable->TieToActor)(this_00, 0, 0, (edF32MATRIX4*)0xffffffff);
						}
					}
					else {
						CActor::FUN_00101110(this_00, this);
						if ((this_00->pTiedActor == (CActor*)0x0) && (param_2 != 0)) {
							(*this_00->pVTable->TieToActor)(this_00, (int)this, 0, (edF32MATRIX4*)0xffffffff);
						}
					}
				}
				iVar6 = iVar6 + 4;
				iVar7 = iVar7 + 1;
			}
			iVar9 = iVar9 + 1;
			peVar8 = (ed_zone_3d*)((int)peVar8 + 8);
		} while (iVar9 < (int)this->noFrictionZoneCount);)
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
		local_10.y = -(this->pTiltData->pushData).oscValue.field_0x0;
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
		ACTOR_LOG(LogLevel::Verbose, "CActorMovingPlatform::Platform_UpdatePosition Tilt {} {}", this->name, this->pTiltData->oscQuat.field_0x0.ToString());

		edQuatToMatrix4Hard(&this->pTiltData->oscQuat.field_0x0, &eStack80);
		edF32Matrix4FromEulerSoft(&eStack144, &this->pCinData->rotationEuler, "XYZ");
		eStack144.rowT = *pPosition;
		edF32Matrix4MulF32Matrix4Hard(&eStack144, &eStack80, &eStack144);

		ACTOR_LOG(LogLevel::Verbose, "CActorMovingPlatform::Platform_UpdatePosition {} {}", this->name, eStack144.ToString());

		SV_UpdateMatrix_Rel(&eStack144, 1, 1, pActorsTable, &local_10);
	}

	if (this->pZoneStream == (S_BRIDGE_ZONE_STREAM*)0x0) {
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
		bool bVar2;
		CActor* pCVar1 = this->field_0x1f0;
		if ((pCVar1 == (CActor*)0x0) ||
			((this->field_0x1ec == (CinNamedObject30*)0x0 || (bVar2 = true, this->field_0x1ec != pCVar1->pCinData)))) {
			bVar2 = false;
		}

		if (bVar2) {
			if (((pCVar1 != (CActor*)0x0) && (this->field_0x1ec != (CinNamedObject30*)0x0)) &&
				(this->field_0x1ec == pCVar1->pCinData)) {
				pCVar1->currentLocation = this->currentLocation;
			}

			pCVar1 = this->field_0x1f0;
			if (((pCVar1 != (CActor*)0x0) && (this->field_0x1ec != (CinNamedObject30*)0x0)) &&
				(this->field_0x1ec == pCVar1->pCinData)) {
				pCVar1->rotationEuler = this->rotationEuler;
			}
		}

		pCVar1 = this->field_0x1f8;
		if (((pCVar1 == (CActor*)0x0) || (this->field_0x1f4 == (CinNamedObject30*)0x0)) ||
			(bVar2 = true, this->field_0x1f4 != pCVar1->pCinData)) {
			bVar2 = false;
		}

		if (bVar2) {
			if (((pCVar1 != (CActor*)0x0) && (this->field_0x1f4 != (CinNamedObject30*)0x0)) &&
				(this->field_0x1f4 == pCVar1->pCinData)) {
				pCVar1->currentLocation = this->currentLocation;
			}

			pCVar1 = this->field_0x1f8;
			if (((pCVar1 != (CActor*)0x0) && (this->field_0x1f4 != (CinNamedObject30*)0x0)) &&
				(this->field_0x1f4 == pCVar1->pCinData)) {
				pCVar1->rotationEuler = this->rotationEuler;
			}
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
	if (this->pZoneStream != (S_BRIDGE_ZONE_STREAM*)0x0) {
		iVar2 = this->pZoneStream->entryCount;
	}

	iVar5 = 0;
	if (0 < iVar2) {
		iVar4 = 0;
		do {
			S_STREAM_REF<ed_zone_3d>* pEntry = &this->pZoneStream->aEntries[iVar5];
			if (pEntry->Get() != 0) {
				pEntry->Get()->pMatrix = STORE_SECTION(&this->field_0x200);
			}
			iVar5 = iVar5 + 1;
		} while (iVar5 < iVar2);
	}

	pSVar3 = this->aNoFrictionZones;
	iVar2 = 0;
	if (0 < (int)this->noFrictionZoneCount) {
		IMPLEMENTATION_GUARD(
		do {
			iVar2 = iVar2 + 1;
			*(edF32MATRIX4**)(pSVar3->field_0x0 + 0x20) = &this->field_0x200;
			pSVar3 = pSVar3 + 1;
		} while (iVar2 < (int)this->noFrictionZoneCount);)
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

void CActorMovingPlatform::GenericManage(int param_2, int param_3, int param_4, int param_5)
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

			iVar10 = 0;
			if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
				iVar10 = this->pCameraStream->entryCount;
			}

			iVar9 = 0;
			if (0 < iVar10) {
				do {
					S_BRIDGE_CAMERA_STREAM_ENTRY* pEntry = &this->pCameraStream->aEntries[iVar9];
					if (pEntry->field_0x4 == 6) {
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

					iVar9 = iVar9 + 1;
				} while (iVar9 < iVar10);
			}
		}
	}
	else {
		if ((this->movingPlatformFlags & 0x20) == 0) {
			this->movingPlatformFlags = this->movingPlatformFlags | 0x20;

			if ((this->pProperties->flags_0x24 & 0x80000) != 0) {
				SetBehaviour(6, -1, -1);

				iVar10 = 0;
				if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
					iVar10 = this->pCameraStream->entryCount;
				}

				iVar9 = 0;
				if (0 < iVar10) {
					iVar7 = 0;
					IMPLEMENTATION_GUARD(
					do {
						iVar3 = (int)&this->pCameraStream->entryCount + iVar7;
						if (*(int*)(iVar3 + 4) == 7) {
							S_STREAM_NTF_TARGET_SWITCH::Switch((S_STREAM_NTF_TARGET_SWITCH*)(iVar3 + 0xc), this);
							S_STREAM_NTF_TARGET_SWITCH::PostSwitch((S_STREAM_NTF_TARGET_SWITCH*)(iVar3 + 0xc), this);
							S_STREAM_EVENT_CAMERA::SwitchOn((S_STREAM_EVENT_CAMERA*)(iVar3 + 0x28), this);
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
							}
						}
						iVar9 = iVar9 + 1;
						iVar7 = iVar7 + 0x48;
					} while (iVar9 < iVar10);)
				}
			}

			iVar10 = 0;
			if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
				iVar10 = this->pCameraStream->entryCount;
			}

			iVar9 = 0;
			if (0 < iVar10) {
				do {
					S_BRIDGE_CAMERA_STREAM_ENTRY* pEntry = &this->pCameraStream->aEntries[iVar9];
					if (pEntry->field_0x4 == 5) {
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
					iVar9 = iVar9 + 1;
					iVar7 = iVar7 + 0x48;
				} while (iVar9 < iVar10);
			}
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

		IMPLEMENTATION_GUARD(
		pCameraView = pCameraManager->GetScenaricCamera(this->pProperties->field_0x28);

		(*((pCameraView->objBase).pVTable)->field_0x34)(pCameraView, this);
		if (bVar2) {
			if (((this->movingPlatformFlags & 0x80) == 0) &&
				(bVar2 = CCameraManager::PushCamera(pCameraManager, pCameraView, 0), bVar2 != false)) {
				this->movingPlatformFlags = this->movingPlatformFlags | 0x80;
			}
		}
		else {
			if ((this->movingPlatformFlags & 0x80) != 0) {
				CCameraManager::PopCamera(pCameraManager, pCameraView);
				this->movingPlatformFlags = this->movingPlatformFlags & 0xffffff7f;
			}
		})
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
						if (piVar6[1] <= param_4) {
							if (((param_4 <= piVar6[2]) && (piVar6[1] <= param_5)) && (param_5 <= piVar6[2])) {
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
						if (piVar6[1] <= param_4) {
							if (((param_4 <= piVar6[2]) && (piVar6[1] <= param_5)) && (param_5 <= piVar6[2])) {
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
					if (piVar6[1] <= param_4) {
						if (((param_4 <= piVar6[2]) && (piVar6[1] <= param_5)) && (param_5 <= piVar6[2])) {
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
			}

			uVar4 = *(uint*)&this->pProperties->field_0x20;
			if (uVar4 != 0xffffffff) {
				if (((this->field_0x1f8 == (CActor*)0x0) || (this->field_0x1f4 == (CinNamedObject30*)0x0)) ||
					(this->field_0x1f4 != this->field_0x1f8->pCinData)) {
					bVar2 = false;
				}
				else {
					bVar2 = true;
				}
				if (!bVar2) {
					CParticlesManager::GetDynamicFx
					(CScene::ptable.g_EffectsManager_004516b8, &this->field_0x1f4, uVar4, 0xffffffffffffffff);
				}
			})

			pCVar1 = this->field_0x1f0;
			if (((pCVar1 == (CActor*)0x0) || (this->field_0x1ec == (CinNamedObject30*)0x0)) ||
				(bVar2 = true, this->field_0x1ec != pCVar1->pCinData)) {
				bVar2 = false;
			}

			if (((bVar2) && (pCVar1 != (CActor*)0x0)) &&
				((this->field_0x1ec != (CinNamedObject30*)0x0 && (this->field_0x1ec == pCVar1->pCinData)))) {
				pCVar1->Manage();
			}

			pCVar1 = this->field_0x1f8;
			if (((pCVar1 == (CActor*)0x0) || (this->field_0x1f4 == (CinNamedObject30*)0x0)) ||
				(bVar2 = true, this->field_0x1f4 != pCVar1->pCinData)) {
				bVar2 = false;
			}

			if (bVar2) {
				if (((pCVar1 != (CActor*)0x0) && (this->field_0x1f4 != (CinNamedObject30*)0x0)) &&
					(this->field_0x1f4 == pCVar1->pCinData)) {
					pCVar1->currentLocation = this->currentLocation;
				}
				pCVar1 = this->field_0x1f8;
				if (((pCVar1 != (CActor*)0x0) && (this->field_0x1f4 != (CinNamedObject30*)0x0)) &&
					(this->field_0x1f4 == pCVar1->pCinData)) {
					pCVar1->rotationEuler = this->rotationEuler;
				}
				pCVar1 = this->field_0x1f8;
				if (((pCVar1 != (CActor*)0x0) && (this->field_0x1f4 != (CinNamedObject30*)0x0)) &&
					(this->field_0x1f4 == pCVar1->pCinData)) {
					pCVar1->InitDlistPatchable(0);
				}
			}

			iVar10 = 0;
			if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
				iVar10 = this->pCameraStream->entryCount;
			}

			iVar9 = 0;
			if (0 < iVar10) {
				do {
					S_BRIDGE_CAMERA_STREAM_ENTRY* pEntry = &this->pCameraStream->aEntries[iVar9];
					if (pEntry->field_0x4 == 10) {
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
					iVar9 = iVar9 + 1;
				} while (iVar9 < iVar10);
			}

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
					if (piVar6[1] <= param_4) {
						if (((param_4 <= piVar6[2]) && (piVar6[1] <= param_5)) && (param_5 <= piVar6[2])) {
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
				if (((this->field_0x1f0 == (CActor*)0x0) || (this->field_0x1ec == (CinNamedObject30*)0x0)) ||
					(this->field_0x1ec != this->field_0x1f0->pCinData)) {
					bVar2 = false;
				}
				else {
					bVar2 = true;
				}

				if (!bVar2) {
					IMPLEMENTATION_GUARD_LOG(
					CParticlesManager::GetDynamicFx
					(CScene::ptable.g_EffectsManager_004516b8, &this->field_0x1ec, uVar4, 0xffffffffffffffff);)
				}
			}

			pCVar1 = this->field_0x1f8;
			if (((pCVar1 == (CActor*)0x0) || (this->field_0x1f4 == (CinNamedObject30*)0x0)) ||
				(bVar2 = true, this->field_0x1f4 != pCVar1->pCinData)) {
				bVar2 = false;
			}

			if ((((bVar2) && (pCVar1 != (CActor*)0x0)) && (this->field_0x1f4 != (CinNamedObject30*)0x0)) &&
				(this->field_0x1f4 == pCVar1->pCinData)) {
				pCVar1->Manage();
			}

			pCVar1 = this->field_0x1f0;
			if (((pCVar1 == (CActor*)0x0) || (this->field_0x1ec == (CinNamedObject30*)0x0)) ||
				(bVar2 = true, this->field_0x1ec != pCVar1->pCinData)) {
				bVar2 = false;
			}

			if (bVar2) {
				if (((pCVar1 != (CActor*)0x0) && (this->field_0x1ec != (CinNamedObject30*)0x0)) &&
					(this->field_0x1ec == pCVar1->pCinData)) {
					pCVar1->currentLocation = this->currentLocation;
				}

				pCVar1 = this->field_0x1f0;
				if (((pCVar1 != (CActor*)0x0) && (this->field_0x1ec != (CinNamedObject30*)0x0)) &&
					(this->field_0x1ec == pCVar1->pCinData)) {
					pCVar1->rotationEuler = this->rotationEuler;
				}

				pCVar1 = this->field_0x1f0;
				if (((pCVar1 != (CActor*)0x0) && (this->field_0x1ec != (CinNamedObject30*)0x0)) &&
					(this->field_0x1ec == pCVar1->pCinData)) {
					pCVar1->InitDlistPatchable(0);
				}
			}

			iVar10 = 0;
			if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
				iVar10 = this->pCameraStream->entryCount;
			}

			iVar9 = 0;
			if (0 < iVar10) {
				do {
					S_BRIDGE_CAMERA_STREAM_ENTRY* pEntry = &this->pCameraStream->aEntries[iVar9];
					if (pEntry->field_0x4 == 9) {
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
					iVar9 = iVar9 + 1;
				} while (iVar9 < iVar10);
			}

			this->movingPlatformFlags = this->movingPlatformFlags | 0x10;
		}
	}

	return;
}

void CActorMovingPlatform::Platform_UpdateMatrixOnTrajectory(CPathFollowReaderAbsolute* pPathFollowerAbs, int param_3, int param_4, S_TRAJ_POS* pTrajPos, CActorsTable* pActorsTable, edF32VECTOR4* param_7)
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

	iVar8 = 1;

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
		local_30.y = -(this->pTiltData->pushData).oscValue.field_0x0;
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
			iVar8 = SV_UpdateMatrixOnTrajectory_Rel(pTrajPos->field_0x0, pPathFollowerAbs, this->pProperties->flags_0x24 & 2, 1, pActorsTable, (edF32MATRIX4*)0x0, &local_30, &pathReaderPosInfo);
		}
	}
	else {
		edQuatToMatrix4Hard(&this->pTiltData->oscQuat.field_0x0, &eStack112);
		iVar8 = SV_UpdateMatrixOnTrajectory_Rel(pTrajPos->field_0x0, pPathFollowerAbs, this->pProperties->flags_0x24 & 2, 1, pActorsTable, &eStack112, &local_30, &pathReaderPosInfo);
	}

	sVar6 = -1;
	sVar7 = sVar6;

	if (param_4 != 0) {
		sVar7 = pathReaderPosInfo.field_0x4;

		if (0.0f < pathReaderPosInfo.field_0x8) {
			sVar7 = pathReaderPosInfo.field_0x0;

			if (iVar8 == 2) {
				if ((pathReaderPosInfo.field_0x8 <= 0.999f) && (sVar7 = pathReaderPosInfo.field_0x4, 0.001f <= pathReaderPosInfo.field_0x8)) {
					sVar7 = sVar6;
				}
			}
			else {
				if ((pPathFollowerAbs->pActor3C_0x0 == (CPathFollow*)0x0) || (pPathFollowerAbs->pActor3C_0x0->splinePointCount != 2)) {
					sVar7 = pathReaderPosInfo.field_0x4;

					if ((pTrajPos->field_0x6 != pathReaderPosInfo.field_0x4) &&
						(sVar7 = sVar6, pTrajPos->field_0x4 == pathReaderPosInfo.field_0x0)) {
						sVar7 = pathReaderPosInfo.field_0x0;
					}
				}
				else {
					if ((pathReaderPosInfo.field_0x8 <= 0.9f) && (sVar7 = pathReaderPosInfo.field_0x4, 0.1f <= pathReaderPosInfo.field_0x8)) {
						sVar7 = sVar6;
					}
				}
			}
		}

		param_4 = 0;

		if ((sVar7 != -1) && (pTrajPos->field_0x8 != sVar7)) {
			param_4 = 1;
		}
	}

	pTrajPos->field_0x4 = pathReaderPosInfo.field_0x4;
	pTrajPos->field_0x6 = pathReaderPosInfo.field_0x0;
	pTrajPos->field_0x8 = sVar7;

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

	if (this->pZoneStream == (S_BRIDGE_ZONE_STREAM*)0x0) {
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
		pCVar3 = this->field_0x1f0;
		if ((pCVar3 == (CActor*)0x0) ||
			((this->field_0x1ec == (CinNamedObject30*)0x0 || (bVar4 = true, this->field_0x1ec != pCVar3->pCinData)))) {
			bVar4 = false;
		}
		if (bVar4) {
			if (((pCVar3 != (CActor*)0x0) && (this->field_0x1ec != (CinNamedObject30*)0x0)) &&
				(this->field_0x1ec == pCVar3->pCinData)) {
				pCVar3->currentLocation = this->currentLocation;
			}

			pCVar3 = this->field_0x1f0;
			if (((pCVar3 != (CActor*)0x0) && (this->field_0x1ec != (CinNamedObject30*)0x0)) &&
				(this->field_0x1ec == pCVar3->pCinData)) {
				pCVar3->rotationEuler = this->rotationEuler;
			}
		}

		pCVar3 = this->field_0x1f8;
		if (((pCVar3 == (CActor*)0x0) || (this->field_0x1f4 == (CinNamedObject30*)0x0)) ||
			(bVar4 = true, this->field_0x1f4 != pCVar3->pCinData)) {
			bVar4 = false;
		}
		if (bVar4) {
			if (((pCVar3 != (CActor*)0x0) && (this->field_0x1f4 != (CinNamedObject30*)0x0)) &&
				(this->field_0x1f4 == pCVar3->pCinData)) {
				pCVar3->currentLocation = this->currentLocation;
			}

			pCVar3 = this->field_0x1f8;
			if (((pCVar3 != (CActor*)0x0) && (this->field_0x1f4 != (CinNamedObject30*)0x0)) &&
				(this->field_0x1f4 == pCVar3->pCinData)) {
				pCVar3->rotationEuler = this->rotationEuler;
			}
		}
	}

	ManageNoFrictionZones(0);
	ComputeRealMoving(&local_20);
	return;
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

	if ((pBehaviour->pathFollowReaderAbs).pActor3C_0x0 != (CPathFollow*)0x0) {
		bBarDepleted = false;
		bVar3 = false;
		bVar4 = true;

		if ((param_4 != false) && (currentFillAmount < 0.0f)) {
			currentFillAmount = 0.0f;
			bBarDepleted = true;
		}

		uVar6 = (long)(int)this->pProperties->flags_0x24 & 0x20;

		if ((uVar6 != 0) && (fVar11 = fmodf(currentFillAmount, (pBehaviour->pathFollowReaderAbs).barFullAmount_0x4), (pBehaviour->pathFollowReaderAbs).field_0xc <= fVar11)) {
			fVar12 = fmodf((pBehaviour->currentFillAmount_0x38).field_0x0, (pBehaviour->pathFollowReaderAbs).barFullAmount_0x4);
			fVar11 = (pBehaviour->pathFollowReaderAbs).field_0xc;
			if (fVar12 < fVar11) {
				bBarDepleted = true;
				bVar3 = true;
				currentFillAmount = fVar11;
			}
		}

		(pBehaviour->currentFillAmount_0x38).field_0x0 = currentFillAmount;

		Platform_UpdateMatrixOnTrajectory(&pBehaviour->pathFollowReaderAbs, 1, 1, &pBehaviour->currentFillAmount_0x38, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);

		if (bBarDepleted) {
			/* Reached zero bar filled. Transition back to inactive state. */
			SetState(6, -1);

			iVar10 = 0;
			if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
				iVar10 = this->pCameraStream->entryCount;
			}

			iVar9 = 0;
			if (0 < iVar10) {
				do {
					S_BRIDGE_CAMERA_STREAM_ENTRY* pEntry = &pCameraStream->aEntries[iVar9];
					if (pEntry->field_0x4 == 0) {
						pEntry->streamTarget.Switch(this);
						pEntry->streamTarget.PostSwitch(this);
						pEntry->streamCameraEvent.SwitchOn(this);

						IMPLEMENTATION_GUARD_AUDIO(
						piVar1 = *(int**)(iVar5 + 0x48);
						if (piVar1 != (int*)0x0) {
							uVar2 = *(uint*)&this->pActorSound->field_0x4;
							uVar8 = (uint)((this->movingPlatformFlags & 1) != 0);
							if ((this->movingPlatformFlags & 2) != 0) {
								uVar8 = uVar8 + 1;
							}
							for (; uVar8 < uVar2; uVar8 = uVar8 + 1) {
								iVar5 = CActorSound::IsInstanceAlive(this->pActorSound, uVar8);
								if (iVar5 == 0) {
									CActorSound::SoundStart(this->pActorSound, this, uVar8, piVar1, 1, 0, (float**)0x0);
									break;
								}
							}
						})
					}

					iVar9 = iVar9 + 1;
				} while (iVar9 < iVar10);
			}

			bVar4 = false;
			if (bVar3) {
				iVar10 = 0;
				if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
					iVar10 = this->pCameraStream->entryCount;
				}

				iVar9 = 0;
				if (0 < iVar10) {
					do {
						S_BRIDGE_CAMERA_STREAM_ENTRY* pEntry = &pCameraStream->aEntries[iVar9];
						if (pEntry->field_0x4 == 1) {
							pEntry->streamTarget.Switch(this);
							pEntry->streamTarget.PostSwitch(this);
							pEntry->streamCameraEvent.SwitchOn(this);

							IMPLEMENTATION_GUARD_AUDIO(
							piVar1 = *(int**)(iVar5 + 0x48);
							if (piVar1 != (int*)0x0) {
								uVar2 = *(uint*)&this->pActorSound->field_0x4;
								uVar8 = (uint)((this->movingPlatformFlags & 1) != 0);
								if ((this->movingPlatformFlags & 2) != 0) {
									uVar8 = uVar8 + 1;
								}
								for (; uVar8 < uVar2; uVar8 = uVar8 + 1) {
									iVar5 = CActorSound::IsInstanceAlive(this->pActorSound, uVar8);
									if (iVar5 == 0) {
										CActorSound::SoundStart(this->pActorSound, this, uVar8, piVar1, 1, 0, (float**)0x0);
										break;
									}
								}
							})
						}

						iVar9 = iVar9 + 1;
					} while (iVar9 < iVar10);
				}
			}
		}
		else {
			if (uVar6 == 1) {
				bVar4 = false;
			}
			else {
				if (uVar6 == 2) {
					uVar2 = this->pProperties->flags_0x24;
					bBarDepleted = false;

					if ((uVar2 & 0x200) == 0) {
						if ((uVar2 & 0x30) != 0) {
							(pBehaviour->currentFillAmount_0x38).field_0x0 = pBehaviour->field_0x28;
						}
						SetState(6, -1);
						bBarDepleted = true;
					}
					else {
						/* Transition to open state. */
						(pBehaviour->currentFillAmount_0x38).field_0x0 = (pBehaviour->pathFollowReaderAbs).barFullAmount_0x4;
						SetState(7, -1);
					}

					iVar10 = 0;
					if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
						iVar10 = this->pCameraStream->entryCount;
					}

					iVar9 = 0;
					if (0 < iVar10) {
						do {
							S_BRIDGE_CAMERA_STREAM_ENTRY* pEntry = &pCameraStream->aEntries[iVar9];
							if (pEntry->field_0x4 == 2) {
								pEntry->streamTarget.Switch(this);
								pEntry->streamTarget.PostSwitch(this);
								pEntry->streamCameraEvent.SwitchOn(this);

								IMPLEMENTATION_GUARD_AUDIO(
								piVar1 = *(int**)(iVar5 + 0x48);
								if (piVar1 != (int*)0x0) {
									uVar2 = *(uint*)&this->pActorSound->field_0x4;
									uVar8 = (uint)((this->movingPlatformFlags & 1) != 0);
									if ((this->movingPlatformFlags & 2) != 0) {
										uVar8 = uVar8 + 1;
									}
									for (; uVar8 < uVar2; uVar8 = uVar8 + 1) {
										iVar5 = CActorSound::IsInstanceAlive(this->pActorSound, uVar8);
										if (iVar5 == 0) {
											CActorSound::SoundStart(this->pActorSound, this, uVar8, piVar1, 1, 0, (float**)0x0);
											break;
										}
									}
								})
							}
							iVar9 = iVar9 + 1;
						} while (iVar9 < iVar10);
					}

					bVar4 = false;
					if (bBarDepleted) {
						iVar10 = 0;
						if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
							iVar10 = this->pCameraStream->entryCount;
						}

						iVar9 = 0;
						if (0 < iVar10) {
							do {
								S_BRIDGE_CAMERA_STREAM_ENTRY* pEntry = &pCameraStream->aEntries[iVar9];
								if (pEntry->field_0x4 == 0) {
									pEntry->streamTarget.Switch(this);
									pEntry->streamTarget.PostSwitch(this);
									pEntry->streamCameraEvent.SwitchOn(this);

									IMPLEMENTATION_GUARD_AUDIO(
									piVar1 = *(int**)(iVar5 + 0x48);
									if (piVar1 != (int*)0x0) {
										uVar2 = *(uint*)&this->pActorSound->field_0x4;
										uVar8 = (uint)((this->movingPlatformFlags & 1) != 0);
										if ((this->movingPlatformFlags & 2) != 0) {
											uVar8 = uVar8 + 1;
										}
										for (; uVar8 < uVar2; uVar8 = uVar8 + 1) {
											iVar5 = CActorSound::IsInstanceAlive(this->pActorSound, uVar8);
											if (iVar5 == 0) {
												CActorSound::SoundStart(this->pActorSound, this, uVar8, piVar1, 1, 0, (float**)0x0);
												break;
											}
										}
									})
								}

								iVar9 = iVar9 + 1;
							} while (iVar9 < iVar10);
						}
					}
				}
			}
		}
	}

	return bVar4;
}

void CActorMovingPlatform::BehaviourTrajectory_Switch(CBehaviourPlatformTrajectory* pBehaviour, int msg)
{
	CSound* pSound;
	StateConfig* pSVar1;
	Timer* pTVar2;
	S_BRIDGE_CAMERA_STREAM_ENTRY* pEntry;
	uint uVar4;
	uint uVar6;
	int iVar7;
	int iVar8;

	if (msg == 0xe) {
		iVar8 = this->actorState;
		if (iVar8 == -1) {
			uVar4 = 0;
		}
		else {
			pSVar1 = GetStateCfg(iVar8);
			uVar4 = pSVar1->flags_0x4 & 0x100;
		}

		if (uVar4 == 0) {
			BehaviourTrajectory_Switch(pBehaviour, 0xf);
		}
		else {
			BehaviourTrajectory_Switch(pBehaviour, 0x10);
		}
	}
	else {
		if (msg == 0x10) {
			if ((this->pProperties->flags_0x24 & 0x200) == 0) {
				iVar8 = this->actorState;
				if (iVar8 == -1) {
					uVar4 = 0;
				}
				else {
					pSVar1 = GetStateCfg(iVar8);
					uVar4 = pSVar1->flags_0x4 & 0x100;
				}

				if ((uVar4 != 0) && ((this->pProperties->flags_0x24 & 8) != 0)) {
					SetState(6, -1);
					iVar8 = 0;

					if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
						iVar8 = this->pCameraStream->entryCount;
					}

					iVar7 = 0;
					if (0 < iVar8) {
						do {
							pEntry = this->pCameraStream->aEntries + iVar7;
							if (pEntry->field_0x4 == 0) {
								pEntry->streamTarget.Switch(this);
								pEntry->streamTarget.PostSwitch(this);
								pEntry->streamCameraEvent.SwitchOn(this);

								IMPLEMENTATION_GUARD_AUDIO(
								pSound = *(CSound**)(pEntry + 0x48);
								if (pSound != (CSound*)0x0) {
									uVar4 = *(uint*)&this->pActorSound->field_0x4;
									uVar6 = (uint)((this->movingPlatformFlags & 1) != 0);
									if ((this->movingPlatformFlags & 2) != 0) {
										uVar6 = uVar6 + 1;
									}
									for (; uVar6 < uVar4; uVar6 = uVar6 + 1) {
										pEntry = CActorSound::IsInstanceAlive(this->pActorSound, uVar6);
										if (pEntry == 0) {
											CActorSound::SoundStart
											(this->pActorSound, (CActor*)this, uVar6, pSound, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);
											break;
										}
									}
								})
							}

							iVar7 = iVar7 + 1;
						} while (iVar7 < iVar8);
					}
				}
			}
			else {
				/* Transition to the drain state. */
				SetState(8, -1);
			}
		}
		else {
			if (msg == 0xf) {
				iVar8 = this->actorState;
				if (iVar8 == -1) {
					uVar4 = 0;
				}
				else {
					pSVar1 = GetStateCfg(iVar8);
					uVar4 = pSVar1->flags_0x4 & 0x100;
				}
				if (uVar4 == 0) {
					/* Transition this to the fill state. */
					SetState(5, -1);
					pTVar2 = GetTimer();
					pBehaviour->goalAmount_0x30 = (pBehaviour->field_0x28 + pTVar2->scaledTotalTime) - (pBehaviour->currentFillAmount_0x38).field_0x0;
				}
			}
		}
	}

	return;
}

void CActorMovingPlatform::BehaviourTrajectory_Manage(CBehaviourPlatformTrajectory* pBehaviour)
{
	int* piVar1;
	uint uVar2;
	bool bVar3;
	int iVar4;
	Timer* pTVar5;
	int iVar6;
	uint uVar7;
	int iVar8;
	int iVar9;
	float currentFillAmount;
	int local_4;

	local_4 = 0;
	currentFillAmount = BehaviourTrajectory_ComputeTime(pBehaviour);

	switch (this->actorState) {
	case 5:
		bVar3 = StateTrajectory(currentFillAmount, pBehaviour, false);
		local_4 = (int)bVar3;
		break;
	case 6:
		Platform_UpdateMatrixOnTrajectory(&pBehaviour->pathFollowReaderAbs, 0, 0, &pBehaviour->currentFillAmount_0x38, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
		break;
	case 7:
		Platform_UpdateMatrixOnTrajectory(&pBehaviour->pathFollowReaderAbs, 0, 0, &pBehaviour->currentFillAmount_0x38, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
		break;
	case 8:
		Platform_UpdateMatrixOnTrajectory(&pBehaviour->pathFollowReaderAbs, 0, 0, &pBehaviour->currentFillAmount_0x38, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);

		if (pBehaviour->field_0x2c <= this->timeInAir) {
			pTVar5 = GetTimer();
			SetState(9, -1);
			iVar9 = 0;

			if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
				iVar9 = this->pCameraStream->entryCount;
			}
			iVar8 = 0;
			if (0 < iVar9) {
				do {
					S_BRIDGE_CAMERA_STREAM_ENTRY* pEntry = this->pCameraStream->aEntries + iVar8;
					if (pEntry->field_0x4 == 3) {
						pEntry->streamTarget.Switch(this);
						pEntry->streamTarget.PostSwitch(this);
						pEntry->streamCameraEvent.SwitchOn(this);
						
						IMPLEMENTATION_GUARD_AUDIO(
						piVar1 = *(int**)(pEntry + 0x48);
						if (piVar1 != (int*)0x0) {
							uVar2 = *(uint*)&this->pActorSound->field_0x4;
							uVar7 = (uint)((this->movingPlatformFlags & 1) != 0);
							if ((this->movingPlatformFlags & 2) != 0) {
								uVar7 = uVar7 + 1;
							}
							for (; uVar7 < uVar2; uVar7 = uVar7 + 1) {
								iVar4 = CActorSound::IsInstanceAlive(this->pActorSound, uVar7);
								if (iVar4 == 0) {
									CActorSound::SoundStart(this->pActorSound, this, uVar7, piVar1, 1, 0, (float**)0x0);
									break;
								}
							}
						})
					}

					iVar8 = iVar8 + 1;
				} while (iVar8 < iVar9);
			}

			pBehaviour->goalAmount_0x30 = (pBehaviour->currentFillAmount_0x38).field_0x0 + pBehaviour->field_0x28 + pTVar5->scaledTotalTime;
		}
		break;
	case 9:
		bVar3 = StateTrajectory(currentFillAmount, pBehaviour, true);
		local_4 = (int)bVar3;
		break;
	case 10:
		IMPLEMENTATION_GUARD(
		bVar3 = StateTrajectory(currentFillAmount, this, pBehaviour, false);
		local_4 = (int)bVar3;
		if (currentFillAmount == pBehaviour->field_0x34) {
			SetState(6, -1);
			iVar9 = 0;
			if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
				iVar9 = this->pCameraStream->entryCount;
			}
			iVar8 = 0;
			if (0 < iVar9) {
				iVar6 = 0;
				do {
					iVar4 = (int)&this->pCameraStream->entryCount + iVar6;
					if (*(int*)(iVar4 + 4) == 0) {
						S_STREAM_NTF_TARGET_SWITCH::Switch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
						S_STREAM_NTF_TARGET_SWITCH::PostSwitch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
						S_STREAM_EVENT_CAMERA::SwitchOn((S_STREAM_EVENT_CAMERA*)(iVar4 + 0x28), this);
						piVar1 = *(int**)(iVar4 + 0x48);
						if (piVar1 != (int*)0x0) {
							uVar2 = *(uint*)&this->pActorSound->field_0x4;
							uVar7 = (uint)((this->movingPlatformFlags & 1) != 0);
							if ((this->movingPlatformFlags & 2) != 0) {
								uVar7 = uVar7 + 1;
							}
							for (; uVar7 < uVar2; uVar7 = uVar7 + 1) {
								iVar4 = CActorSound::IsInstanceAlive(this->pActorSound, uVar7);
								if (iVar4 == 0) {
									CActorSound::SoundStart(this->pActorSound, this, uVar7, piVar1, 1, 0, (float**)0x0);
									break;
								}
							}
						}
					}
					iVar8 = iVar8 + 1;
					iVar6 = iVar6 + 0x48;
				} while (iVar8 < iVar9);
			}
			local_4 = 0;
		})
	}

	GenericManage(1, local_4, (int)(pBehaviour->currentFillAmount_0x38).field_0x4, (int)(pBehaviour->currentFillAmount_0x38).field_0x6);
	return;
}

float CActorMovingPlatform::BehaviourTrajectory_ComputeTime(CBehaviourPlatformTrajectory* pBehaviour)
{
	bool bVar1;
	Timer* pTVar2;
	float fVar3;
	float fVar4;
	int state;

	pTVar2 = GetTimer();
	state = this->actorState;
	fVar3 = (pBehaviour->currentFillAmount_0x38).field_0x0;
	if (state == 10) {
		bVar1 = pBehaviour->field_0x34 < fVar3;
		if ((this->pProperties->flags_0x24 & 1) == 0) {
			if (bVar1) {
				pTVar2 = GetTimer();
				fVar4 = (pBehaviour->currentFillAmount_0x38).field_0x0 - pTVar2->cutsceneDeltaTime;
			}
			else {
				pTVar2 = GetTimer();
				fVar4 = (pBehaviour->currentFillAmount_0x38).field_0x0 + pTVar2->cutsceneDeltaTime;
			}
		}
		else {
			if (bVar1) {
				fVar4 = pBehaviour->goalAmount_0x30 - pTVar2->scaledTotalTime;
			}
			else {
				fVar4 = pTVar2->scaledTotalTime - pBehaviour->goalAmount_0x30;
			}
		}
		if (bVar1) {
			fVar3 = pBehaviour->field_0x34;
			if (fVar3 < fVar4) {
				fVar3 = fVar4;
			}
		}
		else {
			fVar3 = pBehaviour->field_0x34;
			if (fVar4 < fVar3) {
				fVar3 = fVar4;
			}
		}
	}
	else {
		if (state == 9) {
			if ((this->pProperties->flags_0x24 & 1) == 0) {
				pTVar2 = GetTimer();
				fVar3 = (pBehaviour->currentFillAmount_0x38).field_0x0 - pTVar2->cutsceneDeltaTime;
			}
			else {
				fVar3 = pBehaviour->goalAmount_0x30 - pTVar2->scaledTotalTime;
			}
		}
		else {
			if (state == 5) {
				if ((this->pProperties->flags_0x24 & 1) == 0) {
					pTVar2 = GetTimer();
					fVar3 = (pBehaviour->currentFillAmount_0x38).field_0x0 + pTVar2->cutsceneDeltaTime;
				}
				else {
					fVar3 = (pBehaviour->field_0x28 + pTVar2->scaledTotalTime) - pBehaviour->goalAmount_0x30;
				}
			}
		}
	}
	return fVar3;
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

	if (iVar6 == 0xe) {
		bVar1 = Slab_MoveAndDetectCarriedObject(pBehaviour, 0);

		if (bVar1 == false) {
			if (pBehaviour->field_0x1c <= pBehaviour->field_0x28) {
				SetState(0xf, -1);
			}
		}
		else {
			SetState(0xc, -1);
		}
	}
	else {
		if (iVar6 == 0xf) {
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
						pSVar3 = pBehaviour->pTargetStream;
						iVar6 = 0;
						if (0 < pSVar3->entryCount) {
							do {
								IMPLEMENTATION_GUARD(
								pSVar3->aEntries[iVar5].SwitchOff(this);)
								pSVar3 = pBehaviour->pTargetStream;
								iVar6 = iVar6 + 1;
							} while (iVar6 < pSVar3->entryCount);
						}
					}

					SetState(0xb, -1);
				}
			}
			else {
				SetState(0xc, -1);
			}

			iVar5 = 1;
		}
		else {
			if (iVar6 == 0xd) {
				bVar1 = Slab_MoveAndDetectCarriedObject(pBehaviour, 0);

				if (bVar1 == false) {
					if ((this->pProperties->flags_0x24 & 0x1000000) != 0) {
						pSVar3 = pBehaviour->pTargetStream;

						iVar6 = 0;
						if (0 < pSVar3->entryCount) {
							do {
								IMPLEMENTATION_GUARD(
								pSVar3->aEntries[iVar4].SwitchOff(this);)
								pSVar3 = pBehaviour->pTargetStream;
								iVar6 = iVar6 + 1;
							} while (iVar6 < pSVar3->entryCount);
						}
					}

					SetState(0xe, -1);
				}
			}
			else {
				if (iVar6 == 0xc) {
					StateSwitchSlabOff2On(pBehaviour);
					iVar5 = 1;
				}
				else {
					if ((iVar6 == 0xb) && (bVar1 = Slab_MoveAndDetectCarriedObject(pBehaviour, 0), bVar1 != false)) {
						SetState(0xc, -1);
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
		SetState(0xe, -1);
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

			pSVar4 = pBehaviour->pTargetStream;
			iVar6 = 0;
			if (0 < pSVar4->entryCount) {
				do {
					pSVar4->aEntries[iVar6].SwitchOn(this);
					pSVar4 = pBehaviour->pTargetStream;
					iVar6 = iVar6 + 1;
				} while (iVar6 < pSVar4->entryCount);
			}

			pBehaviour->streamEventCamera->SwitchOn(this);

			SetState(0xd, -1);

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
	int iVar6;
	int iVar7;
	int iVar8;
	uint uVar9;
	undefined4 uVar10;

	if (msg == 3) {
		SetBehaviour(MOVING_PLATFORM_BEHAVIOUR_DESTROYED, -1, -1);

		iVar6 = 0;
		if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
			iVar6 = this->pCameraStream->entryCount;
		}

		iVar7 = 0;
		if (0 < iVar6) {
			do {
				S_BRIDGE_CAMERA_STREAM_ENTRY* pEntry = this->pCameraStream->aEntries + iVar7;

				if (pEntry->field_0x4 == 7) {
					pEntry->streamTarget.Switch(this);
					pEntry->streamTarget.PostSwitch(this);
					pEntry->streamCameraEvent.SwitchOn(this);

					IMPLEMENTATION_GUARD_AUDIO(
					pCVar3 = *(CSound**)(iVar4 + 0x48);
					if (pCVar3 != (CSound*)0x0) {
						uVar5 = *(uint*)&this->pActorSound->field_0x4;
						uVar9 = (uint)((this->movingPlatformFlags & 1) != 0);
						if ((this->movingPlatformFlags & 2) != 0) {
							uVar9 = uVar9 + 1;
						}
						for (; uVar9 < uVar5; uVar9 = uVar9 + 1) {
							iVar4 = CActorSound::IsInstanceAlive(this->pActorSound, uVar9);
							if (iVar4 == 0) {
								CActorSound::SoundStart
								(this->pActorSound, this, uVar9, pCVar3, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);
								break;
							}
						}
					})
				}

				iVar7 = iVar7 + 1;
			} while (iVar7 < iVar6);
		}

		uVar5 = 1;
	}
	else {
		if (msg == 2) {
			_msg_hit_param* pHitMessageParams = reinterpret_cast<_msg_hit_param*>(pMsgParam);
			switch (pHitMessageParams->projectileType) {
			case 4:
				IMPLEMENTATION_GUARD(
				iVar6 = *(int*)((int)pMsgParam + 4);
				if (iVar6 == 4) {
					uVar10 = 0;
					if ((this->pProperties->flags_0x24 & 0x1000) != 0) {
						(*(this->pVTable)->SetBehaviour)(this, 6, -1, -1);
						iVar6 = 0;
						if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
							iVar6 = this->pCameraStream->entryCount;
						}
						iVar7 = 0;
						if (0 < iVar6) {
							iVar8 = 0;
							do {
								iVar4 = (int)&this->pCameraStream->entryCount + iVar8;
								if (*(int*)(iVar4 + 4) == 7) {
									S_STREAM_NTF_TARGET_SWITCH::Switch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
									S_STREAM_NTF_TARGET_SWITCH::PostSwitch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
									S_STREAM_EVENT_CAMERA::SwitchOn((S_STREAM_EVENT_CAMERA*)(iVar4 + 0x28), this);
									pCVar3 = *(CSound**)(iVar4 + 0x48);
									if (pCVar3 != (CSound*)0x0) {
										uVar5 = *(uint*)&this->pActorSound->field_0x4;
										uVar9 = (uint)((this->movingPlatformFlags & 1) != 0);
										if ((this->movingPlatformFlags & 2) != 0) {
											uVar9 = uVar9 + 1;
										}
										for (; uVar9 < uVar5; uVar9 = uVar9 + 1) {
											iVar4 = CActorSound::IsInstanceAlive(this->pActorSound, uVar9);
											if (iVar4 == 0) {
												CActorSound::SoundStart
												(this->pActorSound, this, uVar9, pCVar3, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);
												break;
											}
										}
									}
								}
								iVar7 = iVar7 + 1;
								iVar8 = iVar8 + 0x48;
							} while (iVar7 < iVar6);
						}
						uVar10 = 1;
					}
					iVar6 = 0;
					if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
						iVar6 = this->pCameraStream->entryCount;
					}
					iVar7 = 0;
					if (0 < iVar6) {
						iVar8 = 0;
						do {
							iVar4 = (int)&this->pCameraStream->entryCount + iVar8;
							if (*(int*)(iVar4 + 4) == 0xd) {
								S_STREAM_NTF_TARGET_SWITCH::Switch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
								S_STREAM_NTF_TARGET_SWITCH::PostSwitch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
								S_STREAM_EVENT_CAMERA::SwitchOn((S_STREAM_EVENT_CAMERA*)(iVar4 + 0x28), this);
								pCVar3 = *(CSound**)(iVar4 + 0x48);
								if (pCVar3 != (CSound*)0x0) {
									uVar5 = *(uint*)&this->pActorSound->field_0x4;
									uVar9 = (uint)((this->movingPlatformFlags & 1) != 0);
									if ((this->movingPlatformFlags & 2) != 0) {
										uVar9 = uVar9 + 1;
									}
									for (; uVar9 < uVar5; uVar9 = uVar9 + 1) {
										iVar4 = CActorSound::IsInstanceAlive(this->pActorSound, uVar9);
										if (iVar4 == 0) {
											CActorSound::SoundStart
											(this->pActorSound, this, uVar9, pCVar3, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);
											break;
										}
									}
								}
							}
							iVar7 = iVar7 + 1;
							iVar8 = iVar8 + 0x48;
						} while (iVar7 < iVar6);
					}
					*(undefined4*)((int)pMsgParam + 0x74) = uVar10;
				}
				else {
					if ((iVar6 == 3) || (iVar6 == 1)) {
						uVar10 = 0;
						if ((this->pProperties->flags_0x24 & 0x1000) != 0) {
							(*(this->pVTable)->SetBehaviour)(this, 6, -1, -1);
							iVar6 = 0;
							if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
								iVar6 = this->pCameraStream->entryCount;
							}
							iVar7 = 0;
							if (0 < iVar6) {
								iVar8 = 0;
								do {
									iVar4 = (int)&this->pCameraStream->entryCount + iVar8;
									if (*(int*)(iVar4 + 4) == 7) {
										S_STREAM_NTF_TARGET_SWITCH::Switch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
										S_STREAM_NTF_TARGET_SWITCH::PostSwitch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
										S_STREAM_EVENT_CAMERA::SwitchOn((S_STREAM_EVENT_CAMERA*)(iVar4 + 0x28), this);
										pCVar3 = *(CSound**)(iVar4 + 0x48);
										if (pCVar3 != (CSound*)0x0) {
											uVar5 = *(uint*)&this->pActorSound->field_0x4;
											uVar9 = (uint)((this->movingPlatformFlags & 1) != 0);
											if ((this->movingPlatformFlags & 2) != 0) {
												uVar9 = uVar9 + 1;
											}
											for (; uVar9 < uVar5; uVar9 = uVar9 + 1) {
												iVar4 = CActorSound::IsInstanceAlive(this->pActorSound, uVar9);
												if (iVar4 == 0) {
													CActorSound::SoundStart
													(this->pActorSound, this, uVar9, pCVar3, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0
													);
													break;
												}
											}
										}
									}
									iVar7 = iVar7 + 1;
									iVar8 = iVar8 + 0x48;
								} while (iVar7 < iVar6);
							}
							uVar10 = 1;
						}
						iVar6 = 0;
						if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
							iVar6 = this->pCameraStream->entryCount;
						}
						iVar7 = 0;
						if (0 < iVar6) {
							iVar8 = 0;
							do {
								iVar4 = (int)&this->pCameraStream->entryCount + iVar8;
								if (*(int*)(iVar4 + 4) == 0xc) {
									S_STREAM_NTF_TARGET_SWITCH::Switch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
									S_STREAM_NTF_TARGET_SWITCH::PostSwitch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
									S_STREAM_EVENT_CAMERA::SwitchOn((S_STREAM_EVENT_CAMERA*)(iVar4 + 0x28), this);
									pCVar3 = *(CSound**)(iVar4 + 0x48);
									if (pCVar3 != (CSound*)0x0) {
										uVar5 = *(uint*)&this->pActorSound->field_0x4;
										uVar9 = (uint)((this->movingPlatformFlags & 1) != 0);
										if ((this->movingPlatformFlags & 2) != 0) {
											uVar9 = uVar9 + 1;
										}
										for (; uVar9 < uVar5; uVar9 = uVar9 + 1) {
											iVar4 = CActorSound::IsInstanceAlive(this->pActorSound, uVar9);
											if (iVar4 == 0) {
												CActorSound::SoundStart
												(this->pActorSound, this, uVar9, pCVar3, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);
												break;
											}
										}
									}
								}
								iVar7 = iVar7 + 1;
								iVar8 = iVar8 + 0x48;
							} while (iVar7 < iVar6);
						}
						*(undefined4*)((int)pMsgParam + 0x74) = uVar10;
					}
					else {
						if (iVar6 == 2) {
							uVar10 = 0;
							if ((this->pProperties->flags_0x24 & 0x1000) != 0) {
								(*(this->pVTable)->SetBehaviour)(this, 6, -1, -1);
								iVar6 = 0;
								if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
									iVar6 = this->pCameraStream->entryCount;
								}
								iVar7 = 0;
								if (0 < iVar6) {
									iVar8 = 0;
									do {
										iVar4 = (int)&this->pCameraStream->entryCount + iVar8;
										if (*(int*)(iVar4 + 4) == 7) {
											S_STREAM_NTF_TARGET_SWITCH::Switch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
											S_STREAM_NTF_TARGET_SWITCH::PostSwitch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
											S_STREAM_EVENT_CAMERA::SwitchOn((S_STREAM_EVENT_CAMERA*)(iVar4 + 0x28), this);
											pCVar3 = *(CSound**)(iVar4 + 0x48);
											if (pCVar3 != (CSound*)0x0) {
												uVar5 = *(uint*)&this->pActorSound->field_0x4;
												uVar9 = (uint)((this->movingPlatformFlags & 1) != 0);
												if ((this->movingPlatformFlags & 2) != 0) {
													uVar9 = uVar9 + 1;
												}
												for (; uVar9 < uVar5; uVar9 = uVar9 + 1) {
													iVar4 = CActorSound::IsInstanceAlive(this->pActorSound, uVar9);
													if (iVar4 == 0) {
														CActorSound::SoundStart
														(this->pActorSound, this, uVar9, pCVar3, 1, 0,
															(SOUND_SPATIALIZATION_PARAM*)0x0);
														break;
													}
												}
											}
										}
										iVar7 = iVar7 + 1;
										iVar8 = iVar8 + 0x48;
									} while (iVar7 < iVar6);
								}
								uVar10 = 1;
							}
							iVar6 = 0;
							if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
								iVar6 = this->pCameraStream->entryCount;
							}
							iVar7 = 0;
							if (0 < iVar6) {
								iVar8 = 0;
								do {
									iVar4 = (int)&this->pCameraStream->entryCount + iVar8;
									if (*(int*)(iVar4 + 4) == 0xb) {
										S_STREAM_NTF_TARGET_SWITCH::Switch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
										S_STREAM_NTF_TARGET_SWITCH::PostSwitch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
										S_STREAM_EVENT_CAMERA::SwitchOn((S_STREAM_EVENT_CAMERA*)(iVar4 + 0x28), this);
										pCVar3 = *(CSound**)(iVar4 + 0x48);
										if (pCVar3 != (CSound*)0x0) {
											uVar5 = *(uint*)&this->pActorSound->field_0x4;
											uVar9 = (uint)((this->movingPlatformFlags & 1) != 0);
											if ((this->movingPlatformFlags & 2) != 0) {
												uVar9 = uVar9 + 1;
											}
											for (; uVar9 < uVar5; uVar9 = uVar9 + 1) {
												iVar4 = CActorSound::IsInstanceAlive(this->pActorSound, uVar9);
												if (iVar4 == 0) {
													CActorSound::SoundStart
													(this->pActorSound, this, uVar9, pCVar3, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0
													);
													break;
												}
											}
										}
									}
									iVar7 = iVar7 + 1;
									iVar8 = iVar8 + 0x48;
								} while (iVar7 < iVar6);
							}
							*(undefined4*)((int)pMsgParam + 0x74) = uVar10;
						}
					}
				})
				break;
			case 5:
				IMPLEMENTATION_GUARD(
				uVar10 = 0;
				if ((this->pProperties->flags_0x24 & 0x400) != 0) {
					(*(this->pVTable)->SetBehaviour)(this, 6, -1, -1);
					iVar6 = 0;
					if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
						iVar6 = this->pCameraStream->entryCount;
					}
					iVar7 = 0;
					if (0 < iVar6) {
						iVar8 = 0;
						do {
							iVar4 = (int)&this->pCameraStream->entryCount + iVar8;
							if (*(int*)(iVar4 + 4) == 7) {
								S_STREAM_NTF_TARGET_SWITCH::Switch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
								S_STREAM_NTF_TARGET_SWITCH::PostSwitch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
								S_STREAM_EVENT_CAMERA::SwitchOn((S_STREAM_EVENT_CAMERA*)(iVar4 + 0x28), this);
								pCVar3 = *(CSound**)(iVar4 + 0x48);
								if (pCVar3 != (CSound*)0x0) {
									uVar5 = *(uint*)&this->pActorSound->field_0x4;
									uVar9 = (uint)((this->movingPlatformFlags & 1) != 0);
									if ((this->movingPlatformFlags & 2) != 0) {
										uVar9 = uVar9 + 1;
									}
									for (; uVar9 < uVar5; uVar9 = uVar9 + 1) {
										iVar4 = CActorSound::IsInstanceAlive(this->pActorSound, uVar9);
										if (iVar4 == 0) {
											CActorSound::SoundStart
											(this->pActorSound, this, uVar9, pCVar3, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);
											break;
										}
									}
								}
							}
							iVar7 = iVar7 + 1;
							iVar8 = iVar8 + 0x48;
						} while (iVar7 < iVar6);
					}
					uVar10 = 1;
				}
				iVar6 = 0;
				if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
					iVar6 = this->pCameraStream->entryCount;
				}
				iVar7 = 0;
				if (0 < iVar6) {
					iVar8 = 0;
					do {
						iVar4 = (int)&this->pCameraStream->entryCount + iVar8;
						if (*(int*)(iVar4 + 4) == 0xe) {
							S_STREAM_NTF_TARGET_SWITCH::Switch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
							S_STREAM_NTF_TARGET_SWITCH::PostSwitch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
							S_STREAM_EVENT_CAMERA::SwitchOn((S_STREAM_EVENT_CAMERA*)(iVar4 + 0x28), this);
							pCVar3 = *(CSound**)(iVar4 + 0x48);
							if (pCVar3 != (CSound*)0x0) {
								uVar5 = *(uint*)&this->pActorSound->field_0x4;
								uVar9 = (uint)((this->movingPlatformFlags & 1) != 0);
								if ((this->movingPlatformFlags & 2) != 0) {
									uVar9 = uVar9 + 1;
								}
								for (; uVar9 < uVar5; uVar9 = uVar9 + 1) {
									iVar4 = CActorSound::IsInstanceAlive(this->pActorSound, uVar9);
									if (iVar4 == 0) {
										CActorSound::SoundStart
										(this->pActorSound, this, uVar9, pCVar3, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);
										break;
									}
								}
							}
						}
						iVar7 = iVar7 + 1;
						iVar8 = iVar8 + 0x48;
					} while (iVar7 < iVar6);
				}
				*(undefined4*)((int)pMsgParam + 0x74) = uVar10;)
				break;
			default:
				IMPLEMENTATION_GUARD(
				*(undefined4*)((int)pMsgParam + 0x74) = 0;)
				break;
			case 8:
				IMPLEMENTATION_GUARD(
				uVar10 = 0;
				if ((this->pProperties->flags_0x24 & 0x20000) != 0) {
					(*(this->pVTable)->SetBehaviour)(this, 6, -1, -1);
					iVar6 = 0;
					if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
						iVar6 = this->pCameraStream->entryCount;
					}
					iVar7 = 0;
					if (0 < iVar6) {
						iVar8 = 0;
						do {
							iVar4 = (int)&this->pCameraStream->entryCount + iVar8;
							if (*(int*)(iVar4 + 4) == 7) {
								S_STREAM_NTF_TARGET_SWITCH::Switch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
								S_STREAM_NTF_TARGET_SWITCH::PostSwitch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
								S_STREAM_EVENT_CAMERA::SwitchOn((S_STREAM_EVENT_CAMERA*)(iVar4 + 0x28), this);
								pCVar3 = *(CSound**)(iVar4 + 0x48);
								if (pCVar3 != (CSound*)0x0) {
									uVar5 = *(uint*)&this->pActorSound->field_0x4;
									uVar9 = (uint)((this->movingPlatformFlags & 1) != 0);
									if ((this->movingPlatformFlags & 2) != 0) {
										uVar9 = uVar9 + 1;
									}
									for (; uVar9 < uVar5; uVar9 = uVar9 + 1) {
										iVar4 = CActorSound::IsInstanceAlive(this->pActorSound, uVar9);
										if (iVar4 == 0) {
											CActorSound::SoundStart
											(this->pActorSound, this, uVar9, pCVar3, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);
											break;
										}
									}
								}
							}
							iVar7 = iVar7 + 1;
							iVar8 = iVar8 + 0x48;
						} while (iVar7 < iVar6);
					}
					uVar10 = 1;
				}
				iVar6 = 0;
				if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
					iVar6 = this->pCameraStream->entryCount;
				}
				iVar7 = 0;
				if (0 < iVar6) {
					iVar8 = 0;
					do {
						iVar4 = (int)&this->pCameraStream->entryCount + iVar8;
						if (*(int*)(iVar4 + 4) == 0x13) {
							S_STREAM_NTF_TARGET_SWITCH::Switch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
							S_STREAM_NTF_TARGET_SWITCH::PostSwitch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
							S_STREAM_EVENT_CAMERA::SwitchOn((S_STREAM_EVENT_CAMERA*)(iVar4 + 0x28), this);
							pCVar3 = *(CSound**)(iVar4 + 0x48);
							if (pCVar3 != (CSound*)0x0) {
								uVar5 = *(uint*)&this->pActorSound->field_0x4;
								uVar9 = (uint)((this->movingPlatformFlags & 1) != 0);
								if ((this->movingPlatformFlags & 2) != 0) {
									uVar9 = uVar9 + 1;
								}
								for (; uVar9 < uVar5; uVar9 = uVar9 + 1) {
									iVar4 = CActorSound::IsInstanceAlive(this->pActorSound, uVar9);
									if (iVar4 == 0) {
										CActorSound::SoundStart
										(this->pActorSound, this, uVar9, pCVar3, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);
										break;
									}
								}
							}
						}
						iVar7 = iVar7 + 1;
						iVar8 = iVar8 + 0x48;
					} while (iVar7 < iVar6);
				}
				*(undefined4*)((int)pMsgParam + 0x74) = uVar10;)
				break;
			case 9:
				if (pSender->typeID == WEAPON) {
					uVar10 = 0;
					if ((this->pProperties->flags_0x24 & MOVING_PLATFORM_FLAG_CAN_BE_DESTROYED) != 0) {
						SetBehaviour(MOVING_PLATFORM_BEHAVIOUR_DESTROYED, -1, -1);
						iVar6 = 0;
						if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
							iVar6 = this->pCameraStream->entryCount;
						}

						iVar7 = 0;
						if (0 < iVar6) {
							do {
								S_BRIDGE_CAMERA_STREAM_ENTRY* pEntry = this->pCameraStream->aEntries + iVar7;

								if (pEntry->field_0x4 == 7) {
									pEntry->streamTarget.Switch(this);
									pEntry->streamTarget.PostSwitch(this);
									pEntry->streamCameraEvent.SwitchOn(this);

									IMPLEMENTATION_GUARD_AUDIO(
									pCVar3 = *(CSound**)(iVar4 + 0x48);
									if (pCVar3 != (CSound*)0x0) {
										uVar5 = *(uint*)&this->pActorSound->field_0x4;
										uVar9 = (uint)((this->movingPlatformFlags & 1) != 0);
										if ((this->movingPlatformFlags & 2) != 0) {
											uVar9 = uVar9 + 1;
										}
										for (; uVar9 < uVar5; uVar9 = uVar9 + 1) {
											iVar4 = CActorSound::IsInstanceAlive(this->pActorSound, uVar9);
											if (iVar4 == 0) {
												CActorSound::SoundStart
												(this->pActorSound, this, uVar9, pCVar3, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);
												break;
											}
										}
									})
								}
								iVar7 = iVar7 + 1;
							} while (iVar7 < iVar6);
						}
						uVar10 = 1;
					}

					iVar6 = 0;
					if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
						iVar6 = this->pCameraStream->entryCount;
					}

					iVar7 = 0;
					if (0 < iVar6) {
						do {
							S_BRIDGE_CAMERA_STREAM_ENTRY* pEntry = this->pCameraStream->aEntries + iVar7;

							if (pEntry->field_0x4 == 0x12) {
								pEntry->streamTarget.Switch(this);
								pEntry->streamTarget.PostSwitch(this);
								pEntry->streamCameraEvent.SwitchOn(this);

								IMPLEMENTATION_GUARD_AUDIO(
								if (pCVar3 != (CSound*)0x0) {
									uVar5 = *(uint*)&this->pActorSound->field_0x4;
									uVar9 = (uint)((this->movingPlatformFlags & 1) != 0);
									if ((this->movingPlatformFlags & 2) != 0) {
										uVar9 = uVar9 + 1;
									}
									for (; uVar9 < uVar5; uVar9 = uVar9 + 1) {
										iVar4 = CActorSound::IsInstanceAlive(this->pActorSound, uVar9);
										if (iVar4 == 0) {
											CActorSound::SoundStart
											(this->pActorSound, this, uVar9, pCVar3, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);
											break;
										}
									}
								})
							}
							iVar7 = iVar7 + 1;
						} while (iVar7 < iVar6);
					}

					pHitMessageParams->field_0x74 = uVar10;
				}
				break;
			case 10:
				uVar10 = 0;
				if ((this->pProperties->flags_0x24 & 0x800) != 0) {
					SetBehaviour(6, -1, -1);
					iVar6 = 0;
					if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
						iVar6 = this->pCameraStream->entryCount;
					}

					iVar7 = 0;
					if (0 < iVar6) {
						do {
							S_BRIDGE_CAMERA_STREAM_ENTRY* pEntry = this->pCameraStream->aEntries + iVar7;

							if (pEntry->field_0x4 == 7) {
								pEntry->streamTarget.Switch(this);
								pEntry->streamTarget.PostSwitch(this);
								pEntry->streamCameraEvent.SwitchOn(this);
							
								IMPLEMENTATION_GUARD_AUDIO(
								pCVar3 = *(CSound**)(iVar4 + 0x48);
								if (pCVar3 != (CSound*)0x0) {
									uVar5 = *(uint*)&this->pActorSound->field_0x4;
									uVar9 = (uint)((this->movingPlatformFlags & 1) != 0);
									if ((this->movingPlatformFlags & 2) != 0) {
										uVar9 = uVar9 + 1;
									}
									for (; uVar9 < uVar5; uVar9 = uVar9 + 1) {
										iVar4 = CActorSound::IsInstanceAlive(this->pActorSound, uVar9);
										if (iVar4 == 0) {
											CActorSound::SoundStart
											(this->pActorSound, this, uVar9, pCVar3, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);
											break;
										}
									}
								})
							}

							iVar7 = iVar7 + 1;
						} while (iVar7 < iVar6);
					}

					uVar10 = 1;
				}

				iVar6 = 0;
				if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
					iVar6 = this->pCameraStream->entryCount;
				}

				iVar7 = 0;
				if (0 < iVar6) {
					do {
						S_BRIDGE_CAMERA_STREAM_ENTRY* pEntry = this->pCameraStream->aEntries + iVar7;

						if (pEntry->field_0x4 == 0xf) {
							pEntry->streamTarget.Switch(this);
							pEntry->streamTarget.PostSwitch(this);
							pEntry->streamCameraEvent.SwitchOn(this);

							IMPLEMENTATION_GUARD_AUDIO(
							pCVar3 = *(CSound**)(iVar4 + 0x48);
							if (pCVar3 != (CSound*)0x0) {
								uVar5 = *(uint*)&this->pActorSound->field_0x4;
								uVar9 = (uint)((this->movingPlatformFlags & 1) != 0);
								if ((this->movingPlatformFlags & 2) != 0) {
									uVar9 = uVar9 + 1;
								}
								for (; uVar9 < uVar5; uVar9 = uVar9 + 1) {
									iVar4 = CActorSound::IsInstanceAlive(this->pActorSound, uVar9);
									if (iVar4 == 0) {
										CActorSound::SoundStart
										(this->pActorSound, this, uVar9, pCVar3, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);
										break;
									}
								}
							})
						}
						iVar7 = iVar7 + 1;
					} while (iVar7 < iVar6);
				}

				pHitMessageParams->field_0x74 = uVar10;
				break;
			case 0xb:
				IMPLEMENTATION_GUARD(
				uVar10 = 0;
				if ((this->pProperties->flags_0x24 & 0x8000) != 0) {
					(*(this->pVTable)->SetBehaviour)(this, 6, -1, -1);
					iVar6 = 0;
					if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
						iVar6 = this->pCameraStream->entryCount;
					}
					iVar7 = 0;
					if (0 < iVar6) {
						iVar8 = 0;
						do {
							iVar4 = (int)&this->pCameraStream->entryCount + iVar8;
							if (*(int*)(iVar4 + 4) == 7) {
								S_STREAM_NTF_TARGET_SWITCH::Switch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
								S_STREAM_NTF_TARGET_SWITCH::PostSwitch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
								S_STREAM_EVENT_CAMERA::SwitchOn((S_STREAM_EVENT_CAMERA*)(iVar4 + 0x28), this);
								pCVar3 = *(CSound**)(iVar4 + 0x48);
								if (pCVar3 != (CSound*)0x0) {
									uVar5 = *(uint*)&this->pActorSound->field_0x4;
									uVar9 = (uint)((this->movingPlatformFlags & 1) != 0);
									if ((this->movingPlatformFlags & 2) != 0) {
										uVar9 = uVar9 + 1;
									}
									for (; uVar9 < uVar5; uVar9 = uVar9 + 1) {
										iVar4 = CActorSound::IsInstanceAlive(this->pActorSound, uVar9);
										if (iVar4 == 0) {
											CActorSound::SoundStart
											(this->pActorSound, this, uVar9, pCVar3, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);
											break;
										}
									}
								}
							}
							iVar7 = iVar7 + 1;
							iVar8 = iVar8 + 0x48;
						} while (iVar7 < iVar6);
					}
					uVar10 = 1;
				}
				iVar6 = 0;
				if (this->pCameraStream != (S_BRIDGE_CAMERA_STREAM*)0x0) {
					iVar6 = this->pCameraStream->entryCount;
				}
				iVar7 = 0;
				if (0 < iVar6) {
					iVar8 = 0;
					do {
						iVar4 = (int)&this->pCameraStream->entryCount + iVar8;
						if (*(int*)(iVar4 + 4) == 0x11) {
							S_STREAM_NTF_TARGET_SWITCH::Switch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
							S_STREAM_NTF_TARGET_SWITCH::PostSwitch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), this);
							S_STREAM_EVENT_CAMERA::SwitchOn((S_STREAM_EVENT_CAMERA*)(iVar4 + 0x28), this);
							pCVar3 = *(CSound**)(iVar4 + 0x48);
							if (pCVar3 != (CSound*)0x0) {
								uVar5 = *(uint*)&this->pActorSound->field_0x4;
								uVar9 = (uint)((this->movingPlatformFlags & 1) != 0);
								if ((this->movingPlatformFlags & 2) != 0) {
									uVar9 = uVar9 + 1;
								}
								for (; uVar9 < uVar5; uVar9 = uVar9 + 1) {
									iVar4 = CActorSound::IsInstanceAlive(this->pActorSound, uVar9);
									if (iVar4 == 0) {
										CActorSound::SoundStart
										(this->pActorSound, this, uVar9, pCVar3, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);
										break;
									}
								}
							}
						}
						iVar7 = iVar7 + 1;
						iVar8 = iVar8 + 0x48;
					} while (iVar7 < iVar6);
				}
				*(undefined4*)((int)pMsgParam + 0x74) = uVar10;)
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
		pCVar2 = this->field_0x1f0;
		if (((pCVar2 == (CActor*)0x0) || (this->field_0x1ec == (CinNamedObject30*)0x0)) ||
			(bVar4 = true, this->field_0x1ec != pCVar2->pCinData)) {
			bVar4 = false;
		}

		if (bVar4) {
			if (((pCVar2 != (CActor*)0x0) && (pCVar3 = this->field_0x1ec, pCVar3 != (CinNamedObject30*)0x0)) &&
				(pCVar3 == pCVar2->pCinData)) {
				IMPLEMENTATION_GUARD(
				(*pCVar2->pVTable->IsKindOfObject)(pCVar2, (int)pCVar3);)
			}
			this->field_0x1f0 = (CActor*)0x0;
			this->field_0x1ec = (CinNamedObject30*)0x0;
		}

		this->field_0x1ec = (CinNamedObject30*)0x0;
		this->field_0x1f0 = (CActor*)0x0;

		pCVar2 = this->field_0x1f8;
		if (((pCVar2 == (CActor*)0x0) || (this->field_0x1f4 == (CinNamedObject30*)0x0)) ||
			(bVar4 = true, this->field_0x1f4 != pCVar2->pCinData)) {
			bVar4 = false;
		}

		if (bVar4) {
			if (((pCVar2 != (CActor*)0x0) && (pCVar3 = this->field_0x1f4, pCVar3 != (CinNamedObject30*)0x0)) &&
				(pCVar3 == pCVar2->pCinData)) {
				IMPLEMENTATION_GUARD(
				(*pCVar2->pVTable->IsKindOfObject)(pCVar2, (int)pCVar3);)
			}
			this->field_0x1f8 = (CActor*)0x0;
			this->field_0x1f4 = (CinNamedObject30*)0x0;
		}

		this->field_0x1f4 = (CinNamedObject30*)0x0;
		this->field_0x1f8 = (CActor*)0x0;
	}
	else {
		if ((this->movingPlatformFlags & 0x10) != 0) {
			uVar1 = this->pProperties->field_0x1c;
			if (uVar1 != 0xffffffff) {
				if (((this->field_0x1f0 == (CActor*)0x0) || (this->field_0x1ec == (CinNamedObject30*)0x0)) ||
					(this->field_0x1ec != this->field_0x1f0->pCinData)) {
					bVar4 = false;
				}
				else {
					bVar4 = true;
				}
				if (!bVar4) {
					IMPLEMENTATION_GUARD_FX(
					CParticlesManager::GetDynamicFx
					(CScene::ptable.g_EffectsManager_004516b8, &this->field_0x1ec, uVar1, 0xffffffffffffffff);)
				}
			}

			pCVar2 = this->field_0x1f0;
			if (((pCVar2 == (CActor*)0x0) || (this->field_0x1ec == (CinNamedObject30*)0x0)) ||
				(bVar4 = true, this->field_0x1ec != pCVar2->pCinData)) {
				bVar4 = false;
			}
			if (bVar4) {
				if (((pCVar2 != (CActor*)0x0) && (this->field_0x1ec != (CinNamedObject30*)0x0)) &&
					(this->field_0x1ec == pCVar2->pCinData)) {
					pCVar2->currentLocation = this->currentLocation;
				}

				pCVar2 = this->field_0x1f0;
				if (((pCVar2 != (CActor*)0x0) && (this->field_0x1ec != (CinNamedObject30*)0x0)) &&
					(this->field_0x1ec == pCVar2->pCinData)) {
					pCVar2->rotationEuler = this->rotationEuler;
				}

				pCVar2 = this->field_0x1f0;
				if (((pCVar2 != (CActor*)0x0) && (this->field_0x1ec != (CinNamedObject30*)0x0)) &&
					(this->field_0x1ec == pCVar2->pCinData)) {
					pCVar2->InitDlistPatchable(0);
				}
			}
		}
	}

	switch (this->curBehaviourId) {
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7: 
	case 8:
	case 9:
	case 10:
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
		if ((fVar2 == (pBehaviour->currentFillAmount_0x38).field_0x0) || ((pBehaviour->pathFollowReaderAbs).pActor3C_0x0 == (CPathFollow*)0x0)) {
			CActor::FillThisFrameExpectedDifferentialMatrix(pMatrix);
		}
		else {
			if ((this->pProperties->flags_0x24 & 2) == 0) {
				edF32Matrix4FromEulerSoft(&local_50, &(this->pCinData)->rotationEuler, "XYZ");
				local_90 = local_50;
				pBehaviour->pathFollowReaderAbs.ComputePosition(fVar2, &local_50.rowT, (edF32VECTOR4*)0x0, &SStack16);
				pBehaviour->pathFollowReaderAbs.ComputePosition((pBehaviour->currentFillAmount_0x38).field_0x0, &local_90.rowT, (edF32VECTOR4*)0x0, &SStack16);
			}
			else {
				pBehaviour->pathFollowReaderAbs.ComputeMatrix(fVar2, &local_50, (edF32VECTOR4*)0x0, &SStack16);
				pBehaviour->pathFollowReaderAbs.ComputeMatrix((pBehaviour->currentFillAmount_0x38).field_0x0, &local_90, (edF32VECTOR4*)0x0, &SStack16);
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
	uint local_28;
	int local_24;
	undefined4 local_1c;
	undefined4 local_18;
	undefined4 local_14;
	undefined4 local_10;
	uint local_c;
	uint local_8;
	uint* local_4;

	local_4 = &local_28;
	iVar8 = -2;
	uVar7 = 0;
	local_28 = 0;
	local_24 = pBehaviour->field_0x38;
	pSVar4 = pBehaviour->field_0x30;
	iVar5 = 0;
	if (0 < pSVar3->entryCount) {
		iVar4 = 0;
		do {
			iVar1 = DoMessage(LOAD_SECTION_CAST(CActor*, pSVar4->aEntries[iVar5].pRef), (ACTOR_MESSAGE)0x2e, (MSG_PARAM)local_4);
			if (iVar1 != 0) {
				local_24 = local_24 + -1;
				uVar7 = uVar7 | local_28;
			}
			pSVar4 = pBehaviour->field_0x30;
			iVar5 = iVar5 + 1;
			iVar4 = iVar4 + 0x10;
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
			iVar8 = local_24;
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
	iVar5 = pBehaviour->field_0x38;
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

		pBehaviour->field_0x38 = iVar8;
		iVar8 = pBehaviour->field_0x38;
		if (iVar8 != -2) {
			if (iVar8 == -1) {
				local_18 = 0;
				DoMessage(this, (ACTOR_MESSAGE)0xf, 0);
			}
			else {
				local_1c = 0;
				DoMessage(pBehaviour->pActorStreamRef->aEntries[iVar8].Get(), (ACTOR_MESSAGE)0xf, 0);
			}

			iVar8 = pBehaviour->field_0x38;
			pSVar4 = pBehaviour->field_0x30;

			for (iVar5 = 0; iVar5 < pSVar4->entryCount; iVar5++) {
				pSVar4->aEntries[iVar5].Switch(this, iVar8 + 1);
			}

			pBehaviour->pStreamEventCamera->SwitchOn(this);
		}
	}

	iVar8 = this->actorState;
	if (iVar8 == 0xd) {
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
		if (iVar8 == 0xb) {
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
		local_10.y = -(this->pTiltData->pushData).oscValue.field_0x0;
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
		edQuatToMatrix4Hard(&this->pTiltData->oscQuat.field_0x0, &local_50);
		edF32Matrix4MulF32Matrix4Hard(&local_50, &local_50, pMatrix);
		SV_UpdateMatrix_Rel(&local_50, 1, 1, pActorTable, &local_10);
	}

	if (this->pZoneStream == (S_BRIDGE_ZONE_STREAM*)0x0) {
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
		pCVar2 = this->field_0x1f0;
		if ((pCVar2 == (CActor*)0x0) ||
			((this->field_0x1ec == (CinNamedObject30*)0x0 || (bVar3 = true, this->field_0x1ec != pCVar2->pCinData)))) {
			bVar3 = false;
		}
		if (bVar3) {
			if (((pCVar2 != (CActor*)0x0) && (this->field_0x1ec != (CinNamedObject30*)0x0)) &&
				(this->field_0x1ec == pCVar2->pCinData)) {
				pCVar2->currentLocation = this->currentLocation;
			}
			pCVar2 = this->field_0x1f0;
			if (((pCVar2 != (CActor*)0x0) && (this->field_0x1ec != (CinNamedObject30*)0x0)) &&
				(this->field_0x1ec == pCVar2->pCinData)) {
				pCVar2->rotationEuler = this->rotationEuler;
			}
		}

		pCVar2 = this->field_0x1f8;
		if (((pCVar2 == (CActor*)0x0) || (this->field_0x1f4 == (CinNamedObject30*)0x0)) ||
			(bVar3 = true, this->field_0x1f4 != pCVar2->pCinData)) {
			bVar3 = false;
		}

		if (bVar3) {
			if (((pCVar2 != (CActor*)0x0) && (this->field_0x1f4 != (CinNamedObject30*)0x0)) &&
				(this->field_0x1f4 == pCVar2->pCinData)) {
				pCVar2->currentLocation = this->currentLocation;
			}
			pCVar2 = this->field_0x1f8;
			if (((pCVar2 != (CActor*)0x0) && (this->field_0x1f4 != (CinNamedObject30*)0x0)) &&
				(this->field_0x1f4 == pCVar2->pCinData)) {
				pCVar2->rotationEuler = this->rotationEuler;
			}
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

	fVar6 = pBehaviour->trajPos.field_0x0;

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

	fVar4 = pBehaviour->trajPos.field_0x0;
	fVar5 = pTVar2->cutsceneDeltaTime / pBehaviour->field_0x44;
	if (fabs(puVar7 - fVar4) < fVar5) {
		pBehaviour->trajPos.field_0x0 = puVar7;
	}
	else {
		if (fVar4 < puVar7) {
			pBehaviour->trajPos.field_0x0 = fVar4 + fVar5;
		}
		else {
			pBehaviour->trajPos.field_0x0 = fVar4 - fVar5;
		}
	}

	local_4 = &local_10;
	local_10 = -pBehaviour->trajPos.field_0x0;
	local_c = 0;
	DoMessage(pRefActor, (ACTOR_MESSAGE)0x11, (MSG_PARAM)local_4);

	fVar4 = pBehaviour->trajPos.field_0x0;

	for (int i = 0; i < pBehaviour->field_0x34->entryCount; i++) {
		pBehaviour->field_0x34->aEntries[i].NotifyAnalog(fVar6, fVar4, this, pBehaviour->pStreamEventCamera);
	}

	fVar4 = pBehaviour->trajPos.field_0x0;
	pBehaviour->trajPos.field_0x0 = pBehaviour->trajPos.field_0x0 + 1.0f;

	const bool bVar4 = fVar4 != fVar6;

	Platform_UpdateMatrixOnTrajectory(&pBehaviour->pathFollowReaderAbs, bVar4, 1, &pBehaviour->trajPos, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);

	pBehaviour->trajPos.field_0x0 = pBehaviour->trajPos.field_0x0 - 1.0f;
	return bVar4;
}

CBehaviourPlatformStand::CBehaviourPlatformStand()
{
	this->pCinData = (CinNamedObject30*)0x0;
	this->pTiedActor = (CActor*)0x0;
}

void CBehaviourPlatformStand::Create(ByteCode* pByteCode)
{
	int iVar1;

	ACTOR_LOG(LogLevel::Info, "CBehaviourPlatformStand::Create");

	iVar1 = pByteCode->GetS32();
	this->field_0x8 = iVar1;
	iVar1 = pByteCode->GetS32();
	this->field_0xc = iVar1;
	this->pCinData = (CinNamedObject30*)0x0;
	this->pTiedActor = (CActor*)0x0;
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
		CActor* pCVar2 = this->pTiedActor;

		if (((pCVar2 == (CActor*)0x0) || (this->pCinData == (CinNamedObject30*)0x0)) ||
			(bVar3 = true, this->pCinData != pCVar2->pCinData)) {
			bVar3 = false;
		}

		if (bVar3) {
			if (((pCVar2 != (CActor*)0x0) && (this->pCinData != (CinNamedObject30*)0x0)) &&
				(this->pCinData == pCVar2->pCinData)) {
				pCVar2->currentLocation = pCVar1->currentLocation;
			}

			pCVar1 = this->pOwner;
			pCVar2 = this->pTiedActor;

			if (((pCVar2 != (CActor*)0x0) && (this->pCinData != (CinNamedObject30*)0x0)) &&
				(this->pCinData == pCVar2->pCinData)) {
				pCVar2->rotationEuler = pCVar1->rotationEuler;
			}
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

void CBehaviourPlatformStand::ChangeManageState(int state)
{
	CActor* pCVar1;
	CActorMovingPlatform* pCVar2;
	CinNamedObject30* pCVar3;
	bool bVar4;

	if (state == 0) {
		pCVar1 = this->pTiedActor;
		if (((pCVar1 == (CActor*)0x0) || (this->pCinData == (CinNamedObject30*)0x0)) ||
			(bVar4 = true, this->pCinData != pCVar1->pCinData)) {
			bVar4 = false;
		}

		if (bVar4) {
			if (((pCVar1 != (CActor*)0x0) && (pCVar3 = this->pCinData, pCVar3 != (CinNamedObject30*)0x0)) &&
				(pCVar3 == pCVar1->pCinData)) {
				IMPLEMENTATION_GUARD(
				(*pCVar1->pVTable->IsKindOfObject)(pCVar1, (int)pCVar3);)
			}

			this->pTiedActor = (CActor*)0x0;
			this->pCinData = (CinNamedObject30*)0x0;
		}

		this->pCinData = (CinNamedObject30*)0x0;
		this->pTiedActor = (CActor*)0x0;
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
			if (((this->pTiedActor == (CActor*)0x0) || (this->pCinData == (CinNamedObject30*)0x0)) ||
				(this->pCinData != this->pTiedActor->pCinData)) {
				bVar4 = false;
			}
			else {
				bVar4 = true;
			}

			if (!bVar4) {
				IMPLEMENTATION_GUARD(
				CParticlesManager::GetDynamicFx
				(CScene::ptable.g_EffectsManager_004516b8, &this->pCinData, this->field_0xc, 0xffffffffffffffff);)
			}
		}

		pCVar1 = this->pTiedActor;
		if (((pCVar1 == (CActor*)0x0) || (this->pCinData == (CinNamedObject30*)0x0)) ||
			(bVar4 = true, this->pCinData != pCVar1->pCinData)) {
			bVar4 = false;
		}

		if (bVar4) {
			pCVar2 = this->pOwner;
			if (((pCVar1 != (CActor*)0x0) && (this->pCinData != (CinNamedObject30*)0x0)) &&
				(this->pCinData == pCVar1->pCinData)) {
				pCVar1->currentLocation = pCVar2->currentLocation;
			}

			pCVar2 = this->pOwner;
			pCVar1 = this->pTiedActor;
			if (((pCVar1 != (CActor*)0x0) && (this->pCinData != (CinNamedObject30*)0x0)) &&
				(this->pCinData == pCVar1->pCinData)) {
				pCVar1->rotationEuler = pCVar2->rotationEuler;
			}

			pCVar1 = this->pTiedActor;
			if (((pCVar1 != (CActor*)0x0) && (this->pCinData != (CinNamedObject30*)0x0)) &&
				(this->pCinData == pCVar1->pCinData)) {
				pCVar1->InitDlistPatchable(0);
			}
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
	float fVar1;

	ACTOR_LOG(LogLevel::Info, "CBehaviourPlatformTrajectory::Create");

	this->pathFollowReaderAbs.Create(pByteCode);
	fVar1 = pByteCode->GetF32();
	this->field_0x28 = fVar1;
	fVar1 = pByteCode->GetF32();
	this->field_0x2c = fVar1;
}

void CBehaviourPlatformTrajectory::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	Timer* pTVar1;
	CActorMovingPlatform* pAVar1;

	pTVar1 = GetTimer();

	this->pOwner = reinterpret_cast<CActorMovingPlatform*>(pOwner);

	this->field_0x34 = 0.0f;
	this->currentFillAmount_0x38.field_0x0 = this->field_0x28;

	this->currentFillAmount_0x38.field_0x4 = 0;
	this->currentFillAmount_0x38.field_0x6 = 0;
	this->currentFillAmount_0x38.field_0x8 = -1;

	if (newState == -1) {
		pAVar1 = this->pOwner;

		if ((pAVar1->pProperties->flags_0x24 & 4) == 0) {
			pAVar1->SetState(5, -1);
			this->goalAmount_0x30 = pTVar1->scaledTotalTime;
		}
		else {
			pAVar1->SetState(6, -1);
			this->goalAmount_0x30 = 0.0f;
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	this->pOwner->Platform_UpdateMatrixOnTrajectory(&this->pathFollowReaderAbs, 1, 0, & this->currentFillAmount_0x38, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
	this->currentFillAmount_0x38.field_0x8 = this->currentFillAmount_0x38.field_0x4;
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
	int iVar2;
	Timer* pTVar3;
	float fVar4;
	float local_18;
	int local_14;
	int local_10;
	float local_c;
	int local_8;
	int local_4;

	if (msg == 0x68) {
		IMPLEMENTATION_GUARD(
		pCVar1 = (this->pathFollowReaderAbs).pActor3C_0x0;
		if (pCVar1 != (CPathFollow*)0x0) {
			CPathFollowReaderAbsolute::ComputeSegment
			((this->currentFillAmount_0x38).field_0x0, &this->pathFollowReaderAbs, &local_10, &local_14, &local_18);
			if (local_18 < 0.5) {
				local_10 = local_14;
			}
			local_10 = local_10 + -1;
			if (local_10 < pCVar1->field_0x14) {
				if ((pCVar1->type == 0) || (pCVar1->mode == 1)) {
					if (local_10 < 0) {
						local_10 = 0;
					}
				}
				else {
					local_10 = pCVar1->splinePointCount + -1;
				}
			}
			pCVar1 = (this->pathFollowReaderAbs).pActor3C_0x0;
			if (((local_10 < 0) || (pCVar1 == (CPathFollow*)0x0)) || (pCVar1->splinePointCount <= local_10)) {
				local_10 = 0;
			}
			CPathFollowReaderAbsolute::GetClosestTimeToReachWaypoint
			((this->currentFillAmount_0x38).field_0x0, &this->pathFollowReaderAbs, local_10, &this->field_0x34,
				(float*)&this->currentFillAmount_0x38);
			(*((this->pOwner->base).base.pVTable)->SetState)((CActor*)this->pOwner, 10, -1);
			pTVar3 = GetTimer();
			fVar4 = (this->currentFillAmount_0x38).field_0x0;
			if (fVar4 <= this->field_0x34) {
				this->goalAmount_0x30 = (this->field_0x28 + pTVar3->scaledTotalTime) - fVar4;
			}
			else {
				this->goalAmount_0x30 = fVar4 + this->field_0x28 + pTVar3->scaledTotalTime;
			}
		}
		iVar2 = 1;)
	}
	else {
		if (msg == 0x67) {
			IMPLEMENTATION_GUARD(
			pCVar1 = (this->pathFollowReaderAbs).pActor3C_0x0;
			if (pCVar1 != (CPathFollow*)0x0) {
				CPathFollowReaderAbsolute::ComputeSegment
				((this->currentFillAmount_0x38).field_0x0, &this->pathFollowReaderAbs, &local_4, &local_8, &local_c);
				if (local_c < 0.5) {
					local_4 = local_8;
				}
				local_4 = local_4 + 1;
				if (pCVar1->splinePointCount <= local_4) {
					if ((pCVar1->type == 0) || (pCVar1->mode == 1)) {
						local_4 = pCVar1->splinePointCount + -1;
					}
					else {
						local_4 = pCVar1->field_0x14;
					}
				}
				pCVar1 = (this->pathFollowReaderAbs).pActor3C_0x0;
				if (((local_4 < 0) || (pCVar1 == (CPathFollow*)0x0)) || (pCVar1->splinePointCount <= local_4)) {
					local_4 = 0;
				}
				CPathFollowReaderAbsolute::GetClosestTimeToReachWaypoint
				((this->currentFillAmount_0x38).field_0x0, &this->pathFollowReaderAbs, local_4, &this->field_0x34,
					(float*)&this->currentFillAmount_0x38);
				(*((this->pOwner->base).base.pVTable)->SetState)((CActor*)this->pOwner, 10, -1);
				pTVar3 = GetTimer();
				fVar4 = (this->currentFillAmount_0x38).field_0x0;
				if (fVar4 <= this->field_0x34) {
					this->goalAmount_0x30 = (this->field_0x28 + pTVar3->scaledTotalTime) - fVar4;
				}
				else {
					this->goalAmount_0x30 = fVar4 + this->field_0x28 + pTVar3->scaledTotalTime;
				}
			}
			iVar2 = 1;)
		}
		else {
			if (msg == 0x3b) {
				IMPLEMENTATION_GUARD(
				pCVar1 = (this->pathFollowReaderAbs).pActor3C_0x0;
				if ((((int)pMsgParam < 0) || (pCVar1 == (CPathFollow*)0x0)) || (pCVar1->splinePointCount <= (int)pMsgParam)) {
					pMsgParam = (void*)0x0;
				}
				CPathFollowReaderAbsolute::GetClosestTimeToReachWaypoint
				((this->currentFillAmount_0x38).field_0x0, &this->pathFollowReaderAbs, (int)pMsgParam, &this->field_0x34,
					(float*)&this->currentFillAmount_0x38);
				(*((this->pOwner->base).base.pVTable)->SetState)((CActor*)this->pOwner, 10, -1);
				pTVar3 = GetTimer();
				fVar4 = (this->currentFillAmount_0x38).field_0x0;
				if (fVar4 <= this->field_0x34) {
					this->goalAmount_0x30 = (this->field_0x28 + pTVar3->scaledTotalTime) - fVar4;
				}
				else {
					this->goalAmount_0x30 = fVar4 + this->field_0x28 + pTVar3->scaledTotalTime;
				}
				iVar2 = 1;)
			}
			else {
				if (msg == 0xd) {
					IMPLEMENTATION_GUARD(
					if (*(int*)((int)pMsgParam + 4) != 0) {
						this->pOwner->BehaviourTrajectory_Switch(this, 0xf);
					}
					iVar2 = 1;)
				}
				else {
					if (((msg == 0xe) || (msg == 0x10)) || (msg == 0xf)) {
						this->pOwner->BehaviourTrajectory_Switch(this, msg);
						iVar2 = 1;
					}
					else {
						iVar2 = 0;
					}
				}
			}
		}
	}
	return iVar2;
}

void CBehaviourPlatformTrajectory::Manage()
{
	this->pOwner->BehaviourTrajectory_Manage(this);
}

void CBehaviourPlatformTrajectory::ManageFrozen()
{
	if ((GameFlags & 0x20) == 0) {
		this->pOwner->Platform_UpdateMatrixOnTrajectory(&this->pathFollowReaderAbs, 0, 0, &this->currentFillAmount_0x38, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
		this->pOwner->GenericManage(1, 0, (this->currentFillAmount_0x38).field_0x4, (this->currentFillAmount_0x38).field_0x6);
	}

	return;
}

float dot_quat(edF32VECTOR4* a, edF32VECTOR4* b)
{
	return a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
}

void CPathFollowReaderAbsolute::Create(float param_1, float param_2, CPathFollow* pPathFollow, int type, int mode, int timing, int param_8)
{
	int count;
	bool bVar1;
	float* pfVar2;
	edF32VECTOR4* peVar3;
	edF32VECTOR4* peVar4;
	int iVar5;
	int iVar6;
	int iVar7;
	int iVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	edF32MATRIX4 eStack80;
	edF32VECTOR4 local_10;

	if (pPathFollow != (CPathFollow*)0x0) {
		this->pActor3C_0x0 = pPathFollow;
		this->barFullAmount_0x4 = param_1;
		this->field_0x8 = param_2;

		iVar7 = 1;
		count = this->pActor3C_0x0->splinePointCount;
		iVar8 = this->pActor3C_0x0->field_0x14;
		bVar1 = true;

		if ((timing != 3) && (timing != 5)) {
			bVar1 = false;
		}
		if ((!bVar1) && (param_8 == 0)) {
			iVar7 = 0;
		}

		this->field_0x1c = iVar7;

		if (this->pActor3C_0x0->splinePointCount == 1) {
			this->type = 0;
			this->mode = 0;
		}
		else {
			this->type = type;
			this->mode = mode;
		}

		if (timing != 4) {
			if (iVar8 == 0) {
				this->field_0x8 = 0.0f;
			}
			if (count + -1 == iVar8) {
				this->barFullAmount_0x4 = 0.0f;
			}
		}

		pfVar2 = NewPool_edF32(count);
		this->field_0x10 = pfVar2;

		if (pPathFollow->aSplineRotationsEuler != (edF32VECTOR4*)0x0) {
			pPathFollow->aSplineRotationsQuat = pPathFollow->aSplineRotationsEuler;

			for (iVar7 = 0; iVar7 < pPathFollow->splinePointCount; iVar7 = iVar7 + 1) {
				// Convert the points from euler to quat.
				edF32Matrix4FromEulerSoft(&eStack80, &pPathFollow->aSplineRotationsEuler[iVar7].xyz, "XYZ");
				edQuatFromMatrix4(&pPathFollow->aSplineRotationsEuler[iVar7], &eStack80);
			}

			pPathFollow->aSplineRotationsEuler = (edF32VECTOR4*)0x0;
		}

		switch (timing) {
		case 0:
		case 4:
			for (iVar7 = 0; iVar7 < count; iVar7 = iVar7 + 1) {
				if (this->pActor3C_0x0->aSplinePoints == (edF32VECTOR4*)0x0) {
					peVar4 = &gF32Vertex4Zero;
				}
				else {
					if (count == 0) {
						trap(7);
					}
					peVar4 = this->pActor3C_0x0->aSplinePoints + (iVar7 + 1) % count;
				}

				peVar3 = this->pActor3C_0x0->aSplinePoints;

				if (peVar3 == (edF32VECTOR4*)0x0) {
					peVar3 = &gF32Vertex4Zero;
				}
				else {
					peVar3 = peVar3 + iVar7;
				}

				local_10 = *peVar3 - *peVar4;

				fVar10 = edF32Vector4GetDistHard(&local_10);
				this->field_0x10[iVar7] = fVar10;
			}
			break;
		case 1:
			for (iVar7 = 0; iVar7 < count; iVar7 = iVar7 + 1) {
				edF32VECTOR4* pSplineRotationsQuat = this->pActor3C_0x0->aSplineRotationsQuat;

				if (pSplineRotationsQuat == 0) {
					peVar4 = &CPathFollow::gPathDefQuat;
				}
				else {
					peVar4 = &pSplineRotationsQuat[iVar7];
				}

				if (this->pActor3C_0x0->aSplineRotationsQuat == 0) {
					peVar3 = &CPathFollow::gPathDefQuat;
				}
				else {
					if (count == 0) {
						trap(7);
					}
					peVar3 = &this->pActor3C_0x0->aSplineRotationsQuat[((iVar7 + 1) % count)];
				}

				fVar9 = dot_quat(peVar4, peVar3);
				fVar10 = 1.0f;
				if (fabs(fVar9) <= 1.0f) {
					fVar10 = fabs(fVar9);
				}

				fVar10 = acosf(fVar10);
				this->field_0x10[iVar7] = fabs(fVar10);
			}
			break;
		default:
			for (iVar7 = 0; iVar7 < count; iVar7 = iVar7 + 1) {
				this->field_0x10[iVar7] = 1.0f;
			}
			break;
		case 3:
			for (iVar7 = 0; iVar7 < count; iVar7 = iVar7 + 1) {
				float* pFloatData = this->pActor3C_0x0->aDelays;

				if (pFloatData == (float*)0x0) {
					fVar10 = 0.0f;
				}
				else {
					fVar10 = pFloatData[iVar7];
				}

				this->field_0x10[iVar7] = fVar10;
			}
			break;
		case 5:
			for (iVar7 = 0; iVar7 < count; iVar7 = iVar7 + 1) {
				if (this->pActor3C_0x0->aSplinePoints == (edF32VECTOR4*)0x0) {
					peVar4 = &gF32Vertex4Zero;
				}
				else {
					if (count == 0) {
						trap(7);
					}

					peVar4 = this->pActor3C_0x0->aSplinePoints + (iVar7 + 1) % count;
				}

				peVar3 = this->pActor3C_0x0->aSplinePoints;
				if (peVar3 == (edF32VECTOR4*)0x0) {
					peVar3 = &gF32Vertex4Zero;
				}
				else {
					peVar3 = peVar3 + iVar7;
				}
				local_10 = *peVar3 - *peVar4;

				float* pDelays = this->pActor3C_0x0->aDelays;
				if (pDelays == 0) {
					fVar10 = 0.0;
				}
				else {
					fVar10 = pDelays[iVar7];
				}

				fVar9 = 0.0f;
				if (0.0f < fVar10) {
					pDelays = this->pActor3C_0x0->aDelays;
					if (pDelays != 0) {
						fVar9 = pDelays[iVar7];
					}

					fVar10 = edF32Vector4GetDistHard(&local_10);
					this->field_0x10[iVar7] = fVar10 / fVar9;
				}
				else {
					this->field_0x10[iVar7] = 0.0f;
				}
			}
		}

		
		fVar10 = 0.0f;
		for (iVar7 = 0; iVar7 < iVar8; iVar7 = iVar7 + 1) {
			fVar10 = fVar10 + this->field_0x10[iVar7];
		}

		fVar9 = 0.0f;
		for (iVar7 = iVar8; iVar7 < count; iVar7 = iVar7 + 1) {
			fVar9 = fVar9 + this->field_0x10[iVar7];
		}

		iVar7 = 0;
		if (fVar9 + fVar10 < 0.001f) {
			for (; iVar7 < count; iVar7 = iVar7 + 1) {
				this->field_0x10[iVar7] = 1.0f;
			}

			fVar10 = (float)iVar8;
			fVar9 = (float)(count - iVar8);
		}

		fVar13 = 0.0f;
		fVar14 = 0.0f;

		if (this->field_0x1c == 0) {
			for (iVar7 = 0; iVar5 = iVar8, iVar7 < iVar8; iVar7 = iVar7 + 1) {
				float* pFloatData = this->pActor3C_0x0->aDelays;

				if (pFloatData == (float*)0x0) {
					fVar11 = 0.0f;
				}
				else {
					fVar11 = pFloatData[iVar7];
				}

				fVar13 = fVar13 + fVar11;
			}

			for (; iVar5 < count; iVar5 = iVar5 + 1) {
				float* pFloatData = this->pActor3C_0x0->aDelays;

				if (pFloatData == (float*)0x0) {
					fVar11 = 0.0f;
				}
				else {
					fVar11 = pFloatData[iVar5];
				}

				fVar14 = fVar14 + fVar11;
			}
		}

		if ((this->type == 0) || (this->type != 1)) {
			if (this->mode != 1) {
				fVar11 = 0.0;
				fVar12 = this->field_0x10[count - 1];
				this->field_0x10[count - 1] = 0.0;
				fVar9 = fVar9 - fVar12;
				if (this->field_0x1c == 0) {
					float* pFloatData = this->pActor3C_0x0->aDelays;

					if (pFloatData != (float*)0x0) {
						fVar11 = pFloatData[count - 1];
					}

					fVar14 = fVar14 - fVar11;
				}
			}
		}
		else {
			fVar9 = (fVar9 - this->field_0x10[count + -1]) * 2.0f;

			if (this->field_0x1c == 0) {
	
				float* pFloatData = this->pActor3C_0x0->aDelays;

				if (pFloatData == (float*)0x0) {
					fVar11 = 0.0f;
				}
				else {
					fVar11 = pFloatData[iVar8];
				}

				pFloatData = this->pActor3C_0x0->aDelays;

				if (pFloatData == (float*)0x0) {
					fVar12 = 0.0f;
				}
				else {
					fVar12 = pFloatData[count - 1];
				}

				fVar14 = (fVar14 * 2.0f - fVar11) - fVar12;
			}
		}

		if (timing != 4) {
			if (this->field_0x8 == 0.0f) {
				this->field_0x8 = fVar10 + fVar13;
			}

			if (this->barFullAmount_0x4 == 0.0f) {
				this->barFullAmount_0x4 = fVar9 + fVar14;
			}

			fVar11 = this->field_0x8 - fVar13;
			fVar12 = this->barFullAmount_0x4 - fVar14;
		}
		else {
			fVar11 = fVar10 / this->field_0x8;
			fVar12 = fVar9 / this->barFullAmount_0x4;

			this->field_0x8 = fVar11 + fVar13;
			this->barFullAmount_0x4 = fVar12 + fVar14;
		}

		fVar13 = 1.0f;
		if (fVar10 != 0.0f) {
			fVar13 = fVar10;
		}
		fVar10 = 1.0f;
		if (fVar9 != 0.0f) {
			fVar10 = fVar9;
		}

		iVar5 = 0;
		iVar7 = count;
		if (this->field_0x1c == 0) {		
			for (; iVar6 = iVar8, iVar5 < iVar8; iVar5 = iVar5 + 1) {
				float* pFloatData = this->pActor3C_0x0->aDelays;

				if (pFloatData == (float*)0x0) {
					fVar9 = 0.0f;
				}
				else {
					fVar9 = pFloatData[iVar5];
				}

				this->field_0x10[iVar5] = (fVar11 * this->field_0x10[iVar5]) / fVar13 + fVar9;
			}

			for (; iVar6 < count; iVar6 = iVar6 + 1) {
				float* pFloatData = this->pActor3C_0x0->aDelays;

				if (pFloatData == (float*)0x0) {
					fVar9 = 0.0f;
				}
				else {
					fVar9 = pFloatData[iVar6];
				}

				this->field_0x10[iVar6] = (fVar12 * this->field_0x10[iVar6]) / fVar10 + fVar9;
			}
		}
		else {
			for (iVar5 = 0; iVar6 = iVar8, iVar5 < iVar8; iVar5 = iVar5 + 1) {
				this->field_0x10[iVar5] = (fVar11 * this->field_0x10[iVar5]) / fVar13;
			}

			for (; iVar6 < count; iVar6 = iVar6 + 1) {
				this->field_0x10[iVar6] = (fVar12 * this->field_0x10[iVar6]) / fVar10;
			}
		}

		do {
			iVar7 = iVar7 + -1;
			if (iVar7 < iVar8) break;
		} while (this->field_0x10[iVar7] == 0.0f);

		if (iVar8 <= iVar7) {
			this->field_0x10[iVar7] = this->field_0x10[iVar7] + 0.0001f;
		}

		this->field_0xc = 0.0f;

		for (; iVar8 < count + -1; iVar8 = iVar8 + 1) {
			this->field_0xc = this->field_0xc + this->field_0x10[iVar8];
		}
	}
	return;
}

void CPathFollowReaderAbsolute::Create(float param_1, CPathFollow* pPathFollow, int type)
{
	if (pPathFollow != (CPathFollow*)0x0) {
		Create(param_1, param_1, pPathFollow, pPathFollow->mode, pPathFollow->type, 4, type);
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

	piVar1 = reinterpret_cast<S_TARGET_ON_OFF_STREAM_REF*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;
	if (piVar1->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + piVar1->entryCount * sizeof(S_STREAM_NTF_TARGET_ONOFF);
	}
	this->pTargetStream = piVar1;

	pcVar2 = reinterpret_cast<S_STREAM_EVENT_CAMERA*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + sizeof(S_STREAM_EVENT_CAMERA);
	this->streamEventCamera = pcVar2;

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

	pSVar1 = this->pTargetStream;
	iVar3 = 0;
	if (0 < pSVar1->entryCount) {
		do {
			pSVar1->aEntries[iVar3].Init();
			pSVar1 = this->pTargetStream;
			iVar3 = iVar3 + 1;
		} while (iVar3 < pSVar1->entryCount);
	}

	this->streamEventCamera->Init();

	IMPLEMENTATION_GUARD_AUDIO(
	S_STREAM_REF<CSound>::Init(&this->field_0x20);)
	return;
}

void CBehaviourPlatformSlab::Manage()
{
	ACTOR_LOG(LogLevel::Verbose, "CBehaviourPlatformSlab::Manage");

	this->streamEventCamera->Manage(this->pOwner);
	this->pOwner->BehaviourSlab_Manage(this);
}

void CBehaviourPlatformSlab::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	S_TARGET_ON_OFF_STREAM_REF* pSVar1;
	int iVar3;

	pSVar1 = this->pTargetStream;
	iVar3 = 0;
	if (0 < pSVar1->entryCount) {
		do {
			pSVar1->aEntries[iVar3].Reset();
			pSVar1 = this->pTargetStream;
			iVar3 = iVar3 + 1;
		} while (iVar3 < pSVar1->entryCount);
	}

	this->streamEventCamera->Reset(pOwner);

	if (newState == -1) {
		this->pOwner->SetState(0xb, -1);
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
	this->field_0x3c = iVar1;
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
		(this->trajPos).field_0x0 = (this->trajPos).field_0x0 + 1.0f;

		pPlatform->Platform_UpdateMatrixOnTrajectory(&this->pathFollowReaderAbs, 0, 0, &this->trajPos, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);

		(this->trajPos).field_0x0 = (this->trajPos).field_0x0 - 1.0f;

		this->pOwner->GenericManage(1, 0, (int)(this->trajPos).field_0x4, (int)(this->trajPos).field_0x6);
	}
	return;
}

void CBehaviourWeighingMachine::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	if (newState == -1) {
		if ((this->pathFollowReaderAbs).pActor3C_0x0 == (CPathFollow*)0x0) {
			this->pOwner->SetState(6, -1);
		}
		else {
			this->pOwner->SetState(0x10, -1);
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	(this->trajPos).field_0x0 = 0.0f;
	(this->trajPos).field_0x4 = 0;
	(this->trajPos).field_0x6 = 0;
	(this->trajPos).field_0x8 = -1;
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

	S_STREAM_NTF_TARGET_ANALOG_LIST* pList = (S_STREAM_NTF_TARGET_ANALOG_LIST*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(&pList->aEntries);
	if (pList->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pList->entryCount * sizeof(S_STREAM_NTF_TARGET_ANALOG);
	}
	this->field_0x34 = pList;

	S_STREAM_EVENT_CAMERA* pSVar2 = (S_STREAM_EVENT_CAMERA*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(pSVar2 + 1);
	this->pStreamEventCamera = pSVar2;
	return;
}

void CBehaviourWeighingMachineMaster::Init(CActor* pOwner)
{
	int* piVar1;
	int iVar2;
	int iVar3;

	this->pOwner = static_cast<CActorMovingPlatform*>(pOwner);

	for (int i = 0; i < this->field_0x34->entryCount; i++) {
		this->field_0x34->aEntries[i].Init();
	}

	this->pStreamEventCamera->Init();
	this->streamActorRef.Init();
	return;

}

void CBehaviourWeighingMachineMaster::Manage()
{
	bool uVar1;
	S_TRAJ_POS* pTrajPos;
	CActorMovingPlatform* pPlatform;
	int state;

	this->pStreamEventCamera->Manage(this->pOwner);
	pPlatform = this->pOwner;
	state = pPlatform->actorState;

	uVar1 = false;
	if (state == 6) {
		pTrajPos = &this->trajPos;

		this->trajPos.field_0x0 = this->trajPos.field_0x0 + 1.0f;

		pPlatform->Platform_UpdateMatrixOnTrajectory(&this->pathFollowReaderAbs, 0, 0, pTrajPos, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);

		pTrajPos->field_0x0 = pTrajPos->field_0x0 - 1.0f;
	}
	else {
		if (state == 0x10) {
			uVar1 = pPlatform->StateWeighingMaster(this);
		}
	}

	pPlatform->GenericManage(1, uVar1, this->trajPos.field_0x4, this->trajPos.field_0x6);
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
		if (this->pathFollowReaderAbs.pActor3C_0x0 == (CPathFollow*)0x0) {
			pPlatform = this->pOwner;
			pPlatform->SetState(6, -1);
		}
		else {
			pPlatform = this->pOwner;
			pPlatform->SetState(0x10, -1);
		}
	}
	else {
		pPlatform = this->pOwner;
		pPlatform->SetState(newState, newAnimationType);
	}

	this->trajPos.field_0x0 = 0.0f;
	this->trajPos.field_0x4 = 0;
	this->trajPos.field_0x6 = 0;
	this->trajPos.field_0x8 = -1;

	//piVar1 = this->field_0x34;
	//iVar3 = 0;
	//if (0 < *piVar1) {
	//	iVar2 = 0;
	//	do {
	//		S_STREAM_NTF_TARGET_ANALOG::Reset((S_STREAM_NTF_TARGET_ANALOG*)((int)piVar1 + iVar2 + 4));
	//		piVar1 = this->field_0x34;
	//		iVar3 = iVar3 + 1;
	//		iVar2 = iVar2 + 0x28;
	//	} while (iVar3 < *piVar1);
	//}

	this->pStreamEventCamera->Reset(pOwner);
	pTrajPos = &this->trajPos;
	pPlatform = this->pOwner;
	this->trajPos.field_0x0 = this->trajPos.field_0x0 + 1.0f;

	pPlatform->Platform_UpdateMatrixOnTrajectory(&this->pathFollowReaderAbs, 1, 0, pTrajPos, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);

	pTrajPos->field_0x0 = pTrajPos->field_0x0 - 1.0f;

	this->trajPos.field_0x8 = this->trajPos.field_0x4;
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

	if (actorState == 6) {
		pTrajPos = &this->trajPos;
		this->trajPos.field_0x0 = this->trajPos.field_0x0 + 1.0f;
		pPlatform->Platform_UpdateMatrixOnTrajectory(&this->pathFollowReaderAbs, 0, 0, pTrajPos, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
		pTrajPos->field_0x0 = pTrajPos->field_0x0 - 1.0f;
	}
	else {
		if (actorState == 0x10) {
			uVar2 = (uint)(this->field_0x34 != this->trajPos.field_0x0);
			this->trajPos.field_0x0 = this->field_0x34;
			pTrajPos = &this->trajPos;
			this->trajPos.field_0x0 = this->trajPos.field_0x0 + 1.0f;
			pPlatform->Platform_UpdateMatrixOnTrajectory(&this->pathFollowReaderAbs, uVar2, 1, pTrajPos, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
			pTrajPos->field_0x0 = pTrajPos->field_0x0 - 1.0f;
		}
	}

	pPlatform->GenericManage(1, uVar2, this->trajPos.field_0x4, this->trajPos.field_0x6);
	return; 
}

void CBehaviourWeighingMachineSlave::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	S_TRAJ_POS* pTrajPos;
	CActorMovingPlatform* pPlatform;

	if (newState == -1) {
		if (this->pathFollowReaderAbs.pActor3C_0x0 == (CPathFollow*)0x0) {
			pPlatform = this->pOwner;
			pPlatform->SetState(6, -1);
		}
		else {
			pPlatform = this->pOwner;
			pPlatform->SetState(0x10, -1);
		}
	}
	else {
		pPlatform = this->pOwner;
		pPlatform->SetState(newState, newAnimationType);
	}

	this->trajPos.field_0x0 = 0.0f;
	this->trajPos.field_0x4 = 0;
	this->trajPos.field_0x6 = 0;
	this->trajPos.field_0x8 = -1;

	pTrajPos = &this->trajPos;
	pPlatform = this->pOwner;
	this->trajPos.field_0x0 = this->trajPos.field_0x0 + 1.0f;

	pPlatform->Platform_UpdateMatrixOnTrajectory(&this->pathFollowReaderAbs, 1, 0, pTrajPos, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);

	pTrajPos->field_0x0 = pTrajPos->field_0x0 - 1.0f;

	this->trajPos.field_0x8 = this->trajPos.field_0x4;
	return;
}

int CBehaviourWeighingMachineSlave::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	if (msg == 0x11) {
		this->field_0x34 = *reinterpret_cast<float*>(pMsgParam);
	}

	return msg == 0x11;
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
	if ((state == 0xd) && (pSound = (this->streamRefSound).pSound, pSound != (CSound*)0x0)) {
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
		this->pOwner->SetState(0xb, -1);
		uVar3 = 1;
	}
	else {
		if (msg == 0xf) {
			this->pOwner->SetState(0xd, -1);
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
							ed3DHierarchyBankMatLinkG2D(this->pOwner->p3DHierNode, pTexture);
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

	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * sizeof(S_STREAM_NTF_TARGET_SWITCH_EX_LIST);
	}
	this->field_0x30 = (S_STREAM_NTF_TARGET_SWITCH_EX_LIST*)piVar1;

	S_STREAM_EVENT_CAMERA* pcVar2 = (S_STREAM_EVENT_CAMERA*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + sizeof(S_STREAM_EVENT_CAMERA);
	this->pStreamEventCamera = pcVar2;
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

	for (iVar5 = 0; iVar5 < this->field_0x30->entryCount; iVar5 = iVar5 + 1) {
		this->field_0x30->aEntries[iVar5].Init();
	}

	this->pStreamEventCamera->Init();

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
	this->pStreamEventCamera->Manage(this->pOwner);
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
		ed3DHierarchyBankMatLinkG2D(this->pOwner->p3DHierNode, pTexture);
	}

	this->field_0x28 = 1;
	this->field_0x20 = 0.0f;

	//piVar2 = (int*)this->field_0x30;
	//iVar4 = 0;
	//if (0 < *piVar2) {
	//	iVar3 = 0;
	//	do {
	//		S_STREAM_NTF_TARGET_SWITCH_EX::Reset((S_STREAM_NTF_TARGET_SWITCH_EX*)((int)piVar2 + iVar3 + 4));
	//		piVar2 = (int*)this->field_0x30;
	//		iVar4 = iVar4 + 1;
	//		iVar3 = iVar3 + 0x10;
	//	} while (iVar4 < *piVar2);
	//}

	this->pStreamEventCamera->Reset(pOwner);

	for (iVar4 = this->pActorStreamRef->entryCount; iVar4 != 0; iVar4 = iVar4 + -1) {
		this->pActorStreamRef->aEntries[iVar4].Reset();
	}

	this->field_0x38 = -1;

	if (newState == -1) {
		pCVar1 = this->pOwner;
		pCVar1->SetState(0xd, -1);
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
	if (actorState == 0xd) {
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
		if (actorState == 0xb) {
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
		ed3DHierarchyBankMatLinkG2D(this->pOwner->p3DHierNode, pTexture);
	}

	this->field_0x28 = 1;
	this->field_0x20 = 0.0f;

	if (newState == -1) {
		pPlatform = this->pOwner;
		pPlatform->SetState(0xb, -1);
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

	this->field_0xc = 0;

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
