#include "ActorShocker.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "TimeController.h"
#include "ActorHero.h"
#include "ActorBoomy.h"
#include "AnmManager.h"
#include "EventManager.h"
#include "FileManager3D.h"

StateConfig CActorShocker::_gStateCfg_SHK[20] =
{
	StateConfig(0x00, 0x104),
	StateConfig(0x0c, 0x084),
	StateConfig(0x0d, 0x084),
	StateConfig(0x0e, 0x0c4),
	StateConfig(0x0e, 0x0c4),
	StateConfig(0x0e, 0x0c4),
	StateConfig(0x0f, 0x0c4),
	StateConfig(0x10, 0x0c4),
	StateConfig(0x11, 0x1c4),
	StateConfig(0x12, 0x0c4),
	StateConfig(0x13, 0x0c4),
	StateConfig(0x0e, 0x0c4),
	StateConfig(0x16, 0x0c4),
	StateConfig(0x0e, 0x084),
	StateConfig(0x0e, 0x084),
	StateConfig(0x14, 0x084),
	StateConfig(0x15, 0x084),
	StateConfig(0x19, 0x101),
	StateConfig(0x19, 0x901),
	StateConfig(0x19, 0x901)
};

CActorShocker::CActorShocker()
{
	this->field_0x780 = (edNODE*)0x0;
	this->field_0x77c = (ed_3d_hierarchy*)0x0;

	return;
}

CActorShocker::~CActorShocker()
{
	if (this->field_0x780 != (edNODE*)0x0) {
		ed3DHierarchyRemoveFromScene(CScene::_scene_handleA, this->field_0x780);
	}
	this->field_0x780 = (edNODE*)0x0;
	this->addOnGenerator.Term();

	return;
}

void CActorShocker::Create(ByteCode* pByteCode)
{
	CActorAutonomous::Create(pByteCode);

	this->field_0x350 = pByteCode->GetF32();
	this->field_0x354 = pByteCode->GetF32();
	this->field_0x358 = pByteCode->GetF32();
	this->field_0x35c = pByteCode->GetF32();
	this->field_0x364 = pByteCode->GetF32();
	this->field_0x368 = pByteCode->GetF32();
	this->field_0x360 = pByteCode->GetF32();
	this->field_0xf0 = pByteCode->GetF32();
	this->field_0x36c = pByteCode->GetF32();
	this->field_0x374 = pByteCode->GetU32();
	this->field_0x370 = pByteCode->GetU32();
	this->field_0x378.Create(pByteCode);
	this->field_0x380 = pByteCode->GetU32();
	this->field_0x384 = pByteCode->GetS32();
	this->field_0x388 = pByteCode->GetS32();
	this->field_0x38c = pByteCode->GetS32();
	this->field_0x390 = pByteCode->GetS32();
	this->field_0x394 = pByteCode->GetS32();
	this->field_0x398 = pByteCode->GetS32();
	this->boneId_0x39c = pByteCode->GetU32();

	FUN_001156e0(this->field_0x394, 0, (edF32VECTOR4*)0x0, &this->field_0x784);
	FUN_001156e0(this->field_0x394, 1, (edF32VECTOR4*)0x0, &this->field_0x794);
	this->addOnGenerator.Create(this, pByteCode);

	return;
}

void CActorShocker::Init()
{
	CActorAutonomous::Init();
	this->field_0x378.Init();

	this->fxSpark.Create(2, 12, this->fxSpark.aVectorData, this->fxSpark.aFloatData,
		this->behaviourShockerFireWave.circularWaveShoot.field_0x20);
	this->fxSpark.field_0xe4 = this->fxSpark.aUnknown;
	this->fxSpark.SetParameters(0.3f, 0.01f, 10.0f, 0.08333334f, 8.0f, 1);
	this->fxSpark.Init(this->sectorId);

	ClearLocalData();

	memset(&this->field_0x748, 0, sizeof(this->field_0x748));
	this->boundingSphere = edF32VECTOR4{ 0.0f, 0.0f, 0.0f, 200.0f };
	this->field_0x748.pBoundingSphere = &this->boundingSphere;
	this->clipping = 150.0f;
	this->field_0x748.clipping_0x0 = &this->clipping;

	edF32VECTOR4 animSpeed = {};
	FUN_001156e0(this->field_0x394, 0, &animSpeed, (edF32VECTOR4*)0x0);
	FUN_001156e0(this->field_0x394, 1, &animSpeed, (edF32VECTOR4*)0x0);
	this->addOnGenerator.Init(0);

	return;
}

// 0x003d2830: original extrusion duration calculation.
static int GetShockerExtrusionTime(float amount, edF32VECTOR4* pSpeed)
{
	int result = 0;
	if ((pSpeed->x != 0.0f) || (pSpeed->y != 0.0f)) {
		if ((pSpeed->y == 0.0f) || (pSpeed->x <= pSpeed->y)) {
			result = (int)(amount / pSpeed->x);
		}
		else {
			if ((pSpeed->x == 0.0f) || (pSpeed->y <= pSpeed->x)) {
				result = (int)(amount / pSpeed->y);
			}
		}
	}
	return result;
}

void CActorShocker::ClearLocalData()
{
	this->field_0x3a0 = (CActor*)0x0;
	this->field_0x3a9 = true;
	this->field_0x474 = true;
	this->field_0x740 = 1;
	this->field_0x744 = 0.0f;
	this->lightAmbient = gF32Vector4Zero;
	this->lightDirections = gF32Matrix4Unit;
	this->lightColors = gF32Matrix4Unit;
	this->field_0x440.pLightAmbient = &this->lightAmbient;
	this->field_0x440.pLightDirections = &this->lightDirections;
	this->field_0x440.pLightColorMatrix = &this->lightColors;
	this->fxSpark.Reset();
	this->field_0x3a4 = (CActor*)0x0;
	this->field_0x3a8 = 0;
	this->field_0x7a4 = 0;
	this->field_0x3aa = false;
	this->field_0x7b0 = GetShockerExtrusionTime(0.2f, &this->field_0x784);
	this->field_0x7a8 = GetShockerExtrusionTime(1.0f, &this->field_0x784);
	this->field_0x7ac = GetShockerExtrusionTime(1.0f, &this->field_0x794);
	this->pCollisionData->flags_0x0 = (this->pCollisionData->flags_0x0 & 0xfffffffd) | 0x9000;

	return;
}

void CActorShocker::Reset()
{
	CActorAutonomous::Reset();
	ClearLocalData();

	return;
}

CBehaviour* CActorShocker::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;
	if (behaviourType == SHOCKER_BEHAVIOUR_FIRE_WAVE) {
		pBehaviour = &this->behaviourShockerFireWave;
	}
	else {
		pBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
	}
	return pBehaviour;
}

StateConfig* CActorShocker::GetStateCfg(int state)
{
	StateConfig* pStateConfig;
	if (state < SHOCKER_STATE_SLEEP) {
		pStateConfig = CActorAutonomous::GetStateCfg(state);
	}
	else {
		assert((state - SHOCKER_STATE_SLEEP) < 20);
		pStateConfig = _gStateCfg_SHK + state - SHOCKER_STATE_SLEEP;
	}
	return pStateConfig;
}

bool CActorShocker::Can_0x9c()
{
	bool result = CActor::Can_0x9c();
	return result & (this->field_0x3a0 != (CActor*)0x0);
}

void CActorShocker::ChangeVisibleState(int bVisible)
{
	CActor::ChangeVisibleState(bVisible);
	if (bVisible == 0) {
		FUN_003d11e0();
	}
	return;
}

void CActorShocker::ChangeDisplayState(int state)
{
	CActor::ChangeDisplayState(state);
	if (state == 0) {
		FUN_003d11e0();
	}
	return;
}

void CActorShocker::ChangeManageState(int state)
{
	CActorAutonomous::ChangeManageState(state);
	if (state == 0) {
		FUN_003d11e0();
	}
	return;
}

int CActorShocker::CheckArea()
{
	CEventManager* pEventManager = CScene::ptable.g_EventManager_006f5080;
	ed_zone_3d* pInnerZone = (ed_zone_3d*)0x0;
	ed_zone_3d* pOuterZone = (ed_zone_3d*)0x0;
	if ((this->field_0x370 != -1) && (this->field_0x374 != -1)) {
		pInnerZone = edEventGetChunkZone(pEventManager->activeChunkId, this->field_0x374);
		pOuterZone = edEventGetChunkZone(pEventManager->activeChunkId, this->field_0x370);
	}
	CActorHero* pHero = CActorHero::_gThis;
	int innerResult = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pInnerZone, &pHero->currentLocation, 0);
	int outerResult = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pOuterZone, &pHero->currentLocation, 0);
	int result;
	if ((outerResult == 1) || (innerResult == 1)) {
		result = 1;
		if (innerResult == 1) {
			this->field_0x3a0 = pHero;
		}
		else {
			result = -1;
		}
	}
	else {
		this->field_0x3a0 = (CActor*)0x0;
		result = 2;
	}
	return result;
}

void CActorShocker::ComputeLighting()
{
	CScene::ptable.g_LightManager_004516b0->ComputeLighting(this->lightingFloat_0xe0, this, this->lightingFlags, &this->field_0x440);
	if (this->field_0x474 || (this->actorState == SHOCKER_STATE_RECHARGE)) {
		this->field_0x3ac = this->field_0x3ac + 2.0f;
		if (100.0f < this->field_0x3ac) {
			this->field_0x3ac = 100.0f;
		}
		float intensity = this->field_0x3ac + ((float)rand() / 2.147484e+09f * 10.0f) * 15.0f;
		this->lightAmbient.x = intensity;
		this->lightAmbient.y = intensity;
		this->lightAmbient.z = intensity;
	}
	else {
		this->field_0x3ac = 0.0f;
	}
	return;
}

void CActorShocker::BehaviourShockerFireWave_Manage(CBehaviourShockerFireWave* pBehaviour)
{
	CCollision *pCVar1;
	CAnimation* pCVar2;
	bool bVar5;
	float fVar6;
	uint uVar8;
	int iVar9;
	float fVar10;
	_msg_hit_param local_990 = {};
	_msg_hit_param local_910 = {};
	_msg_hit_param local_890 = {};
	_msg_hit_param local_810 = {};
	_msg_hit_param local_790 = {};
	_msg_hit_param local_710 = {};
	edF32VECTOR4 local_690;
	edF32VECTOR4 AStack1664;
	CActorsTable local_670;
	CActorsTable local_560;
	CActorsTable local_450;
	CActorsTable local_340;
	CActorsTable local_230;
	CActorsTable local_120;
	_msg_hit_param* local_18;
	_msg_hit_param* local_14;
	_msg_hit_param* local_10;
	_msg_hit_param* local_c;
	_msg_hit_param* local_8;
	_msg_hit_param* local_4;

	ComputeInvincibility();
	this->field_0x378.pStreamEventCamera->Manage(this);
	uVar8 = GetStateFlags(this->actorState);
	if ((uVar8 & 0x800) != 0) {
		this->field_0x7a4 = this->field_0x7a4 + 1;
	}
	switch (this->actorState) {
	case SHOCKER_STATE_SLEEP:
		local_120.nbEntries = 0;
		ManageDyn(4.0f, 0x100a023b, &local_120);
		iVar9 = 0;
		if (this->field_0x474 != false) {
			for (; iVar9 < local_120.nbEntries; iVar9 = iVar9 + 1) {
				local_710.projectileType = 5;
				local_710.damage = pBehaviour->circularWaveShoot.field_0x18;
				local_4 = &local_710;
				DoMessage(local_120.aEntries[iVar9], MESSAGE_KICKED, local_4);
			}
		}
		iVar9 = CheckArea();
		if (iVar9 == 1) {
			SetState(SHOCKER_STATE_STAND_UP_1_3, -1);
		}
		break;
	case SHOCKER_STATE_STAND_UP_1_3:
		StateShockerStandUp_1_3(pBehaviour);
		break;
	case SHOCKER_STATE_STAND_UP_2_3:
		StateShockerStandUp_2_3(pBehaviour);
		break;
	case SHOCKER_STATE_STAND_UP_3_3:
		StateShockerStandUp_3_3(pBehaviour);
		break;
	case SHOCKER_STATE_CHASE:
		StateShockerChase(pBehaviour);
		break;
	case SHOCKER_STATE_WAIT_COMMAND:
		local_230.nbEntries = 0;
		ManageDyn(4.0f, 1, &local_230);
		iVar9 = 0;
		if (this->field_0x474 != false) {
			for (; iVar9 < local_230.nbEntries; iVar9 = iVar9 + 1) {
				local_790.projectileType = 5;
				local_790.damage = pBehaviour->circularWaveShoot.field_0x18;
				local_8 = &local_790;
				DoMessage(local_230.aEntries[iVar9], MESSAGE_KICKED, local_8);
			}
		}
		fVar10 = (this->field_0x3a0->currentLocation).x - this->currentLocation.x;
		fVar6 = (this->field_0x3a0->currentLocation).z - this->currentLocation.z;
		if ((sqrtf(fVar10 * fVar10 + 0.0f + fVar6 * fVar6) <= this->field_0x368) ||
			(this->field_0xf0 <= this->distanceToGround)) {
			iVar9 = CheckArea();
			if (iVar9 == 2) {
				SetState(SHOCKER_STATE_COME_BACK, -1);
			}
		}
		else {
			SetState(SHOCKER_STATE_CHASE, -1);
		}
		break;
	case SHOCKER_STATE_FIRE_WAVE_WIND_UP:
		if (this->field_0x3a0 != (CActor*)0x0) {
			SV_UpdateOrientationToPosition2D(this->field_0x35c, &this->field_0x3a0->currentLocation);
		}
		local_340.nbEntries = 0;
		ManageDyn(4.0f, 0x129, &local_340);
		iVar9 = 0;
		if (this->field_0x474 != false) {
			for (; iVar9 < local_340.nbEntries; iVar9 = iVar9 + 1) {
				local_810.projectileType = 5;
				local_810.damage = pBehaviour->circularWaveShoot.field_0x18;
				local_c = &local_810;
				DoMessage(local_340.aEntries[iVar9], MESSAGE_KICKED, local_c);
			}
		}
		bVar5 = this->pAnimationController->IsCurrentLayerAnimEndReached(0);
		if (bVar5) {
			SetState(SHOCKER_STATE_FIRE_WAVE_FALL, -1);
		}
		break;
	case SHOCKER_STATE_FIRE_WAVE_FALL:
		if (this->field_0x3a0 != (CActor*)0x0) {
			SV_UpdateOrientationToPosition2D(this->field_0x35c, &this->field_0x3a0->currentLocation);
		}
		local_450.nbEntries = 0;
		ManageDyn(4.0f, 0x129, &local_450);
		iVar9 = 0;
		if (this->field_0x474 != false) {
			for (; iVar9 < local_450.nbEntries; iVar9 = iVar9 + 1) {
				local_890.projectileType = 5;
				local_890.damage = pBehaviour->circularWaveShoot.field_0x18;
				local_10 = &local_890;
				DoMessage(local_450.aEntries[iVar9], MESSAGE_KICKED, local_10);
			}
		}
		pCVar1 = this->pCollisionData;
		if ((pCVar1->flags_0x4 & 2) != 0) {
			pBehaviour->circularWaveShoot.Fire(&pCVar1->aCollisionContact[1].field_0x10);
			SetState(SHOCKER_STATE_FIRE_WAVE_LAND, -1);
		}
		break;
	case SHOCKER_STATE_FIRE_WAVE_LAND:
		if (this->field_0x3a0 != (CActor*)0x0) {
			SV_UpdateOrientationToPosition2D(this->field_0x35c, &this->field_0x3a0->currentLocation);
		}
		local_560.nbEntries = 0;
		ManageDyn(4.0f, 0x1002023b, &local_560);
		iVar9 = 0;
		if (this->field_0x474 != false) {
			for (; iVar9 < local_560.nbEntries; iVar9 = iVar9 + 1) {
				local_910.projectileType = 5;
				local_910.damage = pBehaviour->circularWaveShoot.field_0x18;
				local_14 = &local_910;
				DoMessage(local_560.aEntries[iVar9], MESSAGE_KICKED, local_14);
			}
		}
		pCVar2 = this->pAnimationController;
		iVar9 = GetIdMacroAnim(0x11);
		if (iVar9 < 0) {
			fVar10 = 0.0f;
		}
		else {
			fVar10 = pCVar2->GetAnimLength(iVar9, 2);
		}
		if (fVar10 < this->timeInAir) {
			SetState(SHOCKER_STATE_STAND_UP_TIRED_1_2, -1);
		}
		break;
	case SHOCKER_STATE_STAND_UP_TIRED_1_2:
		StateShockerStandUpTired_1_2(pBehaviour);
		break;
	case SHOCKER_STATE_STAND_UP_TIRED_2_2:
		StateShockerStandUpTired_2_2(pBehaviour);
		break;
	case SHOCKER_STATE_RECHARGE:
		StateShockerRecharge();
		break;
	case SHOCKER_STATE_COME_BACK:
		StateShockerComeBack(pBehaviour);
		break;
	case SHOCKER_STATE_GO_TO_SLEEP_1_3:
		StateShockerGoToSleep_1_3(pBehaviour);
		break;
	case SHOCKER_STATE_GO_TO_SLEEP_2_3:
		StateShockerGoToSleep_2_3(pBehaviour);
		break;
	case SHOCKER_STATE_GO_TO_SLEEP_3_3:
		local_670.nbEntries = 0;
		ManageDyn(4.0f, 0, &local_670);
		iVar9 = 0;
		if (this->field_0x474 != false) {
			for (; iVar9 < local_670.nbEntries; iVar9 = iVar9 + 1) {
				local_990.projectileType = 5;
				local_990.damage = pBehaviour->circularWaveShoot.field_0x18;
				local_18 = &local_990;
				DoMessage(local_670.aEntries[iVar9], MESSAGE_KICKED, local_18);
			}
		}
		bVar5 = this->pAnimationController->IsCurrentLayerAnimEndReached(0);
		if (bVar5) {
			SetState(SHOCKER_STATE_SLEEP, -1);
		}
		break;
	case SHOCKER_STATE_DEATH:
		bVar5 = this->pAnimationController->IsCurrentLayerAnimEndReached(0);
		if (bVar5) {
			if ((this->field_0x394 == -1) && (this->field_0x398 == -1)) {
				this->flags = this->flags & 0xfffffffd;
				this->flags = this->flags | 1;
			}
			this->flags = this->flags & 0xffffff7f;
			this->flags = this->flags | 0x20;
			EvaluateDisplayState();
			FUN_003d11e0();
		}
		if (((0.4f < this->timeInAir) &&
				(pBehaviour->circularWaveShoot.Reset(), this->field_0x394 != -1))
			&& (this->field_0x398 != -1)) {
			SetState(SHOCKER_STATE_DEATH_EXTRUDE, -1);
		}
		break;
	case SHOCKER_STATE_DEATH_EXTRUDE:
		if (this->field_0x7b0 < this->field_0x7a4) {
			SetState(SHOCKER_STATE_DEATH_FADE, -1);
			FUN_003d11e0();
		}
		else {
			FUN_001156e0(this->field_0x394, 1, (edF32VECTOR4*)0x0, &AStack1664);
		}
		break;
	case SHOCKER_STATE_DEATH_FADE:
		FUN_003ce190();
	}
	iVar9 = this->currentAnimType;
	if (((iVar9 == 0x16) || (iVar9 == 0x17)) || (iVar9 == 0x18)) {
		bVar5 = this->pAnimationController->IsCurrentLayerAnimEndReached(0);
		if (bVar5) {
			SetState(this->actorState, -1);
		}
	}
	pBehaviour->circularWaveShoot.UpdateWaveLife();
	ManageSparksBoomy();
	FUN_003d1b10();
	if (this->field_0x464.IsValid()) {
		edF32Vector4ScaleHard(1.0f, &local_690, &this->pMeshTransform->base.transformA.rowY);
		edF32Vector4AddHard(&local_690, &this->pMeshTransform->base.transformA.rowT, &local_690);
		this->field_0x464.SetPosition(&local_690);
	}
	return;
}

void CActorShocker::ManageSparksBoomy()
{
	CActor* pOther = CActorBoomy::_gThis->pHero;
	float distance = 0.0f;
	edF32VECTOR4 source;
	edF32VECTOR4 target;
	edF32VECTOR4 delta;
	if (this->field_0x740 != 1) {
		SV_GetBoneWorldPosition(this->boneId_0x39c, &source);
		target = *pOther->GetBottomPosition();
		edF32Vector4SubHard(&delta, &target, &source);
		distance = edF32Vector4NormalizeHard(&delta, &delta);
	}

	if (this->field_0x740 == 8) {
		uint boneId = DoMessage(pOther, MESSAGE_GET_BONE_ID, (MSG_PARAM)0xc);
		if (boneId != 1) {
			if (!this->field_0x46c.IsValid()) {
				CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&this->field_0x46c, this->field_0x390, FX_MATERIAL_SELECTOR_NONE);
				this->field_0x46c.Start();
				this->field_0x46c.SpatializeOnActor(2, CActorHero::_gThis, boneId);
			}
			pOther->SV_GetBoneWorldPosition(boneId, &target);
		}
		this->field_0x744 = this->field_0x744 + Timer::GetTimer()->cutsceneDeltaTime;
		if (1.0f < this->field_0x744) {
			boneId = DoMessage(pOther, MESSAGE_GET_BONE_ID, (MSG_PARAM)0xc);
			pOther->pAnimationController->UnRegisterBone(boneId);
			this->field_0x740 = 1;
		}
	}
	else {
		if (this->field_0x740 == 4) {
			uint boneId = DoMessage(pOther, MESSAGE_GET_BONE_ID, (MSG_PARAM)0xb);
			if (boneId != 1) {
				pOther->SV_GetBoneWorldPosition(boneId, &target);
				if (!this->field_0x46c.IsValid()) {
					CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&this->field_0x46c, this->field_0x390, FX_MATERIAL_SELECTOR_NONE);
					this->field_0x46c.Start();
					this->field_0x46c.SpatializeOnActor(2, CActorHero::_gThis, boneId);
				}
			}
			if (this->actorState != 0xd7) {
				boneId = DoMessage(pOther, MESSAGE_GET_BONE_ID, (MSG_PARAM)0xc);
				pOther->pAnimationController->RegisterBone(boneId);
				this->field_0x46c.SpatializeOnActor(2, CActorHero::_gThis, boneId);
				boneId = DoMessage(pOther, MESSAGE_GET_BONE_ID, (MSG_PARAM)0xb);
				pOther->pAnimationController->UnRegisterBone(boneId);
				this->field_0x740 = 8;
			}
		}
		else {
			if (this->field_0x740 != 2) {
				if (this->field_0x740 != 1) {
					return;
				}
				if (this->field_0x46c.IsValid()) {
					this->field_0x46c.Kill();
					this->field_0x46c.Reset();
				}
				return;
			}
			if (!this->field_0x46c.IsValid()) {
				CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&this->field_0x46c, this->field_0x390, FX_MATERIAL_SELECTOR_NONE);
				this->field_0x46c.Start();
				this->field_0x46c.SpatializeOnActor(2, CActorBoomy::_gThis, 0);
			}
			target = CActorBoomy::_gThis->currentLocation;
			if (CActorBoomy::_gThis->actorState == 9) {
				this->field_0x740 = 4;
				uint boneId = DoMessage(pOther, MESSAGE_GET_BONE_ID, (MSG_PARAM)0xb);
				pOther->pAnimationController->RegisterBone(boneId);
				this->field_0x46c.SpatializeOnActor(2, CActorHero::_gThis, boneId);
			}
		}
	}

	edF32Vector4SubHard(&delta, &target, &source);
	float length = edF32Vector4NormalizeHard(&delta, &delta);
	edF32Vector4ScaleHard(distance, &delta, &delta);
	edF32Vector4AddHard(&target, &delta, &source);
	this->fxSpark.Manage(&source, &target);
	edF32MATRIX4 matrix;
	edF32Matrix4MulF32Hard(length / distance, &matrix, &this->fxSpark.field_0x20);
	matrix.rowT = source;
	this->fxSpark.field_0x20 = matrix;
	this->fxSpark.Draw(this->field_0x740 != 1);

	return;
}

void CActorShocker::StateShockerRecharge()
{
	edF32VECTOR4 position;
	if (this->field_0x45c.IsValid()) {
		edF32Vector4ScaleHard(1.0f, &position, &this->pMeshTransform->base.transformA.rowY);
		edF32Vector4AddHard(&position, &this->pMeshTransform->base.transformA.rowT, &position);
		this->field_0x45c.SetPosition(&position);
	}
	if (1.0f < this->timeInAir) {
		this->field_0x474 = true;
	}
	if (2.2f < this->timeInAir) {
		if (CheckArea() == 2) {
			SetState(SHOCKER_STATE_COME_BACK, -1);
		}
		else {
			if (this->field_0x3a0 != (CActor*)0x0) {
				float x = this->field_0x3a0->currentLocation.x - this->currentLocation.x;
				float z = this->field_0x3a0->currentLocation.z - this->currentLocation.z;
				if ((this->field_0x364 < sqrtf(x * x + z * z)) || (this->field_0xf0 <= this->distanceToGround)) {
					SetState(SHOCKER_STATE_CHASE, -1);
				}
				else {
					this->dynamic.speed = 0.0f;
					if (this->field_0x3a4 == (CActor*)0x0) {
						SetState(SHOCKER_STATE_FIRE_WAVE_WIND_UP, -1);
					}
					else {
						SetState(SHOCKER_STATE_WAIT_COMMAND, -1);
					}
				}
			}
		}
	}
	return;
}

void CActorShocker::BehaviourShockerFireWave_InitState(int newState)
{
	switch (newState) {
	case SHOCKER_STATE_GO_TO_SLEEP_3_3:
		this->dynamicExt.normalizedTranslation = gF32Vector4Zero;
		this->dynamicExt.field_0x6c = 0.0f;
		this->dynamic.speed = 0.0f;
		break;
	case SHOCKER_STATE_WAIT_COMMAND:
	{
		int command = 2;
		DoMessage(this->field_0x3a4, static_cast<ACTOR_MESSAGE>(0x69), &command);
		break;
	}
	case SHOCKER_STATE_RECHARGE:
		if (!this->field_0x45c.IsValid()) {
			CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&this->field_0x45c, this->field_0x388, FX_MATERIAL_SELECTOR_NONE);
			this->field_0x45c.Start();
			if ((this->field_0x45c.pFx != (CNewFx*)0x0) && (this->field_0x45c.id != 0)) {
				this->field_0x45c.pFx->SetTimeScaler(2.0f);
			}
		}
		break;
	case SHOCKER_STATE_FIRE_WAVE_LAND:
		if (!this->field_0x464.IsValid()) {
			CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&this->field_0x464, this->field_0x38c, FX_MATERIAL_SELECTOR_NONE);
			this->field_0x464.Start();
			if ((this->field_0x464.pFx != (CNewFx*)0x0) && (this->field_0x464.id != 0)) {
				this->field_0x464.pFx->SetTimeScaler(4.0f);
			}
		}
		break;
	case SHOCKER_STATE_DEATH_FADE:
		if (this->field_0x780 != (edNODE*)0x0) {
			FUN_001156e0(this->field_0x394, 0, &this->field_0x784, (edF32VECTOR4*)0x0);
		}
		break;
	case SHOCKER_STATE_DEATH_EXTRUDE:
		FUN_003ce330();
		break;
	case SHOCKER_STATE_DEATH:
	{
		if (this->field_0x45c.IsValid()) {
			this->field_0x45c.Kill();
			this->field_0x45c.Reset();
		}
		if (this->field_0x464.IsValid()) {
			this->field_0x464.Kill();
			this->field_0x464.Reset();
		}
		edF32VECTOR4 position;
		edF32Vector4AddHard(&position, &this->pCollisionData->highestVertex, GetBottomPosition());
		edF32Vector4ScaleHard(0.5f, &position, &position);
		this->pCollisionData->flags_0x0 = this->pCollisionData->flags_0x0 & 0xffff6ffd;
		this->addOnGenerator.Generate(&position);
		break;
	}
	}
	return;
}

void CActorShocker::FUN_003d11e0()
{
	this->fxSpark.Draw(false);
	if (this->field_0x45c.IsValid()) {
		this->field_0x45c.Stop();
	}
	if (this->field_0x454.IsValid()) {
		this->field_0x454.Stop();
		if ((this->pAnimationController != (CAnimation*)0x0) && this->field_0x3aa) {
			this->field_0x3aa = false;
			this->pAnimationController->UnRegisterBone(this->boneId_0x39c);
		}
	}
	if (this->field_0x464.IsValid()) {
		this->field_0x464.Stop();
	}
	if (this->field_0x46c.IsValid()) {
		this->field_0x46c.Stop();
	}
	if (this->field_0x780 != (edNODE*)0x0) {
		ed3DHierarchyRemoveFromScene(CScene::_scene_handleA, this->field_0x780);
	}
	this->field_0x780 = (edNODE*)0x0;
	return;
}

void CActorShocker::FUN_003d1b10()
{
	if (this->field_0x474) {
		if (!this->field_0x454.IsValid()) {
			CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&this->field_0x454, this->field_0x384, FX_MATERIAL_SELECTOR_NONE);
			this->field_0x454.Start();
			if ((this->pAnimationController != (CAnimation*)0x0) && this->field_0x454.IsValid()) {
				this->pAnimationController->RegisterBone(this->boneId_0x39c);
				this->field_0x3aa = true;
				this->field_0x454.SpatializeOnActor(2, this, this->boneId_0x39c);
			}
			else {
				this->field_0x454.SpatializeOnActor(2, this, 0);
			}
		}
	}
	else {
		if (this->field_0x454.IsValid()) {
			this->field_0x454.Stop();
			if ((this->pAnimationController != (CAnimation*)0x0) && this->field_0x3aa) {
				this->field_0x3aa = false;
				this->pAnimationController->UnRegisterBone(this->boneId_0x39c);
			}
		}
	}
	return;
}

void CActorShocker::FUN_003ce330()
{
	if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
		this->flags = (this->flags & 0xffffff7f) | 0x20;
		EvaluateDisplayState();
		FUN_003d11e0();
	}
	if ((this->field_0x780 == (edNODE*)0x0) && (this->field_0x398 != -1) && (this->field_0x394 != -1)) {
		ed_g3d_manager* pMeshManager = CScene::ptable.g_C3DFileManager_00451664->GetG3DManager(this->field_0x398, this->field_0x394);
		SV_InstallMaterialId(this->field_0x394);
		this->field_0x780 = ed3DHierarchyAddToScene(CScene::_scene_handleA, pMeshManager, "Loft_03");
		this->field_0x77c = static_cast<ed_3d_hierarchy*>(this->field_0x780->pData);
		ed3DHierarchySetSetup(this->field_0x77c, &this->field_0x748);
		ed3DHierarchyNodeSetRenderOff(CScene::_scene_handleA, this->field_0x780);
		edF32VECTOR4 position;
		SV_GetBoneWorldPosition(this->boneId_0x39c, &position);
		this->field_0x77c->transformA.rowT = position;
	}
	if (this->field_0x780 != (edNODE*)0x0) {
		FUN_001156e0(this->field_0x394, 1, &this->field_0x794, (edF32VECTOR4*)0x0);
	}
	this->field_0x7a4 = 0;
	return;
}

void CActorShocker::FUN_003ce190()
{
	edF32VECTOR4 zero = {};
	edF32VECTOR4 animSpeedA;
	edF32VECTOR4 animSpeedB;
	if (this->field_0x7ac < this->field_0x7a4) {
		FUN_001156e0(this->field_0x394, 1, &zero, (edF32VECTOR4*)0x0);
	}
	FUN_001156e0(this->field_0x394, 0, (edF32VECTOR4*)0x0, &animSpeedA);
	FUN_001156e0(this->field_0x394, 1, (edF32VECTOR4*)0x0, &animSpeedB);
	if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
		this->flags = (this->flags & 0xffffff7f) | 0x20;
		EvaluateDisplayState();
		FUN_003d11e0();
	}
	if ((this->field_0x394 != -1) && (this->field_0x7a8 < this->field_0x7a4)) {
		FUN_001156e0(this->field_0x394, 0, &zero, (edF32VECTOR4*)0x0);
		this->flags = (this->flags & 0xfffffffd) | 1;
		this->flags = (this->flags & 0xffffff7f) | 0x20;
		EvaluateDisplayState();
		FUN_003d11e0();
	}
	return;
}

int CActorShocker::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	if (msg == 0x69) {
		int command = *static_cast<int*>(pMsgParam);
		if (command == 1) {
			if (((GetStateFlags(this->actorState) & 1) == 0) || ((this->flags & 4) != 0)) {
				SetState(SHOCKER_STATE_FIRE_WAVE_WIND_UP, -1);
				return 1;
			}
		}
		else {
			if (command == 0) {
				this->field_0x3a4 = pSender;
				return 1;
			}
		}
	}
	else {
		if (msg != MESSAGE_KICKED) {
			if (msg != 3) {
				return CActorAutonomous::InterpretMessage(pSender, msg, pMsgParam);
			}
			if (0.0f < GetLifeInterface()->GetValue()) {
				SetState(SHOCKER_STATE_DEATH, -1);
				return 1;
			}
			return 0;
		}
		if (GetLifeInterface()->GetValue() <= 0.0f) {
			return 0;
		}
		_msg_hit_param* pHitParam = static_cast<_msg_hit_param*>(pMsgParam);
		if ((pHitParam->projectileType == 10) || ((pHitParam->projectileType == 4) && !this->field_0x474)) {
			LifeDecrease(pHitParam->damage);
			if (GetLifeInterface()->GetValue() <= 0.0f) {
				SetState(SHOCKER_STATE_DEATH, -1);
			}
			else {
				if (this->currentAnimType == 0x16) {
					ulong random = CScene::_pinstance->field_0x38 * 0x343fd + 0x269ec3;
					CScene::_pinstance->field_0x38 = random;
					if ((random & 0x10000) == 0) {
						PlayAnim(0x17);
					}
					if (((random >> 16) & 1) == 1) {
						PlayAnim(0x18);
					}
				}
				else {
					PlayAnim(0x16);
				}
			}
			if (pHitParam->projectileType == 4) {
				return 1;
			}
		}
		else {
			if (pHitParam->projectileType == 4) {
				if (pSender->typeID == 5) {
					this->field_0x378.SwitchOn(this);
					this->field_0x740 = 2;
					this->field_0x744 = 0.0f;
					this->fxSpark.Reset();
					CActorBoomy* pBoomy = static_cast<CActorBoomy*>(pSender);
					pBoomy->aBoomyTypeInfo[0].hitDamage = (int)this->field_0x36c;
					pBoomy->aBoomyTypeInfo[0].hitProjectileType = 5;
					pBoomy->aBoomyTypeInfo[0].flags = pBoomy->aBoomyTypeInfo[0].flags | 0x10;
				}
				else {
					_msg_hit_param hitParam = {};
					hitParam.projectileType = 5;
					hitParam.damage = this->field_0x36c;
					DoMessage(pSender, MESSAGE_KICKED, &hitParam);
				}
				return 1;
			}
		}
	}
	return 0;
}

void CBehaviourShocker::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourShocker::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorShocker*>(pOwner);
	return;
}

int CBehaviourShocker::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

int CBehaviourShocker::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourShocker::Reset()
{
	return;
}

void CBehaviourShockerFireWave::Create(ByteCode* pByteCode)
{
	this->circularWaveShoot.Create(pByteCode);
	return;
}

void CBehaviourShockerFireWave::Init(CActor* pOwner)
{
	this->circularWaveShoot.Init(pOwner);
	return;
}

void CBehaviourShockerFireWave::Manage()
{
	this->pOwner->BehaviourShockerFireWave_Manage(this);
	return;
}

void CBehaviourShockerFireWave::Draw()
{
	this->circularWaveShoot.Draw();
	return;
}

void CBehaviourShockerFireWave::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourShocker::Begin(pOwner, newState, newAnimationType);
	this->circularWaveShoot.Reset();
	if (newState == -1) {
		this->pOwner->SetState(SHOCKER_STATE_SLEEP, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}
	return;
}

void CBehaviourShockerFireWave::InitState(int newState)
{
	this->pOwner->BehaviourShockerFireWave_InitState(newState);
	return;
}

void CBehaviourShockerFireWave::TermState(int oldState, int newState)
{
	if (oldState == SHOCKER_STATE_WAIT_COMMAND) {
		int command = 3;
		this->pOwner->DoMessage(this->pOwner->field_0x3a4, static_cast<ACTOR_MESSAGE>(0x69), &command);
	}
	else {
		if (oldState == SHOCKER_STATE_FIRE_WAVE_LAND) {
			this->pOwner->dynamic.speed = 0.0f;
			this->pOwner->dynamicExt.normalizedTranslation = gF32Vector4Zero;
			this->pOwner->dynamicExt.field_0x6c = 0.0f;
		}
	}
	return;
}

void CBehaviourShockerFireWave::Reset()
{
	this->circularWaveShoot.Reset();
	return;
}

void CActorShocker::ComputeInvincibility()
{
	switch (this->actorState) {
	case SHOCKER_STATE_SLEEP:
		this->field_0x474 = false;
		this->field_0x3a9 = (this->field_0x380 & 1) == 0;
		break;
	case SHOCKER_STATE_STAND_UP_1_3:
	case SHOCKER_STATE_STAND_UP_2_3:
	case SHOCKER_STATE_STAND_UP_3_3:
		this->field_0x474 = (this->field_0x380 & 2) != 0;
		this->field_0x3a9 = !this->field_0x474;
		break;
	case SHOCKER_STATE_CHASE:
		this->field_0x474 = (this->field_0x380 & 4) != 0;
		this->field_0x3a9 = !this->field_0x474;
		break;
	case SHOCKER_STATE_WAIT_COMMAND:
		this->field_0x474 = true;
		this->field_0x3a9 = false;
		break;
	case SHOCKER_STATE_FIRE_WAVE_WIND_UP:
	case SHOCKER_STATE_FIRE_WAVE_FALL:
		this->field_0x3a9 = (this->field_0x380 & 8) == 0;
		this->field_0x474 = true;
		break;
	case SHOCKER_STATE_FIRE_WAVE_LAND:
		this->field_0x3a9 = (this->field_0x380 & 8) == 0;
		this->field_0x474 = false;
		break;
	case SHOCKER_STATE_STAND_UP_TIRED_1_2:
	case SHOCKER_STATE_STAND_UP_TIRED_2_2:
		this->field_0x3a9 = (this->field_0x380 & 0x10) == 0;
		break;
	case SHOCKER_STATE_RECHARGE:
		this->field_0x3a9 = false;
		break;
	case 0x12:
		this->field_0x474 = (this->field_0x380 & 0x40) != 0;
		this->field_0x3a9 = !this->field_0x474;
		break;
	case SHOCKER_STATE_COME_BACK:
		this->field_0x474 = (this->field_0x380 & 0x20) != 0;
		this->field_0x3a9 = !this->field_0x474;
		break;
	case SHOCKER_STATE_GO_TO_SLEEP_1_3:
	case SHOCKER_STATE_GO_TO_SLEEP_2_3:
	case SHOCKER_STATE_GO_TO_SLEEP_3_3:
	case SHOCKER_STATE_DEATH:
	case SHOCKER_STATE_DEATH_EXTRUDE:
		this->field_0x474 = true;
		this->field_0x3a9 = false;
		break;
	case SHOCKER_STATE_DEATH_FADE:
		this->field_0x474 = false;
		this->field_0x3a9 = false;
		break;
	default:
		this->field_0x474 = false;
		this->field_0x3a9 = true;
	}
	return;
}

void CActorShocker::StateShockerGoToSleep_2_3(CBehaviourShockerFireWave* pBehaviour)
{
	int iVar2;
	float fVar3;
	_msg_hit_param local_200 = {};
	CActorsTable local_180;
	edF32VECTOR4 local_70;
	CActorMovLevitateParamOut CStack96;
	CActorMovLevitateParamIn local_30;
	_msg_hit_param* local_4;

	local_30.pRotation = (edF32VECTOR4*)0x0;
	CStack96.flags = 0;
	local_30.rotSpeed = this->field_0x35c;
	local_30.speed = this->field_0x350;
	local_30.flags = 0x456;
	local_30.acceleration = 10.0f;
	local_30.field_0x20 = 0.0f;
	local_30.field_0x1c = this->field_0x350 / 2.0f;
	local_70 = this->currentLocation;
	fVar3 = this->distanceToGround;
	local_70.y = local_70.y - fmaxf(fVar3, 0.0f);
	SV_MOV_MoveInLevitation(&CStack96, &local_30, &local_70);
	local_180.nbEntries = 0;
	ManageDyn(4.0f, 0, &local_180);
	iVar2 = 0;
	if (this->field_0x474 != false) {
		for (; iVar2 < local_180.nbEntries; iVar2 = iVar2 + 1) {
			local_200.projectileType = 5;
			local_200.damage = pBehaviour->circularWaveShoot.field_0x18;
			local_4 = &local_200;
			DoMessage(local_180.aEntries[iVar2], MESSAGE_KICKED, local_4);
		}
	}
	if (this->field_0xf0 <= this->distanceToGround) {
		SetState(SHOCKER_STATE_SLEEP, -1);
	}
	else {
		iVar2 = CheckArea();
		if (iVar2 == 1) {
			SetState(SHOCKER_STATE_STAND_UP_2_3, -1);
		}
		else {
			if ((CStack96.moveVelocity < 0.2f) ||
				(((this->pCollisionData)->flags_0x4 & 2) != 0)) {
				SetState(SHOCKER_STATE_GO_TO_SLEEP_3_3, -1);
			}
		}
	}
	return;
}

void CActorShocker::StateShockerGoToSleep_1_3(CBehaviourShockerFireWave* pBehaviour)
{
	int iVar2;
	float fVar4;
	_msg_hit_param local_200 = {};
	CActorsTable local_180;
	edF32VECTOR4 eStack112;
	edF32VECTOR4 local_60;
	CActorMovParamsIn local_50;
	CActorMovParamsOut CStack48;
	_msg_hit_param* local_4;

	CStack48.flags = 0;
	local_50.pRotation = (edF32VECTOR4*)0x0;
	local_50.rotSpeed = this->field_0x35c;
	local_50.speed = this->field_0x350;
	local_50.acceleration = 10.0f;
	local_50.flags = 0x406;
	local_60 = this->baseLocation;
	SetVectorFromAngles(&eStack112, &(this->pCinData)->rotationEuler);
	SV_UpdateOrientation2D(2.0f, &eStack112, 0);
	SV_MOV_MoveTo(&CStack48, &local_50, &local_60);
	local_180.nbEntries = 0;
	ManageDyn(4.0f, 0, &local_180);
	iVar2 = 0;
	if ((this->field_0x474 != false) && (0 < local_180.nbEntries)) {
		local_4 = &local_200;
		for (; iVar2 < local_180.nbEntries; iVar2 = iVar2 + 1) {
			local_200.projectileType = 5;
			local_200.damage = pBehaviour->circularWaveShoot.field_0x18;
			DoMessage(local_180.aEntries[iVar2], MESSAGE_KICKED, local_4);
		}
	}
	iVar2 = CheckArea();
	if (iVar2 == 1) {
		SetState(SHOCKER_STATE_STAND_UP_2_3, -1);
	}
	else {
		fVar4 = this->distanceToGround;
		if (this->field_0xf0 <= fVar4) {
			SetState(SHOCKER_STATE_SLEEP, -1);
		}
		else {
			if ((fVar4 < 2.0f) || (CStack48.moveVelocity < 0.5f)) {
				SetState(SHOCKER_STATE_GO_TO_SLEEP_2_3, -1);
			}
		}
	}
	return;
}

void CActorShocker::StateShockerComeBack(CBehaviourShockerFireWave* pBehaviour)
{
	float fVar1;
	int iVar2;
	float fVar3;
	_msg_hit_param local_1f0 = {};
	CActorsTable local_170;
	CActorMovLevitateParamOut CStack96;
	CActorMovLevitateParamIn local_30;
	_msg_hit_param* local_4;

	local_30.pRotation = (edF32VECTOR4*)0x0;
	CStack96.flags = 0;
	local_30.rotSpeed = this->field_0x35c;
	local_30.speed = this->field_0x350;
	local_30.flags = 0x452;
	local_30.acceleration = 10.0f;
	local_30.field_0x20 = this->field_0x360;
	local_30.field_0x1c = this->field_0x350 / 2.0f;
	SV_MOV_MoveInLevitation(&CStack96, &local_30, &this->baseLocation);
	local_170.nbEntries = 0;
	ManageDyn(4.0f, 0, &local_170);
	iVar2 = 0;
	if (this->field_0x474 != false) {
		for (; iVar2 < local_170.nbEntries; iVar2 = iVar2 + 1) {
			local_1f0.projectileType = 5;
			local_1f0.damage = pBehaviour->circularWaveShoot.field_0x18;
			local_4 = &local_1f0;
			DoMessage(local_170.aEntries[iVar2], MESSAGE_KICKED, local_4);
		}
	}
	fVar3 = this->baseLocation.x - this->currentLocation.x;
	fVar1 = this->baseLocation.z - this->currentLocation.z;
	if (0.5f <= sqrtf(fVar3 * fVar3 + 0.0f + fVar1 * fVar1)) {
		iVar2 = CheckArea();
		if (iVar2 == 1) {
			SetState(SHOCKER_STATE_CHASE, -1);
		}
	}
	else {
		this->dynamic.speed = 0.0f;
		if (this->field_0xf0 < this->distanceToGround) {
			SetState(SHOCKER_STATE_SLEEP, -1);
		}
		else {
			if (0.0f < this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y) {
				this->dynamic.speed = 0.0f;
			}
			fVar3 = this->distanceToGround;
			if (2.0f < fVar3) {
				SetState(SHOCKER_STATE_GO_TO_SLEEP_1_3, -1);
			}
			else {
				if ((fVar3 < 0.2f) || (((this->pCollisionData)->flags_0x4 & 2) != 0)) {
					SetState(SHOCKER_STATE_GO_TO_SLEEP_3_3, -1);
				}
				else {
					SetState(SHOCKER_STATE_GO_TO_SLEEP_2_3, -1);
				}
			}
		}
	}
	return;
}

void CActorShocker::StateShockerStandUpTired_2_2(CBehaviourShockerFireWave* pBehaviour)
{
	int iVar2;
	float fVar3;
	_msg_hit_param local_1f0 = {};
	CActorsTable local_170;
	CActorMovParamsIn local_60;
	CActorMovParamsOut CStack64;
	edF32VECTOR4 local_20;

	local_20 = this->currentLocation;
	CheckArea();
	fVar3 = this->distanceToGround;
	if (fVar3 < this->field_0xf0) {
		local_20.y = local_20.y + (this->field_0x360 - fmaxf(fVar3, 0.0f));
	}
	else {
		local_20.y = this->baseLocation.y + this->field_0x360;
	}
	CStack64.flags = 0;
	local_60.pRotation = (edF32VECTOR4*)0x0;
	local_60.rotSpeed = this->field_0x35c;
	local_60.speed = this->field_0x358;
	local_60.flags = 0x406;
	local_60.acceleration = 2.0f;
	SV_MOV_MoveTo(&CStack64, &local_60, &local_20);
	if (this->field_0x3a0 != (CActor*)0x0) {
		SV_UpdateOrientationToPosition2D(this->field_0x35c, &this->field_0x3a0->currentLocation);
	}
	local_170.nbEntries = 0;
	ManageDyn(4.0f, 0, &local_170);
	if (this->field_0x474 != false) {
		for (iVar2 = 0; iVar2 < local_170.nbEntries; iVar2 = iVar2 + 1) {
			local_1f0.projectileType = 5;
			local_1f0.damage = pBehaviour->circularWaveShoot.field_0x18;
			DoMessage(local_170.aEntries[iVar2], MESSAGE_KICKED, &local_1f0);
		}
	}
	if (CStack64.moveVelocity < 0.2f) {
		this->dynamic.speed = 0.0f;
		SetState(SHOCKER_STATE_RECHARGE, -1);
	}
	return;
}

void CActorShocker::StateShockerStandUpTired_1_2(CBehaviourShockerFireWave* pBehaviour)
{
	CAnimation* this_00;
	int iVar2;
	float fVar3;
	_msg_hit_param local_1f0 = {};
	CActorsTable local_170;
	CActorMovParamsIn local_60;
	CActorMovParamsOut CStack64;
	edF32VECTOR4 local_20;
	_msg_hit_param* local_4;

	local_20 = this->currentLocation;
	CheckArea();
	fVar3 = this->distanceToGround;
	if (fVar3 < this->field_0xf0) {
		local_20.y = local_20.y + (this->field_0x360 - fmaxf(fVar3, 0.0f));
	}
	CStack64.flags = 0;
	local_60.pRotation = (edF32VECTOR4*)0x0;
	local_60.rotSpeed = this->field_0x35c / 2.0f;
	local_60.flags = 0x406;
	local_60.speed = this->field_0x358;
	local_60.acceleration = 2.0f;
	SV_MOV_MoveTo(&CStack64, &local_60, &local_20);
	if (this->field_0x3a0 != (CActor*)0x0) {
		SV_UpdateOrientationToPosition2D(1.0f, &this->field_0x3a0->currentLocation);
	}
	local_170.nbEntries = 0;
	ManageDyn(4.0f, 0, &local_170);
	iVar2 = 0;
	if (this->field_0x474 != false) {
		for (; iVar2 < local_170.nbEntries; iVar2 = iVar2 + 1) {
			local_1f0.projectileType = 5;
			local_1f0.damage = pBehaviour->circularWaveShoot.field_0x18;
			local_4 = &local_1f0;
			DoMessage(local_170.aEntries[iVar2], MESSAGE_KICKED, local_4);
		}
	}
	this_00 = this->pAnimationController;
	iVar2 = GetIdMacroAnim(0x12);
	if (iVar2 < 0) {
		fVar3 = 0.0f;
	}
	else {
		fVar3 = this_00->GetAnimLength(iVar2, 2);
	}
	if (fVar3 < this->timeInAir) {
		SetState(SHOCKER_STATE_STAND_UP_TIRED_2_2, -1);
	}
	return;
}

void CActorShocker::StateShockerChase(CBehaviourShockerFireWave* pBehaviour)
{
	float fVar1;
	float fVar2;
	int iVar3;
	_msg_hit_param local_1f0 = {};
	CActorsTable local_170;
	CActorMovLevitateParamOut CStack96;
	CActorMovLevitateParamIn local_30;
	_msg_hit_param* local_4;

	local_30.pRotation = (edF32VECTOR4*)0x0;
	CStack96.flags = 0;
	local_30.rotSpeed = this->field_0x35c;
	local_30.speed = this->field_0x350;
	local_30.flags = 0x452;
	local_30.acceleration = 10.0f;
	local_30.field_0x20 = this->field_0x360;
	local_30.field_0x1c = this->field_0x350 / 2.0f;
	SV_MOV_MoveInLevitation(&CStack96, &local_30, &this->field_0x3a0->currentLocation);
	local_170.nbEntries = 0;
	ManageDyn(4.0f, 1, &local_170);
	iVar3 = 0;
	if (this->field_0x474 != false) {
		for (; iVar3 < local_170.nbEntries; iVar3 = iVar3 + 1) {
			local_1f0.projectileType = 5;
			local_1f0.damage = pBehaviour->circularWaveShoot.field_0x18;
			local_4 = &local_1f0;
			DoMessage(local_170.aEntries[iVar3], MESSAGE_KICKED, local_4);
		}
	}
	fVar1 = (this->field_0x3a0->currentLocation).x - this->currentLocation.x;
	fVar2 = (this->field_0x3a0->currentLocation).z - this->currentLocation.z;
	if ((this->field_0x364 < sqrtf(fVar1 * fVar1 + 0.0f + fVar2 * fVar2)) ||
		(CStack96.field_0x20 == false)) {
		iVar3 = CheckArea();
		if (iVar3 == 2) {
			SetState(SHOCKER_STATE_COME_BACK, -1);
		}
	}
	else {
		this->dynamic.speed = 0.0f;
		if (this->field_0x3a4 == (CActor*)0x0) {
			SetState(SHOCKER_STATE_FIRE_WAVE_WIND_UP, -1);
		}
		else {
			SetState(SHOCKER_STATE_WAIT_COMMAND, -1);
		}
	}
	return;
}

void CActorShocker::StateShockerStandUp_3_3(CBehaviourShockerFireWave* pBehaviour)
{
	int iVar2;
	float fVar3;
	_msg_hit_param local_1f0 = {};
	CActorsTable local_170;
	CActorMovParamsIn local_60;
	CActorMovParamsOut CStack64;
	edF32VECTOR4 local_20;

	local_20 = this->currentLocation;
	fVar3 = this->distanceToGround;
	if (fVar3 < this->field_0xf0) {
		local_20.y = local_20.y + (this->field_0x360 - fmaxf(fVar3, 0.0f));
	}
	CStack64.flags = 0;
	local_60.pRotation = (edF32VECTOR4*)0x0;
	local_60.rotSpeed = this->field_0x35c;
	local_60.speed = this->field_0x354;
	local_60.flags = 0x406;
	local_60.acceleration = 8.0f;
	SV_MOV_MoveTo(&CStack64, &local_60, &local_20);
	if (this->field_0x3a0 != (CActor*)0x0) {
		SV_UpdateOrientationToPosition2D(2.0f, &this->field_0x3a0->currentLocation);
	}
	local_170.nbEntries = 0;
	ManageDyn(4.0f, 0, &local_170);
	iVar2 = 0;
	if (this->field_0x474 != false) {
		for (; iVar2 < local_170.nbEntries; iVar2 = iVar2 + 1) {
			local_1f0.projectileType = 5;
			local_1f0.damage = pBehaviour->circularWaveShoot.field_0x18;
			DoMessage(local_170.aEntries[iVar2], MESSAGE_KICKED, &local_1f0);
		}
	}
	fVar3 = this->distanceToGround;
	if ((this->field_0xf0 < fVar3) || (this->field_0x360 < fVar3)) {
		SetState(SHOCKER_STATE_CHASE, -1);
	}
	else {
		iVar2 = CheckArea();
		if (iVar2 == 2) {
			if (0.0f < this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y) {
				this->dynamic.speed = 0.0f;
			}
			fVar3 = this->distanceToGround;
			if (2.0f < fVar3) {
				SetState(SHOCKER_STATE_GO_TO_SLEEP_1_3, -1);
			}
			else {
				if ((fVar3 < 0.2f) || (((this->pCollisionData)->flags_0x4 & 2) != 0)) {
					SetState(SHOCKER_STATE_GO_TO_SLEEP_3_3, -1);
				}
				else {
					SetState(SHOCKER_STATE_GO_TO_SLEEP_2_3, -1);
				}
			}
		}
	}
	return;
}

void CActorShocker::StateShockerStandUp_2_3(CBehaviourShockerFireWave* pBehaviour)
{
	bool bVar3;
	int iVar5;
	float fVar6;
	_msg_hit_param local_1f0 = {};
	CActorsTable local_170;
	CActorMovParamsIn local_60;
	CActorMovParamsOut CStack64;
	edF32VECTOR4 local_20;
	_msg_hit_param* local_4;

	local_20 = this->currentLocation;
	fVar6 = this->distanceToGround;
	if (fVar6 < this->field_0xf0) {
		local_20.y = local_20.y + (this->field_0x360 - fmaxf(fVar6, 0.0f));
	}
	CStack64.flags = 0;
	local_60.pRotation = (edF32VECTOR4*)0x0;
	local_60.rotSpeed = this->field_0x35c;
	local_60.speed = 3.0f;
	local_60.acceleration = 8.0f;
	local_60.flags = 0x406;
	SV_MOV_MoveTo(&CStack64, &local_60, &local_20);
	if (this->field_0x3a0 != (CActor*)0x0) {
		SV_UpdateOrientationToPosition2D(2.0f, &this->field_0x3a0->currentLocation);
	}
	local_170.nbEntries = 0;
	ManageDyn(4.0f, 0, &local_170);
	iVar5 = 0;
	if (this->field_0x474 != false) {
		for (; iVar5 < local_170.nbEntries; iVar5 = iVar5 + 1) {
			local_1f0.projectileType = 5;
			local_1f0.damage = pBehaviour->circularWaveShoot.field_0x18;
			local_4 = &local_1f0;
			DoMessage(local_170.aEntries[iVar5], MESSAGE_KICKED, local_4);
		}
	}
	fVar6 = this->distanceToGround;
	if ((this->field_0xf0 < fVar6) || (this->field_0x360 < fVar6)) {
		SetState(SHOCKER_STATE_CHASE, -1);
	}
	else {
		if (fVar6 <= 2.0f) {
			bVar3 = this->pAnimationController->IsCurrentLayerAnimEndReached(0);
			if (!bVar3) {
				iVar5 = CheckArea();
				if (iVar5 != 2) {
					return;
				}
				if (0.0f < this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y) {
					this->dynamic.speed = 0.0f;
				}
				fVar6 = this->distanceToGround;
				if (fVar6 <= 2.0f) {
					if ((0.2f <= fVar6) && (((this->pCollisionData)->flags_0x4 & 2) == 0)) {
						SetState(SHOCKER_STATE_GO_TO_SLEEP_2_3, -1);
						return;
					}
					SetState(SHOCKER_STATE_GO_TO_SLEEP_3_3, -1);
					return;
				}
				SetState(SHOCKER_STATE_GO_TO_SLEEP_1_3, -1);
				return;
			}
		}
		SetState(SHOCKER_STATE_STAND_UP_3_3, -1);
	}
	return;
}

void CActorShocker::StateShockerStandUp_1_3(CBehaviourShockerFireWave* pBehaviour)
{
	bool bVar3;
	int iVar5;
	float fVar6;
	_msg_hit_param local_1f0 = {};
	CActorsTable local_170;
	CActorMovParamsIn local_60;
	CActorMovParamsOut CStack64;
	edF32VECTOR4 local_20;

	local_20 = this->currentLocation;
	fVar6 = this->distanceToGround;
	if (fVar6 < this->field_0xf0) {
		local_20.y = local_20.y + (this->field_0x360 - fmaxf(fVar6, 0.0f));
	}
	CStack64.flags = 0;
	local_60.pRotation = (edF32VECTOR4*)0x0;
	local_60.rotSpeed = this->field_0x35c / 2.0f;
	local_60.acceleration = 8.0f;
	local_60.speed = 1.7f;
	local_60.flags = 0x406;
	SV_MOV_MoveTo(&CStack64, &local_60, &local_20);
	if (this->field_0x3a0 != (CActor*)0x0) {
		SV_UpdateOrientationToPosition2D(2.0f, &this->field_0x3a0->currentLocation);
	}
	local_170.nbEntries = 0;
	ManageDyn(4.0f, 0, &local_170);
	iVar5 = 0;
	if (this->field_0x474 != false) {
		for (; iVar5 < local_170.nbEntries; iVar5 = iVar5 + 1) {
			local_1f0.projectileType = 5;
			local_1f0.damage = pBehaviour->circularWaveShoot.field_0x18;
			DoMessage(local_170.aEntries[iVar5], MESSAGE_KICKED, &local_1f0);
		}
	}
	iVar5 = CheckArea();
	if (iVar5 == 2) {
		if (0.0f < this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y) {
			this->dynamic.speed = 0.0f;
		}
		fVar6 = this->distanceToGround;
		if (2.0f < fVar6) {
			SetState(SHOCKER_STATE_GO_TO_SLEEP_1_3, -1);
		}
		else {
			if ((fVar6 < 0.2f) || (((this->pCollisionData)->flags_0x4 & 2) != 0)) {
				SetState(SHOCKER_STATE_GO_TO_SLEEP_3_3, -1);
			}
			else {
				SetState(SHOCKER_STATE_GO_TO_SLEEP_2_3, -1);
			}
		}
	}
	else {
		fVar6 = this->distanceToGround;
		if ((this->field_0xf0 < fVar6) || (this->field_0x360 < fVar6)) {
			SetState(SHOCKER_STATE_CHASE, -1);
		}
		else {
			if (2.0f < fVar6) {
				SetState(SHOCKER_STATE_STAND_UP_3_3, -1);
			}
			else {
				bVar3 = this->pAnimationController->IsCurrentLayerAnimEndReached(0);
				if (bVar3) {
					SetState(SHOCKER_STATE_STAND_UP_2_3, -1);
				}
			}
		}
	}
	return;
}
