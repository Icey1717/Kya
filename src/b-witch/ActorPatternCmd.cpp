#include "ActorPatternCmd.h"
#include "ActorHero.h"
#include "MemoryStream.h"
#include "EventManager.h"
#include "CameraViewManager.h"
#include "MathOps.h"

void CActorPatternCmd::Create(ByteCode* pByteCode)
{
	CActor::Create(pByteCode);

	this->field_0x164 = pByteCode->GetU32();
	this->field_0x160 = pByteCode->GetU32();
	this->pActorStreamRef = S_ACTOR_STREAM_REF::Create(pByteCode);
	this->field_0x1f0 = pByteCode->GetF32();
	this->field_0x1f4 = pByteCode->GetF32();

	this->patternPart.Create(pByteCode);

	return;
}

void CActorPatternCmd::Init()
{
	CActor::Init();

	this->pActorStreamRef->Init();
	this->patternPart.Init(-1.0f, this);

	ClearLocalData();

	return;
}

void CActorPatternCmd::Draw()
{
	CActor::Draw();
	this->patternPart.Draw();

	return;
}

void CActorPatternCmd::Reset()
{
	CActor::Reset();
	ClearLocalData();

	return;
}

CBehaviour* CActorPatternCmd::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == PATTERN_CMD_BEHAVIOUR_STAND) {
		pBehaviour = &this->behaviourStand;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

StateConfig CActorPatternCmd::_gStateCfg_PTC[5] = {
	StateConfig(0x0, 0x0),
	StateConfig(0x6, 0x0),
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x1),
};


StateConfig* CActorPatternCmd::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		assert((state - 5) < 5);
		pStateConfig = _gStateCfg_PTC + state + -5;
	}

	return pStateConfig;
}

int CActorPatternCmd::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActor* pReceiver;
	S_ACTOR_STREAM_REF* pStream;
	int iVar2;
	int result;
	_msg_params_0x54 uStack32;

	if (msg == 0x54) {
		_msg_params_0x54* pCastParams = static_cast<_msg_params_0x54*>(pMsgParam);
		if (pCastParams->field_0x0 == 5) {
			this->nbStreamRefEntries = this->nbStreamRefEntries + -1;
			int curActorIndex = 0;
			if (this->nbStreamRefEntries == 1) {
				while (true) {
					pStream = this->pActorStreamRef;
					iVar2 = 0;
					if (pStream != (S_ACTOR_STREAM_REF*)0x0) {
						iVar2 = pStream->entryCount;
					}

					if (iVar2 <= curActorIndex) break;

					pReceiver = pStream->aEntries[curActorIndex].Get();
					if ((pReceiver != pSender) && ((pReceiver->flags & 4) != 0)) {
						uStack32.field_0x0 = 0;
						uStack32.field_0x4 = 1;
						DoMessage(pReceiver, (ACTOR_MESSAGE)0x54, &uStack32);
						SetState(PATTERN_CMD_STAND_STATE_CHASE, -1);
					}

					curActorIndex = curActorIndex + 1;
				}
			}

			if (this->nbStreamRefEntries == 0) {
				this->flags = this->flags & 0xfffffffd;
				this->flags = this->flags | 1;
				SetState(PATTERN_CMD_STAND_STATE_UNKNOWN, -1);
			}

			result = 0;
		}
		else {
			result = 0;
		}
	}
	else {
		result = CActor::InterpretMessage(pSender, msg, pMsgParam);
	}

	return result;
}

void CActorPatternCmd::BehaviourStand_Manage()
{
	this->patternPart.UpdatePatternPartLife();

	const int state = this->actorState;
	if (state != 9) {
		if (state == PATTERN_CMD_STAND_STATE_DECELERATE) {
			StatePatternCmdDecelerate();
		}
		else {
			if (state == PATTERN_CMD_STAND_STATE_FIRE) {
				StatePatternCmdFire();
			}
			else {
				if (state == PATTERN_CMD_STAND_STATE_CHASE) {
					StatePatternCmdChase();
				}
				else {
					if (state == PATTERN_CMD_STAND_STATE_STAND) {
						StatePatternCmdStand();
					}
				}
			}
		}
	}

	return;
}

void CActorPatternCmd::ClearLocalData()
{
	int iVar1;

	this->field_0x280 = (CActorHero*)0x0;

	this->pActorStreamRef->Reset();

	this->patternPart.Reset();

	if (this->pActorStreamRef == (S_ACTOR_STREAM_REF*)0x0) {
		iVar1 = 0;
	}
	else {
		iVar1 = this->pActorStreamRef->entryCount;
	}
	this->nbStreamRefEntries = iVar1;
}

void CActorPatternCmd::StatePatternCmdStand()
{
	CCamera* pCVar1;
	CCameraManager* pCVar2;
	int iVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	_msg_params_0x54 local_20;
	CActorPatternCmd* pPatternCmd;

	if (iVar3 = CheckArea(), iVar3 == 1) {
		if (this->nbStreamRefEntries == 2) {
			pCVar1 = pCVar2->pActiveCamera;
			this->field_0x270 = pCVar1->transformationMatrix.rowZ;
			this->field_0x270.y = 0.0f;
			edF32Vector4NormalizeHard(&this->field_0x270, &this->field_0x270);
		}
		else {
			if (this->nbStreamRefEntries == 1) {
				local_20.field_0x4 = 1;
				local_20.field_0x0 = 0;
				DoMessage(this->pActorStreamRef->aEntries[0].Get(), (ACTOR_MESSAGE)0x54, &local_20);
			}
		}

		SetState(PATTERN_CMD_STAND_STATE_CHASE, -1);
	}

	return;
}

void CActorPatternCmd::StatePatternCmdChase()
{
	CActor* pReceiver;
	CActor* pCVar1;
	float fVar2;
	float fVar3;
	int iVar4;
	int iVar5;
	char cVar6;
	_msg_params_0x54 local_20;

	this->flags = this->flags | 0x400;
	if (this->nbStreamRefEntries == 2) {
		local_20.field_0x4 = 0;
		local_20.field_0x0 = 0;

		this->field_0x280->TestState_IsCrouched(0xffffffff);

		ComputePositionsTwoPattern(&this->field_0x200, &this->field_0x210, &this->field_0x220, &this->field_0x230);
		ChooseTwoPosBetweenFour(&this->field_0x240, &this->field_0x250, &this->field_0x200, &this->field_0x210, &this->field_0x220, &this->field_0x230);

		pCVar1 = this->pActorStreamRef->aEntries[0].Get();
		pReceiver = this->pActorStreamRef->aEntries[1].Get();

		if (pCVar1->currentAnimType != 0x13) {
			DoMessage(pCVar1, (ACTOR_MESSAGE)0x54, &local_20);
		}

		fVar2 = (this->field_0x240).x - (pCVar1->currentLocation).x;
		fVar3 = (this->field_0x240).z - (pCVar1->currentLocation).z;

		cVar6 = sqrtf(fVar2 * fVar2 + 0.0f + fVar3 * fVar3) < 0.5f;
		if (pReceiver->currentAnimType != 0x13) {
			DoMessage(pReceiver, (ACTOR_MESSAGE)0x54, &local_20);
		}

		fVar2 = (this->field_0x250).x - (pReceiver->currentLocation).x;
		fVar3 = (this->field_0x250).z - (pReceiver->currentLocation).z;

		if (sqrtf(fVar2 * fVar2 + 0.0f + fVar3 * fVar3) < 0.5f) {
			cVar6 = cVar6 + '\x01';
		}

		if (cVar6 == '\x02') {
			SetState(PATTERN_CMD_STAND_STATE_FIRE, -1);
			return;
		}
	}

	iVar4 = CheckArea();
	if (iVar4 == 2) {
		iVar4 = 0;
		local_20.field_0x0 = 1;
		while (true) {
			if (this->pActorStreamRef == (S_ACTOR_STREAM_REF*)0x0) {
				iVar5 = 0;
			}
			else {
				iVar5 = this->pActorStreamRef->entryCount;
			}

			if (iVar5 <= iVar4) break;

			pCVar1 = this->pActorStreamRef->aEntries[iVar4].Get();
			if ((pCVar1->flags & 4) != 0) {
				DoMessage(pCVar1, (ACTOR_MESSAGE)0x54, &local_20);
			}
			iVar4 = iVar4 + 1;
		}

		this->field_0x280 = (CActorHero*)0x0;
		SetState(PATTERN_CMD_STAND_STATE_STAND, -1);
	}

	return;
}

void CActorPatternCmd::StatePatternCmdFire()
{
	bool bVar1;
	bool bVar2;
	CActor* pReceiver;
	CActor* pReceiver_00;
	int iVar3;
	int iVar4;
	char cVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	_msg_params_0x54 local_a0;
	edF32VECTOR4 local_90;
	edF32VECTOR4 eStack128;
	float local_70;
	float fStack108;
	float fStack104;
	float fStack100;
	edF32VECTOR4 eStack96;
	_msg_params_0x54 local_50;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;

	this->flags = this->flags | 0x400;

	ComputePositionsTwoPattern(&this->field_0x200, &this->field_0x210, &this->field_0x220, &this->field_0x230);

	pReceiver = this->pActorStreamRef->aEntries[1].Get();
	pReceiver_00 = this->pActorStreamRef->aEntries[0].Get();
	switch (this->field_0x260) {
	case 0:
	case 3:
		this->field_0x240 = this->field_0x230;
		edF32Vector4SubHard(&eStack48, &this->field_0x230, &this->field_0x210);
		edF32Vector4ScaleHard(1.25f, &eStack48, &eStack48);
		edF32Vector4AddHard(&this->field_0x240, &eStack48, &this->field_0x210);
		this->field_0x250 = this->field_0x220;
		edF32Vector4SubHard(&eStack64, &this->field_0x220, &this->field_0x200);
		edF32Vector4ScaleHard(1.25f, &eStack64, &eStack64);
		edF32Vector4AddHard(&this->field_0x250, &eStack64, &this->field_0x200);
		break;
	case 1:
	case 2:
		this->field_0x240 = this->field_0x220;
		edF32Vector4SubHard(&eStack48, &this->field_0x220, &this->field_0x200);
		edF32Vector4ScaleHard(1.25f, &eStack48, &eStack48);
		edF32Vector4AddHard(&this->field_0x240, &eStack48, &this->field_0x200);
		this->field_0x250 = this->field_0x230;
		edF32Vector4SubHard(&eStack64, &this->field_0x230, &this->field_0x210);
		edF32Vector4ScaleHard(1.25f, &eStack64, &eStack64);
		edF32Vector4AddHard(&this->field_0x250, &eStack64, &this->field_0x210);
		break;
	case 4:
	case 7:
		edF32Vector4SubHard(&eStack48, &this->field_0x210, &this->field_0x230);
		edF32Vector4ScaleHard(1.25f, &eStack48, &eStack48);
		edF32Vector4AddHard(&this->field_0x240, &eStack48, &this->field_0x230);
		edF32Vector4SubHard(&eStack64, &this->field_0x200, &this->field_0x220);
		edF32Vector4ScaleHard(1.25f, &eStack64, &eStack64);
		edF32Vector4AddHard(&this->field_0x250, &eStack64, &this->field_0x220);
		break;
	case 5:
	case 6:
		this->field_0x240 = this->field_0x200;
		edF32Vector4SubHard(&eStack48, &this->field_0x200, &this->field_0x220);
		edF32Vector4ScaleHard(1.25f, &eStack48, &eStack48);
		edF32Vector4AddHard(&this->field_0x240, &eStack48, &this->field_0x220);
		this->field_0x250 = this->field_0x210;
		edF32Vector4SubHard(&eStack64, &this->field_0x210, &this->field_0x230);
		edF32Vector4ScaleHard(1.25f, &eStack64, &eStack64);
		edF32Vector4AddHard(&this->field_0x250, &eStack64, &this->field_0x230);
	}

	local_50.field_0xc = &this->field_0x240;
	local_50.field_0x0 = 2;
	local_50.field_0x4 = 0;

	bVar1 = pReceiver_00->currentAnimType == 0x13;
	if (!bVar1) {
		DoMessage(pReceiver_00, (ACTOR_MESSAGE)0x54, &local_50);
	}

	fVar6 = (this->field_0x240).x - (pReceiver_00->currentLocation).x;
	fVar8 = (this->field_0x240).z - (pReceiver_00->currentLocation).z;
	cVar5 = sqrtf(fVar6 * fVar6 + 0.0f + fVar8 * fVar8) < 0.5f;
	local_50.field_0xc = &this->field_0x250;
	bVar2 = pReceiver->currentAnimType == 0x13;
	if (!bVar2) {
		DoMessage(pReceiver, (ACTOR_MESSAGE)0x54, &local_50);
	}

	fVar6 = (this->field_0x250).x - (pReceiver->currentLocation).x;
	fVar8 = (this->field_0x250).z - (pReceiver->currentLocation).z;
	if (sqrtf(fVar6 * fVar6 + 0.0f + fVar8 * fVar8) < 0.5f) {
		cVar5 = cVar5 + '\x01';
	}

	fVar8 = 0.0f;
	(this->patternPart).field_0x50 = 0;
	edF32Vector4SubHard(&eStack96, &pReceiver_00->currentLocation, &pReceiver->currentLocation);
	fVar6 = edF32Vector4GetDistHard(&eStack96);
	edF32Vector4NormalizeHard(&eStack96, &eStack96);
	local_70 = (pReceiver->currentLocation).x;
	fStack108 = (pReceiver->currentLocation).y;
	fStack104 = (pReceiver->currentLocation).z;
	fStack100 = (pReceiver->currentLocation).w;
	IMPLEMENTATION_GUARD();
	if (fVar6 < (float)*(int*)&pReceiver[3].field_0xf4 * 1.5f) {
		for (; fVar8 < fVar6; fVar8 = fVar8 + 0.15f) {
			local_90 = pReceiver->currentLocation;
			edF32Vector4ScaleHard(fVar8, &eStack128, &eStack96);
			edF32Vector4AddHard(&local_90, &local_90, &eStack128);

			this->patternPart.FireContinue(this->field_0x280, &gF32Vector4UnitY, &local_90);
		}
	}

	if ((cVar5 == '\x02') || (bVar2 || bVar1)) {
		iVar4 = 0;
		local_a0.field_0x8 = 0.0f;

		while (true) {
			if (this->pActorStreamRef == (S_ACTOR_STREAM_REF*)0x0) {
				iVar3 = 0;
			}
			else {
				iVar3 = this->pActorStreamRef->entryCount;
			}

			if (iVar3 <= iVar4) break;
			IMPLEMENTATION_GUARD();
			//fVar8 = this->pActorStreamRef->aEntries[iVar4].pActor[1].rotationQuat.y;
			if (fVar8 <= local_a0.field_0x8) {
				fVar8 = local_a0.field_0x8;
			}

			iVar4 = iVar4 + 1;
			local_a0.field_0x8 = fVar8;
		}

		local_a0.field_0x4 = 0;
		local_a0.field_0x0 = 3;
		local_a0.field_0xc = &this->field_0x240;

		if (pReceiver_00->currentAnimType != 0x13) {
			DoMessage(pReceiver_00, (ACTOR_MESSAGE)0x54, &local_a0);
		}

		local_a0.field_0xc = &this->field_0x250;

		if (pReceiver->currentAnimType != 0x13) {
			DoMessage(pReceiver, (ACTOR_MESSAGE)0x54, &local_a0);
		}

		SetState(PATTERN_CMD_STAND_STATE_DECELERATE, -1);
	}
	else {
		iVar4 = CheckArea();
		if (iVar4 == 2) {
			iVar4 = 0;
			local_50.field_0x0 = 1;
			while (true) {
				if (this->pActorStreamRef == (S_ACTOR_STREAM_REF*)0x0) {
					iVar3 = 0;
				}
				else {
					iVar3 = this->pActorStreamRef->entryCount;
				}

				if (iVar3 <= iVar4) break;

				DoMessage(this->pActorStreamRef->aEntries[iVar4].Get(), (ACTOR_MESSAGE)0x54, &local_50);
				iVar4 = iVar4 + 1;
			}

			SetState(PATTERN_CMD_STAND_STATE_STAND, -1);
		}
	}

	return;
}

void CActorPatternCmd::StatePatternCmdDecelerate()
{
	CActor* pCVar1;
	CActor* pCVar2;
	CCamera* pCVar3;
	CCameraManager* pCVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	edF32VECTOR4 local_30;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	pCVar4 = CScene::ptable.g_CameraManager_0045167c;
	fVar7 = 0.0f;
	pCVar1 = this->pActorStreamRef->aEntries[0].Get();
	pCVar2 = this->pActorStreamRef->aEntries[1].Get();
	IMPLEMENTATION_GUARD();
	(this->patternPart).field_0x50 = 0;
	edF32Vector4SubHard(&eStack16, &pCVar1->currentLocation, &pCVar2->currentLocation);
	fVar5 = edF32Vector4GetDistHard(&eStack16);
	edF32Vector4NormalizeHard(&eStack16, &eStack16);
	if (fVar5 < (float)*(int*)&pCVar2[3].field_0xf4 * 1.5f) {
		for (; fVar7 < fVar5; fVar7 = fVar7 + 0.15f) {
			local_30 = pCVar2->currentLocation;
			edF32Vector4ScaleHard(fVar7, &eStack32, &eStack16);
			edF32Vector4AddHard(&local_30, &local_30, &eStack32);
			this->patternPart.FireContinue(this->field_0x280, &gF32Vector4UnitY, &local_30);
		}
	}

	if ((pCVar1[1].rotationQuat.x < 0.2f) && (pCVar2[1].rotationQuat.x < 0.2f)) {
		pCVar3 = pCVar4->pActiveCamera;
		fVar6 = (pCVar3->transformationMatrix).cb;
		fVar7 = (pCVar3->transformationMatrix).cc;
		fVar5 = (pCVar3->transformationMatrix).cd;
		(this->field_0x270).x = (pCVar3->transformationMatrix).ca;
		(this->field_0x270).y = fVar6;
		(this->field_0x270).z = fVar7;
		(this->field_0x270).w = fVar5;
		(this->field_0x270).y = 0.0f;
		edF32Vector4NormalizeHard(&this->field_0x270, &this->field_0x270);
		SetState(PATTERN_CMD_STAND_STATE_CHASE, -1);
	}

	return;
}

void CActorPatternCmd::ComputePositionsTwoPattern(edF32VECTOR4* param_2, edF32VECTOR4* param_3, edF32VECTOR4* param_4, edF32VECTOR4* param_5)
{
	CCamera* pCVar1;
	CCameraManager* pCVar2;
	float fVar3;
	float t;
	edF32VECTOR4 local_50;
	edF32VECTOR4 local_40;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	pCVar2 = CScene::ptable.g_CameraManager_0045167c;
	this->field_0x280->SV_GetGroundPosition(&eStack48);
	pCVar1 = pCVar2->pActiveCamera;

	local_40.x = (pCVar1->transformationMatrix).ca;
	local_40.z = (pCVar1->transformationMatrix).cc;
	local_40.w = (pCVar1->transformationMatrix).cd;
	local_40.y = 0.0f;

	edF32Vector4NormalizeHard(&local_40, &local_40);

	if (this->actorState == PATTERN_CMD_STAND_STATE_CHASE) {
		SV_Vector4SLERP(1.5f, &this->field_0x270, &local_40);
	}

	t = 1.5f;
	local_40.x = (this->field_0x270).x;
	local_40.z = (this->field_0x270).z;
	local_40.w = (this->field_0x270).w;
	local_40.y = 0.0f;

	fVar3 = this->field_0x280->dynamic.horizontalLinearSpeed * this->field_0x1f4;
	if (1.5f <= fVar3) {
		t = fVar3;
	}

	edF32Vector4ScaleHard(t, &local_40, &local_40);
	edF32Vector4AddHard(&eStack16, &eStack48, &local_40);
	edF32Vector4GetNegHard(&local_40, &local_40);
	edF32Vector4AddHard(&eStack32, &eStack48, &local_40);

	local_50.x = (this->field_0x270).z;
	local_50.y = 0.0f;
	local_50.z = -(this->field_0x270).x;

	edF32Vector4NormalizeHard(&local_50, &local_50);
	edF32Vector4ScaleHard(3.0f, &local_50, &local_50);
	edF32Vector4AddHard(param_2, &eStack16, &local_50);
	edF32Vector4AddHard(param_4, &eStack32, &local_50);
	edF32Vector4GetNegHard(&local_50, &local_50);
	edF32Vector4AddHard(param_3, &eStack16, &local_50);
	edF32Vector4AddHard(param_5, &eStack32, &local_50);

	return;
}

void CActorPatternCmd::ChooseTwoPosBetweenFour(edF32VECTOR4* param_2, edF32VECTOR4* param_3, edF32VECTOR4* param_4, edF32VECTOR4* param_5, edF32VECTOR4* param_6, edF32VECTOR4* param_7)
{
	CActor* pCVar1;
	CActor* pCVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;

	pCVar1 = this->pActorStreamRef->aEntries[0].Get();
	pCVar2 = this->pActorStreamRef->aEntries[1].Get();

	fVar7 = 0.0f;
	fVar4 = param_4->x - (pCVar1->currentLocation).x;
	fVar5 = param_4->y - (pCVar1->currentLocation).y;
	fVar8 = param_4->z - (pCVar1->currentLocation).z;
	fVar9 = sqrtf(fVar4 * fVar4 + fVar5 * fVar5 + fVar8 * fVar8);
	fVar4 = param_5->x - (pCVar1->currentLocation).x;
	fVar5 = param_5->y - (pCVar1->currentLocation).y;
	fVar8 = param_5->z - (pCVar1->currentLocation).z;
	fVar6 = sqrtf(fVar4 * fVar4 + fVar5 * fVar5 + fVar8 * fVar8);
	fVar4 = param_4->x - (pCVar2->currentLocation).x;
	fVar5 = param_4->y - (pCVar2->currentLocation).y;
	fVar8 = param_4->z - (pCVar2->currentLocation).z;
	fVar3 = sqrtf(fVar4 * fVar4 + fVar5 * fVar5 + fVar8 * fVar8);
	fVar4 = param_5->x - (pCVar2->currentLocation).x;
	fVar5 = param_5->y - (pCVar2->currentLocation).y;
	fVar8 = param_5->z - (pCVar2->currentLocation).z;

	if (0.0f < fVar9) {
		this->field_0x260 = 0;
		fVar7 = fVar9;
	}
	if (fVar7 < fVar6) {
		this->field_0x260 = 1;
		fVar7 = fVar6;
	}
	if (fVar7 < fVar3) {
		this->field_0x260 = 2;
		fVar7 = fVar3;
	}

	if (fVar7 < sqrtf(fVar4 * fVar4 + fVar5 * fVar5 + fVar8 * fVar8)) {
		this->field_0x260 = 3;
	}

	switch (this->field_0x260) {
	case 0:
	case 3:
		*param_2 = *param_5;
		*param_3 = *param_4;
		break;
	case 1:
	case 2:
		*param_2 = *param_4;
		*param_3 = *param_5;
		break;
	case 4:
	case 7:
		*param_2 = *param_7;
		*param_3 = *param_6;
		break;
	case 5:
	case 6:
		*param_2 = *param_6;
		*param_3 = *param_7;
	}

	return;
}

int CActorPatternCmd::CheckArea()
{
	uint zoneId;
	ed_zone_3d* peVar1;
	int iVar2;
	int result;
	ed_zone_3d* unaff_s0_lo;
	ed_zone_3d* unaff_s1_lo;
	CEventManager* pEventManager;
	CActorHero* pHero;

	pEventManager = CScene::ptable.g_EventManager_006f5080;
	if (this->field_0x160 != 0xffffffff) {
		zoneId = this->field_0x164;
		peVar1 = (ed_zone_3d*)0x0;

		if (zoneId != 0xffffffff) {
			if (zoneId != 0xffffffff) {
				peVar1 = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, zoneId);
			}

			unaff_s1_lo = (ed_zone_3d*)0x0;
			unaff_s0_lo = peVar1;
			if (this->field_0x160 != 0xffffffff) {
				unaff_s1_lo = edEventGetChunkZone(pEventManager->activeChunkId, this->field_0x160);
			}
		}
	}

	pHero = CActorHero::_gThis;
	iVar2 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, unaff_s0_lo, &CActorHero::_gThis->currentLocation, 0);
	result = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, unaff_s1_lo, &pHero->currentLocation, 0);
	if ((result == 1) || (iVar2 == 1)) {
		result = 1;
		if (iVar2 == 1) {
			this->field_0x280 = pHero;
		}
		else {
			result = -1;
		}
	}
	else {
		this->field_0x280 = (CActorHero*)0x0;
		result = 2;
	}

	return result;
}

void CBehaviourPatternCmd::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourPatternCmd::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorPatternCmd*>(pOwner);

	return;
}

int CBehaviourPatternCmd::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return 0;
}

int CBehaviourPatternCmd::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourPatternCmdStand::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourPatternCmdStand::Manage()
{
	this->pOwner->BehaviourStand_Manage();
}

void CBehaviourPatternCmdStand::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	CBehaviourPatternCmd::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		this->pOwner->SetState(PATTERN_CMD_STAND_STATE_STAND, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourPatternCmdStand::InitState(int newState)
{
	return;
}

void CBehaviourPatternCmdStand::TermState(int oldState, int newState)
{
	return;
}

int CBehaviourPatternCmdStand::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return 0;
}
