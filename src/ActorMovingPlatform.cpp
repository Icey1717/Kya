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
};


PlatformHeader<CBehaviourPlatformTrajectory>* gPlatform_00448e14;
PlatformHeader<CBehaviourPlatformSlab>* gPlatform_00448e18;
PlatformHeader<CBehaviourPlatformDestroyed>* gPlatform_00448e1c;
PlatformHeader<CBehaviourPlatformStand>* gPlatform_00448e20;
PlatformHeader<CBehaviourSelectorNew>* gPlatform_00448e24;
PlatformHeader<CBehaviourSelectorSlave>* gPlatform_00448e28;
PlatformHeader<S_TILT_DATA>* gTiltDataAllocator;

CActorMovingPlatform::CActorMovingPlatform()
{
	CActor();
	this->dynamic.weightB = 0.0f;
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
	this->field_0x244 = (S_BRIDGE_ACTOR_STREAM*)piVar5;

	piVar5 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar5 + 1);
	if (*piVar5 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar5 * sizeof(S_BRIDGE_ZONE_STREAM);
	}
	this->field_0x248 = (S_BRIDGE_ZONE_STREAM*)piVar5;

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
	this->field_0x254 = (S_BRIDGE_CAMERA_STREAM*)piVar5;

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
	//this->field_0x1dc = piVar5;
	piVar5 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar5 + 1);
	if (*piVar5 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar5 * 0x10;
	}
#if 0
	this->field_0x1e0 = piVar5;
	pcVar1 = this->field_0x1d0;
	fVar12 = *(float*)(pcVar1 + 0x14);
	*(undefined4*)(pcVar1 + 0x14) = *(undefined4*)(pcVar1 + 0x18);
	*(float*)(this->field_0x1d0 + 0x18) = -fVar12;
	puVar2 = *(uint**)&this->field_0x2c;
	if (puVar2 != (uint*)0x0) {
		*puVar2 = *puVar2 & 0xffffffc4;
		*puVar2 = *puVar2 | 0x1800480;
		if ((*(uint*)(this->field_0x1d0 + 0x24) & 0x40) != 0) {
			*puVar2 = *puVar2 | 0x100;
		}
		if ((*(uint*)(this->field_0x1d0 + 0x24) & 0x400000) != 0) {
			*puVar2 = *puVar2 | 0x200;
		}
		if ((*(uint*)(this->field_0x1d0 + 0x24) & 0x4000) != 0) {
			*puVar2 = *puVar2 & 0xffefffff;
		}
		uVar9 = 0;
		if ((*(uint*)(this->field_0x1d0 + 0x24) & 0x80) != 0) {
			uVar9 = 0x400;
		}
		if ((*(uint*)(this->field_0x1d0 + 0x24) & 0x100) != 0) {
			uVar9 = uVar9 | 0x800;
		}
		if (uVar9 != 0) {
			CCollision::PatchObbTreeFlagsRecurse((CollisionDataSubObj_176*)puVar2[4], uVar9, 0xffffffffffffffff, 0);
		}
	}
	if (this->field_0x254 == (int*)0x0) {
		iVar11 = 0;
	}
	else {
		iVar11 = *this->field_0x254;
	}
	iVar7 = 0;
	if (0 < iVar11) {
		iVar10 = 0;
		do {
			iVar3 = *(int*)((int)this->field_0x254 + iVar10 + 4);
			if (iVar3 == 0xe) {
				this->field_0x1d8 = this->field_0x1d8 | 8;
			}
			else {
				if ((iVar3 == 0xc) || (iVar3 == 0xb)) {
					this->field_0x1d8 = this->field_0x1d8 | 4;
				}
			}
			iVar7 = iVar7 + 1;
			iVar10 = iVar10 + 0x48;
		} while (iVar7 < iVar11);
	}
	if ((*(uint*)(this->field_0x1d0 + 0x24) & 0x1000) != 0) {
		this->field_0x1d8 = this->field_0x1d8 | 4;
	}
	if ((*(uint*)(this->field_0x1d0 + 0x24) & 0x400) != 0) {
		this->field_0x1d8 = this->field_0x1d8 | 8;
	}
	uVar9 = 0;
	iVar7 = 0;
	if (0 < iVar11) {
		piVar5 = this->field_0x254;
		do {
			if (piVar5[0x12] != -1) {
				uVar9 = SEXT48((int)uVar9 + 1);
			}
			iVar7 = iVar7 + 1;
			piVar5 = piVar5 + 0x12;
		} while (iVar7 < iVar11);
	}
	if (3 < uVar9) {
		uVar9 = 3;
	}
	piVar5 = (int*)this->field_0x1d0;
	iVar11 = *piVar5;
	if (iVar11 == -1) {
		iVar11 = piVar5[1];
	}
	if (iVar11 == -1) {
		iVar11 = piVar5[2];
		if (iVar11 == -1) {
			iVar11 = piVar5[3];
		}
		if (iVar11 == -1) goto LAB_0015d240;
	}
	this->field_0x1d8 = this->field_0x1d8 | 1;
LAB_0015d240:
	pCVar8 = CActor::GetBehaviour((CActor*)this, 7);
	if ((pCVar8 != (CBehaviour*)0x0) &&
		(pCVar8 = CActor::GetBehaviour((CActor*)this, 7), pCVar8[2].pVTable != (CBehaviourVtable*)0xffffffff)) {
		this->field_0x1d8 = this->field_0x1d8 | 1;
	}
	pCVar8 = CActor::GetBehaviour((CActor*)this, 3);
	if ((pCVar8 != (CBehaviour*)0x0) &&
		(pCVar8 = CActor::GetBehaviour((CActor*)this, 3), pCVar8[8].pVTable != (CBehaviourVtable*)0xffffffff)) {
		this->field_0x1d8 = this->field_0x1d8 | 2;
	}
	pCVar8 = CActor::GetBehaviour((CActor*)this, 8);
	if ((pCVar8 != (CBehaviour*)0x0) || (pCVar8 = CActor::GetBehaviour((CActor*)this, 9), pCVar8 != (CBehaviour*)0x0)) {
		pCVar8 = CActor::GetBehaviour((CActor*)this, 8);
		if (pCVar8 == (CBehaviour*)0x0) {
			pCVar8 = CActor::GetBehaviour((CActor*)this, 9);
		}
		if (pCVar8[5].pVTable != (CBehaviourVtable*)0xffffffff) {
			this->field_0x1d8 = this->field_0x1d8 | 2;
		}
	}
	if ((this->field_0x1d8 & 2) != 0) {
		uVar9 = SEXT48((int)uVar9 + 1);
	}
	if ((this->field_0x1d8 & 1) != 0) {
		uVar9 = SEXT48((int)uVar9 + 1);
	}
	if (uVar9 == 0) {
		this->field_0x240 = (int*)0x0;
	}
	else {
		piVar5 = CActor::CreateActorSound((CActor*)this, uVar9);
		this->field_0x240 = piVar5;
	}
	this->field_0x1e4 = 0;
	this->field_0x1e8 = 0;
#endif
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
		S_BRIDGE_ACTOR_STREAM_ENTRY* pEntry = this->field_0x244->aEntries;
		for (iVar10 = this->field_0x244->entryCount; iVar10 != 0; iVar10 = iVar10 + -1) {
			pEntry->actorRef.Init();
			pEntry = pEntry + 1;
		}
	}

	{
		S_STREAM_REF<ed_zone_3d>* pEntry = this->field_0x248->aEntries;
		for (iVar10 = this->field_0x248->entryCount; iVar10 != 0; iVar10 = iVar10 + -1) {
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
		S_BRIDGE_CAMERA_STREAM_ENTRY* pEntry = this->field_0x254->aEntries;
		for (iVar10 = this->field_0x254->entryCount; iVar10 != 0; iVar10 = iVar10 + -1) {
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
		case 0x2:
		{
			pBehaviour = PlatformHeader<CBehaviourPlatformTrajectory>::Get(&gPlatform_00448e14);
		}
		break;
		case 0x3:
		{
			pBehaviour = PlatformHeader<CBehaviourPlatformSlab>::Get(&gPlatform_00448e18);
		}
		break;
		case 0x4:
		{
			pBehaviour = new CBehaviourTeleportRandom;
		}
		break;
		case 0x5:
		{
			pBehaviour = new CBehaviourWeighingMachineSlave;
		}
		break;
		case 0x6:
		{
			pBehaviour = PlatformHeader<CBehaviourPlatformDestroyed>::Get(&gPlatform_00448e1c);
		}
		break;
		case 0x7:
		{
			pBehaviour = PlatformHeader<CBehaviourPlatformStand>::Get(&gPlatform_00448e20);
		}
		break;
		case 0x8:
		{
			pBehaviour = PlatformHeader<CBehaviourSelectorSlave>::Get(&gPlatform_00448e28);
		}
		break;
		case 0x9:
		{
			pBehaviour = PlatformHeader<CBehaviourSelectorNew>::Get(&gPlatform_00448e24);
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
				IMPLEMENTATION_GUARD(
				fVar9 = FUN_00212560(this->pCollisionData->pObbTree, pCVar4->currentLocation);
				if (fVar9 <= 0.3f) {
					fVar9 = pCVar4->GetCumulatedWeight();
					bVar5 = pBehaviour->field_0x10 <= fVar8 + fVar9;
				})
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
						CActor::FUN_00101110(this_00, (CActor*)this);
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
		IMPLEMENTATION_GUARD(
		param_3 = 1;
		local_10.y = -(this->pTiltData->pushData).oscValue.field_0x0;)
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
		IMPLEMENTATION_GUARD(
		edQuatToMatrix4Hard(&this->pTiltData->field_0x10, &eStack80);
		edF32Matrix4FromEulerSoft(&eStack144, (edF32VECTOR3*)(*(int*)&this->field_0x18 + 0x18), (char*)&PTR_DAT_00429cc8);
		eStack144.da = pPosition->x;
		eStack144.db = pPosition->y;
		eStack144.dc = pPosition->z;
		eStack144.dd = pPosition->w;
		edF32Matrix4MulF32Matrix4Hard(&eStack144, &eStack80, &eStack144);
		CActor::SV_UpdateMatrix_Rel((CActor*)this, &eStack144, 1, 1, pActorsTable, &local_10);)
	}

	if (this->field_0x248 == (S_BRIDGE_ZONE_STREAM*)0x0) {
		iVar2 = 0;
	}
	else {
		iVar2 = this->field_0x248->entryCount;
	}

	if (iVar2 == 0) {

		if (this->field_0x244 == (S_BRIDGE_ACTOR_STREAM*)0x0) {
			iVar2 = 0;
		}
		else {
			iVar2 = this->field_0x244->entryCount;
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

	if (this->field_0x244 == (S_BRIDGE_ACTOR_STREAM*)0x0) {
		iVar2 = 0;
	}
	else {
		iVar2 = this->field_0x244->entryCount;
	}

	iVar5 = 0;
	if (0 < iVar2) {
		iVar4 = 0;
		do {
			S_BRIDGE_ACTOR_STREAM_ENTRY* pEntry = &this->field_0x244->aEntries[iVar5];
			if (pEntry->actorRef.Get() != (CActor*)0x0) {
				pEntry->actorRef.Get()->TieToActor(this, pEntry->carryMethod, 1, &this->field_0x200);
			}
			iVar5 = iVar5 + 1;
		} while (iVar5 < iVar2);
	}

	iVar2 = 0;
	if (this->field_0x248 != (S_BRIDGE_ZONE_STREAM*)0x0) {
		iVar2 = this->field_0x248->entryCount;
	}

	iVar5 = 0;
	if (0 < iVar2) {
		iVar4 = 0;
		do {
			S_STREAM_REF<ed_zone_3d>* pEntry = &this->field_0x248->aEntries[iVar5];
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

AnimResult CActorMovingPlatform::gStateCfg_MPF[13] =
{
	AnimResult(6, 0x104),
	AnimResult(0, 0x004),
	AnimResult(0, 0x104),
	AnimResult(0, 0x004),
	AnimResult(0, 0x004),
	AnimResult(6, 0x004),
	AnimResult(0, 0x004),
	AnimResult(6, 0x004),
	AnimResult(0, 0x004),
	AnimResult(0, 0x004),
	AnimResult(6, 0x004),
	AnimResult(0, 0x004),
	AnimResult(7, 0x021),
};

AnimResult* CActorMovingPlatform::GetStateCfg(int state)
{
	AnimResult* pAVar1;

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
	AnimResult* pAVar5;
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
			if (this->field_0x254 != (S_BRIDGE_CAMERA_STREAM*)0x0) {
				iVar10 = this->field_0x254->entryCount;
			}

			iVar9 = 0;
			if (0 < iVar10) {
				IMPLEMENTATION_GUARD(
				iVar7 = 0;
				do {
					iVar3 = (int)&this->field_0x254->entryCount + iVar7;
					if (*(int*)(iVar3 + 4) == 6) {
						S_STREAM_NTF_TARGET_SWITCH::Switch((S_STREAM_NTF_TARGET_SWITCH*)(iVar3 + 0xc), (CActor*)this);
						S_STREAM_NTF_TARGET_SWITCH::PostSwitch((S_STREAM_NTF_TARGET_SWITCH*)(iVar3 + 0xc), (CActor*)this);
						S_STREAM_EVENT_CAMERA::SwitchOn((S_STREAM_EVENT_CAMERA*)(iVar3 + 0x28), (CActor*)this);
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
									CActorSound::SoundStart(this->pActorSound, (CActor*)this, uVar8, piVar6, 1, 0, (float**)0x0);
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
	}
	else {
		if ((this->movingPlatformFlags & 0x20) == 0) {
			this->movingPlatformFlags = this->movingPlatformFlags | 0x20;

			if ((this->pProperties->flags_0x24 & 0x80000) != 0) {
				SetBehaviour(6, -1, -1);

				iVar10 = 0;
				if (this->field_0x254 != (S_BRIDGE_CAMERA_STREAM*)0x0) {
					iVar10 = this->field_0x254->entryCount;
				}

				iVar9 = 0;
				if (0 < iVar10) {
					iVar7 = 0;
					IMPLEMENTATION_GUARD(
					do {
						iVar3 = (int)&this->field_0x254->entryCount + iVar7;
						if (*(int*)(iVar3 + 4) == 7) {
							S_STREAM_NTF_TARGET_SWITCH::Switch((S_STREAM_NTF_TARGET_SWITCH*)(iVar3 + 0xc), (CActor*)this);
							S_STREAM_NTF_TARGET_SWITCH::PostSwitch((S_STREAM_NTF_TARGET_SWITCH*)(iVar3 + 0xc), (CActor*)this);
							S_STREAM_EVENT_CAMERA::SwitchOn((S_STREAM_EVENT_CAMERA*)(iVar3 + 0x28), (CActor*)this);
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
										CActorSound::SoundStart(this->pActorSound, (CActor*)this, uVar8, piVar6, 1, 0, (float**)0x0);
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
			if (this->field_0x254 != (S_BRIDGE_CAMERA_STREAM*)0x0) {
				iVar10 = this->field_0x254->entryCount;
			}

			iVar9 = 0;
			if (0 < iVar10) {
				iVar7 = 0;
				IMPLEMENTATION_GUARD(
				do {
					iVar3 = (int)&this->field_0x254->entryCount + iVar7;
					if (*(int*)(iVar3 + 4) == 5) {
						S_STREAM_NTF_TARGET_SWITCH::Switch((S_STREAM_NTF_TARGET_SWITCH*)(iVar3 + 0xc), (CActor*)this);
						S_STREAM_NTF_TARGET_SWITCH::PostSwitch((S_STREAM_NTF_TARGET_SWITCH*)(iVar3 + 0xc), (CActor*)this);
						S_STREAM_EVENT_CAMERA::SwitchOn((S_STREAM_EVENT_CAMERA*)(iVar3 + 0x28), (CActor*)this);
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
									CActorSound::SoundStart(this->pActorSound, (CActor*)this, uVar8, piVar6, 1, 0, (float**)0x0);
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
	}

	iVar10 = 0;
	if (this->field_0x254 != (S_BRIDGE_CAMERA_STREAM*)0x0) {
		iVar10 = this->field_0x254->entryCount;
	}

	iVar9 = 0;
	if (0 < iVar10) {
		do {
			this->field_0x254->aEntries[iVar9].streamCameraEvent.Manage(this);
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
						CActorSound::SoundStart(this->pActorSound, (CActor*)this, 0, (int*)this->field_0x1e8, 1, 0, (float**)0x0);
					}
				}
			})
		}
		else {
			IMPLEMENTATION_GUARD(
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
						CActorSound::SoundStart(this->pActorSound, (CActor*)this, 0, (int*)this->field_0x1e4, 1, 0, (float**)0x0);
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
			IMPLEMENTATION_GUARD(
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
				CActorSound::SoundStart(this->pActorSound, (CActor*)this, 0, (int*)this->field_0x1e8, 1, 0, (float**)0x0);
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
			}
			pCVar1 = this->field_0x1f0;
			if (((pCVar1 == (CActor*)0x0) || (this->field_0x1ec == (CinNamedObject30*)0x0)) ||
				(bVar2 = true, this->field_0x1ec != pCVar1->pCinData)) {
				bVar2 = false;
			}
			if (((bVar2) && (pCVar1 != (CActor*)0x0)) &&
				((this->field_0x1ec != (CinNamedObject30*)0x0 && (this->field_0x1ec == pCVar1->pCinData)))) {
				(*(code*)pCVar1->pVTable->Manage)(&DAT_bf800000);
			}
			pCVar1 = this->field_0x1f8;
			if (((pCVar1 == (CActor*)0x0) || (this->field_0x1f4 == (CinNamedObject30*)0x0)) ||
				(bVar2 = true, this->field_0x1f4 != pCVar1->pCinData)) {
				bVar2 = false;
			}
			if (bVar2) {
				if (((pCVar1 != (CActor*)0x0) && (this->field_0x1f4 != (CinNamedObject30*)0x0)) &&
					(this->field_0x1f4 == pCVar1->pCinData)) {
					(pCVar1->currentLocation).x = this->currentLocation.x;
					(pCVar1->currentLocation).y = this->currentLocation.y;
					(pCVar1->currentLocation).z = this->currentLocation.z;
					(pCVar1->currentLocation).w = this->currentLocation.w;
				}
				pCVar1 = this->field_0x1f8;
				if (((pCVar1 != (CActor*)0x0) && (this->field_0x1f4 != (CinNamedObject30*)0x0)) &&
					(this->field_0x1f4 == pCVar1->pCinData)) {
					(pCVar1->rotationEuler).x = this->rotationEuler.x;
					(pCVar1->rotationEuler).y = this->rotationEuler.y;
					(pCVar1->rotationEuler).z = this->rotationEuler.z;
					(pCVar1->rotationEuler).w = this->rotationEuler.w;
				}
				pCVar1 = this->field_0x1f8;
				if (((pCVar1 != (CActor*)0x0) && (this->field_0x1f4 != (CinNamedObject30*)0x0)) &&
					(this->field_0x1f4 == pCVar1->pCinData)) {
					(*(code*)pCVar1->pVTable->InitDlistPatchable)(0, 0);
				}
			}
			iVar10 = 0;
			if (this->field_0x254 != (S_BRIDGE_CAMERA_STREAM*)0x0) {
				iVar10 = this->field_0x254->entryCount;
			}
			iVar9 = 0;
			if (0 < iVar10) {
				iVar7 = 0;
				do {
					iVar3 = (int)&this->field_0x254->entryCount + iVar7;
					if (*(int*)(iVar3 + 4) == 10) {
						S_STREAM_NTF_TARGET_SWITCH::Switch((S_STREAM_NTF_TARGET_SWITCH*)(iVar3 + 0xc), (CActor*)this);
						S_STREAM_NTF_TARGET_SWITCH::PostSwitch((S_STREAM_NTF_TARGET_SWITCH*)(iVar3 + 0xc), (CActor*)this);
						S_STREAM_EVENT_CAMERA::SwitchOn((S_STREAM_EVENT_CAMERA*)(iVar3 + 0x28), (CActor*)this);
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
									CActorSound::SoundStart(this->pActorSound, (CActor*)this, uVar8, piVar6, 1, 0, (float**)0x0);
									break;
								}
							}
						}
					}
					iVar9 = iVar9 + 1;
					iVar7 = iVar7 + 0x48;
				} while (iVar9 < iVar10);
			}
			this->movingPlatformFlags = this->movingPlatformFlags & 0xffffffef;)
		}
		else {
			IMPLEMENTATION_GUARD(
			iVar10 = this->actorState;
			if (iVar10 == -1) {
				uVar4 = 0;
			}
			else {
				pAVar5 = (*(this->pVTable)->GetStateCfg)((CActor*)this, iVar10);
				uVar4 = pAVar5->flags_0x4 & 1;
			}
			if (uVar4 == 0) {
				CActor::PlayAnim((CActor*)this, New_Name_(6));
			}
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
			if ((int*)this->field_0x1e4 != (int*)0x0) {
				CActorSound::SoundStart(this->pActorSound, (CActor*)this, 0, (int*)this->field_0x1e4, 1, 0, (float**)0x0);
			}
			uVar4 = *(uint*)&this->pProperties->field_0x1c;
			if (uVar4 != 0xffffffff) {
				if (((this->field_0x1f0 == (CActor*)0x0) || (this->field_0x1ec == (CinNamedObject30*)0x0)) ||
					(this->field_0x1ec != this->field_0x1f0->pCinData)) {
					bVar2 = false;
				}
				else {
					bVar2 = true;
				}
				if (!bVar2) {
					CParticlesManager::GetDynamicFx
					(CScene::ptable.g_EffectsManager_004516b8, &this->field_0x1ec, uVar4, 0xffffffffffffffff);
				}
			}
			pCVar1 = this->field_0x1f8;
			if (((pCVar1 == (CActor*)0x0) || (this->field_0x1f4 == (CinNamedObject30*)0x0)) ||
				(bVar2 = true, this->field_0x1f4 != pCVar1->pCinData)) {
				bVar2 = false;
			}
			if ((((bVar2) && (pCVar1 != (CActor*)0x0)) && (this->field_0x1f4 != (CinNamedObject30*)0x0)) &&
				(this->field_0x1f4 == pCVar1->pCinData)) {
				(*(code*)pCVar1->pVTable->Manage)(&DAT_bf800000);
			}
			pCVar1 = this->field_0x1f0;
			if (((pCVar1 == (CActor*)0x0) || (this->field_0x1ec == (CinNamedObject30*)0x0)) ||
				(bVar2 = true, this->field_0x1ec != pCVar1->pCinData)) {
				bVar2 = false;
			}
			if (bVar2) {
				if (((pCVar1 != (CActor*)0x0) && (this->field_0x1ec != (CinNamedObject30*)0x0)) &&
					(this->field_0x1ec == pCVar1->pCinData)) {
					(pCVar1->currentLocation).x = this->currentLocation.x;
					(pCVar1->currentLocation).y = this->currentLocation.y;
					(pCVar1->currentLocation).z = this->currentLocation.z;
					(pCVar1->currentLocation).w = this->currentLocation.w;
				}
				pCVar1 = this->field_0x1f0;
				if (((pCVar1 != (CActor*)0x0) && (this->field_0x1ec != (CinNamedObject30*)0x0)) &&
					(this->field_0x1ec == pCVar1->pCinData)) {
					(pCVar1->rotationEuler).x = this->rotationEuler.x;
					(pCVar1->rotationEuler).y = this->rotationEuler.y;
					(pCVar1->rotationEuler).z = this->rotationEuler.z;
					(pCVar1->rotationEuler).w = this->rotationEuler.w;
				}
				pCVar1 = this->field_0x1f0;
				if (((pCVar1 != (CActor*)0x0) && (this->field_0x1ec != (CinNamedObject30*)0x0)) &&
					(this->field_0x1ec == pCVar1->pCinData)) {
					(*(code*)pCVar1->pVTable->InitDlistPatchable)(0, 0);
				}
			}
			iVar10 = 0;
			if (this->field_0x254 != (S_BRIDGE_CAMERA_STREAM*)0x0) {
				iVar10 = this->field_0x254->entryCount;
			}
			iVar9 = 0;
			if (0 < iVar10) {
				iVar7 = 0;
				do {
					iVar3 = (int)&this->field_0x254->entryCount + iVar7;
					if (*(int*)(iVar3 + 4) == 9) {
						S_STREAM_NTF_TARGET_SWITCH::Switch((S_STREAM_NTF_TARGET_SWITCH*)(iVar3 + 0xc), (CActor*)this);
						S_STREAM_NTF_TARGET_SWITCH::PostSwitch((S_STREAM_NTF_TARGET_SWITCH*)(iVar3 + 0xc), (CActor*)this);
						S_STREAM_EVENT_CAMERA::SwitchOn((S_STREAM_EVENT_CAMERA*)(iVar3 + 0x28), (CActor*)this);
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
									CActorSound::SoundStart(this->pActorSound, (CActor*)this, uVar8, piVar6, 1, 0, (float**)0x0);
									break;
								}
							}
						}
					}
					iVar9 = iVar9 + 1;
					iVar7 = iVar7 + 0x48;
				} while (iVar9 < iVar10);
			}
			this->movingPlatformFlags = this->movingPlatformFlags | 0x10;)
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
	int iVar10;
	uint uVar11;
	int iVar12;
	int local_80;
	edF32MATRIX4 eStack112;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	S_PATHREADER_POS_INFO local_10;

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
			iVar8 = SV_UpdateMatrixOnTrajectory_Rel(pTrajPos->field_0x0, pPathFollowerAbs, this->pProperties->flags_0x24 & 2, 1, pActorsTable, (edF32MATRIX4*)0x0, &local_30, &local_10);
		}
	}
	else {
		edQuatToMatrix4Hard(&this->pTiltData->oscQuat.field_0x0, &eStack112);
		IMPLEMENTATION_GUARD(
		iVar8 = SV_UpdateMatrixOnTrajectory_Rel(pTrajPos->field_0x0, pPathFollowerAbs, this->pProperties->flags_0x24 & 2, 1, 
			pActorsTable, &eStack112, &local_30, &local_10);)
	}

	sVar6 = -1;
	sVar7 = sVar6;

	if (param_4 != 0) {
		sVar7 = local_10.field_0x4;

		if (0.0f < local_10.field_0x8) {
			sVar7 = local_10.field_0x0;

			if (iVar8 == 2) {
				if ((local_10.field_0x8 <= 0.999f) && (sVar7 = local_10.field_0x4, 0.001f <= local_10.field_0x8)) {
					sVar7 = sVar6;
				}
			}
			else {
				if ((pPathFollowerAbs->pActor3C_0x0 == (CPathFollow*)0x0) || (pPathFollowerAbs->pActor3C_0x0->splinePointCount != 2)) {
					sVar7 = local_10.field_0x4;

					if ((pTrajPos->field_0x6 != local_10.field_0x4) &&
						(sVar7 = sVar6, pTrajPos->field_0x4 == local_10.field_0x0)) {
						sVar7 = local_10.field_0x0;
					}
				}
				else {
					if ((local_10.field_0x8 <= 0.9f) && (sVar7 = local_10.field_0x4, 0.1f <= local_10.field_0x8)) {
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

	pTrajPos->field_0x4 = local_10.field_0x4;
	pTrajPos->field_0x6 = local_10.field_0x0;
	pTrajPos->field_0x8 = sVar7;

	if (param_4 != 0) {
		iVar8 = 0;

		if (this->field_0x254 != (S_BRIDGE_CAMERA_STREAM*)0x0) {
			iVar8 = this->field_0x254->entryCount;
		}

		iVar12 = 0;
		if (0 < iVar8) {
			iVar10 = 0;
			IMPLEMENTATION_GUARD(
			do {
				iVar9 = (int)&this->field_0x254->entryCount + iVar10;
				if ((*(int*)(iVar9 + 4) == 4) && ((long)sVar7 == (long)*(int*)(iVar9 + 8))) {
					S_STREAM_NTF_TARGET_SWITCH::Switch((S_STREAM_NTF_TARGET_SWITCH*)(iVar9 + 0xc), (CActor*)this);
					S_STREAM_NTF_TARGET_SWITCH::PostSwitch((S_STREAM_NTF_TARGET_SWITCH*)(iVar9 + 0xc), (CActor*)this);
					S_STREAM_EVENT_CAMERA::SwitchOn((S_STREAM_EVENT_CAMERA*)(iVar9 + 0x28), (CActor*)this);
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
								CActorSound::SoundStart(this->pActorSound, (CActor*)this, uVar11, piVar1, 1, 0, (float**)0x0);
								break;
							}
						}
					}
				}
				iVar12 = iVar12 + 1;
				iVar10 = iVar10 + 0x48;
			} while (iVar12 < iVar8);)
		}
	}

	if (this->field_0x248 == (S_BRIDGE_ZONE_STREAM*)0x0) {
		iVar8 = 0;
	}
	else {
		iVar8 = this->field_0x248->entryCount;
	}

	if (iVar8 == 0) {
		if (this->field_0x244 == (S_BRIDGE_ACTOR_STREAM*)0x0) {
			iVar8 = 0;
		}
		else {
			iVar8 = this->field_0x244->entryCount;
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
		IMPLEMENTATION_GUARD(
		bVar3 = StateTrajectory(currentFillAmount, this, pBehaviour, false);
		local_4 = (int)bVar3;)
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

			if (this->field_0x254 != (S_BRIDGE_CAMERA_STREAM*)0x0) {
				iVar9 = this->field_0x254->entryCount;
			}
			iVar8 = 0;
			if (0 < iVar9) {
				iVar6 = 0;
				IMPLEMENTATION_GUARD(
				do {
					iVar4 = (int)&this->field_0x254->entryCount + iVar6;
					if (*(int*)(iVar4 + 4) == 3) {
						S_STREAM_NTF_TARGET_SWITCH::Switch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), (CActor*)this);
						S_STREAM_NTF_TARGET_SWITCH::PostSwitch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), (CActor*)this);
						S_STREAM_EVENT_CAMERA::SwitchOn((S_STREAM_EVENT_CAMERA*)(iVar4 + 0x28), (CActor*)this);
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
									CActorSound::SoundStart(this->pActorSound, (CActor*)this, uVar7, piVar1, 1, 0, (float**)0x0);
									break;
								}
							}
						}
					}
					iVar8 = iVar8 + 1;
					iVar6 = iVar6 + 0x48;
				} while (iVar8 < iVar9);)
			}

			pBehaviour->goalAmount_0x30 = (pBehaviour->currentFillAmount_0x38).field_0x0 + pBehaviour->field_0x28 + pTVar5->scaledTotalTime;
		}
		break;
	case 9:
		IMPLEMENTATION_GUARD(
		bVar3 = StateTrajectory(currentFillAmount, this, pBehaviour, true);
		local_4 = (int)bVar3;)
		break;
	case 10:
		IMPLEMENTATION_GUARD(
		bVar3 = StateTrajectory(currentFillAmount, this, pBehaviour, false);
		local_4 = (int)bVar3;
		if (currentFillAmount == pBehaviour->field_0x34) {
			SetState(6, -1);
			iVar9 = 0;
			if (this->field_0x254 != (S_BRIDGE_CAMERA_STREAM*)0x0) {
				iVar9 = this->field_0x254->entryCount;
			}
			iVar8 = 0;
			if (0 < iVar9) {
				iVar6 = 0;
				do {
					iVar4 = (int)&this->field_0x254->entryCount + iVar6;
					if (*(int*)(iVar4 + 4) == 0) {
						S_STREAM_NTF_TARGET_SWITCH::Switch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), (CActor*)this);
						S_STREAM_NTF_TARGET_SWITCH::PostSwitch((S_STREAM_NTF_TARGET_SWITCH*)(iVar4 + 0xc), (CActor*)this);
						S_STREAM_EVENT_CAMERA::SwitchOn((S_STREAM_EVENT_CAMERA*)(iVar4 + 0x28), (CActor*)this);
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
									CActorSound::SoundStart(this->pActorSound, (CActor*)this, uVar7, piVar1, 1, 0, (float**)0x0);
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
	S_MOVING_PLATFORM_TARGET_STREAM* pSVar3;
	int iVar4;
	int iVar5;
	int iVar6;
	float fVar7;

	iVar6 = this->actorState;
	iVar5 = 0;

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
					IMPLEMENTATION_GUARD(
					StateSwitchSlabOff2On(pBehaviour);)
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

void CBehaviourPlatformTrajectory::Manage()
{
	this->pOwner->BehaviourTrajectory_Manage(this);
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
				float* pFloatData = this->pActor3C_0x0->field_0x2c;

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
			IMPLEMENTATION_GUARD(
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
				local_10.x = peVar3->x - peVar4->x;
				local_10.y = peVar3->y - peVar4->y;
				local_10.z = peVar3->z - peVar4->z;
				local_10.w = peVar3->w - peVar4->w;
				iVar5 = this->pActor3C_0x0->field_0x2c;
				if (iVar5 == 0) {
					fVar10 = 0.0;
				}
				else {
					fVar10 = *(float*)(iVar5 + iVar7 * 4);
				}
				fVar9 = 0.0;
				if (0.0 < fVar10) {
					iVar5 = this->pActor3C_0x0->field_0x2c;
					if (iVar5 != 0) {
						fVar9 = *(float*)(iVar5 + iVar7 * 4);
					}
					fVar10 = edF32Vector4GetDistHard(&local_10);
					this->field_0x10[iVar7] = fVar10 / fVar9;
				}
				else {
					this->field_0x10[iVar7] = 0.0;
				}
			})
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
				float* pFloatData = this->pActor3C_0x0->field_0x2c;

				if (pFloatData == (float*)0x0) {
					fVar11 = 0.0f;
				}
				else {
					fVar11 = pFloatData[iVar7];
				}

				fVar13 = fVar13 + fVar11;
			}

			for (; iVar5 < count; iVar5 = iVar5 + 1) {
				float* pFloatData = this->pActor3C_0x0->field_0x2c;

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
					float* pFloatData = this->pActor3C_0x0->field_0x2c;

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
	
				float* pFloatData = this->pActor3C_0x0->field_0x2c;

				if (pFloatData == (float*)0x0) {
					fVar11 = 0.0f;
				}
				else {
					fVar11 = pFloatData[iVar8];
				}

				pFloatData = this->pActor3C_0x0->field_0x2c;

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
				float* pFloatData = this->pActor3C_0x0->field_0x2c;

				if (pFloatData == (float*)0x0) {
					fVar9 = 0.0f;
				}
				else {
					fVar9 = pFloatData[iVar5];
				}

				this->field_0x10[iVar5] = (fVar11 * this->field_0x10[iVar5]) / fVar13 + fVar9;
			}

			for (; iVar6 < count; iVar6 = iVar6 + 1) {
				float* pFloatData = this->pActor3C_0x0->field_0x2c;

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

void CBehaviourPlatformSlab::Create(ByteCode* pByteCode)
{
	int* piVar1;
	char* pcVar2;
	int iVar3;
	float pCVar2;
	float fVar4;

	ACTOR_LOG(LogLevel::Info, "CBehaviourPlatformSlab::Create");

	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * sizeof(S_MOVING_PLATFORM_TARGET_STREAM);
	}
	this->pTargetStream = reinterpret_cast<S_MOVING_PLATFORM_TARGET_STREAM*>(piVar1);

	pcVar2 = pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pcVar2 + sizeof(S_STREAM_EVENT_CAMERA);
	this->streamEventCamera = reinterpret_cast<S_STREAM_EVENT_CAMERA*>(pcVar2);

	pCVar2 = pByteCode->GetF32();
	this->field_0x10 = pCVar2;
	fVar4 = pByteCode->GetF32();
	this->field_0x14 = fVar4;
	fVar4 = pByteCode->GetF32();
	this->field_0x18 = fVar4;
	fVar4 = pByteCode->GetF32();
	this->field_0x1c = fVar4;
	iVar3 = pByteCode->GetS32();
	this->field_0x20 = iVar3;
	return;
}

void CBehaviourPlatformSlab::Init(CActor* pOwner)
{
	S_MOVING_PLATFORM_TARGET_STREAM* pSVar1;
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
	this->streamEventCamera->Manage(this->pOwner);
	this->pOwner->BehaviourSlab_Manage(this);
}

void CBehaviourPlatformSlab::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	S_MOVING_PLATFORM_TARGET_STREAM* pSVar1;
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

void CBehaviourWeighingMachineSlave::Create(ByteCode* pByteCode)
{
	CPathManager* pCVar1;
	int iVar2;
	CPathFollow* pPathFollow;

	pCVar1 = CScene::ptable.g_PathManager_004516a0;

	iVar2 = pByteCode->GetS32();
	if (iVar2 == -1) {
		pPathFollow = (CPathFollow*)0x0;
	}
	else {
		pPathFollow = pCVar1->aPathFollow + iVar2;
	}

	iVar2 = pByteCode->GetS32();
	this->pathFollowReaderAbs.Create(2.0f, 0.0f, pPathFollow, 0, 0, iVar2, 0);
}

void S_STREAM_NTF_TARGET_ONOFF::Reset()
{
	this->flags = this->flags & 1;
	return;
}
