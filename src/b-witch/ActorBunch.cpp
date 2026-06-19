#include "ActorBunch.h"
#include "MemoryStream.h"
#include "EventManager.h"
#include "MathOps.h"

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
		this->field_0xfd4 = 0x1a;
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

	pBehaviour = GetBehaviour(0x1a);
	if (pBehaviour != (CBehaviour*)0x0) {
		CBehaviourBunch* pBehaviourBunch = static_cast<CBehaviourBunch*>(GetBehaviour(0x1a));
		pBehaviourBunch->Reset();
	}

	pBehaviour = GetBehaviour(0x1b);
	if (pBehaviour != (CBehaviour*)0x0) {
		CBehaviourBunch* pBehaviourBunch = static_cast<CBehaviourBunch*>(GetBehaviour(0x1b));
		pBehaviourBunch->Reset();
	}

	return;
}

CBehaviour* CActorBunch::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 0x1b) {
		pBehaviour = new CBehaviourBunchFence;
	}
	else {
		if (behaviourType == 0x1a) {
			pBehaviour = new CBehaviourBunchRepel;
		}
		else {
			if (behaviourType == 4) {
				pBehaviour = new CBehaviourBunchFighterProjected;
			}
			else {
				if (behaviourType == 3) {
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

	if (state < 0x1a) {
		behaviourFlags = CActorWolfen::GetBehaviourFlags(state);
	}
	else {
		assert(state - 0x1a < 2);
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
		bVar1 = this->actorState - 0xbfU < 5;
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
	if ((iVar1 == 0x1a) || (iVar1 == 0x1b)) {
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

CShockWaveBunch::CShockWaveBunch()
{
	this->field_0x1c = 1.0f;
	this->field_0xc = 1.0f;
	this->field_0x10 = 30.0f;
	this->field_0x14 = 10.0f;
	this->field_0x18 = 1.0f;
	this->field_0x20c = 0.0f;
	this->field_0x0 = 0;
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

void CShockWaveBunch::Manage(CActorsTable* pOutTable)
{
	IMPLEMENTATION_GUARD();
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
	this->behaviourId = 0x1a;

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
			this->behaviourId = 0x1a;
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

void CBehaviourBunch::Reset()
{
	return;
}
