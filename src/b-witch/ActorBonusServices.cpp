#include "ActorBonusServices.h"
#include "MemoryStream.h"
#include "Actor.h"
#include "LevelScheduleManager.h"
#include "ActorManager.h"
#include "TimeController.h"
#include "CollisionRay.h"
#include "ActorHero.h"
#include "ActorMoney.h"
#include "ActorBonus.h"
#include "MathOps.h"
#include "Frontend.h"
#include "CameraGame.h"

SectorAddOnData* CAddOnGenerator::_gAddOn_sectors = (SectorAddOnData*)0x0;
int CAddOnGenerator::_gAddOn_NbTotalAddOn = 0;
MoneyTypeEntry CAddOnGenerator::_gAddOn_aMoney[3] = {};
int CAddOnGenerator::_gAddOn_NbTotalBonusInLevel = 0;
int CAddOnGenerator::_gAddOn_bComputeDone = 0;
int CAddOnGenerator::_gAddOn_NbRemaining = 0;
int CAddOnGenerator::_gAddOn_NbMaxBonusInSector = 0;
CActorBonus* CAddOnGenerator::_gAddOn_pBonusAct = (CActorBonus*)0x0;
CBehaviourBonusAddOn* CAddOnGenerator::_gAddOn_pBonusBhv = (CBehaviourBonusAddOn*)0x0;


CActInstance::CActInstance()
{
	this->field_0x7c = -1.0f;
	this->field_0x84 = 0;
	this->field_0x88 = 0;

	return;
}

void CActInstance::SetState(int newState)
{
	this->state = newState;

	this->field_0x5c = 0.0f;

	if (this->state == 1) {
		this->flags = this->flags | 0x10;
	}

	if (this->state == 3) {
		this->field_0x54 = 0.0f;

		this->pathDelta = this->currentPosition;
	}
	return;
}

// Should be in: D:/Projects/b-witch/ActorBonusServices.h
void CActInstance::CheckpointReset()
{
	return;
}

void CActInstance::Init(CActor* pOwner, edF32VECTOR4* pPosition, edF32VECTOR4* pBoundSphere, int instanceIndex)
{
	ed_3d_hierarchy* peVar1;
	ed_g3d_manager* pG3D;
	edNODE* peVar2;
	ed_3D_Scene* pScene;
	int textureIndex;
	float fVar3;
	float fVar4;
	float fVar5;

	this->pOwner = pOwner;

	this->basePosition = *pPosition;

	this->instanceIndex = instanceIndex;
	this->field_0x5c = 0.0f;
	this->distanceToKim = 0.0f;

	this->flags = ACTOR_INSTANCE_FLAG_ALIVE;

	if (pOwner->p3DHierNode != (ed_3d_hierarchy_node*)0x0) {
		textureIndex = pOwner->pCinData->textureIndex;
		if (textureIndex == -1) {
			textureIndex = CScene::_pinstance->defaultTextureIndex_0x28;
		}

		pG3D = CScene::ptable.g_C3DFileManager_00451664->GetG3DManager(pOwner->pCinData->meshIndex, textureIndex);

		this->flags = this->flags | 0x80;
		peVar2 = ed3DHierarchyAddToScene(CScene::_scene_handleA, pG3D, (char*)0x0);
		this->pNode = peVar2;
		this->pHierarchy = (ed_3d_hierarchy*)this->pNode->pData;
		ed3DHierarchySetSetup(this->pHierarchy, &pOwner->hierarchySetup);
		this->pHierarchy->pHierarchySetup->pBoundingSphere = pBoundSphere;
	}

	this->currentPosition = this->basePosition;

	if ((this->flags & 0x80) != 0) {
		this->pHierarchy->transformA.rowT = this->currentPosition;
	}

	this->field_0x64 = this->basePosition.xyz;
	this->field_0x70 = gF32Vector3Zero;

	this->flags = this->flags & 0xfffffffb;

	if ((this->flags & 0x80) != 0) {
		pScene = GetStaticMeshMasterA_001031b0();
		ed3DHierarchyNodeSetRenderOn(pScene, this->pNode);
	}

	return;
}


void CActInstance::SetVisible(int bVisible)
{
	ed_3D_Scene* peVar1;

	if (bVisible == 0) {
		this->flags = this->flags & ~ACTOR_INSTANCE_FLAG_VISIBLE;
	}
	else {
		this->flags = this->flags | ACTOR_INSTANCE_FLAG_VISIBLE;
	}

	if ((this->flags & 0x80) != 0) {
		if (bVisible == 1) {
			peVar1 = GetStaticMeshMasterA_001031b0();
			ed3DHierarchyNodeSetRenderOff(peVar1, this->pNode);
		}
		else {
			peVar1 = GetStaticMeshMasterA_001031b0();
			ed3DHierarchyNodeSetRenderOn(peVar1, this->pNode);
		}
	}

	return;
}


void CActInstance::Reset()
{
	this->currentPosition = this->basePosition;

	if ((this->flags & 0x80) != 0) {
		this->pHierarchy->transformA.rowT = this->currentPosition;
	}

	this->field_0x64 = this->basePosition.xyz;
	this->field_0x70 = gF32Vector3Zero;

	this->field_0x5c = 0.0f;
	this->distanceToKim = 0.0f;
	this->flags = this->flags | ACTOR_INSTANCE_FLAG_ALIVE;

	SetState(1);

	return;
}


void CActInstance::SetAlive(int bAlive)
{
	ed_3D_Scene* pScene;

	if (bAlive == 0) {
		this->flags = this->flags & ~ACTOR_INSTANCE_FLAG_ALIVE;
		this->flags = this->flags & ~ACTOR_INSTANCE_FLAG_VISIBLE;

		if ((this->flags & 0x80) != 0) {
			pScene = GetStaticMeshMasterA_001031b0();
			ed3DHierarchyNodeSetRenderOn(pScene, this->pNode);
		}
	}
	else {
		this->flags = this->flags | ACTOR_INSTANCE_FLAG_ALIVE;
	}

	return;
}

float CActInstance::DistSquared(edF32VECTOR4* pPosition)
{
	float fVar1;
	float fVar2;
	float fVar3;

	fVar1 = (this->currentPosition).x - pPosition->x;
	fVar2 = (this->currentPosition).y - pPosition->y;
	fVar3 = (this->currentPosition).z - pPosition->z;
	return fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3;
}


void CActInstance::ComputeDistanceToKim(edF32VECTOR4* pPosition)
{
	edF32VECTOR4* pHeroPosition;
	edF32VECTOR4 deltaToKim;
	CActorHero* pHero;

	pHero = CActorHero::_gThis;
	pHeroPosition = CActorHero::_gThis->GetPosition_00369c80();
	edF32Vector4SubHard(&deltaToKim, pHeroPosition, &this->currentPosition);
	deltaToKim.y = deltaToKim.y + (pHero->subObjA->boundingSphere).w;
	this->distanceToKim = edF32Vector4GetDistHard(&deltaToKim);

	if (pPosition != (edF32VECTOR4*)0x0) {
		*pPosition = deltaToKim;
	}

	return;
}


void CActInstance::State_GotoKim()
{
	ed_3d_hierarchy* peVar1;
	CActorHero* pCVar2;
	edF32VECTOR4* peVar3;
	Timer* pTVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	pCVar2 = CActorHero::_gThis;
	peVar3 = CActorHero::_gThis->GetPosition_00369c80();

	local_10.x = peVar3->x;
	local_10.z = peVar3->z;
	local_10.w = peVar3->w;
	local_10.y = peVar3->y + pCVar2->subObjA->boundingSphere.w;

	edF32Vector4LERPHard(this->field_0x54, &local_20, &this->pathDelta, &local_10);
	local_20.w = 1.0f;

	(this->currentPosition).xyz = local_20.xyz;
	(this->currentPosition).w = 1.0f;

	if ((this->flags & 0x80) != 0) {
		(this->pHierarchy->transformA).rowT = this->currentPosition;
	}

	this->field_0x54 = this->field_0x54 + GetTimer()->cutsceneDeltaTime / 0.25f;
	edF32Vector4SubHard(&eStack48, &local_10, &this->currentPosition);
	fVar5 = edF32Vector4GetDistHard(&eStack48);
	if ((1.0f <= this->field_0x54) || (fVar5 <= 0.25f)) {
		SetState(4);
	}

	return;
}


void CActInstance::State_Wait()
{
	CActorHero* pCVar1;
	edF32VECTOR4* v1;
	int iVar2;
	long newState;
	float fVar4;
	edF32VECTOR4 eStack32;
	undefined4 local_4;

	pCVar1 = CActorHero::_gThis;
	v1 = CActorHero::_gThis->GetPosition_00369c80();
	edF32Vector4SubHard(&eStack32, v1, &this->currentPosition);
	eStack32.y = eStack32.y + pCVar1->subObjA->boundingSphere.w;
	fVar4 = edF32Vector4GetDistHard(&eStack32);
	this->distanceToKim = fVar4;

	newState = 7;
	if (this->distanceToKim <= 0.85f) {
		newState = 4;
	}
	else {
		if (this->distanceToKim <= 2.0f) {
			newState = 3;
		}
	}

	if (newState != 7) {
		if ((this->flags & 8) == 0) {
			SetState(newState);
		}
		else {
			local_4 = 0xffffffff;
			iVar2 = this->pOwner->DoMessage(CActorHero::_gThis, (ACTOR_MESSAGE)9, (void*)0xffffffff);
			if (iVar2 == 0) {
				CActorHero::_gThis->magicInterface.FUN_001dcda0(this->distanceToKim);
			}
			else {
				SetState(newState);
			}
		}
	}
	return;
}

void CActInstance::UpdateVisibility()
{
	bool bFlagsVisible;
	bool bInFrustum;

	if (((this->flags & ACTOR_INSTANCE_FLAG_ALIVE) == 0) && ((this->flags & ACTOR_INSTANCE_FLAG_VISIBLE) != 0)) {
		SetVisible(0);
	}
	else {
		bInFrustum = CCameraManager::_gThis->InFrustum(0.35f, this->pOwner->subObjA->field_0x1c, &this->currentPosition);
		bFlagsVisible = (this->flags & ACTOR_INSTANCE_FLAG_VISIBLE) != 0;

		if ((bFlagsVisible) || (bInFrustum == false)) {
			if (((bFlagsVisible) && (bInFrustum == false))) {
				SetVisible(0);
			}
		}
		else {
			SetVisible(1);
		}
	}

	return;
}


void CActInstance::FUN_00397ba0()
{
	ed_3d_hierarchy* peVar1;
	CActorHero* pHero;
	edF32VECTOR4* pHeroPosition;
	int iVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	edF32VECTOR4 eStack160;
	edF32VECTOR4 local_50;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	undefined4 local_8;
	undefined4 local_4;

	pHero = CActorHero::_gThis;

	pHeroPosition = CActorHero::_gThis->GetPosition_00369c80();
	edF32Vector4SubHard(&eStack160, pHeroPosition, &this->currentPosition);

	eStack160.y = eStack160.y + pHero->subObjA->boundingSphere.w;
	fVar5 = edF32Vector4GetDistHard(&eStack160);
	this->distanceToKim = fVar5;
	if ((this->flags & 0x10) == 0) {
		if ((this->flags & 0x40) != 0) {
			local_20.y = -1.0f;
			local_20.x = 0.0f;
			local_20.z = 0.0f;
			local_20.w = 0.0f;

			CCollisionRay CStack144 = CCollisionRay(0.5f, &this->currentPosition, &local_20);
			if (CStack144.Intersect(3, (CActor*)0x0, (CActor*)0x0, 0x40000008, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0) < 0.5f) {
				edF32Vector4ScaleHard(GetTimer()->cutsceneDeltaTime * 1.5f, &local_20, &gF32Vector4UnitY);
				edF32Vector4AddHard(&local_20, &this->currentPosition, &local_20);
				this->currentPosition = local_20;
				if ((this->flags & 0x80) != 0) {
					peVar1 = this->pHierarchy;
					peVar1->transformA.rowT = this->currentPosition;
				}
			}
			else {
				this->flags = this->flags & 0xffffffbf;
			}
		}
	}
	else {
		local_30.y = -0.25f;
		local_30.x = 0.0f;
		local_30.z = 0.0f;
		local_30.w = 0.0f;

		edF32Vector4AddHard(&this->pathDelta, &this->pathDelta, &local_30);
		edF32Vector4ScaleHard(GetTimer()->cutsceneDeltaTime, &local_20, &this->pathDelta);
		fVar5 = edF32Vector4NormalizeHard(&local_20, &local_20);

		if (fVar5 < 0.02f) {
			this->flags = this->flags & 0xffffffef;
			this->flags = this->flags | 0x40;
		}
		else {
			if ((this->pathDelta).y < -50.0f) {
				SetState(5);
			}
		}

		local_50.z = this->currentPosition.z;
		local_50.w = this->currentPosition.w;
		local_50.x = this->currentPosition.x;
		local_50.y = this->currentPosition.y - 0.2f;

		CCollisionRay CStack112 = CCollisionRay(fVar5, &local_50, &local_20);
		if (CStack112.Intersect(3, (CActor*)0x0, (CActor*)0x0, 0x40000008, &eStack64, (_ray_info_out*)0x0) == 1e+30) {
			edF32Vector4ScaleHard(fVar5, &local_20, &local_20);
		}
		else {
			edReflectVectorOnPlane(0.7f, &this->pathDelta, &this->pathDelta, &eStack64, 1);
			edF32Vector4ScaleHard(GetTimer()->cutsceneDeltaTime, &local_20, &this->pathDelta);
		}

		edF32Vector4AddHard(&local_20, &this->currentPosition, &local_20);
		this->currentPosition = local_20;
		if ((this->flags & 0x80) != 0) {
			peVar1 = this->pHierarchy;
			peVar1->transformA.rowT = this->currentPosition;
		}
	}

	if (this->distanceToKim <= 0.85f) {
		if ((this->flags & 8) == 0) {
			SetState(4);
		}
		else {
			local_4 = 0xffffffff;
			iVar4 = this->pOwner->DoMessage(CActorHero::_gThis, (ACTOR_MESSAGE)9, (MSG_PARAM)0xffffffff);
			if (iVar4 == 0) {
				CActorHero::_gThis->magicInterface.FUN_001dcda0(this->distanceToKim);
			}
			else {
				SetState(4);
			}
		}
	}
	else {
		if (this->distanceToKim <= 2.0f) {
			if ((this->flags & 8) == 0) {
				SetState(CActInstance::STT_INS_GOTO_KIM);
			}
			else {
				local_8 = 0xffffffff;
				iVar4 = this->pOwner->DoMessage(CActorHero::_gThis, (ACTOR_MESSAGE)9, (MSG_PARAM)0xffffffff);
				if (iVar4 == 0) {
					CActorHero::_gThis->magicInterface.FUN_001dcda0(this->distanceToKim);
				}
				else {
					SetState(CActInstance::STT_INS_GOTO_KIM);
				}
			}
		}
	}

	return;
}

void CActInstance::SetBasePosition(edF32VECTOR4* pBasePosition)
{
	this->basePosition = *pBasePosition;

	return;
}

void CActInstance::SetPosition(edF32VECTOR4* pPosition)
{
	this->currentPosition = *pPosition;
	if ((this->flags & 0x80) != 0) {
		this->pHierarchy->transformA.rowT = this->currentPosition;
	}

	return;
}


void CActInstance::Term()
{
	if ((this->flags & 0x80) != 0) {
		ed3DHierarchyRemoveFromScene(CScene::_scene_handleA, this->pNode);
	}
	this->pNode = (edNODE*)0x0;

	return;
}

void CAddOnGenerator_SubObj::Create(ByteCode* pByteCode)
{
	uint uVar1;
	float fVar2;

	uVar1 = pByteCode->GetU32();
	if (uVar1 == 1) {
		this->field_0x0 = pByteCode->GetF32() * 0.01745329f;
		this->field_0x4 = pByteCode->GetF32() * 0.01745329f;
		this->field_0x8 = 0.0f;
		this->field_0xc = pByteCode->GetF32() * 0.01745329f;
		this->field_0x10 = pByteCode->GetF32();
		this->field_0x14 = pByteCode->GetF32();
	}
	else {
		this->field_0x0 = 0.7853982f;
		this->field_0x4 = 0.3926991f;
		this->field_0x8 = 0.0f;
		this->field_0xc = 3.141593f;
		this->field_0x10 = 3.0f;
		this->field_0x14 = 2.5f;
	}
	return;
}

void CAddOnGenerator::Create(CActor* pActor, ByteCode* pByteCode)
{
	uint uVar1;
	int iVar2;
	int iVar3;
	int sectorId;
	int iVar4;

	sectorId = pActor->sectorId;
	this->field_0x0 = (ushort)pByteCode->GetU32() & 3;
	this->moneyType = (ushort)pByteCode->GetU32();
	this->nbMoney = pByteCode->GetS32();
	this->nbBonus = pByteCode->GetS32();
	this->field_0xc = (float)pByteCode->GetS32() * 0.01f;
	this->subObj.Create(pByteCode);
	this->aInstances = (CActInstance**)0x0;
	this->maxOrbs_0x2c = 0;
	this->field_0x30 = 0;

	if (_gAddOn_sectors == (SectorAddOnData*)0x0) {
		iVar4 = CLevelScheduler::gThis->aLevelInfo[CLevelScheduler::gThis->currentLevelID].maxSectorId + 1;
		_gAddOn_sectors = new SectorAddOnData[iVar4];

		iVar2 = 0;
		if (0 < iVar4) {
			iVar3 = 0;
			do {
				iVar2 = iVar2 + 1;
				_gAddOn_sectors[iVar3].aNbMonies[0] = 0;
				_gAddOn_sectors[iVar3].aNbMonies[1] = 0;
				_gAddOn_sectors[iVar3].aNbMonies[2] = 0;
				_gAddOn_sectors[iVar3].nbBonus = 0;
				iVar3 = iVar3 + 1;
			} while (iVar2 < iVar4);
		}
	}

	_gAddOn_NbTotalAddOn = _gAddOn_NbTotalAddOn + 1;

	_gAddOn_aMoney[this->moneyType]._gAddOn_NbTotalMoneyInLevel = _gAddOn_aMoney[this->moneyType]._gAddOn_NbTotalMoneyInLevel + this->nbMoney;
	_gAddOn_NbTotalBonusInLevel = _gAddOn_NbTotalBonusInLevel + this->nbBonus;

	if (sectorId == -1) {
		sectorId = 0;
	}

	_gAddOn_sectors[sectorId].aNbMonies[this->moneyType] = _gAddOn_sectors[sectorId].aNbMonies[this->moneyType] + this->nbMoney;
	_gAddOn_sectors[sectorId].nbBonus = _gAddOn_sectors[sectorId].nbBonus + this->nbBonus;

	return;
}

bool CriterionMoneyAddOn(CActor* pActorA, CActor* pActorB)
{
	return pActorB->curBehaviourId == MONEY_BEHAVIOUR_ADD_ON;
}

bool CriterionBonusAddOn(CActor* pActorA, CActor* pActorB)
{
	return pActorB->curBehaviourId == BONUS_BEHAVIOUR_ADD_ON;
}

bool PredicateGetMoneyByValue(CActor* pActor, void* pParams)
{
	bool bMatch;
	int pvVar1;

	if (((pActor->typeID == MONEY) && (pActor->curBehaviourId == MONEY_BEHAVIOUR_ADD_ON)) && (pvVar1 = ((CActorMoney*)pActor)->GetType(), reinterpret_cast<int>(pParams) == pvVar1)) {
		bMatch = true;
	}
	else {
		bMatch = false;
	}
	return bMatch;
}

void CAddOnGenerator::Init(int param_2)
{
	SectorAddOnData* pCurSectorData;
	CActorManager* pActorManager;
	CActInstance** ppCVar2;
	CActorMoney* pActor;
	CBehaviourMoneyAddOn* pCVar3;
	int curSectorIndex;
	MoneyTypeEntry* pCurMoneyEntry;
	int iVar7;
	int nbSectors;
	int curMoneyType;
	CActorsTable local_110;
	int nbTotalMoneyAndBonus;

	if ((param_2 != 0) && (nbTotalMoneyAndBonus = this->nbMoney + this->nbBonus, nbTotalMoneyAndBonus != 0)) {
		this->aInstances = (CActInstance**)edMemAlloc(TO_HEAP(H_MAIN), nbTotalMoneyAndBonus * sizeof(CActInstance*));
	}

	if (_gAddOn_bComputeDone == 0) {
		_gAddOn_bComputeDone = 1;
		_gAddOn_NbRemaining = _gAddOn_NbTotalAddOn;
	}

	_gAddOn_NbRemaining = _gAddOn_NbRemaining + -1;
	if (_gAddOn_NbRemaining == 0) {
		_gAddOn_aMoney[2].field_0x4 = 0;

		nbSectors = CLevelScheduler::gThis->aLevelInfo[CLevelScheduler::gThis->currentLevelID].maxSectorId + 1;
		_gAddOn_aMoney[1].field_0x4 = 0;
		_gAddOn_aMoney[0].field_0x4 = 0;
		_gAddOn_NbMaxBonusInSector = 0;

		curSectorIndex = 0;
		pCurSectorData = _gAddOn_sectors;
		if (0 < nbSectors) {
			do {
				iVar7 = 0;
				pCurMoneyEntry = _gAddOn_aMoney;
				int* pNbMoney = pCurSectorData->aNbMonies;
				do {
					if (pCurMoneyEntry->field_0x4 < *pNbMoney) {
						pCurMoneyEntry->field_0x4 = *pNbMoney;
					}
					iVar7 = iVar7 + 1;
					pNbMoney = pNbMoney + 1;
					pCurMoneyEntry = pCurMoneyEntry + 1;
				} while (iVar7 < 3);

				if (_gAddOn_NbMaxBonusInSector < pCurSectorData->nbBonus) {
					_gAddOn_NbMaxBonusInSector = pCurSectorData->nbBonus;
				}

				curSectorIndex = curSectorIndex + 1;
				pCurSectorData = pCurSectorData + 1;
			} while (curSectorIndex < nbSectors);
		}

		pActorManager = CScene::ptable.g_ActorManager_004516a4;
		local_110.nbEntries = 0;

		if ((_gAddOn_NbMaxBonusInSector != 0) && (pActorManager->GetClassActorsWithCriterion(&local_110, BONUS, 0, CriterionBonusAddOn), local_110.nbEntries == 1)) {
			_gAddOn_pBonusAct = (CActorBonus*)local_110.aEntries[0];
			_gAddOn_pBonusBhv = (CBehaviourBonusAddOn*)local_110.aEntries[0]->GetBehaviour(local_110.aEntries[0]->curBehaviourId);
			_gAddOn_pBonusBhv->Allocate(_gAddOn_NbMaxBonusInSector);
		}

		local_110.nbEntries = 0;
		pActorManager->GetClassActorsWithCriterion(&local_110, MONEY, 0, CriterionMoneyAddOn);
		curMoneyType = 0;
		pCurMoneyEntry = _gAddOn_aMoney;
		do {
			if ((pCurMoneyEntry->field_0x4 != 0) && (pActor = (CActorMoney*)local_110.GetByPredicate(PredicateGetMoneyByValue, (void*)curMoneyType), pActor != (CActorMoney*)0x0)) {
				pCurMoneyEntry->pActor = pActor;
				pCVar3 = (CBehaviourMoneyAddOn*)pActor->GetBehaviour(pActor->curBehaviourId);
				pCurMoneyEntry->pBehaviour = pCVar3;
				pCurMoneyEntry->pBehaviour->Allocate(pCurMoneyEntry->field_0x4);
			}

			curMoneyType = curMoneyType + 1;
			pCurMoneyEntry = pCurMoneyEntry + 1;
		} while (curMoneyType < 3);

		delete[] _gAddOn_sectors;
		_gAddOn_sectors = (SectorAddOnData*)0x0;
	}

	return;
}

void CAddOnGenerator::Term()
{
	if (this->aInstances != (CActInstance**)0x0) {
		edMemFree(this->aInstances);
	}

	_gAddOn_aMoney[this->moneyType]._gAddOn_NbTotalMoneyInLevel = _gAddOn_aMoney[this->moneyType]._gAddOn_NbTotalMoneyInLevel - this->nbMoney;
	_gAddOn_NbTotalBonusInLevel = _gAddOn_NbTotalBonusInLevel - this->nbBonus;
	if (_gAddOn_aMoney[0]._gAddOn_NbTotalMoneyInLevel + _gAddOn_aMoney[1]._gAddOn_NbTotalMoneyInLevel + _gAddOn_aMoney[2]._gAddOn_NbTotalMoneyInLevel + _gAddOn_NbTotalBonusInLevel == 0) {
		_gAddOn_bComputeDone = 0;
		_gAddOn_aMoney[0].field_0x4 = 0;
		_gAddOn_NbRemaining = 0;
		_gAddOn_aMoney[0].pActor = (CActorMoney*)0x0;
		_gAddOn_NbTotalAddOn = 0;
		_gAddOn_aMoney[0].pBehaviour = (CBehaviourMoneyAddOn*)0x0;
		_gAddOn_aMoney[1].field_0x4 = 0;
		_gAddOn_aMoney[1].pActor = (CActorMoney*)0x0;
		_gAddOn_aMoney[1].pBehaviour = (CBehaviourMoneyAddOn*)0x0;
		_gAddOn_aMoney[2].field_0x4 = 0;
		_gAddOn_aMoney[2].pActor = (CActorMoney*)0x0;
		_gAddOn_aMoney[2].pBehaviour = (CBehaviourMoneyAddOn*)0x0;
		_gAddOn_NbMaxBonusInSector = 0;
		_gAddOn_pBonusAct = (CActorBonus*)0x0;
		_gAddOn_pBonusBhv = (CBehaviourBonusAddOn*)0x0;
	}

	return;
}

void CAddOnGenerator::Generate(edF32VECTOR4* pPosition)
{
	uint uVar1;
	CLevelScheduler* pLevelScheduler;
	CBnsInstance** pInstance;
	int iVar2;
	CMnyInstance** uVar3;
	CBnsInstance** pCVar4;
	int iVar5;
	CMagicInterface* pMagicInterface;
	float fVar6;
	float fVar7;
	float fVar8;

	ManageInstances();

	pCVar4 = (CBnsInstance**)(this->aInstances + this->maxOrbs_0x2c + this->field_0x30);
	pInstance = pCVar4;

	if (this->nbBonus != 0) {
		pMagicInterface = &CActorHero::_gThis->magicInterface;
		if ((this->field_0x0 & 2) == 0) {
			fVar8 = 0.0f;
			fVar6 = this->field_0xc;
			fVar6 = (float)this->nbBonus * fVar6 * ((float)rand() / 2.147484e+09f);
		}
		else {
			fVar6 = pMagicInterface->GetValue();
			fVar7 = pMagicInterface->GetValueMax();

			fVar8 = 1.0f;
			if (fVar6 / fVar7 <= 1.0f) {
				fVar8 = fVar6 / fVar7;
			}

			fVar6 = 0.0f;
		}

		fVar8 = floorf((1.0f - fVar8) * ((float)this->nbBonus - fVar6));

		if (_gAddOn_pBonusBhv != (CBehaviour*)0x0) {
			pInstance = _gAddOn_pBonusBhv->Generate(pPosition, &this->subObj, (int)fVar8 - this->maxOrbs_0x2c, pCVar4);
			int diff = (char*)pInstance - (char*)pCVar4;
			if (diff < 0) {
				diff = diff + 3;
			}
			this->maxOrbs_0x2c = this->maxOrbs_0x2c + (diff / sizeof(CBnsInstance*));
		}
	}

	pLevelScheduler = CLevelScheduler::gThis;
	iVar2 = this->nbMoney;
	if ((iVar2 != 0) && (_gAddOn_aMoney[this->moneyType].pActor != (CActorMoney*)0x0)) {
		if ((this->field_0x0 & 1) != 0) {
			iVar2 = rand();
			iVar5 = this->nbMoney + 1;
			iVar2 = iVar2 % iVar5;
			if (iVar5 == 0) {
				trap(7);
			}
		}

		uVar1 = (_gAddOn_aMoney[this->moneyType].pActor)->moneyValue;
		if ((CLevelScheduler::_gGameNfo.monster <= (int)(iVar2 * uVar1)) && (iVar2 = CLevelScheduler::_gGameNfo.monster / (int)uVar1, uVar1 == 0)) {
			trap(7);
		}

		iVar2 = iVar2 - this->field_0x30;
		if (_gAddOn_aMoney[this->moneyType].pBehaviour != (CBehaviourMoneyAddOn*)0x0) {
			uVar3 = _gAddOn_aMoney[this->moneyType].pBehaviour->Generate(pPosition, &this->subObj, iVar2, (CMnyInstance**)pInstance);
			iVar5 = (char*)uVar3 - (char*)pInstance;
			if (iVar5 < 0) {
				iVar5 = iVar5 + 3;
			}
			this->field_0x30 = this->field_0x30 + (iVar5 / sizeof(CActInstance*));
		}

		pLevelScheduler->Money_TakeFromMonster(iVar2 * (_gAddOn_aMoney[this->moneyType].pActor)->moneyValue);
	}

	return;
}

void CAddOnGenerator::ManageInstances()
{
	if (this->aInstances == nullptr) {
		return;
	}

	// Get a temporary buffer for filtering active instances
	const int totalInstanceCount = this->nbMoney + this->nbBonus;
	CActInstance** pFilteredBuffer = static_cast<CActInstance**>(
		gSP_Manager.GetFreeBuffer(totalInstanceCount * sizeof(CActInstance*))
		);

	// First pass: filter active instances and count by type
	int activeCount = 0;
	int moneyCount = 0;
	int bonusCount = 0;

	const int totalStored = this->maxOrbs_0x2c + this->field_0x30;
	for (int i = 0; i < totalStored; ++i) {
		CActInstance* pInstance = this->aInstances[i];

		if ((pInstance->flags & ACTOR_INSTANCE_FLAG_ALIVE) != 0) {
			pFilteredBuffer[activeCount] = pInstance;
			++activeCount;

			// Categorize instance type
			const uint instanceFlags = *(uint*)(*(int*)pInstance + 4);
			if ((instanceFlags & 8) == 0) {
				++moneyCount;
			}
			else {
				++bonusCount;
			}
		}
	}

	// Second pass: copy filtered instances back to main buffer in blocks of 8
	if (activeCount > 0) {
		int destOffset = 0;

		// Process blocks of 8 instances
		if (activeCount > 8) {
			int blockCount = 0;
			for (; blockCount < activeCount - 8; blockCount += 8) {
				for (int j = 0; j < 8; ++j) {
					this->aInstances[destOffset + j] = pFilteredBuffer[blockCount + j];
				}
				destOffset += 8;
			}
		}

		// Process remaining instances
		const int remaining = activeCount - (destOffset / sizeof(CActInstance*));
		for (int i = 0; i < remaining; ++i) {
			this->aInstances[destOffset + i] = pFilteredBuffer[destOffset / sizeof(CActInstance*) + i];
		}
	}

	this->maxOrbs_0x2c = bonusCount;
	this->field_0x30 = moneyCount;

	gSP_Manager.ReleaseBuffer(pFilteredBuffer);
}
