#include "ActorShoot.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "TimeController.h"
#include "ActorHero.h"

CActorShoot::~CActorShoot()
{
	//(this->base).base.base.pVTable = (CActorFighterVTable*)&_vt;
	//lVar1 = (**(code**)(*(int*)&this->field_0x360 + 0x14))();
	//if (lVar1 != 0) {
	//	StaticMeshComponent::Term((StaticMeshComponent*)&this->field_0x360, CScene::_scene_handleA);
	//}
	this->addOnGenerator.Term();
	//if (this != (CActorShoot*)0xfffff810) {
	//	*(CBehaviourVtable**)&this->field_0x7f0 = &CBehaviourShootFireWave::_vt;
	//	CConicalWaveShoot::~CConicalWaveShoot((long)(int)&this->conicalWaveShoot, 0xffffffffffffffff);
	//	if ((this != (CActorShoot*)0xfffff810) && (*(CBehaviourVtable**)&this->field_0x7f0 = &CBehaviourShoot::_vt, this != (CActorShoot*)0xfffff810)) {
	//		*(CBehaviourVtable**)&this->field_0x7f0 = &CBehaviour::_vt;
	//	}
	//}
	//if (((this != (CActorShoot*)0xfffffad0) && (*(CBehaviourVtable**)&this->field_0x530 = &CBehaviourShootFire::_vt, this != (CActorShoot*)0xfffffad0)) &&
	//	(*(CBehaviourVtable**)&this->field_0x530 = &CBehaviourShoot::_vt, this != (CActorShoot*)0xfffffad0)) {
	//	*(CBehaviourVtable**)&this->field_0x530 = &CBehaviour::_vt;
	//}
	//StaticMeshComponent::~StaticMeshComponent((StaticMeshComponent*)&this->field_0x360, -1);
	//if (lVar2 != 0) {
	//	(this->base).base.base.pVTable = (CActorFighterVTable*)&CActorAutonomous::_vt;
	//	CLifeInterface::~CLifeInterface((long)(int)&(this->base).lifeInterface, 0xffffffffffffffff);
	//	CActorMovable::~CActorMovable(lVar2, 0);
	//}
}

void CActorShoot::Create(ByteCode* pByteCode)
{
	uint uVar1;
	int iVar2;
	float fVar3;

	CActorAutonomous::Create(pByteCode);

	uVar1 = pByteCode->GetU32();
	//this->field_0x350 = uVar1;
	uVar1 = pByteCode->GetU32();
	//this->field_0x354 = uVar1;
	iVar2 = pByteCode->GetS32();
	//this->field_0x364 = iVar2;
	iVar2 = pByteCode->GetS32();
	//this->field_0x368 = iVar2;
	//(**(code**)(this->field_0x360 + 0x10))(&this->field_0x360);
	fVar3 = pByteCode->GetF32();
	//this->field_0x3c0 = fVar3;
	fVar3 = pByteCode->GetF32();
	//this->field_0x3c4 = fVar3;
	fVar3 = pByteCode->GetF32();
	//this->field_0x3c8 = fVar3;
	fVar3 = pByteCode->GetF32();
	//this->field_0x3cc = fVar3;
	uVar1 = pByteCode->GetU32();
	//this->field_0x3d0 = uVar1;
	fVar3 = pByteCode->GetF32();
	//this->field_0x3d4 = fVar3;
	fVar3 = pByteCode->GetF32();
	//this->field_0x3d8 = fVar3;
	fVar3 = pByteCode->GetF32();
	//this->field_0x3dc = fVar3;
	fVar3 = pByteCode->GetF32();
	//this->field_0x3e0 = fVar3;
	uVar1 = pByteCode->GetU32();
	//this->field_0x3e8 = uVar1;
	uVar1 = pByteCode->GetU32();
	//this->field_0x3e4 = uVar1;
	uVar1 = pByteCode->GetU32();
	//this->field_0x3ec = uVar1;
	uVar1 = pByteCode->GetU32();
	//this->field_0x3f0 = uVar1;

	this->addOnGenerator.Create(this, pByteCode);

	return;
}

void CActorShoot::Init()
{
	CActorAutonomous::Init();
	this->addOnGenerator.Init(0);
	return;
}

void CActorShoot::ComputeLighting()
{
	CScene::ptable.g_LightManager_004516b0->ComputeLighting(this->lightingFloat_0xe0, this, this->lightingFlags, &this->lightingConfig);

	return;
}

void CActorShoot::Reset()
{
	CActorAutonomous::Reset();
	ClearLocalData();

	return;
}

struct S_SAVE_CLASS_SHOOT
{
	uint field_0x0;
};

void CActorShoot::SaveContext(void* pData, uint mode, uint maxSize)
{
	S_SAVE_CLASS_SHOOT* pSaveData = reinterpret_cast<S_SAVE_CLASS_SHOOT*>(pData);

	if (mode == 1) {
		pSaveData->field_0x0 = 0.0f < GetLifeInterface()->GetValue();
	}

	return;
}

void CActorShoot::LoadContext(void* pData, uint mode, uint maxSize)
{
	S_SAVE_CLASS_SHOOT* pSaveData = reinterpret_cast<S_SAVE_CLASS_SHOOT*>(pData);

	if ((mode == 1) && (pSaveData->field_0x0 == 0)) {
		LifeAnnihilate();

		this->flags = this->flags & 0xffffff7f;
		this->flags = this->flags | 0x20;

		EvaluateDisplayState();

		this->flags = this->flags & 0xfffffffd;
		this->flags = this->flags | 1;

		SetBehaviour(SHOOT_BEHAVIOUR_FIRE_WAVE, 0xf, -1);
	}

	return;
}

CBehaviour* CActorShoot::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == SHOOT_BEHAVIOUR_FIRE_WAVE) {
		IMPLEMENTATION_GUARD();
		pBehaviour = &this->behaviourShootFireWave;
	}
	else {
		if (behaviourType == SHOOT_BEHAVIOUR_FIRE) {
			pBehaviour = &this->behaviourShootFire;
		}
		else {
			pBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
		}
	}

	return pBehaviour;
}

StateConfig CActorShoot::_gStateCfg_SHT[10] = {
	{ 0x00, 0x104 },
	{ 0x0C, 0x144 },
	{ 0x0D, 0x944 },
	{ 0x06, 0x9C4 },
	{ 0x0E, 0x9C4 },
	{ 0x0F, 0x9C4 },
	{ 0x10, 0x9C4 },
	{ 0x06, 0x104 },
	{ 0x12, 0x104 },
	{ 0x14, 0x101 },
};

StateConfig* CActorShoot::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		assert((state - 6) < 10);
		pStateConfig = _gStateCfg_SHT + state + -6;
	}

	return pStateConfig;
}

void CActorShoot::ChangeManageState(int state)
{
	bool bVar1;

	CActor::ChangeManageState(state);

	if ((state == 0) && (bVar1 = this->staticMeshComponent.HasMesh(), bVar1 != false)) {
		this->staticMeshComponent.Term(CScene::_scene_handleA);
	}

	return;
}

int CActorShoot::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CLifeInterface* pCVar2;
	int iVar3;
	edF32VECTOR4* v0;
	float fVar4;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	if (msg == 3) {
		pCVar2 = GetLifeInterface();
		fVar4 = pCVar2->GetValue();
		if (0.0f < fVar4) {
			if ((pSender->typeID != JAMGUT) && (pSender->typeID != ACTOR_HERO_PRIVATE)) {
				SetState(0xf, -1);

				return 1;
			}

			if ((this->field_0x3f0 & 4) == 0) {
				SetState(0xf, -1);

				return 1;
			}
		}
	}
	else {
		if (msg != 2) {
			iVar3 = CActor::InterpretMessage(pSender, msg, pMsgParam);
			return iVar3;
		}

		_msg_hit_param* pHitParam = (_msg_hit_param*)pMsgParam;

		if ((this->field_0x43c != 0) && (pHitParam->projectileType == 4)) {
			this->dynamicExt.normalizedTranslation.x = 0.0f;
			this->dynamicExt.normalizedTranslation.y = 0.0f;
			this->dynamicExt.normalizedTranslation.z = 0.0f;
			this->dynamicExt.normalizedTranslation.w = 0.0f;
			this->dynamicExt.field_0x6c = 0.0f;

			eStack16.y = 1.0f;
			eStack16.x = 0.0f;
			eStack16.z = 0.0f;
			eStack16.w = 0.0f;

			edF32Vector4ScaleHard(300.0f, &eStack16, &eStack16);
			edF32Vector4ScaleHard(0.02f / GetTimer()->cutsceneDeltaTime, &eStack32, &eStack16);
			v0 = this->dynamicExt.aImpulseVelocities;
			edF32Vector4AddHard(v0, v0, &eStack32);
			fVar4 = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities);
			this->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar4;

			LifeDecrease(pHitParam->damage);

			pCVar2 = GetLifeInterface();
			fVar4 = pCVar2->GetValue();
			if (fVar4 <= 0.0f) {
				SetState(0xf, -1);
			}
			else {
				if (this->field_0x438 == (CActorHero*)0x0) {
					SetState(8, 0x13);
					this->field_0x438 = CActorHero::_gThis;
				}
				else {
					if (this->currentAnimType == 0x13) {
						RestartCurAnim();
					}
					else {
						PlayAnim(0x13);
					}
				}
			}

			return 1;
		}
	}

	return 0;
}

void CActorShoot::ClearLocalData()
{
	float fVar1;
	float fVar2;
	bool bVar3;
	int iVar4;
	edF32MATRIX4* peVar5;
	edF32MATRIX4* peVar6;
	float fVar7;
	CAnimation* pAnim;

	bVar3 = this->staticMeshComponent.HasMesh();
	if (bVar3 != false) {
		this->staticMeshComponent.Term((ed_3D_Scene*)0x0);
	}
	this->staticMeshComponent.Reset();

	this->field_0x438 = (CActorHero*)0x0;
	this->field_0x43c = true;

	this->field_0x3f8 = this->field_0x3cc;
	this->field_0x3fc = this->field_0x3d4;
	this->field_0x3f4 = 0.0f;
	this->field_0x400 = 0;
	this->field_0x440 = 0;

	this->lightAmbient = gF32Vector4Zero;
	this->lightDirection = gF32Matrix4Unit;
	this->lightColor = gF32Matrix4Unit;
	
	(this->lightingConfig).pLightAmbient = &this->lightAmbient;
	(this->lightingConfig).pLightDirections = &this->lightDirection;
	(this->lightingConfig).pLightColorMatrix = &this->lightColor;

	this->field_0x43f = 0;
	this->field_0x43e = false;

	pAnim = this->pAnimationController;
	iVar4 = GetIdMacroAnim(0x11);
	if (iVar4 < 0) {
		fVar7 = 0.0f;
	}
	else {
		fVar7 = pAnim->GetAnimLength(iVar4, 0);
	}

	if (this->field_0x3cc < fVar7 + 0.15f) {
		this->field_0x3cc = fVar7 + 0.15f;
	}

	return;
}

void CBehaviourShoot::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourShoot::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorShoot*>(pOwner);

	return;
}

int CBehaviourShoot::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return 0;
}

int CBehaviourShoot::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourShootFire::Create(ByteCode* pByteCode)
{
	this->field_0x2b0 = pByteCode->GetU32();
	this->field_0x8 = pByteCode->GetF32();
	this->field_0x2b4 = pByteCode->GetF32();

	this->fireshot.Create(pByteCode);

	return;
}

void CBehaviourShootFire::Init(CActor * pOwner)
{
	this->fireshot.Init();

	return;
}

void CBehaviourShootFire::Manage()
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourShootFire::Draw()
{
	return;
}

void CBehaviourShootFire::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorShoot* pShoot;

	CBehaviourShoot::Begin(pOwner, newState, newAnimationType);

	this->fireshot.Reset();

	if (newState == -1) {
		pShoot = this->pOwner;
		pShoot->SetState(6, -1);
	}
	else {
		pShoot = this->pOwner;
		pShoot->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourShootFire::InitState(int newState)
{
	int iVar1;
	ed_3d_hierarchy_node* peVar2;
	CCollision* pCVar3;
	bool bVar4;
	StateConfig* pSVar5;
	edF32VECTOR4* peVar6;
	uint uVar7;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;
	CActorShoot* pShoot;

	pShoot = this->pOwner;

	if ((pShoot->GetStateFlags(newState) & 0x800) == 0) {
		iVar1 = pShoot->prevActorState;

		if ((pShoot->GetStateFlags(iVar1) & 0x800) != 0) {
			pShoot->pAnimationController->UnRegisterBone(pShoot->field_0x350);
			pShoot->pAnimationController->UnRegisterBone(pShoot->field_0x354);
			pShoot->SetLookingAtOff();
		}
	}

	if ((pShoot->GetStateFlags(newState) & 0x800) != 0) {
		iVar1 = pShoot->prevActorState;

		if ((pShoot->GetStateFlags(iVar1) & 0x800) != 0) {
			pShoot->pAnimationController->RegisterBone(pShoot->field_0x350);
			pShoot->pAnimationController->RegisterBone(pShoot->field_0x354);
			pShoot->SetLookingAtBones(pShoot->field_0x354, pShoot->field_0x350);
			pShoot->SetLookingAtBounds(-0.08726646f, 0.08726646f, -0.7853982f, 0.7853982f);
		}
	}

	if (newState != 8) {
		if (newState == 0xf) {
			pCVar3 = pShoot->pCollisionData;
			peVar6 = pShoot->GetBottomPosition();
			edF32Vector4AddHard(&eStack16, &pCVar3->highestVertex, peVar6);
			edF32Vector4ScaleHard(0.5f, &eStack16, &eStack16);
			pShoot->addOnGenerator.Generate(&eStack16);
		}
		else {
			if (newState == 0xc) {
				pShoot->pAnimationController->RegisterBone(this->field_0x2b0);

				IMPLEMENTATION_GUARD(
				pShoot->dynamicExt.rotationEuler.x = 0.0f;
				pShoot->dynamicExt.rotationEuler.y = 0.0f;
				pShoot->dynamicExt.rotationEuler.z = 0.0f;
				pShoot->dynamicExt.rotationEuler.w = 0.0f;
				pShoot->dynamicExt.scale.w = 0.0f;)
				pShoot->dynamic.speed = 0.0f;
			}
			else {
				if (newState == 7) {
					if ((pShoot->field_0x3f0 & 1) != 0) {
						pShoot->flags = pShoot->flags & 0xffffff7f;
						pShoot->flags = pShoot->flags | 0x20;
						pShoot->EvaluateDisplayState();
					}

					bVar4 = (pShoot->staticMeshComponent).textureIndex != -1;
					if (bVar4) {
						bVar4 = (pShoot->staticMeshComponent).meshIndex != -1;
					}

					if (bVar4) {
						pShoot->staticMeshComponent.Init(CScene::_scene_handleA, (ed_g3d_manager*)0x0, &pShoot->altHierarchySetup, (char*)0x0);
						eStack48 = gF32Vector4UnitY;
						peVar6 = &pShoot->currentLocation;
						bVar4 = pShoot->staticMeshComponent.HasMesh();
						if ((bVar4 != false) && (peVar6 != (edF32VECTOR4*)0x0)) {
							edFIntervalLERP(0.0f, 0.0f, 0.4f, 0.0f, 0.32f);
							edF32Vector4ScaleHard(-0.32f, &eStack48, &eStack48);
							edF32Vector4AddHard(&eStack32, peVar6, &eStack48);
							peVar2 = (pShoot->staticMeshComponent).pMeshTransformData;
							if (peVar2 != (ed_3d_hierarchy_node*)0x0) {
								pShoot->staticMeshComponent.pMeshTransformData->base.transformA.rowT = eStack32;
							}
						}
					}
				}
				else {
					if ((newState == 6) && ((pShoot->field_0x3f0 & 1) != 0)) {
						pShoot->flags = pShoot->flags & 0xffffff7f;
						pShoot->flags = pShoot->flags | 0x20;
						pShoot->EvaluateDisplayState();
					}
				}
			}
		}
	}

	return;
}

void CBehaviourShootFire::TermState(int oldState, int newState)
{
	bool bVar1;
	CActorShoot* pShoot;

	pShoot = this->pOwner;
	if (oldState == 0xe) {
		bVar1 = pShoot->staticMeshComponent.HasMesh();
		if (bVar1 != false) {
			pShoot->staticMeshComponent.Term(CScene::_scene_handleA);
		}
	}
	else {
		if (oldState == 0xc) {
			pShoot->pAnimationController->UnRegisterBone(this->field_0x2b0);
			pShoot->SetLookingAtOff();
		}
		else {
			if (oldState == 7) {
				if ((pShoot->field_0x3f0 & 1) != 0) {
					pShoot->flags = pShoot->flags & 0xffffff5f;
					pShoot->EvaluateDisplayState();
				}
			}
			else {
				if ((oldState == 6) && ((pShoot->field_0x3f0 & 1) != 0)) {
					pShoot->flags = pShoot->flags & 0xffffff5f;
					pShoot->EvaluateDisplayState();
				}
			}
		}
	}

	return;
}

int CBehaviourShootFire::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return 0;
}

void CBehaviourShootFire::Reset()
{
	this->fireshot.Reset();

	return;
}
