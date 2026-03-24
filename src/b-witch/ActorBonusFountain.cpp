#include "ActorBonusFountain.h"
#include "MemoryStream.h"
#include "TimeController.h"
#include "FrontEndDisp.h"
#include "FrontEndMagic.h"
#include "CameraViewManager.h"
#include "MathOps.h"
#include "DlistManager.h"

void CActorBonusFountain::Create(ByteCode* pByteCode)
{
	CActor::Create(pByteCode);
	this->addOnGenerator.Create(this, pByteCode);

	return;
}

void CActorBonusFountain::Init()
{
	CActor::Init();
	this->addOnGenerator.Init(1);

	return;
}

void CActorBonusFountain::Term()
{
	CActor::Term();
	this->addOnGenerator.Term();

	return;
}

void CActorBonusFountain::Draw()
{
	uint* pCVar2;
	float* pCVar3;
	int index;
	float fVar3;
	edF32VECTOR4 eStack16;

	index = 0;
	pCVar2 = this->field_0x1a0;
	pCVar3 = this->field_0x1b0[0];

	do {
		fVar3 = *pCVar3 + FLOAT_00428e68 * GetTimer()->cutsceneDeltaTime;
		*pCVar3 = fVar3;
		if (16.0f <= fVar3) {
			*pCVar3 = fVar3 - 16.0f;
		}

		SV_GetBoneWorldPosition(*pCVar2, &eStack16);
		FUN_003ea280(&eStack16, index);

		index = index + 1;
		pCVar2 = pCVar2 + 1;
		pCVar3 = pCVar3 + 1;
	} while (index < 4);

	return;
}

CBehaviour* CActorBonusFountain::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == BONUS_FOUNTAIN_BEHAVIOUR_STAND) {
		pBehaviour = &this->behaviourStand;

	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

StateConfig CActorBonusFountain::_gStateCfg_BNF[4] = {
	StateConfig(0, 4),
	StateConfig(6, 1),
	StateConfig(6, 1),
	StateConfig(7, 1),
};

StateConfig* CActorBonusFountain::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		assert((state - 5) < 4);
		pStateConfig = _gStateCfg_BNF + state + -5;
	}

	return pStateConfig;
}

void CActorBonusFountain::FUN_003ea280(edF32VECTOR4* pPosition, int index)
{
	CFrontendMagicGauge* pMagicOrbs;
	CCameraManager* pCameraManager;
	bool bVar3;
	CFrontendDisplay* pFrontendDisplay;
	int iVar5;
	uint uVar6;
	uint uVar9;
	float fVar7;
	float fVar10;
	float s;
	edF32VECTOR4 local_50;
	edF32MATRIX4 eStack64;

	pFrontendDisplay = (CFrontendDisplay*)CScene::GetManager(MO_Frontend);
	pCameraManager = CCameraManager::_gThis;
	pMagicOrbs = pFrontendDisplay->pMagicOrbs;
	bVar3 = GameDList_BeginCurrent();
	if (bVar3 != false) {
		edDListUseMaterial(&(pMagicOrbs->field_0x3d0).materialInfo);
		edDListLoadIdentity();
		fVar7 = edF32Between_0_2Pi(this->field_0x1b0[1][index] + FLOAT_00428e70 * GetTimer()->cutsceneDeltaTime);
		this->field_0x1b0[1][index] = fVar7;

		local_50.x = FLOAT_00428e78 * cosf(this->field_0x1b0[1][index]);
		local_50.y = FLOAT_00428e78 * cosf(this->field_0x1b0[1][index] - 1.570796f);
		local_50.z = 0.0f;
		local_50.w = 1.0f;

		edF32Matrix4CopyHard(&eStack64, &pCameraManager->transMatrix_0x390);
		eStack64.rowT = *pPosition;
		edF32Matrix4MulF32Vector4Hard(&local_50, &eStack64, &local_50);
		edDListBegin(0.0f, 0.0f, 0.0f, 0xb, 1);
		edDListTexCoo2f(0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 1.0f);
		edDListWidthHeight2f(0.1714286f, 0.1714286f);
		iVar5 = rand();
		edDListColor4u8(0x80, 0x80, 0x80, (byte)(int)(((float)iVar5 / 2.147484e+09f) * 96.0f + 64.0f));
		edDListVertex4f(local_50.x, local_50.y, local_50.z, 0.0f);
		edDListEnd();

		fVar10 = (float)(int)this->field_0x1b0[index];
		uVar9 = (uint)fVar10;
		uVar6 = uVar9 & 7;
		if (((int)uVar9 < 0) && (uVar6 != 0)) {
			uVar6 = uVar6 - 8;
		}

		s = (float)uVar6 * 0.125f;
		iVar5 = (int)fVar10;
		if (iVar5 < 0) {
			iVar5 = iVar5 + 7;
		}

		fVar10 = (float)(iVar5 >> 3) * 0.5f;
		edDListUseMaterial(&(pMagicOrbs->field_0x490).materialInfo);
		edDListBegin(0.0f, 0.0f, 0.0f, 0xb, 1);
		edDListTexCoo2f(s, fVar10);
		edDListTexCoo2f(s + 0.125f, fVar10 + 0.5f);
		edDListWidthHeight2f(FLOAT_00428e60, FLOAT_00428e60);
		edDListColor4u8(0x80, 0x80, 0x80, 0x80);
		edDListVertex4f(pPosition->x, pPosition->y, pPosition->z, 0.0f);
		edDListEnd();

		GameDList_EndCurrent();
	}

	return;
}

edF32VECTOR4 gOrbSpawnVelocity = { 0.0f, 0.4f, 0.85f, 0.0f };

void CBehaviourBonusFountainStand::Manage()
{
	int iVar1;
	CAnimation* pCVar2;
	edAnmLayer* peVar3;
	bool bVar4;
	edF32VECTOR4 local_10;
	CActorBonusFountain* pBonusFountain;

	pBonusFountain = this->pOwner;
	iVar1 = pBonusFountain->actorState;
	if ((iVar1 == 8) || (iVar1 == 7)) {
		if (pBonusFountain->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			pBonusFountain->SetState(5, -1);
		}
	}
	else {
		if ((iVar1 == 6) && (0.6f < pBonusFountain->timeInAir)) {
			local_10 = pBonusFountain->currentLocation;

			edF32Matrix4MulF32Vector4Hard(&local_10, &pBonusFountain->pMeshTransform->base.transformA, &gOrbSpawnVelocity);
			edF32Vector4AddHard(&local_10, &local_10, &pBonusFountain->currentLocation);
			(pBonusFountain->addOnGenerator).subObj.field_0x8 = pBonusFountain->rotationEuler.y;
			pBonusFountain->addOnGenerator.Generate(&local_10);
			pBonusFountain->SetState(7, -1);
		}
	}

	return;
}

void CBehaviourBonusFountainStand::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorBonusFountain*>(pOwner);

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	this->field_0x8 = 0;

	return;
}

void CBehaviourBonusFountainStand::InitState(int newState)
{
	return;
}

void CBehaviourBonusFountainStand::TermState(int oldState, int newState)
{
	return;
}

int CBehaviourBonusFountainStand::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	bool bVar1;
	CActorBonusFountain* pBonusFountain;

	if (msg == 3) {
		this->field_0x8 = 1;
		return 1;
	}

	if (msg == MESSAGE_KICKED) {
		this->pOwner->addOnGenerator.ManageInstances();

		if (this->field_0x8 != 0) {
			pBonusFountain = this->pOwner;

			bVar1 = (pBonusFountain->addOnGenerator).nbGeneratedBonus < (pBonusFountain->addOnGenerator).nbBonus;
			if (!bVar1) {
				bVar1 = (pBonusFountain->addOnGenerator).nbGeneratedMoney < (pBonusFountain->addOnGenerator).nbMoney;
			}

			if (bVar1) {
				pBonusFountain->SetState(6, -1);
				return 1;
			}
		}

		this->pOwner->SetState(8, -1);

		return 1;
	}

	return 0;
}
