#include "ActorBunch.h"
#include "MemoryStream.h"
#include "ActorManager.h"
#include "EventManager.h"
#include "TimeController.h"
#include "MathOps.h"
#include "WayPoint.h"

void CActorBunch::Create(ByteCode* pByteCode)
{
	CNewFx* pCVar1;
	uint uVar2;
	uint uVar4;
	int meshIndex;
	int iVar5;
	float fVar6;

	CActorWolfen::Create(pByteCode);
	this->field_0xfc8 = pByteCode->GetF32();

	this->nbBunchStreamEntries = pByteCode->GetU32();
	if (this->nbBunchStreamEntries == 0) {
		this->pBunchStreamEntries = (S_BUNCH_STREAM_ENTRY*)0x0;
	}
	else {
		this->pBunchStreamEntries = static_cast<S_BUNCH_STREAM_ENTRY*>(edMemAlloc(TO_HEAP(H_MAIN), this->nbBunchStreamEntries * sizeof(S_BUNCH_STREAM_ENTRY)));
		uVar2 = 0;
		if (this->nbBunchStreamEntries != 0) {
			do {
				this->pBunchStreamEntries[uVar2].field_0x0 = pByteCode->GetU32();
				uVar4 = pByteCode->GetU32();
				if (uVar4 == 0) {
					this->pBunchStreamEntries[uVar2].behaviourId = -1;
				}
				else {
					this->pBunchStreamEntries[uVar2].behaviourId = pByteCode->GetU32();
				}
				this->pBunchStreamEntries[uVar2].zoneId = pByteCode->GetU32();
				uVar4 = pByteCode->GetU32();
				this->pBunchStreamEntries[uVar2].field_0xc = uVar4;
				this->pBunchStreamEntries[uVar2].zoneResult = 2;
				uVar2 = uVar2 + 1;
			} while (uVar2 < this->nbBunchStreamEntries);
		}
	}

	iVar5 = pByteCode->GetS32();
	meshIndex = pByteCode->GetS32();
	this->field_0x1060.staticMeshComponentAdvanced.ResetInternal(iVar5, meshIndex);
	this->field_0x1060.field_0xc = 0.0f;
	this->field_0x1060.field_0x10 = 2.5f;
	this->field_0x1060.field_0x14 = 2.5f;
	this->field_0x1060.field_0x1d0.type = 0xffffffff;
	this->field_0x1060.field_0x1d0.Stop();
	this->field_0x1060.field_0x1dc.type = 0xffffffff;
	this->field_0x1060.field_0x1dc.Stop();

	this->field_0x1060.Create(0xffffffff);

	return;
}

void CActorBunch::Term()
{
	CActorWolfen::Term();

	if (this->pBunchStreamEntries != (S_BUNCH_STREAM_ENTRY*)0x0) {
		edMemFree(this->pBunchStreamEntries);
		this->pBunchStreamEntries = (S_BUNCH_STREAM_ENTRY*)0x0;
	}

	return;
}

void CActorBunch::Manage()
{
	ed_zone_3d* pZone;
	uint uVar2;
	uint uVar3;
	edF32VECTOR4* v0;
	S_BUNCH_STREAM_ENTRY* pSVar4;
	edF32VECTOR4* v0_00;
	CActor** pActorIt;
	int iVar5;
	float fVar6;
	_msg_hit_param msgHitParam;
	CActorsTable actorsTable;
	_msg_hit_param* local_4;
	CEventManager* pEventManager;
	CActor* pHitActor;

	actorsTable.nbEntries = 0;

	CActorWolfen::Manage();

	this->field_0xfd8 = this->field_0xfd8 + GetTimer()->cutsceneDeltaTime;

	if ((this->curBehaviourId == 3) && (this->field_0xfc8 <= this->field_0xfd8)) {
		this->field_0xfd4 = BUNCH_BEHAVIOUR_REPEL;
	}

	pEventManager = CScene::ptable.g_EventManager_006f5080;
	pSVar4 = this->pBunchStreamEntries;
	if ((pSVar4 != (S_BUNCH_STREAM_ENTRY*)0x0) && (this->pAdversary != (CActorFighter*)0x0)) {
		uVar3 = 0;

		while ((uVar3 < this->nbBunchStreamEntries && (this->field_0xfd4 == -1))) {
			if ((pSVar4->behaviourId == -1) || (pSVar4->behaviourId == this->curBehaviourId)) {
				pZone = (ed_zone_3d*)0x0;
				if (pSVar4->zoneId != 0xffffffff) {
					pZone = edEventGetChunkZone(pEventManager->activeChunkId, pSVar4->zoneId);
				}

				uVar2 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, &this->pAdversary->currentLocation, 0);
				if (uVar2 != pSVar4->zoneResult) {
					pSVar4->zoneResult = uVar2;
					if ((uVar2 & 1) == 0) {
						uVar2 = uVar2 | 10;
					}
					else {
						uVar2 = uVar2 | 5;
					}
				}

				if (pSVar4->field_0xc == (pSVar4->field_0xc & uVar2)) {
					this->field_0xfd4 = pSVar4->field_0x0;
				}
			}

			uVar3 = uVar3 + 1;
			pSVar4 = pSVar4 + 1;
		}
	}

	fVar6 = this->field_0xfc0 - GetTimer()->cutsceneDeltaTime;
	this->field_0xfc0 = fVar6;
	if (fVar6 <= 0.0f) {
		this->field_0xfc0 = 0.0f;
		this->field_0xfc4 = 0;
	}

	if ((this->field_0x1060).field_0x4 != 0) {
		actorsTable.nbEntries = 0;
		this->field_0x1060.Manage(&actorsTable);

		if (actorsTable.nbEntries != 0) {
			msgHitParam.projectileType = 9;
			iVar5 = 0;
			msgHitParam.damage = 5.0f;
			msgHitParam.field_0x30 = 80.0f;

			if (0 < actorsTable.nbEntries) {
				pActorIt = actorsTable.aEntries;
				local_4 = &msgHitParam;
				v0_00 = &msgHitParam.field_0x20;
				v0 = &msgHitParam.field_0x40;

				for (; iVar5 < actorsTable.nbEntries; iVar5 = iVar5 + 1) {
					pHitActor = *pActorIt;
					edF32Vector4SubHard(v0_00, &pHitActor->currentLocation, &this->currentLocation);
					msgHitParam.field_0x20.y = 0.5f;
					edF32Vector4SafeNormalize0Hard(v0_00, v0_00);
					edF32Vector4ScaleHard((pHitActor->pCollisionData->pObbPrim->scale).y, v0, &gF32Vector4UnitY);
					edF32Vector4AddHard(v0, v0, &pHitActor->currentLocation);
					DoMessage(pHitActor, MESSAGE_KICKED, local_4);

					pActorIt = pActorIt + 1;
				}
			}
		}

		(this->field_0x1060).actorsTable.nbEntries = 0;
	}

	return;
}

void CActorBunch::Reset()
{
	CBehaviour* pBehaviour;

	CActorWolfen::Reset();

	pBehaviour = GetBehaviour(BUNCH_BEHAVIOUR_REPEL);
	if (pBehaviour != (CBehaviour*)0x0) {
		CBehaviourBunch* pBehaviourBunch = static_cast<CBehaviourBunch*>(GetBehaviour(BUNCH_BEHAVIOUR_REPEL));
		pBehaviourBunch->Reset();
	}

	pBehaviour = GetBehaviour(BUNCH_BEHAVIOUR_FENCE);
	if (pBehaviour != (CBehaviour*)0x0) {
		CBehaviourBunch* pBehaviourBunch = static_cast<CBehaviourBunch*>(GetBehaviour(BUNCH_BEHAVIOUR_FENCE));
		pBehaviourBunch->Reset();
	}

	return;
}

CBehaviour* CActorBunch::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == BUNCH_BEHAVIOUR_FENCE) {
		pBehaviour = new CBehaviourBunchFence;
	}
	else {
		if (behaviourType == BUNCH_BEHAVIOUR_REPEL) {
			pBehaviour = new CBehaviourBunchRepel;
		}
		else {
			if (behaviourType == FIGHTER_BEHAVIOUR_PROJECTED) {
				pBehaviour = new CBehaviourBunchFighterProjected;
			}
			else {
				if (behaviourType == FIGHTER_BEHAVIOUR_DEFAULT) {
					pBehaviour = new CBehaviourFighterBunch;
				}
				else {
					pBehaviour = CActorWolfen::BuildBehaviour(behaviourType);
				}
			}
		}
	}

	return pBehaviour;
}

StateConfig CActorBunch::gStateCfg_BUN[14] = {
	StateConfig(0xE6, 0x02100100), // [0]
	StateConfig(0xE7, 0x02000100), // [1]
	StateConfig(0xE8, 0x02000100), // [2]
	StateConfig(0xE9, 0x02000100), // [3]
	StateConfig(0xEA, 0x02100100), // [4]
	StateConfig(0xEB, 0x02000100), // [5]
	StateConfig(0xEC, 0x02000100), // [6]
	StateConfig(0xED, 0x02000100), // [7]
	StateConfig(0x15, 0x00100100), // [8]
	StateConfig(0xEE, 0x02000000), // [9]
	StateConfig(0xEF, 0x02000000), // [10]
	StateConfig(0xF0, 0x02000000), // [11]
	StateConfig(0xF1, 0x02000100), // [12]
	StateConfig(0xF2, 0x02000100)  // [13]
};

StateConfig* CActorBunch::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 0xb6) {
		pStateConfig = CActorWolfen::GetStateCfg(state);
	}
	else {
		assert((state - 0xb6) < 14);
		pStateConfig = gStateCfg_BUN + state + -0xb6;
	}

	return pStateConfig;
}

uint UINT_ARRAY_00437b40[2] = { 0x3D71F0, 0x3D7330 };

uint CActorBunch::GetBehaviourFlags(int state)
{
	uint behaviourFlags;

	if (state < BUNCH_BEHAVIOUR_REPEL) {
		behaviourFlags = CActorWolfen::GetBehaviourFlags(state);
	}
	else {
		assert(state - BUNCH_BEHAVIOUR_REPEL < 2);
		behaviourFlags = UINT_ARRAY_00437b40[state];
	}

	return behaviourFlags;
}

bool CActorBunch::SetBehaviour(int behaviourId, int newState, int animationType)
{
	bool uVar1;
	bool bSuccess;

	bSuccess = IsFightRelated(behaviourId);
	if ((bSuccess == false) || (uVar1 = IsFightRelated(this->curBehaviourId), uVar1 == false)) {
		this->field_0xfd4 = -1;
		this->field_0xfd8 = 0.0f;
	}

	return CActorWolfen::SetBehaviour(behaviourId, newState, animationType);
}

bool CActorBunch::Can_0x9c()
{
	bool bVar1;

	bVar1 = (this->actorFieldS & 0x60) != 0;
	if (bVar1) {
		bVar1 = this->actorState - BUNCH_STATE_JUMP_CENTRE_PREPARE_B < 5;
	}
	return bVar1;
}

int CActorBunch::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int result;

	if (msg == 0x65) {
		result = 0;
	}
	else {
		result = CActorWolfen::InterpretMessage(pSender, msg, pMsgParam);
	}

	return result;
}

int CActorBunch::Func_0x18c()
{
	int iVar1;

	iVar1 = this->curBehaviourId;
	if ((iVar1 == BUNCH_BEHAVIOUR_REPEL) || (iVar1 == BUNCH_BEHAVIOUR_FENCE)) {
		iVar1 = 0;
	}
	else {
		iVar1 = 1;
	}

	return iVar1;
}

bool CActorBunch::AquareAdversaryB(CActor* param_2)
{
	IMPLEMENTATION_GUARD();
	return false;
}

void CActorBunch::FUN_003ded20(bool bPlayImpact)
{
	CActor* pCVar1;
	uint uVar2;
	CLifeInterface* uVar3;
	long unaff_s0;
	float fVar3;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	uVar2 = FUN_0031b4d0(this->actorState);

	if ((uVar2 == 4) && ((this->hitFlags & 0x10) == 0)) {
		if ((this->flags & 0x1000) == 0) {
			local_10 = this->rotationQuat;
		}
		else {
			SetVectorFromAngleY(this->rotationEuler.y, &local_10);
		}

		if (fabsf(this->field_0x6a0.y) == 1.0f) {
			pCVar1 = this->field_0x634;

			if ((pCVar1->flags & 0x1000) == 0) {
				local_20 = (pCVar1->rotationQuat);
			}
			else {
				SetVectorFromAngleY((pCVar1->rotationEuler).y, &local_20);
			}
		}
		else {
			local_20.x = this->field_0x6a0.x;
			local_20.z = this->field_0x6a0.z;
			local_20.w = this->field_0x6a0.w;
			local_20.y = 0.0f;

			edF32Vector4NormalizeHard(&local_20, &local_20);
		}

		fVar3 = edF32Vector4DotProductHard(&local_10, &local_20);
		if (fVar3 < 0.5f) {
			if (this->distanceToGround <= 0.3f) {
				SetState(0x18, -1);
			}
			else {
				SetState(0x1b, -1);
			}

			if (bPlayImpact != true) {
				return;
			}

			PlayImpactFx(&this->field_0x690, &this->field_0x6a0, 0, true);

			return;
		}
	}

	uVar2 = _SV_HIT_GetHitZoneFromImpact(&this->fighterAnatomyZones, &this->field_0x690, &this->currentLocation);
	if ((uVar2 == 2) || (uVar2 == 1)) {
		this->field_0xfc0 = 1.0f;
		this->field_0xfc4 = this->field_0xfc4 + 1;
		if (1 < this->field_0xfc4) {
			unaff_s0 = 1;
		}
	}
	else {
		if (uVar2 == 0) {
			this->hitDamage = this->hitDamage * 1.6f;
			unaff_s0 = 1;
		}
	}

	LifeDecrease(this->hitDamage);

	this->hitDamage = 0.0f;
	if (unaff_s0 != 1) {
		uVar3 = GetLifeInterface();
		fVar3 = uVar3->GetValue();
		if (0.0f < fVar3) goto LAB_003df018;
	}

	this->field_0xfc0 = 0.0f;
	this->field_0xfc4 = 0;

	SetLookingAtOff();
	SetState(0x4f, -1);
LAB_003df018:

	if (bPlayImpact == true) {
		PlayImpactFx(&this->field_0x690, &this->field_0x6a0, 0, false);
	}

	return;
}

void CActorBunch::FUN_003dea20()
{
	int iVar1;
	float fVar2;
	bool bVar3;
	CBehaviourBunch* pCVar4;
	CWayPoint* pCVar5;
	undefined4 uVar6;
	float fVar7;
	CActorMovParamsIn movParamsIn;
	CActorMovParamsOut movParamsOut;
	edF32VECTOR4 local_10;
	CActorFighter* pFighter;
	s_fighter_combo* pFighterCombo;

	iVar1 = this->curBehaviourId;
	bVar3 = true;
	if ((iVar1 != BUNCH_BEHAVIOUR_REPEL) && (iVar1 != BUNCH_BEHAVIOUR_FENCE)) {
		bVar3 = false;
	}

	if (bVar3) {
		pCVar4 = static_cast<CBehaviourBunch*>(GetBehaviour(this->curBehaviourId));
		pCVar5 = pCVar4->GetWayPoint();
		local_10.xyz = pCVar5->location;
		local_10.w = 1.0f;
		SV_UpdateOrientationToPosition2D(3.141593f, &this->pAdversary->currentLocation);
		movParamsOut.flags = 0;
		movParamsIn.pRotation = (edF32VECTOR4*)0x0;
		movParamsIn.speed = 0.0f;
		movParamsIn.flags = 0x101;
		movParamsIn.rotSpeed = static_cast<float>(GetRunRotSpeed());
		movParamsIn.flags = movParamsIn.flags | 2;
		movParamsIn.acceleration = static_cast<float>(GetRunAcceleration());
		movParamsIn.speed = static_cast<float>(GetRunSpeed());
		movParamsIn.flags = movParamsIn.flags | 0x400;
		SV_AUT_MoveTo(&movParamsOut, &movParamsIn, &local_10);

		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		if (this->dynamic.speed != 0.0f) {
			fVar7 = GetAngleYFromVector(&this->rotationQuat);
			RunInternal(fVar7, &this->dynamic.rotationQuat);
		}

		if ((this->combatFlags_0xb78 & 0x20) == 0) {
			pFighter = this->pAdversary;
			if ((pFighter != (CActorFighter*)0x0) && (bVar3 = pFighter->FUN_0031b790(pFighter->actorState), bVar3 != false)) {
				pFighter = this->pAdversary;
				pFighterCombo = pFighter->pFighterCombo;
				if ((pFighterCombo != (s_fighter_combo*)0x0) &&
					(((pFighterCombo->field_0x4.field_0x0ushort & 0x400U) == 0 &&
						(fVar7 = pFighter->currentLocation.x - this->currentLocation.x,
							fVar2 = pFighter->currentLocation.z - this->currentLocation.z,
							(fVar7 * fVar7 + fVar2 * fVar2) - ((this->pCollisionData)->pObbPrim->scale).z * 1.5f <= LOAD_POINTER_CAST(s_fighter_blow*, pFighterCombo->actionHash.pData)->canActivateRange)))) {
					SetState(BUNCH_STATE_JUMP_CENTRE_PREPARE_B, 0xffffffff);
					return;
				}
			}

			if (movParamsOut.moveVelocity <= 0.45f) {
				uVar6 = pCVar4->GetSpecialState();
				SetState(uVar6, 0xffffffff);
			}
		}
		else {
			SetState(BUNCH_STATE_JUMP_CENTRE_PREPARE_B, 0xffffffff);
		}
	}

	return;
}

void CActorBunch::FUN_003de8e0(int nextState, int param_3)
{
	bool bVar1;
	bool bVar2;
	uint dynFlags;
	float fVar4;
	float fVar5;

	dynFlags = 0x40029;
	bVar1 = false;
	if (param_3 == 1) {
		dynFlags = 0x4002b;
		if (((this->pCollisionData)->flags_0x4 & 2) != 0) {
			bVar1 = true;
		}
	}
	else {
		if ((this->field_0xfe0).field_0x40 <= this->timeInAir) {
			bVar1 = true;
		}
	}
	this->field_0xfe0.Integrate(GetTimer()->cutsceneDeltaTime);
	bVar2 = this->field_0xfe0.IsFinished();

	if (bVar2 == false) {
		fVar4 = (this->field_0xfe0).field_0x60.y;
		fVar5 = (this->field_0xfe0).field_0x60.z;
		dynFlags = dynFlags | 0x1c000;
		this->dynamicExt.instanceIndex.x = (this->field_0xfe0).field_0x60.x;
		this->dynamicExt.instanceIndex.y = fVar4;
		this->dynamicExt.instanceIndex.z = fVar5;
		this->dynamic.field_0x4c = this->dynamic.field_0x4c | 0x1c000;
	}
	else {
		this->dynamic.speed = 0.0f;
	}

	ManageDyn(4.0f, dynFlags, (CActorsTable*)0x0);

	if (bVar1) {
		SetState(nextState, 0xffffffff);
	}

	return;
}

CShockWaveBunch::CShockWaveBunch()
{
	this->field_0x1c = 1.0f;
	this->field_0xc = 1.0f;
	this->field_0x10 = 30.0f;
	this->field_0x14 = 10.0f;
	this->field_0x18 = 1.0f;
	this->field_0x20c = 0.0f;
	this->pOwner = (CActorBunch*)0x0;
	this->field_0x4 = 0;
	this->field_0x8 = 0;
	this->field_0x40 = 0.0f;
	this->field_0x44 = 1.0f;
	(this->actorsTable).nbEntries = 0;

	return;
}

void CShockWaveBunch::Create(uint defaultId)
{
	uint uVar3;

	uVar3 = 0;

	do {
		this->field_0x1e8[uVar3].type = 0xffffffff;
		this->field_0x1e8[uVar3].Stop();
		uVar3 = uVar3 + 1;
	}
	while (uVar3 < 3);

	return;
}

struct astruct_20
{
	CActor* field_0x0;
	edF32VECTOR4 field_0x10;
	float field_0x20;
};

bool FUN_003e1130(CActor* param_1, void* pParams)
{
	float fVar1;
	float fVar2;
	bool bVar3;
	uint uVar5;

	astruct_20* param_2 = reinterpret_cast<astruct_20*>(pParams);

	if (param_1 != param_2->field_0x0) {
		uVar5 = param_1->GetStateFlags(param_1->actorState);
	
		if (((uVar5 & 1) == 0) && ((param_1->flags & 4) != 0)) {
			bVar3 = param_1->IsKindOfObject(4);
			if ((bVar3 != false) &&
				(fVar1 = (param_2->field_0x10).x - (param_1->currentLocation).x, fVar2 = (param_2->field_0x10).z - (param_1->currentLocation).z,
					fVar1 * fVar1 + fVar2 * fVar2 <= param_2->field_0x20 * param_2->field_0x20)) {
				return true;
			}
		}
	}

	return false;
}

void CShockWaveBunch::Manage(CActorsTable* pTable)
{
	edNODE* peVar1;
	CNewFx* pCVar2;
	edF32VECTOR4* peVar3;
	CActorManager* pCVar4;
	bool bVar5;
	edF32VECTOR4* peVar6;
	edF32VECTOR4* peVar8;
	byte alpha;
	CActor** pCVar9;
	uint uVar10;
	CFxHandleExt* pCVar11;
	int iVar12;
	int iVar13;
	float fVar14;
	float fVar15;
	edF32MATRIX4 eStack608;
	edF32VECTOR4 eStack544;
	edF32VECTOR4 local_210;
	edF32MATRIX4 eStack512;
	edF32VECTOR4 eStack448;
	edF32VECTOR4 local_1b0;
	CActorsTable local_1a0;
	astruct_20 local_90;
	edF32VECTOR4 local_60;
	edF32MATRIX4 eStack80;
	edF32VECTOR4 local_10;

	pCVar4 = CScene::ptable.g_ActorManager_004516a4;
	if (this->field_0x4 != 0) {
		local_10 = {};
		local_10.xyz = this->field_0x20.xyz;
		local_10.w = this->field_0x40;
		fVar14 = this->field_0x40 + this->field_0x48 * GetTimer()->cutsceneDeltaTime;
		this->field_0x40 = fVar14;
		iVar13 = this->field_0x4;
		fVar15 = fVar14 / this->field_0x1c;
		if (iVar13 == 3) {
			this->staticMeshComponentAdvanced.Func_0x30((edF32MATRIX4*)0x0, GetTimer()->cutsceneDeltaTime);
			edF32Vector4ScaleHard(this->field_0x40, &eStack544, &this->field_0x30);
			edF32Matrix4RotateYHard(2.094395f, &eStack608, &gF32Matrix4Unit);
			uVar10 = 0;
			pCVar11 = this->field_0x1e8;
			do {
				if (pCVar11->IsValid()) {
					edF32Vector4AddHard(&local_210, &eStack544, &this->field_0x20);
					pCVar11->SetPosition(&local_210);
					edF32Matrix4MulF32Vector4Hard(&eStack544, &eStack608, &eStack544);
				}
				uVar10 = uVar10 + 1;
				pCVar11 = pCVar11 + 1;
			} while (uVar10 < 3);

			bVar5 = this->staticMeshComponentAdvanced.HasMesh();
			if (bVar5 == false) {
				this->field_0x4 = 0;
				this->field_0x1d0.Stop();
				this->field_0x1dc.Stop();
				uVar10 = 0;
				pCVar11 = this->field_0x1e8;
				do {
					pCVar11->Stop();
					uVar10 = uVar10 + 1;
					pCVar11 = pCVar11 + 1;
				} while (uVar10 < 3);
			}
		}
		else {
			if (iVar13 == 2) {
				local_60.y = 1.0f;
				if (this->field_0x20c != 0.0f) {
					local_60.y = edFIntervalUnitDstLERP(fVar14, this->field_0xc, this->field_0x10);
					local_60.y = 1.0f - local_60.y;
					fVar14 = local_60.y * this->field_0x20c;
					if (1.0f < fVar14) {
						peVar1 = this->staticMeshComponentAdvanced.pMeshTransformParent;
						if (peVar1 != (edNODE*)0x0) {
							ed3DHierarchyNodeSetAlpha(peVar1, 0x80);
						}
					}
					else {
						peVar1 = this->staticMeshComponentAdvanced.pMeshTransformParent;
						if (peVar1 != (edNODE*)0x0) {
							fVar14 = fVar14 * 128.0f;
							if (fVar14 < 2.147484e+09f) {
								alpha = (byte)static_cast<int>(fVar14);
							}
							else {
								alpha = (byte)static_cast<int>(fVar14 - 2.147484e+09f);
							}

							ed3DHierarchyNodeSetAlpha(peVar1, alpha);
						}
					}
				}
				local_60.w = 1.0f;
				local_60.x = fVar15;
				local_60.z = fVar15;
				edF32Matrix4ScaleHard(&eStack80, &gF32Matrix4Unit, &local_60);
				edF32Matrix4TranslateHard(&eStack80, &eStack80, &this->field_0x20);
				this->staticMeshComponentAdvanced.Func_0x30(&eStack80, GetTimer()->cutsceneDeltaTime);
				edF32Vector4ScaleHard(this->field_0x40, &eStack448, &this->field_0x30);
				edF32Matrix4RotateYHard(2.094395f, &eStack512, &gF32Matrix4Unit);
				uVar10 = 0;
				pCVar11 = this->field_0x1e8;
				do {
					if (pCVar11->IsValid()) {
						edF32Vector4AddHard(&local_1b0, &eStack448, &this->field_0x20);
						pCVar11->SetPosition(&local_1b0);
						edF32Matrix4MulF32Vector4Hard(&eStack448, &eStack512, &eStack448);
					}

					uVar10 = uVar10 + 1;
					pCVar11 = pCVar11 + 1;
				} while (uVar10 < 3);

				if (this->field_0x10 <= this->field_0x40) {
					if ((this->field_0x8 & 1) == 0) {
						this->field_0x1dc.Stop();
						uVar10 = 0;
						pCVar11 = this->field_0x1e8;
						do {
							pCVar11->Stop();
							uVar10 = uVar10 + 1;
							pCVar11 = pCVar11 + 1;
						} while (uVar10 < 3);

						this->staticMeshComponentAdvanced.Func_0x2c(0.0f);
						bVar5 = this->staticMeshComponentAdvanced.HasMesh();
						if (bVar5 == false) {
							this->field_0x4 = 0;
						}
					}
					else {
						this->field_0x4 = 3;
						this->field_0x48 = 0.0f;
					}
				}
			}
			else {
				if (iVar13 == 1) {
					local_60.y = 1.0f;
					local_60.w = 1.0f;
					local_60.x = fVar15;
					local_60.z = fVar15;
					edF32Matrix4ScaleHard(&eStack80, &gF32Matrix4Unit, &local_60);
					edF32Matrix4TranslateHard(&eStack80, &eStack80, &this->field_0x20);
					this->staticMeshComponentAdvanced.Func_0x30(&eStack80, GetTimer()->cutsceneDeltaTime);
					if (this->field_0xc <= this->field_0x40) {
						this->field_0x4 = 2;
						this->field_0x48 = this->field_0x14;
						this->field_0x1d0.Stop();
						uVar10 = 0;
						pCVar11 = this->field_0x1e8;
						do {
							pCVar11->InitPositionRotation((edF32VECTOR4*)0x0, (edF32VECTOR4*)0x0);
							uVar10 = uVar10 + 1;
							pCVar11 = pCVar11 + 1;
						} while (uVar10 < 3);
					}
				}
			}
		}

		if ((this->field_0x4 == 3) || (this->field_0x4 == 2)) {
			local_90.field_0x0 = this->pOwner;
			local_90.field_0x10 = (this->field_0x20);
			local_90.field_0x20 = this->field_0x40;
			if (pTable == (CActorsTable*)0x0) {
				local_1a0.nbEntries = 0;
				pCVar4->cluster.GetActorsIntersectingSphereWithCriterion(&local_1a0, &local_10, FUN_003e1130, &local_90);
				this->actorsTable.MergeWithTable(&local_1a0);
			}
			else {
				pCVar4->cluster.GetActorsIntersectingSphereWithCriterion(pTable, &local_10, FUN_003e1130, &local_90);
				iVar13 = 0;
				CActor** ppActor = this->actorsTable.aEntries;
				if (0 < (this->actorsTable).nbEntries) {
					do {
						iVar12 = 0;
						bVar5 = false;
						pCVar9 = pTable->aEntries;
						while ((iVar12 < pTable->nbEntries && (!bVar5))) {
							if (*pCVar9 == *ppActor) {
								pTable->Pop(iVar12);
								bVar5 = true;
							}
							else {
								pCVar9 = pCVar9 + 1;
								iVar12 = iVar12 + 1;
							}
						}
						iVar13 = iVar13 + 1;
						ppActor = ppActor + 1;
					} while (iVar13 < (this->actorsTable).nbEntries);
				}

				this->actorsTable.MergeWithTable(pTable);
			}
		}
	}

	return;
}

void CShockWaveBunch::Draw()
{
	return;
}

void CShockWaveBunch::Term(float param_1)
{
	bool bVar1;
	uint uVar2;

	if (param_1 == 0.0f) {
		this->field_0x4 = 0;
		this->field_0x8 = 0;
		this->staticMeshComponentAdvanced.Func_0x2c(0.0f);
		this->field_0x1d0.Stop();
		this->field_0x1dc.Stop();
		uVar2 = 0;
		do {
			this->field_0x1e8[uVar2].Stop();
			uVar2 = uVar2 + 1;
		} while (uVar2 < 3);
	}
	else {
		this->field_0x4 = 3;
		this->field_0x8 = 0;
		this->field_0x48 = 0.0f;

		bVar1 = this->staticMeshComponentAdvanced.HasMesh();
		if (bVar1 != false) {
			this->staticMeshComponentAdvanced.Func_0x2c(param_1);
		}
	}

	return;
}

void CShockWaveBunch::InitState(CActorBunch* pBunch, edF32VECTOR4* param_3, int param_4)
{
	CActorBunch* pCVar1;
	float fVar2;
	float fVar3;
	float fVar4;

	if (this->field_0x4 == 0) {
		this->pOwner = pBunch;
		this->field_0x4 = 1;

		if (param_3 == (edF32VECTOR4*)0x0) {
			pCVar1 = this->pOwner;
			this->field_0x20 = pCVar1->currentLocation;
		}
		else {
			this->field_0x20 = *param_3;
		}

		if (param_4 == 0) {
			this->field_0x8 = 1;
		}
		else {
			this->field_0x8 = 0;
		}

		this->field_0x40 = 0.0f;
		this->field_0x48 = this->field_0x18;
		(this->actorsTable).nbEntries = 0;
		this->staticMeshComponentAdvanced.Func_0x28(0.0f, 0.0f);
		this->field_0x1d0.InitPositionRotation(&this->field_0x20, &gF32Vector4Zero);
		this->field_0x1dc.InitPositionRotation(&this->field_0x20, &gF32Vector4Zero);
	}

	return;
}

int CBehaviourBunchFighterProjected::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int result;

	_msg_hit_param* pHitParam = reinterpret_cast<_msg_hit_param*>(pMsgParam);

	if (((msg == 2) && (((result = pHitParam->projectileType, result == 10 || (result == 8)) || (result == 7)))) && (this->pOwner->FUN_0031cdb0() != false)) {
		this->pOwner->PlayImpactFx(&pHitParam->field_0x40, &pHitParam->field_0x20, (pHitParam->flags & 1) != 0, false);
		result = 1;
	}
	else {
		result = CBehaviourWolfenFighterProjected::InterpretMessage(pSender, msg, pMsgParam);
	}

	return result;
}

void CBehaviourFighterBunch::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourFighterWolfen::Begin(pOwner, newState, newAnimationType);

	this->behaviourId = BUNCH_BEHAVIOUR_REPEL;

	CActorBunch* pBunch = static_cast<CActorBunch*>(pOwner);

	pBunch->field_0xfc0 = 0.0f;
	pBunch->field_0xfc4 = 0;

	return;
}

int CBehaviourFighterBunch::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int result;

	if (msg == 0x65) {
		result = 0;
	}
	else {
		result = CBehaviourFighterWolfen::InterpretMessage(pSender, msg, pMsgParam);
	}

	return result;
}

void CBehaviourFighterBunch::_ManageHit(bool bPlayImpact)
{
	CActorBunch* pBunch;

	pBunch = (CActorBunch*)this->pOwner;
	pBunch->hitFlags = pBunch->hitFlags & 0xfffffff0;
	pBunch->FUN_003ded20(bPlayImpact);

	return;
}

void CBehaviourFighterBunch::ManageExit()
{
	int iVar1;
	bool bVar2;
	CLifeInterface* pCVar3;
	StateConfig* pSVar4;
	uint uVar5;
	CBehaviour* pCVar6;
	ulong uVar7;
	float fVar8;
	CActorFighter* pAdversary;
	CActorBunch* pBunch;

	pAdversary = this->pOwner->pAdversary;
	if (pAdversary != (CActorFighter*)0x0) {
		pCVar3 = pAdversary->GetLifeInterface();
		fVar8 = pCVar3->GetValue();
		if (0.0f < fVar8) {
			this->behaviourId = BUNCH_BEHAVIOUR_REPEL;
			goto LAB_003ddd00;
		}
	}

	this->behaviourId = this->pOwner->subObjA->defaultBehaviourId;

LAB_003ddd00:
	CBehaviourFighterWolfen::ManageExit();

	pBunch = static_cast<CActorBunch*>(this->pOwner);

	if (((((pBunch->GetStateFlags(pBunch->actorState) & 0x100000) != 0) && (pBunch = static_cast<CActorBunch*>(this->pOwner), pBunch->curBehaviourId == 3)) && (pBunch->field_0xfd4 != -1)) && (pBunch->field_0xfd4 != 3)) {
		uVar5 = FUN_001fad80(this->currentCommandId);
		if (pBunch->field_0xfd4 == -1) {
			pCVar6 = (CBehaviour*)0x0;
		}
		else {
			pCVar6 = pBunch->GetBehaviour(pBunch->field_0xfd4);
		}

		if (((pCVar6 != (CBehaviour*)0x0) && ((bVar2 = IsCommandFinished(uVar5), bVar2 != false || (uVar7 = FUN_001f7a80(uVar5), uVar7 != 0)))) &&
			(iVar1 = pBunch->field_0xfd4, iVar1 != -1)) {
			if (iVar1 == 3) {
				pBunch->SetFightBehaviour();
			}
			else {
				pBunch->SetBehaviour(iVar1, -1, -1);
			}
		}
	}

	return;
}

void CBehaviourBunch::Init(CActor* pOwner)
{
	this->pOwner = (CActorBunch*)0x0;

	return;
}

void CBehaviourBunch::Manage()
{
	CActorFighter* pCVar1;
	bool bVar4;
	CBehaviourBunch* pCVar5;
	int iVar6;
	undefined4 uVar7;
	float puVar10;
	float puVar9;
	float puVar11;
	float fVar9;
	float fVar10;
	float fVar11;
	edF32VECTOR4 eStack192;
	edF32VECTOR4 local_b0;
	edF32MATRIX4 eStack160;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 local_50;
	edF32MATRIX4 eStack64;
	CActorBunch* pBunch;

	pBunch = this->pOwner;
	switch (pBunch->actorState) {
	case BUNCH_STATE_JUMP_CENTRE_PREPARE_A:
		pBunch->FUN_003dea20();
		break;
	case BUNCH_STATE_JUMP_CENTRE_PREPARE_B:
		fVar10 = pBunch->field_0x7b0 + pBunch->field_0x7b4 * GetTimer()->cutsceneDeltaTime;
		pBunch->field_0x7b0 = fVar10;
		edF32Matrix4RotateYHard(fVar10, &eStack64, &gF32Matrix4Unit);
		edF32Matrix4MulF32Vector4Hard(&local_50, &eStack64, &pBunch->field_0x7a0);
		pBunch->rotationQuat = local_50;
		pBunch->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		if (pBunch->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			pBunch->SetState(BUNCH_STATE_JUMP_CENTRE, 0xffffffff);
		}
		break;
	case BUNCH_STATE_JUMP_CENTRE:
		pBunch->FUN_003de8e0(0xc1, 0);
		break;
	case 0xc1:
		pBunch->FUN_003de8e0(0xc2, 1);
		break;
	case 0xc2:
		pBunch->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		if (pBunch->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			pCVar5 = static_cast<CBehaviourBunch*>(pBunch->GetBehaviour(pBunch->curBehaviourId));
			pCVar1 = pBunch->pAdversary;
			if (pCVar1 != (CActorFighter*)0x0) {
				edF32Vector4SubHard(&eStack96, &pCVar1->currentLocation, &pBunch->currentLocation);
				edF32Vector4SafeNormalize1Hard(&eStack96, &eStack96);

				fVar10 = edF32Vector4DotProductHard(&eStack96, &pBunch->rotationQuat);
				if (fVar10 < 0.866f) {
					edF32Vector4SubHard(&eStack192, &pBunch->pAdversary->currentLocation, &pBunch->currentLocation);
					edF32Vector4SafeNormalize1Hard(&eStack192, &eStack192);
					edF32Vector4CrossProductHard(&pBunch->field_0x7a0, &pBunch->rotationQuat, &eStack192);
					iVar6 = pBunch->GetIdMacroAnim(0xf2);
					if (iVar6 < 0) {
						pBunch->field_0x7b4 = 0.0f;
					}
					else {
						fVar10 = pBunch->pAnimationController->GetAnimLength(iVar6, 1);
						pBunch->field_0x7b4 = fVar10;
					}

					puVar9 = edF32Vector4DotProductHard(&eStack192, &pBunch->rotationQuat);
					if (1.0f < puVar9) {
						puVar11 = 1.0f;
					}
					else {
						puVar11 = -1.0f;
						if (-1.0f <= puVar9) {
							puVar11 = puVar9;
						}
					}

					fVar10 = acosf(puVar11);
					pBunch->field_0x7b4 = fVar10 / pBunch->field_0x7b4;
					if (0.0f <= pBunch->field_0x7a0.y) {
						puVar10 = 1.0f;
					}
					else {
						puVar10 = -1.0f;
					}

					pBunch->field_0x7b4 = pBunch->field_0x7b4 * puVar10;
					pBunch->field_0x7b0 = 0.0f;
					pBunch->field_0x7a0 = pBunch->rotationQuat;

					if (pBunch->field_0x7b4 < 0.0f) {
						pBunch->SetState(0xc3, 0xf3);
						return;
					}

					pBunch->SetState(0xc3, 0xf2);
					return;
				}
			}

			pBunch->SetState(pCVar5->GetSpecialState(), 0xffffffff);
		}
		break;
	case 0xc3:
		uVar7 = GetSpecialState();
		pBunch = this->pOwner;
		fVar10 = pBunch->field_0x7b0 + pBunch->field_0x7b4 * GetTimer()->cutsceneDeltaTime;
		pBunch->field_0x7b0 = fVar10;
		edF32Matrix4RotateYHard(fVar10, &eStack160, &gF32Matrix4Unit);
		edF32Matrix4MulF32Vector4Hard(&local_b0, &eStack160, &pBunch->field_0x7a0);
		pBunch->rotationQuat = local_b0;
		pBunch->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		if (pBunch->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			pBunch->SetState(uVar7, 0xffffffff);
		}
	}

	return;
}

CAM_QUAKE CAM_QUAKE_00426ca0 = {
	{
		0.05f, 0.25f, 0.05f, 0.0f
	},
	{ 
		20.0f, 20.0f, 20.0f, 0.0f 
	},
	8,
	0.35f,
	0.1f,
	0.05f
};

void CBehaviourBunch::InitState(int newState)
{
	CBehaviourBunch* pBehaviourBunch;
	CWayPoint* pWayPoint;
	CBehaviourBunch* pCVar2;
	StateConfig* pSVar1;
	int iVar2;
	CBehaviourBunch* pCVar3;
	float puVar9;
	float fVar3;
	float puVar10;
	float puVar11;
	float fVar4;
	float fVar5;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 local_50;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 local_30;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;
	CActorBunch* pBunch;

	if (newState == 0xc2) {
		CScene::ptable.g_CameraManager_0045167c->SetEarthQuake(&CAM_QUAKE_00426ca0);
	}
	else {
		if (newState == BUNCH_STATE_JUMP_CENTRE) {
			pBunch = this->pOwner;
			pCVar3 = static_cast<CBehaviourBunch*>(pBunch->GetBehaviour(pBunch->curBehaviourId));
			pWayPoint = pCVar3->GetWayPoint();
			local_50.xyz = pWayPoint->location;
			local_50.w = 1.0f;
			edF32Vector4SubHard(&eStack64, &local_50, &pBunch->currentLocation);
			pBunch->field_0xfe0.BuildFromAccelDistAmplitude(eStack64.y + 3.0f, &CDynamicExt::gForceGravity, &eStack64, 1);
			pSVar1 = pBunch->GetStateCfg(pBunch->actorState);
			iVar2 = pBunch->GetIdMacroAnim(pSVar1->animId);
			if (iVar2 < 0) {
				fVar4 = 0.0f;
			}
			else {
				fVar4 = pBunch->pAnimationController->GetAnimLength(iVar2, 1);
			}

			fVar3 = (pBunch->field_0xfe0).field_0x40;
			pBunch->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(fVar3 / fVar4, 0);
		}
		else {
			if (newState == BUNCH_STATE_JUMP_CENTRE_PREPARE_B) {
				pBunch = this->pOwner;
				pCVar2 = static_cast<CBehaviourBunch*>(pBunch->GetBehaviour(pBunch->curBehaviourId));
				pBunch->dynamic.speed = 0.0f;
				pWayPoint = pCVar2->GetWayPoint();
				local_30.xyz = pWayPoint->location;
				local_30.w = 1.0f;
				pSVar1 = pBunch->GetStateCfg(pBunch->actorState);
				iVar2 = pSVar1->animId;
				edF32Vector4SubHard(&eStack96, &local_30, &pBunch->currentLocation);
				edF32Vector4SafeNormalize1Hard(&eStack96, &eStack96);
				edF32Vector4CrossProductHard(&pBunch->field_0x7a0, &pBunch->rotationQuat, &eStack96);
				iVar2 = pBunch->GetIdMacroAnim(iVar2);
				if (iVar2 < 0) {
					pBunch->field_0x7b4 = 0.0f;
				}
				else {
					fVar3 = pBunch->pAnimationController->GetAnimLength(iVar2, 1);
					pBunch->field_0x7b4 = fVar3;
				}

				puVar10 = edF32Vector4DotProductHard(&eStack96, &pBunch->rotationQuat);
				if (1.0f < puVar10) {
					puVar11 = 1.0f;
				}
				else {
					puVar11 = -1.0f;
					if (-1.0f <= puVar10) {
						puVar11 = puVar10;
					}
				}

				fVar3 = acosf(puVar11);
				pBunch->field_0x7b4 = fVar3 / pBunch->field_0x7b4;
				if (0.0f <= pBunch->field_0x7a0.y) {
					puVar9 = 1.0f;
				}
				else {
					puVar9 = -1.0f;
				}

				pBunch->field_0x7b4 = pBunch->field_0x7b4 * puVar9;
				pBunch->field_0x7b0 = 0.0f;
				pBunch->field_0x7a0 = pBunch->rotationQuat;
			}
			else {
				if (newState == BUNCH_STATE_JUMP_CENTRE_PREPARE_A) {
					pBunch = this->pOwner;
					if (pBunch->curBehaviourId - BUNCH_BEHAVIOUR_REPEL < 2) {
						pBehaviourBunch = static_cast<CBehaviourBunch*>(pBunch->GetBehaviour(pBunch->curBehaviourId));
						pWayPoint = pBehaviourBunch->GetWayPoint();
						local_10.xyz = pWayPoint->location;
						local_10.w = 1.0f;
						edF32Vector4SubHard(&eStack32, &local_10, &pBunch->currentLocation);
						edF32Vector4SafeNormalize1Hard(&eStack32, &eStack32);
						fVar3 = edF32Vector4DotProductHard(&eStack32, &pBunch->rotationQuat);
						if (0.0f <= fVar3) {
							pBunch->fightFlags = pBunch->fightFlags & 0xfffffffb;
						}
						else {
							pBunch->fightFlags = pBunch->fightFlags | 4;
						}
					}

					pBunch->field_0x4e0 = gF32Vector4UnitZ;
				}
			}
		}
	}

	return;
}

void CBehaviourBunch::TermState(int oldState, int newState)
{
	if (oldState == 0xc1) {
		this->pOwner->dynamic.speed = 0.0f;
	}
	else {
		if (oldState == BUNCH_STATE_JUMP_CENTRE) {
			this->pOwner->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
		}
		else {
			if (oldState == BUNCH_STATE_JUMP_CENTRE_PREPARE_A) {
				this->pOwner->dynamic.speed = 0.0f;
			}
		}
	}

	return;
}

void CBehaviourBunch::ManageExit()
{
	CActorBunch* pBunch;
	int iVar2;
	StateConfig* pSVar3;
	uint uVar4;

	pBunch = this->pOwner;

	if ((pBunch->GetStateFlags(pBunch->actorState) & 0x100000) != 0) {
		pBunch = this->pOwner;
		iVar2 = pBunch->field_0xfd4;
		if (((iVar2 != -1) && (iVar2 != pBunch->curBehaviourId)) && (iVar2 != -1)) {
			if (iVar2 == 3) {
				pBunch->SetFightBehaviour();
			}
			else {
				pBunch->SetBehaviour(iVar2, -1, -1);
			}
		}
	}

	return;
}

void CBehaviourBunch::Reset()
{
	return;
}

int CBehaviourBunch::GetSpecialState()
{
	return -1;
}

CWayPoint* CBehaviourBunch::GetWayPoint()
{
	return (this->wayPointStreamRef).Get();
}

_msg_hit_param _msgHit;

void CBehaviourBunchRepel::Create(ByteCode* pByteCode)
{
	CWayPoint* pWayPoint;
	uint uVar1;
	int* piVar2;
	uint* puVar3;
	uint uVar4;
	int meshIndex;
	uint defaultId;
	int iVar5;
	int iVar6;
	uint uVar7;
	int iVar8;
	float fVar9;
	float fVar10;

	this->wayPointStreamRef.index = pByteCode->GetS32();
	this->field_0x10 = pByteCode->GetU32();
	this->field_0x14 = pByteCode->GetF32();
	fVar9 = pByteCode->GetF32();
	fVar10 = pByteCode->GetF32();
	uVar1 = this->field_0x10;
	if (uVar1 == 0) {
		this->field_0xc = (CShockWaveBunch*)0x0;
	}
	else {
		this->field_0xc = new CShockWaveBunch[uVar1];
		uVar1 = 0;
		if (this->field_0x10 != 0) {
			iVar6 = 0;
			do {
				this->field_0xc[uVar1].field_0x1c = 11.0f;
				this->field_0xc[uVar1].field_0xc = 0.15f;
				this->field_0xc[uVar1].field_0x10 = fVar9;
				this->field_0xc[uVar1].field_0x14 = fVar10;
				this->field_0xc[uVar1].field_0x20c = 3.5f;
				uVar1 = uVar1 + 1;
			} while (uVar1 < this->field_0x10);
		}
	}

	this->field_0x1c = pByteCode->GetU32();
	if (this->field_0x1c == 0) {
		this->field_0x18 = (uint*)0x0;
	}
	else {
		this->field_0x18 = new uint[this->field_0x1c];
		uVar1 = 0;
		if (this->field_0x1c != 0) {
			do {
				uVar4 = pByteCode->GetU32();
				this->field_0x18[uVar1] = uVar4;
				uVar1 = uVar1 + 1;
			} while (uVar1 < this->field_0x1c);
		}
	}

	_msgHit.damage = pByteCode->GetF32();
	_msgHit.field_0x30 = pByteCode->GetF32();
	_msgHit.field_0x70 = pByteCode->GetF32();

	(this->field_0x20).x = pByteCode->GetF32();
	(this->field_0x20).y = pByteCode->GetF32();
	(this->field_0x20).z = pByteCode->GetF32();
	(this->field_0x20).w = 0.0f;

	(this->field_0x30).x = pByteCode->GetF32();
	(this->field_0x30).y = pByteCode->GetF32();
	(this->field_0x30).z = pByteCode->GetF32();
	(this->field_0x30).w = 0.0f;

	edF32Vector4NormalizeHard(&this->field_0x20, &this->field_0x20);
	edF32Vector4NormalizeHard(&this->field_0x30, &this->field_0x30);

	_msgHit.field_0x10 = 0.0f;
	_msgHit.projectileType = 7;
	_msgHit.flags = 0x85;
	_msgHit.field_0x50 = 1;

	iVar6 = pByteCode->GetS32();
	meshIndex = pByteCode->GetS32();
	uVar1 = pByteCode->GetU32();
	uVar4 = pByteCode->GetU32();
	defaultId = pByteCode->GetU32();
	uVar7 = 0;
	if (this->field_0x10 != 0) {
		do {
			this->field_0xc[uVar7].staticMeshComponentAdvanced.ResetInternal(iVar6, meshIndex);
			this->field_0xc[uVar7].field_0x1d0.type = uVar1;
			this->field_0xc[uVar7].field_0x1d0.Stop();
			this->field_0xc[uVar7].field_0x1dc.type = uVar1;
			this->field_0xc[uVar7].field_0x1dc.Stop();
			this->field_0xc[uVar7].Create(defaultId);

			uVar7 = uVar7 + 1;
		} while (uVar7 < this->field_0x10);
	}

	return;
}

void CBehaviourBunchRepel::Init(CActor * pOwner)
{
	int iVar1;
	uint uVar2;
	float fVar3;

	iVar1 = pOwner->GetIdMacroAnim(0xe7);
	if (iVar1 < 0) {
		fVar3 = 0.0f;
	}
	else {
		fVar3 = pOwner->pAnimationController->GetAnimLength(iVar1, 1);
	}

	uVar2 = 0;
	if (this->field_0x10 != 0) {
		do {
			this->field_0xc[uVar2].field_0x18 = 0.15f / fVar3;
			uVar2 = uVar2 + 1;
		} while (uVar2 < this->field_0x10);
	}

	this->wayPointStreamRef.Init();

	return;
}

void CBehaviourBunchRepel::Term()
{
	CShockWaveBunch* pShockWaveBunch;
	uint uVar2;

	if (this->field_0xc != (CShockWaveBunch*)0x0) {
		uVar2 = 0;
		if (this->field_0x10 != 0) {
			do {
				pShockWaveBunch = this->field_0xc + uVar2;
				if (pShockWaveBunch->field_0x4 != 0) {
					pShockWaveBunch->Term(0.0f);
				}

				uVar2 = uVar2 + 1;
			} while (uVar2 < this->field_0x10);
		}

		delete[] this->field_0xc;
		this->field_0xc = (CShockWaveBunch*)0x0;
	}

	if (this->field_0x18 != (uint*)0x0) {
		delete[] this->field_0x18;
		this->field_0x18 = (uint*)0x0;
	}

	return;
}

void CBehaviourBunchRepel::Manage()
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	CActorFighter* pCVar3;
	CActor* pReceiver;
	uint zoneId;
	bool bVar4;
	CEventManager* pCVar5;
	CBehaviourBunchRepel* pBehaviourBunchRepel;
	CBehaviourBunch* pCVar7;
	edF32VECTOR4* v1;
	ed_zone_3d* pZone;
	int iVar7;
	CShockWaveBunch* pCVar9;
	int iVar10;
	uint uVar11;
	int iVar12;
	uint uVar13;
	CActor** pCVar14;
	undefined* puVar15;
	undefined* puVar16;
	float fVar17;
	float fVar18;
	float fVar19;
	int local_250;
	edF32VECTOR4 eStack576;
	edF32VECTOR4 local_230;
	edF32MATRIX4 eStack544;
	edF32VECTOR4 eStack480;
	edF32VECTOR4 local_1d0;
	edF32MATRIX4 eStack448;
	edF32VECTOR4 eStack384;
	edF32MATRIX4 eStack368;
	CActorsTable local_130;
	edF32VECTOR4 local_20;
	_msg_hit_param* local_4;
	CActorBunch* pBunch;

	local_20 = gF32Vector4UnitZ;

	pBunch = this->pOwner;
	iVar12 = pBunch->actorState;
	if (iVar12 == 0xb9) {
		pBunch->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		if (pBunch->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			pBunch->SetState(0xb6, -1);
		}
	}
	else {
		if (iVar12 == 0xb8) {
			pBunch->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

			if (pBunch->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
				pBunch->SetState(0xb9, -1);
			}
		}
		else {
			if (iVar12 == 0xb7) {
				pBunch->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

				if (pBunch->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
					pBunch->SetState(0xb8, -1);
				}
			}
			else {
				if (iVar12 == 0xb6) {
					pBehaviourBunchRepel = static_cast<CBehaviourBunchRepel*>(pBunch->GetBehaviour(pBunch->curBehaviourId));
					pBunch->SV_UpdateOrientationToPosition2D(3.141593f, &pBunch->pAdversary->currentLocation);
					pBunch->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
					if (pBunch->field_0xfd4 == -1) {
						if (pBehaviourBunchRepel->field_0x14 <= pBehaviourBunchRepel->pOwner->timeInAir) {
							iVar12 = 0;
							uVar13 = 0;

							if (pBehaviourBunchRepel->field_0x10 != 0) {
								pCVar9 = pBehaviourBunchRepel->field_0xc;
								do {
									if (pCVar9->field_0x4 == 0) {
										iVar12 = iVar12 + 1;
									}
									uVar13 = uVar13 + 1;
									pCVar9 = pCVar9 + 1;
								} while (uVar13 < pBehaviourBunchRepel->field_0x10);
							}

							bVar4 = iVar12 != 0;
						}
						else {
							bVar4 = false;
						}

						if (bVar4) {
							pBunch->SetState(0xb7, -1);
						}
					}
				}
				else {
					CBehaviourBunch::Manage();
				}
			}
		}
	}

	ManageExit();

	pBunch = this->pOwner;
	iVar12 = pBunch->actorState;
	bVar4 = iVar12 == BUNCH_STATE_JUMP_CENTRE_PREPARE_A;
	if (!bVar4) {
		bVar4 = iVar12 - BUNCH_STATE_JUMP_CENTRE_PREPARE_B < 5;
	}
	if ((!bVar4) && ((pBunch->field_0x1060).field_0x4 == 0)) {
		IMPLEMENTATION_GUARD(
		pBunch->field_0x1060.FUN_003e1320(pBunch, &pBunch->currentLocation, 0);)
	}

	pBunch = this->pOwner;
	if (pBunch->pAdversary != (CActorFighter*)0x0) {
		v1 = pBunch->GetAdversaryPos();
		edF32Vector4SubHard(&local_20, v1, &pBunch->currentLocation);
		local_20.y = 0.0f;
		edF32Vector4SafeNormalize0Hard(&local_20, &local_20);
	}

	uVar13 = 0;
	if (this->field_0x10 != 0) {
		do {
			pCVar5 = CScene::ptable.g_EventManager_006f5080;
			local_130.nbEntries = 0;
			this->field_0xc[uVar13].field_0x30 = local_20;
	
			this->field_0xc[uVar13].Manage(&local_130);
			iVar12 = 0;
			if (0 < local_130.nbEntries) {
				pCVar14 = local_130.aEntries;
				do {
					pReceiver = *pCVar14;
					uVar11 = 0;
					bVar4 = false;
					while ((uVar11 < this->field_0x1c && (!bVar4))) {
						zoneId = this->field_0x18[uVar11];
						pZone = (ed_zone_3d*)0x0;
						if (zoneId != 0xffffffff) {
							pZone = edEventGetChunkZone(pCVar5->activeChunkId, zoneId);
						}

						iVar7 = edEventComputeZoneAgainstVertex(pCVar5->activeChunkId, pZone, &pReceiver->currentLocation, 0);
						if (iVar7 == 1) {
							bVar4 = true;
						}

						uVar11 = uVar11 + 1;
					}

					if (!bVar4) {
						edF32Vector4SubHard(&eStack384, &pReceiver->currentLocation, &this->pOwner->currentLocation);
						edF32Vector4NormalizeHard(&eStack384, &eStack384);
						fVar18 = GetAngleYFromVector(&eStack384);
						edF32Matrix4RotateYHard(fVar18, &eStack368, &gF32Matrix4Unit);
						edF32Matrix4MulF32Vector4Hard(&_msgHit.field_0x20, &eStack368, &this->field_0x20);
						edF32Matrix4MulF32Vector4Hard(&_msgHit.field_0x60, &eStack368, &this->field_0x30);
						local_4 = &_msgHit;
						this->pOwner->DoMessage(pReceiver, MESSAGE_KICKED, &_msgHit);
					}

					iVar12 = iVar12 + 1;
					pCVar14 = pCVar14 + 1;
				} while (iVar12 < local_130.nbEntries);
			}

			uVar13 = uVar13 + 1;
		} while (uVar13 < this->field_0x10);
	}

	return;
}

void CBehaviourBunchRepel::Draw()
{
	uint uVar1;

	uVar1 = 0;
	if (this->field_0x10 != 0) {
		do {
			this->field_0xc[uVar1].Draw();
			uVar1 = uVar1 + 1;
		} while (uVar1 < this->field_0x10);
	}

	return;
}

void CBehaviourBunchRepel::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorBunch* pBunch;

	this->pOwner = static_cast<CActorBunch*>(pOwner);
	pBunch = this->pOwner;
	pBunch->flags = pBunch->flags | 0x400;
	this->pOwner->dynamic.speed = 0.0f;

	if (newState == -1) {
		pBunch = this->pOwner;
		pBunch->SetState(BUNCH_STATE_JUMP_CENTRE_PREPARE_A, 0xffffffff);
	}
	else {
		pBunch = this->pOwner;
		pBunch->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourBunchRepel::End(int newBehaviourId)
{
	CActorBunch* pBunch;

	pBunch = this->pOwner;
	pBunch->flags = pBunch->flags & 0xfffffbff;
	if (1 < newBehaviourId - BUNCH_BEHAVIOUR_REPEL) {
		this->pOwner->field_0x1060.Term(0.5f);
	}

	return;
}

void CBehaviourBunchRepel::InitState(int newState)
{
	edF32VECTOR4 eStack16;

	if (newState == 0xb7) {
		uint uVar6 = 0;
		bool bVar1 = false;
		while ((uVar6 < this->field_0x10 && (!bVar1))) {
			if (this->field_0xc[uVar6].field_0x4 == 0) {
				edF32Vector4ScaleHard(2.0f, &eStack16, &this->pOwner->rotationQuat);
				edF32Vector4AddHard(&eStack16, &eStack16, &this->pOwner->currentLocation);
				this->field_0xc[uVar6].InitState(this->pOwner, &eStack16, 1);
				bVar1 = true;
			}
			else {
				uVar6 = uVar6 + 1;
			}
		}
	}
	else {
		CBehaviourBunch::InitState(newState);
	}

	return;
}

void CBehaviourBunchRepel::TermState(int oldState, int newState)
{
	CBehaviourBunch::TermState(oldState, newState);

	return;
}

int CBehaviourBunchRepel::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return msg == MESSAGE_KICKED;
}

void CBehaviourBunchRepel::ManageExit()
{
	int iVar1;
	StateConfig* pSVar2;
	CShockWaveBunch* pShockWaveBunch;
	uint uVar3;
	uint uVar4;
	CActorBunch* pBunch;

	uVar4 = 0;
	uVar3 = 0;
	if (this->field_0x10 != 0) {
		pShockWaveBunch = this->field_0xc;
		do {
			if (pShockWaveBunch->field_0x4 == 0) {
				uVar4 = uVar4 + 1;
			}

			uVar3 = uVar3 + 1;
			pShockWaveBunch = pShockWaveBunch + 1;
		} while (uVar3 < this->field_0x10);
	}

	if (this->field_0x10 == uVar4) {
		pBunch = this->pOwner;

		if ((pBunch->GetStateFlags(pBunch->actorState) & 0x100000) != 0) {
			pBunch = this->pOwner;
			iVar1 = pBunch->field_0xfd4;
			if (((iVar1 != -1) && (iVar1 != pBunch->curBehaviourId)) && (iVar1 != -1)) {
				if (iVar1 == FIGHTER_BEHAVIOUR_DEFAULT) {
					pBunch->SetFightBehaviour();
				}
				else {
					pBunch->SetBehaviour(iVar1, -1, -1);
				}
			}
		}
	}

	return;
}

void CBehaviourBunchRepel::Reset()
{
	uint uVar2;

	uVar2 = 0;
	if (this->field_0x10 != 0) {
		do {
			this->field_0xc[uVar2].Term(0.0f);
			uVar2 = uVar2 + 1;
		} while (uVar2 < this->field_0x10);
	}

	return;
}

int CBehaviourBunchRepel::GetSpecialState()
{
	return 0xb7;
}
