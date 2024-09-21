#include "ActorAton.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "edDlist.h"
#include "FileManager3D.h"
#include "TimeController.h"
#include "ActorHero.h"

#define ATON_ESCAPE_STATE_PATH_BEGIN				0x7
#define ATON_ESCAPE_STATE_PATH_STAND_CALL_NEAR		0xa
#define ATON_ESCAPE_STATE_PATH_STAND_CALL_FAR		0xb
#define ATON_ESCAPE_STATE_PATH_STAND_CALL_BACK		0xc
#define ATON_ESCAPE_STATE_JUMP						0x12
#define ATON_ESCAPE_STATE_PATH_BEFORE_WAIT_JUMP		0x15
#define ATON_ESCAPE_STATE_PATH_JUMP_1_4				0x1c
#define ATON_ESCAPE_STATE_PATH_JUMP_2_4				0x1d
#define ATON_ESCAPE_STATE_PATH_JUMP_3_4				0x1e
#define ATON_ESCAPE_STATE_PATH_JUMP_4_4				0x1f

CActorAton::CActorAton()
{
	
}

void CActorAton::Create(ByteCode* pByteCode)
{
	uint uVar1;
	int iVar2;
	float fVar3;

	CActorAutonomous::Create(pByteCode);

	// This could be inlined static mesh component create.
	this->field_0x354 = pByteCode->GetU32();
	this->field_0x358 = pByteCode->GetU32();
	this->field_0x35c = pByteCode->GetU32();
	this->field_0x360 = pByteCode->GetU32();
	this->field_0x63c = pByteCode->GetS32();
	this->field_0x644 = pByteCode->GetS32();
	this->field_0x648 = pByteCode->GetS32();

	staticMeshComponent.Reset();

	this->trailMaterialId = pByteCode->GetS32();
	if (this->trailMaterialId != -1) {
		SV_InstallMaterialId(this->trailMaterialId);
		this->fxTailA.Create(1.0f, 0x10, 4, this->trailMaterialId);
		this->fxTailB.Create(1.0f, 0x10, 4, this->trailMaterialId);
	}

	this->pathDefaultDelay = pByteCode->GetF32();
	this->pathPlaneArray.Create(pByteCode);
	behaviourAddOn.Create(pByteCode);
	return;
}

void CActorAton::Init()
{
	uint uVar1;
	CPathPlane* pPathPlane;
	float fVar4;

	this->pathPlaneArray.Init();

	ClearLocalData();

	uVar1 = this->pathPlaneArray.GetNbPathPlane();
	if ((int)uVar1 < 0) {
		fVar4 = (float)(uVar1 >> 1 | uVar1 & 1);
		fVar4 = fVar4 + fVar4;
	}
	else {
		fVar4 = (float)uVar1;
	}
	if (0.0 < fVar4) {
		pPathPlane = this->pathPlaneArray.GetCurPathPlane();
		this->waypointLocation = *pPathPlane->pathFollowReader.GetWayPoint();
		this->bWaypointSet = 1;
	}

	CActorAutonomous::Init();

	this->pTrailMaterial = CScene::ptable.g_C3DFileManager_00451664->GetMaterialFromId(this->trailMaterialId, 0);
	this->field_0x475 = 0;

	if (this->trailMaterialId != -1) {
		this->fxTailA.Init(0.3333333f, this->objectId);
		this->fxTailB.Init(0.3333333f, this->objectId);
	}

	this->behaviourAddOn.Init(this);
	return;
}

void CActorAton::Term()
{
	CActorAutonomous::Term();
}

void CActorAton::Draw()
{
	CActorAutonomous::Draw();
}

void CActorAton::Reset()
{
	int nbPathPlane;
	CPathPlane* pPathPlane;
	edF32VECTOR4* pWayPoint;

	ClearLocalData();

	nbPathPlane = this->pathPlaneArray.GetNbPathPlane();
	if ((nbPathPlane != 0) && (this->bWaypointSet == 0)) {
		pPathPlane = this->pathPlaneArray.GetCurPathPlane();
		pWayPoint = pPathPlane->pathFollowReader.GetWayPoint();
		this->waypointLocation = *pWayPoint;
		this->bWaypointSet = 1;
	}

	CActorAutonomous::Reset();
	return;
}

void CActorAton::CheckpointReset()
{
	IMPLEMENTATION_GUARD();
}

void CActorAton::SaveContext(uint*, int)
{
	IMPLEMENTATION_GUARD();
}

void CActorAton::LoadContext(uint*, int)
{
	IMPLEMENTATION_GUARD();
}

CBehaviour* CActorAton::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 3) {
		pBehaviour = &this->behaviourAtonEscape;
	}
	else {
		pBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
	}
	return pBehaviour;
}

StateConfig CActorAton::_gStateCfg_ATO[46]
{
	StateConfig(
0x0,
0x100
),
StateConfig(
0x0,
0x900
),
StateConfig(
0xC,
0x82900
),
StateConfig(
0xD,
0xA2900
),
StateConfig(
0xE,
0xA0900
),
StateConfig(
0xF,
0xA0900
),
StateConfig(
0x10,
0xA2900
),
StateConfig(
0x11,
0x48900
),
StateConfig(
0x11,
0x48900
),
StateConfig(
0x11,
0x48900
),
StateConfig(
0x11,
0x48900
),
StateConfig(
0x11,
0x48900
),
StateConfig(
0x11,
0x48000
),
StateConfig(
0x1B,
0x5000
),
StateConfig(
0x1B,
0x5000
),
StateConfig(
0x0,
0x100
),
StateConfig(
0x0,
0x80100
),
StateConfig(
0xC,
0x82100
),
StateConfig(
0xD,
0x82100
),
StateConfig(
0xE,
0x80100
),
StateConfig(
0xF,
0x80100
),
StateConfig(
0x10,
0x82100
),
StateConfig(
0x14,
0x100
),
StateConfig(
0x15,
0x0
),
StateConfig(
0x16,
0x0
),
StateConfig(
0x17,
0x100
),
StateConfig(
0x1B,
0x401800
),
StateConfig(
0x19,
0x104900
),
StateConfig(
0x1A,
0x186900
),
StateConfig(
0x18,
0x4900
),
StateConfig(
0x18,
0x104900
),
StateConfig(
0x1B,
0x485800
),
StateConfig(
0x1B,
0x485800
),
StateConfig(
0x1B,
0x405800
),
StateConfig(
0x1B,
0x405000
),
StateConfig(
0x1B,
0x485000
),
StateConfig(
0x11,
0x40C000
),
StateConfig(
0x11,
0x248100
),
StateConfig(
0x0,
0x200100
),
StateConfig(
0x0,
0x200100
),
StateConfig(
0x11,
0x208100
),
StateConfig(
0x1C,
0x24100
),
StateConfig(
0x11,
0x8100
),
StateConfig(
0x0,
0x100
),
StateConfig(
0x0,
0x100
),
StateConfig(
0x0,
0x1)
};

StateConfig* CActorAton::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 6) {
		pStateConfig = CActorAutonomous::GetStateCfg(state);
	}
	else {
		pStateConfig = _gStateCfg_ATO + state + -6;
	}
	return pStateConfig;
}

void CActorAton::SetState(int newState, int animType)
{
	CActorMovable::SetState(newState, animType);
}

void CActorAton::AnimEvaluate(uint param_2, edAnmMacroAnimator* pAnimator, uint newAnim)
{
	bool bVar1;
	Timer* pTVar2;
	int* piVar3;
	edANM_HDR* peVar4;
	float fVar5;
	float fVar6;
	float puVar7;
	float puVar8;
	float puVar11;
	float puVar10;
	float fVar7;
	float puVar9;
	float puVar12;
	edAnmMacroBlendN local_c;
	edANM_HDR* local_8;
	edAnmMacroBlendN local_4;

	if (newAnim == 0x1c) {
		IMPLEMENTATION_GUARD(
		local_c = ((edAnmMacroBlendN*)&pAnimator->pAnimKeyTableEntry)->pHdr;
		fVar6 = this->dynamic.speed;
		fVar7 = edF32Vector4DotProductHard
		((edF32VECTOR4*)&((this->pMeshTransform)->base).transformA.ba,
			&CDynamicExt::gForceGravity);
		puVar11 = ABS(fVar7);
		if (1.0 < puVar11) {
			puVar10 = 1.0;
		}
		else {
			puVar10 = (float)&DAT_bf800000;
			if (-1.0 <= puVar11) {
				puVar10 = puVar11;
			}
		}
		fVar5 = acosf(puVar10);
		fVar7 = 1.0;
		fVar6 = (((fVar5 * 57.29578) / 90.0 + fVar6 / 20.0) * 2.0) / 3.0;
		if (fVar6 <= 1.0) {
			fVar7 = fVar6;
		}
		fVar6 = this->field_0x47c;
		if (fVar6 < this->field_0x478 * 10.0) {
			this->field_0x47c = fVar6 + 0.04;
			fVar5 = this->field_0x478 * 10.0;
			if (fVar5 < fVar6 + 0.04) {
				this->field_0x47c = fVar5;
			}
		}
		else {
			this->field_0x47c = fVar6 - 0.04;
			fVar5 = this->field_0x478 * 10.0;
			if (fVar6 - 0.04 < fVar5) {
				this->field_0x47c = fVar5;
			}
		}
		if (1.0 < this->field_0x47c) {
			this->field_0x47c = 1.0;
		}
		else {
			if (this->field_0x47c < -1.0) {
				this->field_0x47c = (float)&DAT_bf800000;
			}
		}
		if (0.0 < this->field_0x47c) {
			CActor::SV_Blend4AnimationsWith2Ratios(this->field_0x47c, fVar7, &local_c, 0, 2, 3, 4);
		}
		else {
			CActor::SV_Blend4AnimationsWith2Ratios(ABS(this->field_0x47c), fVar7, &local_c, 0, 1, 3, 4);
		})
	}
	else {
		if (newAnim == 0x1b) {
			IMPLEMENTATION_GUARD(
			local_8 = pAnimator->pAnimKeyTableEntry;
			if ((this->actorState != 0x20) ||
				(fVar6 = this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y,
					0.0 <= fVar6)) {
				fVar6 = this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y;
			}
			else {
				fVar6 = fVar6 + 10.0;
			}
			puVar9 = fVar6 / 10.0;
			if (1.0 < puVar9) {
				puVar12 = 1.0;
			}
			else {
				puVar12 = (float)&DAT_bf800000;
				if (-1.0 <= puVar9) {
					puVar12 = puVar9;
				}
			}
			puVar7 = this->field_0x478 * 10.0;
			if (1.0 < puVar7) {
				puVar8 = 1.0;
			}
			else {
				puVar8 = (float)&DAT_bf800000;
				if (-1.0 <= puVar7) {
					puVar8 = puVar7;
				}
			}
			fVar7 = edFIntervalDotSrcLERP(puVar8, 0.0, 1.0);
			fVar6 = this->field_0x484;
			if (fVar6 < fVar7) {
				this->field_0x484 = fVar6 + 0.04;
				if (fVar7 < fVar6 + 0.04) {
					this->field_0x484 = fVar7;
				}
			}
			else {
				this->field_0x484 = fVar6 - 0.04;
				if (fVar6 - 0.04 < fVar7) {
					this->field_0x484 = fVar7;
				}
			}
			if (this->field_0x484 < 0.4) {
				bVar1 = false;
			}
			else {
				if (0.6 < this->field_0x484) {
					bVar1 = false;
				}
				else {
					bVar1 = true;
				}
			}
			if (bVar1) {
				piVar3 = &local_8[1].count_0x0 + local_8->keyIndex_0x8;
				fVar6 = (float)piVar3[2];
				if (fVar6 < 0.0) {
					piVar3[2] = (int)(fVar6 + 0.02);
					if (0.0 < (float)(&local_8[1].count_0x0 + local_8->keyIndex_0x8)[2]) {
						(&local_8[1].count_0x0 + local_8->keyIndex_0x8)[2] = 0;
					}
				}
				else {
					piVar3[2] = (int)(fVar6 - 0.02);
					if ((float)(&local_8[1].count_0x0 + local_8->keyIndex_0x8)[2] < 0.0) {
						(&local_8[1].count_0x0 + local_8->keyIndex_0x8)[2] = 0;
					}
				}
				piVar3 = &local_8[1].count_0x0 + local_8->keyIndex_0x8;
				fVar6 = (float)piVar3[1];
				if (fVar6 < 0.0) {
					piVar3[1] = (int)(fVar6 + 0.02);
					if (0.0 < (float)(&local_8[1].count_0x0 + local_8->keyIndex_0x8)[1]) {
						(&local_8[1].count_0x0 + local_8->keyIndex_0x8)[1] = 0;
					}
				}
				else {
					piVar3[1] = (int)(fVar6 - 0.02);
					if ((float)(&local_8[1].count_0x0 + local_8->keyIndex_0x8)[1] < 0.0) {
						(&local_8[1].count_0x0 + local_8->keyIndex_0x8)[1] = 0;
					}
				}
			}
			else {
				piVar3 = &local_8[1].count_0x0 + local_8->keyIndex_0x8;
				fVar6 = (float)piVar3[1];
				if (fVar6 < this->field_0x484) {
					piVar3[1] = (int)(fVar6 + 0.02);
					if (this->field_0x484 < (float)(&local_8[1].count_0x0 + local_8->keyIndex_0x8)[1]) {
						(&local_8[1].count_0x0 + local_8->keyIndex_0x8)[1] = (int)this->field_0x484;
					}
				}
				else {
					piVar3[1] = (int)(fVar6 - 0.02);
					if ((float)(&local_8[1].count_0x0 + local_8->keyIndex_0x8)[1] < this->field_0x484) {
						(&local_8[1].count_0x0 + local_8->keyIndex_0x8)[1] = (int)this->field_0x484;
					}
				}
				piVar3 = &local_8[1].count_0x0 + local_8->keyIndex_0x8;
				fVar6 = (float)piVar3[2];
				if (fVar6 < 1.0 - this->field_0x484) {
					piVar3[2] = (int)(fVar6 + 0.02);
					fVar6 = 1.0 - this->field_0x484;
					if (fVar6 < (float)(&local_8[1].count_0x0 + local_8->keyIndex_0x8)[2]) {
						(&local_8[1].count_0x0 + local_8->keyIndex_0x8)[2] = (int)fVar6;
					}
				}
				else {
					piVar3[2] = (int)(fVar6 - 0.02);
					fVar6 = 1.0 - this->field_0x484;
					if ((float)(&local_8[1].count_0x0 + local_8->keyIndex_0x8)[2] < fVar6) {
						(&local_8[1].count_0x0 + local_8->keyIndex_0x8)[2] = (int)fVar6;
					}
				}
			}
			if (0.0 <= puVar12) {
				(&local_8[2].field_0x4)[local_8->keyIndex_0x8] = (int)puVar12;
				(&local_8[1].count_0x0)[local_8->keyIndex_0x8] =
					(int)(1.0 - (float)(&local_8[1].count_0x0 + local_8->keyIndex_0x8)[4]);
			}
			else {
				(&local_8[2].count_0x0)[local_8->keyIndex_0x8] = (int)ABS(puVar12);
				(&local_8[1].count_0x0)[local_8->keyIndex_0x8] =
					(int)(1.0 - (float)(&local_8[2].count_0x0)[local_8->keyIndex_0x8]);
			})
		}
		else {
			if (newAnim == 0x11) {
				edAnmMacroBlendN macroBlendN = edAnmMacroBlendN(pAnimator->pAnimKeyTableEntry);

				// Original code jumped straight to the floats after the 3 int values here.
				char* pBase = (char*)pAnimator->pAnimKeyTableEntry;
				AnimKeySomething* pValue = (AnimKeySomething*)(pBase + pAnimator->pAnimKeyTableEntry->keyIndex_0x8.asKey * 4);

				fVar6 = 1.0f;
				fVar7 = this->dynamic.speed / 5.0f;

				if (fVar7 <= 1.0f) {
					fVar6 = fVar7;
				}

				pTVar2 = GetTimer();
				fVar5 = this->field_0x3dc;
				fVar7 = pTVar2->cutsceneDeltaTime / 0.15f;
				if (fVar5 < fVar6) {
					fVar5 = fVar5 + fVar7;
					this->field_0x3dc = fVar5;
					if (fVar6 < fVar5) {
						this->field_0x3dc = fVar6;
					}
				}
				else {
					fVar5 = fVar5 - fVar7;
					this->field_0x3dc = fVar5;
					if (fVar5 < fVar6) {
						this->field_0x3dc = fVar6;
					}
				}

				if (1.0f < this->field_0x3dc) {
					this->field_0x3dc = 1.0f;
				}
				else {
					if (this->field_0x3dc < 0.0f) {
						this->field_0x3dc = 0.0f;
					}
				}

				if ((this->actorState == 0xe) && (this->field_0x3d8 == 0.0f)) {
					pTVar2 = GetTimer();
					this->field_0x3d4 = this->field_0x3d4 + pTVar2->cutsceneDeltaTime / 2.0f;
				}
				else {
					pTVar2 = GetTimer();
					this->field_0x3d4 = this->field_0x3d4 - pTVar2->cutsceneDeltaTime / 2.0f;
				}

				if (1.0f < this->field_0x3d4) {
					this->field_0x3d4 = 1.0f;
				}
				else {
					if (this->field_0x3d4 < 0.0f) {
						this->field_0x3d4 = 0.0f;
					}
				}

				if ((this->actorState == 0x10) && (this->field_0x3d4 == 0.0f)) {
					pTVar2 = GetTimer();
					this->field_0x3d8 = this->field_0x3d8 + pTVar2->cutsceneDeltaTime / 2.0f;
				}
				else {
					pTVar2 = GetTimer();
					this->field_0x3d8 = this->field_0x3d8 - pTVar2->cutsceneDeltaTime / 2.0f;
				}

				if (1.0f < this->field_0x3d8) {
					this->field_0x3d8 = 1.0f;
				}
				else {
					if (this->field_0x3d8 < 0.0f) {
						this->field_0x3d8 = 0.0f;
					}
				}

				if (0.0f < this->field_0x3d4) {
					CActor::SV_Blend4AnimationsWith2Ratios(this->field_0x3dc, this->field_0x3d4, &macroBlendN, 0, 1, 2, 3);
					pValue->field_0x1c = 0.0f;
					pValue->field_0x20 = 0.0f;
				}
				else {
					fVar6 = this->field_0x3d8;
					if (0.0f < fVar6) {
						CActor::SV_Blend4AnimationsWith2Ratios(this->field_0x3dc, fVar6, &macroBlendN, 0, 1, 4, 5);
						pValue->field_0x14 = 0.0f;
						pValue->field_0x18 = 0.0f;
					}
					else {
						if ((fVar6 == 0.0f) && (fVar6 == 0.0f)) {
							CActor::SV_Blend4AnimationsWith2Ratios(this->field_0x3dc, fVar6, &macroBlendN, 0, 1, 2, 3);
							pValue->field_0x1c = 0.0f;
							pValue->field_0x20 = 0.0f;
							pValue->field_0x14 = 0.0f;
							pValue->field_0x18 = 0.0f;
						}
					}
				}
			}
			else {
				CActor::AnimEvaluate(param_2, pAnimator, newAnim);
			}
		}
	}

	return;
}

bool CActorAton::CarriedByActor(CActor* pActor, edF32MATRIX4* m0)
{
	bool bVar1;
	Timer* pTVar2;
	float fVar3;
	float fVar4;

	fVar3 = this->currentLocation.y;
	bVar1 = CActorAutonomous::CarriedByActor(pActor, m0);
	this->field_0x3e8 = (int)bVar1;
	fVar4 = this->currentLocation.y;
	pTVar2 = GetTimer();
	this->field_0x3e4 = (fVar4 - fVar3) / pTVar2->cutsceneDeltaTime;
	return this->field_0x3e8;
}

int CActorAton::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CPathPlane* pPathPlane;
	int result;

	if (msg == 0xf) {
		if (this->curBehaviourId != 1) {
			pPathPlane = this->pathPlaneArray.GetCurPathPlane();

			if (((pPathPlane->pathFollowReader).pPathFollow)->field_0xc == 0xd) {
				this->pathPlaneArray.NextWayPoint();
				AnalysePathType();
			}
		}
		result = 0;
	}
	else {
		result = CActorAutonomous::InterpretMessage(pSender, msg, pMsgParam);
	}

	return result;
}

int CActorAton::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return CActor::InterpretEvent(pEventMessage, param_3, param_4, param_5);
}

CPathPlaneArray::CPathPlaneArray()
{
	this->nbPathPlanes = 0;
	this->aPathPlanes = (CPathPlane*)0x0;
	this->curIndex = 0;
}

void CPathPlaneArray::Create(ByteCode* pByteCode)
{
	int iVar1;
	uint uVar2;
	int* piVar3;
	int iVar4;
	int iVar5;

	this->nbPathPlanes = pByteCode->GetU32();

	uVar2 = this->nbPathPlanes;
	if (uVar2 != 0) {
		this->aPathPlanes = new CPathPlane[uVar2];
	}

	if ((this->nbPathPlanes != 0) && (uVar2 = 0, this->nbPathPlanes != 0)) {
		iVar5 = 0;
		do {
			this->aPathPlanes[uVar2].pathFollowReader.index = pByteCode->GetS32();
			uVar2 = uVar2 + 1;
		} while (uVar2 < this->nbPathPlanes);
	}
	return;
}

void CActorAton::ClearLocalData()
{
	this->dynamic.speed = 0.0f;

	this->field_0x3b0 = 0.0f;
	(this->fleeOnPathParams).delay = this->pathDefaultDelay;
	(this->fleeOnPathParams).acceleration = 0.0f;
	(this->fleeOnPathParams).speed = 0.0f;
	this->pathPlaneArray.Reset();
	this->vectorDyn.Reset();

	this->field_0x3e4 = 0.0f;
	this->field_0x3e8 = 0;

	this->field_0x3d4 = 0.0f;
	this->field_0x3d8 = 0.0f;
	this->field_0x478 = 0.0f;
	this->field_0x47c = 0.0f;
	//this->field_0x484 = 0.5f;
	this->field_0x3dc = 0.0f;
	this->field_0x474 = 0;
	//this->field_0x488 = 0;
	this->behaviourAddOn.Manage();
	return;
}

bool CActorAton::ComputeSpeedAndAccelerationAndAnalyseForRun()
{
	float fVar1;
	bool bVar2;
	bool bVar3;
	CPathPlane* pCurPathPlane;
	edF32VECTOR4* peVar5;
	Timer* pTVar6;
	float fVar7;
	float fVar8;
	float fVar9;

	bVar2 = false;
	if (this->field_0x474 == 0) {
		(this->fleeOnPathParams).field_0x4 = 1;
		pCurPathPlane = this->pathPlaneArray.GetCurPathPlane();
		(this->fleeOnPathParams).delay = SV_MOV_ComputeDistIdealPos(&pCurPathPlane->pathFollowReader, (this->fleeOnPathParams).delay, this->pathDefaultDelay);

		pCurPathPlane = this->pathPlaneArray.GetCurPathPlane();
		SV_MOV_ComputeSpeedAccelerationToFleeActor(3.0f, CActorHero::_gThis, pCurPathPlane, &this->fleeOnPathParams);

		if (7.0f < (this->fleeOnPathParams).acceleration) {
			(this->fleeOnPathParams).acceleration = 7.0f;
		}

		pCurPathPlane = this->pathPlaneArray.GetCurPathPlane();
		if (((pCurPathPlane->pathFollowReader).pPathFollow)->field_0xc == 1) {
			fVar9 = (this->fleeOnPathParams).acceleration;
			if (fVar9 < 4.0f) {
				(this->fleeOnPathParams).acceleration = 4.0f;
			}
			bVar2 = fVar9 < 4.0f && fVar9 < 3.0f;
			fVar9 = (this->fleeOnPathParams).speed;
			if ((fVar9 < 20.0f) && (0.0f < fVar9)) {
				(this->fleeOnPathParams).speed = 20.0f;
			}
		}

		pCurPathPlane = this->pathPlaneArray.GetCurPathPlane();
		if ((((((pCurPathPlane->pathFollowReader).pPathFollow)->field_0xc == 0) || (pCurPathPlane = this->pathPlaneArray.GetCurPathPlane(),
				((pCurPathPlane->pathFollowReader).pPathFollow)->field_0xc == 0xd)) && (fVar9 = (this->fleeOnPathParams).acceleration, fVar9 < 2.0f)) && (fVar9 < 1.0f)) {
			bVar2 = true;
		}

		pCurPathPlane = this->pathPlaneArray.GetCurPathPlane();
		if (((pCurPathPlane->pathFollowReader).pPathFollow)->field_0xc == 0xd) {
			pCurPathPlane = this->pathPlaneArray.GetCurPathPlane();
			bVar3 = pCurPathPlane->pathFollowReader.AtGoal((pCurPathPlane->pathFollowReader).splinePointIndex, (pCurPathPlane->pathFollowReader).field_0xc);
			if (bVar3 != false) {
				pCurPathPlane = this->pathPlaneArray.GetCurPathPlane();
				peVar5 = pCurPathPlane->pathFollowReader.GetWayPoint();
				fVar9 = peVar5->x - this->currentLocation.x;
				fVar1 = peVar5->z - this->currentLocation.z;
				pTVar6 = GetTimer();
				fVar8 = 0.5f;

				fVar7 = this->dynamic.linearAcceleration * pTVar6->cutsceneDeltaTime;
				if (0.5f <= fVar7) {
					fVar8 = fVar7;
				}

				if (sqrtf(fVar9 * fVar9 + 0.0f + fVar1 * fVar1) < fVar8) {
					bVar2 = true;
				}
			}
		}
	}
	else {
		bVar2 = true;
	}

	return bVar2;
}

void CActorAton::AnalysePathType()
{
	float* pfVar1;
	float fVar2;
	int iVar3;
	CPathPlane* pCurPathPlane;
	edF32VECTOR4* peVar5;
	Timer* pTVar6;
	uint uVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;

	iVar3 = this->pathPlaneArray.AtGoal();
	if (iVar3 != 0) {
		pCurPathPlane = this->pathPlaneArray.GetCurPathPlane();
		peVar5 = pCurPathPlane->pathFollowReader.GetWayPoint();
		fVar11 = peVar5->x - this->currentLocation.x;
		fVar2 = peVar5->y - this->currentLocation.y;
		fVar10 = peVar5->z - this->currentLocation.z;
		pTVar6 = GetTimer();
		fVar9 = 0.5f;

		fVar8 = this->dynamic.linearAcceleration * pTVar6->cutsceneDeltaTime;
		if (0.5f <= fVar8) {
			fVar9 = fVar8;
		}

		if (sqrtf(fVar11 * fVar11 + fVar2 * fVar2 + fVar10 * fVar10) < fVar9) {
			SetState(0x32, -1);
			return;
		}
	}

	pCurPathPlane = this->pathPlaneArray.GetCurPathPlane();

	switch (((pCurPathPlane->pathFollowReader).pPathFollow)->field_0xc) {
	case 0:
	case 1:
	case 0xd:
		uVar7 = ComputeSpeedAndAccelerationAndAnalyseForRun();

		if (uVar7 == 0) {
			SetState(0xd, -1);
		}
		else {
			SetState(ATON_ESCAPE_STATE_PATH_BEGIN, -1);
		}
		break;
	case 3:
		this->field_0x3e4 = 0.0f;
	case 2:
		if (this->actorState == 0x28) {
			SetState(0x13, -1);
		}
		else {
			pCurPathPlane = this->pathPlaneArray.GetCurPathPlane();
			peVar5 = pCurPathPlane->pathFollowReader.GetWayPoint();
			fVar11 = peVar5->x - this->currentLocation.x;
			fVar2 = peVar5->z - this->currentLocation.z;
			pTVar6 = GetTimer();
			fVar10 = 0.5f;

			fVar9 = this->dynamic.linearAcceleration * pTVar6->cutsceneDeltaTime;
			if (0.5f <= fVar9) {
				fVar10 = fVar9;
			}

			if (sqrtf(fVar11 * fVar11 + 0.0f + fVar2 * fVar2) < fVar10) {
				IMPLEMENTATION_GUARD(
				this->pathPlaneArray.NextWayPoint();
				FUN_00387660();)
			}
			else {
				SetState(ATON_ESCAPE_STATE_JUMP, -1);
			}
		}
		break;
	case 4:
		SetState(0x20, -1);
		break;
	case 5:
		SetState(0x21, -1);
		break;
	case 7:
		this->dynamic.speed = 0.0;
		SetState(0x2f, -1);
		break;
	case 8:
		SetState(0x27, -1);
		break;
	case 9:
		SetState(0x28, -1);
		break;
	case 10:
		SetState(0x2a, -1);
		break;
	case 0xb:
		SetState(0x2b, -1);
		break;
	case 0xc:
		SetState(0x30, -1);
		break;
	case 0xe:
		IMPLEMENTATION_GUARD(
		pCurPathPlane = this->pathPlaneArray.GetCurPathPlane();

		pfVar1 = ((pCurPathPlane->pathFollowReader).pPathFollow)->field_0x2c;
		if (pfVar1 == (float*)0x0) {
			fVar11 = 0.0f;
		}
		else {
			fVar11 = pfVar1[1];
		}

		this->field_0x48c = fVar11;

		if (fVar11 == 0.0f) {
			this->field_0x48c = this->pathDefaultDelay;
		}

		fVar11 = CActorHero::_gThis->currentLocation.x - this->currentLocation.x;
		fVar2 = CActorHero::_gThis->currentLocation.y - this->currentLocation.y;
		fVar10 = CActorHero::_gThis->currentLocation.z - this->currentLocation.z;
		if (sqrtf(fVar11 * fVar11 + fVar2 * fVar2 + fVar10 * fVar10) < this->field_0x48c) {
			this->pathPlaneArray.NextPathPlane();
			AnalysePathType(this);
		}
		else {
			SetState(0x29, -1);
		})
	}

	return;
}

void CActorAton::BehaviourAtonEscape_InitState(int newState)
{
	CAnimation* pCVar1;
	CCollision* pCVar2;
	int* piVar3;
	bool bVar4;
	StateConfig* pSVar5;
	CPathPlane* pCVar6;
	int iVar7;
	edF32VECTOR4* peVar8;
	uint uVar9;
	ulong uVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 local_50;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	if (newState == -1) {
		uVar9 = 0;
	}
	else {
		pSVar5 = GetStateCfg(newState);
		uVar9 = pSVar5->flags_0x4;
	}

	if ((uVar9 & 0x1000) != 0) {
		this->pAnimationController->RegisterBone(this->field_0x354);
		this->pAnimationController->RegisterBone(this->field_0x358);
		this->pAnimationController->RegisterBone(this->field_0x35c);
		this->pAnimationController->RegisterBone(this->field_0x360);
	}

	uVar9 = 0;
	if (newState != -1) {
		pSVar5 = GetStateCfg(newState);
		uVar9 = pSVar5->flags_0x4;
	}
	if ((uVar9 & 0x8000) == 0) {
		iVar7 = this->prevActorState;
		uVar9 = 0;
		if (iVar7 != -1) {
			pSVar5 = GetStateCfg(iVar7);
			uVar9 = pSVar5->flags_0x4;
		}

		if ((uVar9 & 0x8000) != 0) {
			this->field_0x3d4 = 0.0f;
			this->field_0x3d8 = 0.0f;
			this->field_0x3dc = 0.0f;
		}
	}

	uVar9 = 0;
	if (newState != -1) {
		pSVar5 = GetStateCfg(newState);
		uVar9 = pSVar5->flags_0x4;
	}

	if ((uVar9 & 0x8000) != 0) {
		iVar7 = this->prevActorState;
		uVar9 = 0;
		if (iVar7 != -1) {
			pSVar5 = GetStateCfg(iVar7);
			uVar9 = pSVar5->flags_0x4;
		}

		if ((uVar9 & 0x2000) != 0) {
			this->field_0x3d4 = 1.0f;
			this->field_0x3d8 = 0.0f;
			this->field_0x3dc = 0.0f;
		}
	}

	if (newState == 0x33) {
		this->field_0x475 = 1;
		this->flags = this->flags & 0xfffffffd;
		this->flags = this->flags | 1;
		this->flags = this->flags & 0xffffff7f;
		this->flags = this->flags | 0x20;
		EvaluateDisplayState();
	}
	else {
		if (newState == 0x32) {
			this->dynamic.speed = 0.0f;
			this->dynamicExt.normalizedTranslation.x = 0.0f;
			this->dynamicExt.normalizedTranslation.y = 0.0f;
			this->dynamicExt.normalizedTranslation.z = 0.0f;
			this->dynamicExt.normalizedTranslation.w = 0.0f;
			this->dynamicExt.field_0x6c = 0.0f;
		}
		else {
			if (newState == 0x28) {
				pCVar6 = this->pathPlaneArray.GetCurPathPlane();
				pCVar6->pathFollowReader.GetWayPoint();
				pCVar6 = this->pathPlaneArray.GetCurPathPlane();
				pCVar6->pathFollowReader.GetWayPoint();
				this->dynamicExt.normalizedTranslation.x = 0.0f;
				this->dynamicExt.normalizedTranslation.y = 0.0f;
				this->dynamicExt.normalizedTranslation.z = 0.0f;
				this->dynamicExt.normalizedTranslation.w = 0.0f;
				this->dynamicExt.field_0x6c = 0.0f;
			}
			else {
				if (newState == 0x29) {
					this->field_0x478 = 0.0f;
				}
				else {
					if (newState == 0x25) {
						this->field_0x478 = 0.0f;
					}
					else {
						if (newState == 0x26) {
							this->field_0x478 = 0.0f;
						}
						else {
							if (newState == 0x2f) {
								this->field_0x478 = 0.0f;
								this->field_0x47c = 0.0f;
								this->field_0x3c0 = this->rotationQuat;
								this->flags = this->flags | 0x1000;

								IMPLEMENTATION_GUARD(
								CFxHandle::SV_FX_Start((CFxHandle*)&this->field_0x634);
								piVar3 = (int*)this->field_0x638;
								if (((piVar3 != (int*)0x0) && (this->field_0x634 != 0)) && (this->field_0x634 == piVar3[6])) {
									(**(code**)(*piVar3 + 0x10))(0, 0);
								}
								bVar4 = false;
								if ((this->field_0x644 != -1) && (this->field_0x648 != -1)) {
									bVar4 = true;
								}
								if (bVar4) {
									FUN_00114bc0(0.0, 0.0, (StaticMeshComponent*)&this->staticMeshComponent, (ed_3D_Scene*)0x0,
										(ed_g3d_manager*)0x0, (char*)0x0);
									this->field_0x700 = 0;
								})
							}
							else {
								if (newState == ATON_ESCAPE_STATE_PATH_JUMP_2_4) {
									iVar7 = this->pathPlaneArray.GetCurPathPlane()->pathFollowReader.GetNextPlace((this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).splinePointIndex,
										(this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).field_0xc);
									local_30 = *this->pathPlaneArray.GetCurPathPlane()->pathFollowReader.GetWayPoint(iVar7);
									fVar11 = this->currentLocation.y;
									bVar4 = fVar11 < local_30.y;
									uVar9 = (int)local_30.y * (uint)bVar4;
									fVar12 = this->pathPlaneArray.GetCurPathPlane()->pathFollowReader.GetWayPoint()->y;
									edF32Vector4SubHard(&eStack64, &local_30, &this->currentLocation);
									vectorDyn.BuildFromAccelDistAmplitude(fVar12 - (float)(uVar9 | (int)fVar11 * (uint)!bVar4), &CDynamicExt::gForceGravity, &eStack64, 1);
									pCVar1 = this->pAnimationController;

									iVar7 = GetIdMacroAnim(0x15);
									if (-1 < iVar7) {
										pCVar1->GetAnimLength(iVar7, 1);
									}

									pCVar1 = this->pAnimationController;
									fVar11 = (this->vectorDyn).field_0x40;
									iVar7 = GetIdMacroAnim(0x15);
									if (iVar7 < 0) {
										fVar12 = 0.0f;
									}
									else {
										fVar12 = pCVar1->GetAnimLength(iVar7, 1);
									}

									this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(fVar12 / fVar11, 0);
									this->field_0x3e8 = 0;

									pCVar2 = this->pCollisionData;
									if (pCVar2 != (CCollision*)0x0) {
										pCVar2->actorFieldA = (CActor*)CActorHero::_gThis;
										(CActorHero::_gThis->pCollisionData)->actorFieldA = this;
									}
								}
								else {
									if (newState == 0x14) {
										IMPLEMENTATION_GUARD(
										this->pathPlaneArray.GetCurPathPlane() = CPathPlaneArray::GetCurPathPlane();
										iVar7 = this->pathPlaneArray.GetCurPathPlane()->pathFollowReader.GetNextPlace
										((CPathFollowReader*)this->pathPlaneArray.GetCurPathPlane(), (this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).splinePointIndex,
											(this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).field_0xc);
										this->pathPlaneArray.GetCurPathPlane() = CPathPlaneArray::GetCurPathPlane();
										peVar8 = this->pathPlaneArray.GetCurPathPlane()->pathFollowReader.GetWayPoint((CPathFollowReader*)this->pathPlaneArray.GetCurPathPlane(), iVar7);
										local_50.z = peVar8->z;
										local_50.w = peVar8->w;
										local_50.x = (float)*(undefined8*)peVar8;
										local_50.y = (float)((ulong) * (undefined8*)peVar8 >> 0x20);
										fVar11 = this->currentLocation.y;
										bVar4 = fVar11 < local_50.y;
										uVar9 = (int)local_50.y * (uint)bVar4;
										this->pathPlaneArray.GetCurPathPlane() = CPathPlaneArray::GetCurPathPlane();
										peVar8 = this->pathPlaneArray.GetCurPathPlane()->pathFollowReader.GetWayPoint((CPathFollowReader*)this->pathPlaneArray.GetCurPathPlane());
										fVar12 = peVar8->y;
										edF32Vector4SubHard(&eStack96, &local_50, &this->currentLocation);
										CVectorDyn::BuildFromAccelDistAmplitude
										(fVar12 - (float)(uVar9 | (int)fVar11 * (uint)!bVar4), &this->vectorDyn,
											&CDynamicExt::gForceGravity, &eStack96, 1);
										pCVar1 = this->pAnimationController;
										iVar7 = CActor::GetIdMacroAnim((CActor*)this, New_Name_(21));
										if (-1 < iVar7) {
											CAnimation::GetAnimLength(pCVar1, iVar7, 1);
										}
										pCVar1 = this->pAnimationController;
										fVar11 = (this->vectorDyn).field_0x40;
										iVar7 = CActor::GetIdMacroAnim((CActor*)this, New_Name_(21));
										if (iVar7 < 0) {
											fVar12 = 0.0;
										}
										else {
											fVar12 = CAnimation::GetAnimLength(pCVar1, iVar7, 1);
										}
										edAnmMetaAnimator::SetLayerTimeWarper
										(fVar12 / fVar11, (edAnmBinMetaAnimator*)this->pAnimationController, 0);
										this->field_0x3e8 = 0;
										pCVar2 = this->pCollisionData;
										if (pCVar2 != (CCollision*)0x0) {
											pCVar2->actorFieldA = (CActor*)CActorHero::_gThis;
											(CActorHero::_gThis->pCollisionData)->actorFieldA =
												(CActor*)this;
										}
										this->field_0x478 = 0.0;)
									}
									else {
										if ((newState == ATON_ESCAPE_STATE_PATH_BEFORE_WAIT_JUMP) || (newState == ATON_ESCAPE_STATE_PATH_JUMP_1_4)) {
											this->dynamic.speed = 0.0f;
											this->dynamicExt.normalizedTranslation.x = 0.0f;
											this->dynamicExt.normalizedTranslation.y = 0.0f;
											this->dynamicExt.normalizedTranslation.z = 0.0f;
											this->dynamicExt.normalizedTranslation.w = 0.0f;
											this->dynamicExt.field_0x6c = 0.0f;
										}
										else {
											if ((((((newState == 0x10) || (newState == 0xe)) || (newState == 0x1b)) ||
												((newState == 0x1a || (newState == 0x19)))) ||
												((newState == ATON_ESCAPE_STATE_PATH_STAND_CALL_BACK || ((newState == ATON_ESCAPE_STATE_PATH_STAND_CALL_FAR ||
													(newState == ATON_ESCAPE_STATE_PATH_STAND_CALL_NEAR)))))) || (newState == 0x22)) {
												this->field_0x480 = 0;
											}
											else {
												if (newState == 0x24) {
													if (this->pCollisionData != (CCollision*)0x0) {
														local_10.x = 0.0f;
														local_10.z = 0.0f;
														local_10.y = 0.34f;
														local_20.w = 0.0f;
														local_10.w = 1.0f;
														local_20.x = 0.3f;
														local_20.z = 0.3f;
														local_20.y = 0.45f;
														ChangeCollisionSphere(0.0f, &local_20, &local_10);
													}
												}
												else {
													if (newState == 0x21) {
														this->dynamic.speed = 0.0f;
														this->dynamicExt.normalizedTranslation.x = 0.0f;
														this->dynamicExt.normalizedTranslation.y = 0.0f;
														this->dynamicExt.normalizedTranslation.z = 0.0f;
														this->dynamicExt.normalizedTranslation.w = 0.0f;
														this->dynamicExt.field_0x6c = 0.0f;
													}
													else {
														if (newState == 0xd) {
															uVar10 = CScene::_pinstance->field_0x38 * 0x343fd + 0x269ec3;
															CScene::_pinstance->field_0x38 = uVar10;
															this->field_0x3d0 = ((float)((uint)(uVar10 >> 0x10) & 0x7fff) * 2.0f) / 32767.0f + 1.0f;
														}
														else {
															if ((newState == 9) || (newState == ATON_ESCAPE_STATE_PATH_BEGIN)) {
																this->dynamic.speed = 0.0f;
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return;
}

void CActorAton::BehaviourAtonEscape_Manage()
{
	uint uVar1;
	CCollision* pCVar2;
	CAnimation* pCVar3;
	edAnmLayer* peVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	CActorHero* pCVar8;
	CActorHero* pCVar9;
	bool bVar10;
	uint uVar11;
	uint uVar12;
	int iVar13;
	undefined4 uVar14;
	edF32VECTOR4* peVar15;
	StateConfig* pSVar16;
	Timer* pTVar17;
	CPathPlane* pCVar18;
	long lVar19;
	ulong uVar21;
	undefined uVar22;
	float fVar23;
	float fVar24;
	float puVar25;
	float puVar26;
	float fVar27;
	edF32VECTOR4 eStack1296;
	edF32VECTOR4 eStack1280;
	float fStack1264;
	float fStack1260;
	float fStack1256;
	float fStack1252;
	edF32VECTOR4 eStack1248;
	edF32VECTOR4 eStack1232;
	float fStack1216;
	float fStack1212;
	float fStack1208;
	float fStack1204;
	edF32VECTOR4 eStack1200;
	edF32VECTOR4 eStack1184;
	float fStack1168;
	float fStack1164;
	float fStack1160;
	float fStack1156;
	float fStack1152;
	float fStack1148;
	float fStack1144;
	float fStack1140;
	float fStack1136;
	float fStack1132;
	float fStack1128;
	float fStack1124;
	edF32VECTOR4 eStack1120;
	edF32VECTOR4 eStack1104;
	edF32VECTOR4 eStack1088;
	edF32VECTOR4 eStack1072;
	edF32VECTOR4 eStack1056;
	float fStack1040;
	undefined4 uStack1036;
	float fStack1032;
	float fStack1028;
	edF32VECTOR4 eStack1024;
	edF32VECTOR4 eStack1008;
	edF32VECTOR4 eStack992;
	edF32VECTOR4 eStack976;
	edF32VECTOR4 eStack960;
	float fStack944;
	undefined4 uStack940;
	float fStack936;
	float fStack932;
	edF32VECTOR4 eStack928;
	edF32VECTOR4 eStack912;
	edF32VECTOR4 eStack896;
	edF32VECTOR4 eStack880;
	edF32VECTOR4 eStack864;
	edF32VECTOR4 eStack848;
	edF32VECTOR4 eStack832;
	float fStack816;
	undefined4 uStack812;
	float fStack808;
	float fStack804;
	edF32VECTOR4 eStack800;
	edF32VECTOR4 eStack784;
	float fStack768;
	undefined4 uStack764;
	float fStack760;
	float fStack756;
	SV_MOV_PATH_PARAM SStack752;
	SV_MOV_PATH_PARAM SStack720;
	SV_MOV_PATH_PARAM SStack688;
	SV_MOV_PATH_PARAM SStack656;
	SV_MOV_PATH_PARAM SStack624;
	SV_MOV_PATH_PARAM SStack592;
	edF32VECTOR4 eStack560;
	edF32VECTOR4 eStack544;
	SV_MOV_PATH_PARAM SStack528;
	SV_MOV_PATH_PARAM SStack496;
	SV_MOV_PATH_PARAM SStack464;
	SV_MOV_PATH_PARAM SStack432;
	SV_MOV_PATH_PARAM SStack400;
	SV_MOV_PATH_PARAM SStack368;
	edF32VECTOR4 local_150;
	edF32VECTOR4 local_140;
	edF32VECTOR4 eStack304;
	edF32VECTOR4 eStack288;
	edF32VECTOR4 eStack272;
	edF32VECTOR4 eStack256;
	CPathPlaneOutData CStack240;
	CPathPlaneOutData CStack224;
	CPathPlaneOutData CStack208;
	CPathPlaneOutData CStack192;
	CPathPlaneOutData CStack176;
	CPathPlaneOutData CStack160;
	CPathPlaneOutData CStack144;
	CPathPlaneOutData CStack128;
	CPathPlaneOutData CStack112;
	CPathPlaneOutData CStack96;
	CPathPlaneOutData CStack80;
	CPathPlaneOutData CStack64;
	float fStack48;
	float fStack44;
	float fStack40;
	float fStack36;
	float fStack32;
	float fStack28;
	float fStack24;
	float fStack20;
	float fStack16;
	float fStack12;
	float fStack8;
	float fStack4;

	this->pathPlaneArray.GetCurPathPlane();
	uVar12 = this->field_0x358;
	uVar1 = this->field_0x354;
	if (this->trailMaterialId != -1) {
		iVar13 = this->actorState;
		if (iVar13 == -1) {
			uVar11 = 0;
		}
		else {
			pSVar16 = GetStateCfg(iVar13);
			uVar11 = pSVar16->flags_0x4;
		}

		if ((uVar11 & 0x1000) == 0) {
			IMPLEMENTATION_GUARD_LOG(
			this->fxTailA.Manage(&gF32Vertex4Zero, &gF32Vertex4Zero, 1);)
		}
		else {
			IMPLEMENTATION_GUARD_LOG(
			SV_GetBoneWorldPosition(uVar12, &eStack256);
			SV_GetBoneWorldPosition(uVar1, &eStack272);
			fVar23 = this->dynamic.linearAcceleration;
			if (fVar23 < 30.0) {
				fVar23 = fVar23 / 30.0;
				fVar27 = 1.0 - fVar23;
				(this->fxTailA).field_0x28 = (this->fxTailA).field_0x24;
				fVar24 = fVar27 * 128.0;
				*(undefined*)&(this->fxTailA).field_0x24 = 0x80;
				if (fVar24 < 2.147484e+09) {
					uVar22 = (undefined)(int)fVar24;
				}
				else {
					uVar22 = (undefined)(int)(fVar24 - 2.147484e+09);
				}
				fVar27 = fVar27 * 128.0;
				*(undefined*)((int)&(this->fxTailA).field_0x24 + 1) = uVar22;
				if (fVar27 < 2.147484e+09) {
					uVar22 = (undefined)(int)fVar27;
				}
				else {
					uVar22 = (undefined)(int)(fVar27 - 2.147484e+09);
				}
				fVar23 = fVar23 * 128.0;
				*(undefined*)((int)&(this->fxTailA).field_0x24 + 2) = uVar22;
				if (fVar23 < 2.147484e+09) {
					*(char*)((int)&(this->fxTailA).field_0x24 + 3) = (char)(int)fVar23;
				}
				else {
					*(char*)((int)&(this->fxTailA).field_0x24 + 3) = (char)(int)(fVar23 - 2.147484e+09);
				}
			}
			else {
				(this->fxTailA).field_0x28 = (this->fxTailA).field_0x24;
				*(undefined*)&(this->fxTailA).field_0x24 = 0x80;
				*(undefined*)((int)&(this->fxTailA).field_0x24 + 1) = 0;
				*(undefined*)((int)&(this->fxTailA).field_0x24 + 2) = 0;
				*(undefined*)((int)&(this->fxTailA).field_0x24 + 3) = 0x80;
			}
			CFxTail::Manage(&this->fxTailA, (undefined4*)&eStack256, (undefined4*)&eStack272, 0);)
		}
	}

	uVar12 = this->field_0x360;
	uVar1 = this->field_0x35c;
	if (this->trailMaterialId != -1) {
		iVar13 = this->actorState;
		if (iVar13 == -1) {
			uVar11 = 0;
		}
		else {
			pSVar16 = GetStateCfg(iVar13);
			uVar11 = pSVar16->flags_0x4;
		}

		if ((uVar11 & 0x1000) == 0) {
			IMPLEMENTATION_GUARD_LOG(
			CFxTail::Manage(&this->fxTailB, (undefined4*)&gF32Vertex4Zero, (undefined4*)&gF32Vertex4Zero, 1);)
		}

		else {
			IMPLEMENTATION_GUARD_LOG(
			CActor::SV_GetBoneWorldPosition((CActor*)this, uVar12, &eStack288);
			CActor::SV_GetBoneWorldPosition((CActor*)this, uVar1, &eStack304);
			fVar23 = this->dynamic.linearAcceleration;
			if (fVar23 < 30.0) {
				fVar23 = fVar23 / 30.0;
				fVar27 = 1.0 - fVar23;
				(this->fxTailB).field_0x28 = (this->fxTailB).field_0x24;
				fVar24 = fVar27 * 128.0;
				*(undefined*)&(this->fxTailB).field_0x24 = 0x80;
				if (fVar24 < 2.147484e+09) {
					uVar22 = (undefined)(int)fVar24;
				}
				else {
					uVar22 = (undefined)(int)(fVar24 - 2.147484e+09);
				}
				fVar27 = fVar27 * 128.0;
				*(undefined*)((int)&(this->fxTailB).field_0x24 + 1) = uVar22;
				if (fVar27 < 2.147484e+09) {
					uVar22 = (undefined)(int)fVar27;
				}
				else {
					uVar22 = (undefined)(int)(fVar27 - 2.147484e+09);
				}
				fVar23 = fVar23 * 128.0;
				*(undefined*)((int)&(this->fxTailB).field_0x24 + 2) = uVar22;
				if (fVar23 < 2.147484e+09) {
					*(char*)((int)&(this->fxTailB).field_0x24 + 3) = (char)(int)fVar23;
				}
				else {
					*(char*)((int)&(this->fxTailB).field_0x24 + 3) = (char)(int)(fVar23 - 2.147484e+09);
				}
			}
			else {
				(this->fxTailB).field_0x28 = (this->fxTailB).field_0x24;
				*(undefined*)&(this->fxTailB).field_0x24 = 0x80;
				*(undefined*)((int)&(this->fxTailB).field_0x24 + 1) = 0;
				*(undefined*)((int)&(this->fxTailB).field_0x24 + 2) = 0;
				*(undefined*)((int)&(this->fxTailB).field_0x24 + 3) = 0x80;
			}
			CFxTail::Manage(&this->fxTailB, (undefined4*)&eStack288, (undefined4*)&eStack304, 0);)
		}
	}

	if (this->pCollisionData != (CCollision*)0x0) {
		iVar13 = this->actorState;
		uVar12 = 0;
		if (iVar13 != -1) {
			pSVar16 = GetStateCfg(iVar13);
			uVar12 = pSVar16->flags_0x4;
		}
		if ((uVar12 & 0x4000) == 0) {
			pCVar2 = this->pCollisionData;
			if (((pCVar2->flags_0x0 & 0x1000) == 0) && ((pCVar2->flags_0x0 & 2) == 0)) {
				pCVar2->flags_0x0 = pCVar2->flags_0x0 | 0x1000;
				pCVar2 = this->pCollisionData;
				pCVar2->flags_0x0 = pCVar2->flags_0x0 | 2;
			}
		}
		else {
			pCVar2 = this->pCollisionData;
			if (((pCVar2->flags_0x0 & 0x1000) != 0) && ((pCVar2->flags_0x0 & 2) != 0)) {
				pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xffffefff;
				pCVar2 = this->pCollisionData;
				pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xfffffffd;
			}
		}
	}

	iVar13 = this->actorState;
	uVar12 = 0;
	if (iVar13 != -1) {
		pSVar16 = GetStateCfg(iVar13);
		uVar12 = pSVar16->flags_0x4;
	}

	if ((uVar12 & 0x10000) != 0) {
		iVar13 = this->prevActorState;
		uVar12 = 0;
		if (iVar13 != -1) {
			pSVar16 = GetStateCfg(iVar13);
			uVar12 = pSVar16->flags_0x4;
		}

		if (((uVar12 & 0x10000) == 0) && (this->pCollisionData != (CCollision*)0x0)) {
			local_140.x = 0.0f;
			local_140.z = 0.0f;
			local_140.y = 0.34f;
			local_150.w = 0.0f;
			local_140.w = 1.0f;
			local_150.x = 0.3f;
			local_150.z = 0.3f;
			local_150.y = 0.45f;
			ChangeCollisionSphere(0.0f, &local_150, &local_140);
		}
	}

	iVar13 = this->actorState;
	uVar12 = 0;
	if (iVar13 != -1) {
		pSVar16 = GetStateCfg(iVar13);
		uVar12 = pSVar16->flags_0x4;
	}

	if ((uVar12 & 0x10000) == 0) {
		iVar13 = this->prevActorState;
		uVar12 = 0;
		if (iVar13 != -1) {
			pSVar16 = GetStateCfg(iVar13);
			uVar12 = pSVar16->flags_0x4;
		}

		if (((uVar12 & 0x10000) != 0) && (this->pCollisionData != (CCollision*)0x0)) {
			this->RestoreCollisionSphere(0.0f);
		}
	}

	IMPLEMENTATION_GUARD_LOG(
	for (uVar21 = 0; (long)uVar21 < 0xb; uVar21 = SEXT48((int)uVar21 + 1)) {
		lVar19 = FUN_003872d0(lVar20, uVar21);
		if (lVar19 != 0) {
			(*(code*)((this->behaviourAddOn).pVTable)->InitState)(&this->behaviourAddOn, uVar21);
		}
	}

	iVar13 = FUN_003e35e0((int)&this->behaviourAddOn);
	if (iVar13 == 0) {
		uVar14 = FUN_00387070((int)this);
		behaviourAddOn.SectorChange(uVar14);
	}

	behaviourAddOn.Term();)

	pCVar8 = CActorHero::_gThis;
	iVar13 = this->actorState;
	switch (iVar13) {
	case ATON_ESCAPE_STATE_PATH_BEGIN:
		bVar10 = ComputeSpeedAndAccelerationAndAnalyseForRun();
		pCVar18 = this->pathPlaneArray.GetCurPathPlane();
		peVar15 = pCVar18->pathFollowReader.GetWayPoint();
		fStack768 = peVar15->x - this->currentLocation.x;
		fStack760 = peVar15->z - this->currentLocation.z;
		fStack756 = peVar15->w - this->currentLocation.w;
		uStack764 = 0;
		this->field_0x3b0 = sqrtf(fStack768 * fStack768 + 0.0f + fStack760 * fStack760);
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
		pCVar9 = CActorHero::_gThis;
		if (bVar10 == false) {
			SetState(0xd, -1);
		}
		else {
			peVar15 = &this->currentLocation;
			fStack1136 = pCVar8->currentLocation.x - peVar15->x;
			fStack1132 = pCVar8->currentLocation.y - this->currentLocation.y;
			fStack1128 = pCVar8->currentLocation.z - this->currentLocation.z;
			fStack1124 = pCVar8->currentLocation.w - this->currentLocation.w;
			if (1.0f < this->timeInAir) {
				edF32Vector4SubHard(&eStack784, peVar15, &CActorHero::_gThis->currentLocation);
				fVar23 = edF32Vector4DotProductHard(&eStack784, &this->rotationQuat);
				if (fVar23 < 0.0f) {
					fVar23 = pCVar9->currentLocation.x - this->currentLocation.x;
					fVar24 = pCVar9->currentLocation.y - this->currentLocation.y;
					fVar27 = pCVar9->currentLocation.z - this->currentLocation.z;
					if (5.0f < sqrtf(fVar23 * fVar23 + fVar24 * fVar24 + fVar27 * fVar27)) {
						SetState(ATON_ESCAPE_STATE_PATH_STAND_CALL_FAR, -1);
					}
					else {
						SetState(ATON_ESCAPE_STATE_PATH_STAND_CALL_NEAR, -1);
					}
				}
				else {
					fVar23 = pCVar9->currentLocation.x - this->currentLocation.x;
					fVar24 = pCVar9->currentLocation.y - this->currentLocation.y;
					fVar27 = pCVar9->currentLocation.z - this->currentLocation.z;
					if (5.0f < sqrtf(fVar23 * fVar23 + fVar24 * fVar24 + fVar27 * fVar27)) {
						SetState(8, -1);
					}
				}
			}
		}
		break;
	case 8:
		if (iVar13 == -1) {
			uVar12 = 0;
		}
		else {
			pSVar16 = GetStateCfg(iVar13);
			uVar12 = pSVar16->flags_0x4;
		}

		if ((uVar12 & 0x2000) == 0) {
			edF32Vector4SubHard(&eStack800, &pCVar8->currentLocation, &this->currentLocation);
		}
		else {
			edF32Vector4SubHard(&eStack800, &this->currentLocation, &pCVar8->currentLocation);
		}

		eStack800.y = 0.0f;
		edF32Vector4NormalizeHard(&eStack800, &eStack800);
		SV_UpdateOrientation2D(6.283185f, &eStack800, 0);
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
		bVar10 = ComputeSpeedAndAccelerationAndAnalyseForRun();
		if (bVar10 == false) {
			SetState(0xf, -1);
		}
		else {
			pCVar3 = this->pAnimationController;
			peVar4 = (pCVar3->anmBinMetaAnimator).aAnimData;

			if ((peVar4->currentAnimDesc).animType == pCVar3->currentAnimType_0x30) {
				bVar10 = false;
				if (peVar4->animPlayState != 0) {
					bVar10 = (peVar4->field_0xcc & 2) != 0;
				}
			}
			else {
				bVar10 = false;
			}

			if (bVar10) {
				SetState(9, -1);
			}
		}
		break;
	case 9:
		bVar10 = ComputeSpeedAndAccelerationAndAnalyseForRun();
		pCVar18 = this->pathPlaneArray.GetCurPathPlane();
		peVar15 = pCVar18->pathFollowReader.GetWayPoint();
		fStack816 = peVar15->x - this->currentLocation.x;
		fStack808 = peVar15->z - this->currentLocation.z;
		fStack804 = peVar15->w - this->currentLocation.w;
		uStack812 = 0;
		this->field_0x3b0 = sqrtf(fStack816 * fStack816 + 0.0f + fStack808 * fStack808);

		pCVar9 = CActorHero::_gThis;
		iVar13 = this->actorState;
		if (iVar13 == -1) {
			uVar12 = 0;
		}
		else {
			pSVar16 = GetStateCfg(iVar13);
			uVar12 = pSVar16->flags_0x4;
		}

		if ((uVar12 & 0x2000) == 0) {
			edF32Vector4SubHard(&eStack832, &pCVar9->currentLocation, &this->currentLocation);
		}
		else {
			edF32Vector4SubHard(&eStack832, &this->currentLocation, &pCVar9->currentLocation);
		}

		eStack832.y = 0.0f;
		edF32Vector4NormalizeHard(&eStack832, &eStack832);
		CActor::SV_UpdateOrientation2D(6.283185f, &eStack832, 0);
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		if (bVar10 == false) {
			SetState(0xf, -1);
		}
		else {
			fStack1152 = pCVar8->currentLocation.x - this->currentLocation.x;
			fStack1148 = pCVar8->currentLocation.y - this->currentLocation.y;
			fStack1144 = pCVar8->currentLocation.z - this->currentLocation.z;
			fStack1140 = pCVar8->currentLocation.w - this->currentLocation.w;

			if ((2.5f < this->timeInAir) && (3.0f < sqrtf(fStack1152 * fStack1152 + fStack1148 * fStack1148 + fStack1144 * fStack1144))) {
				SetState(ATON_ESCAPE_STATE_PATH_STAND_CALL_BACK, -1);
			}
		}
		break;
	case ATON_ESCAPE_STATE_PATH_STAND_CALL_NEAR:
		StateAtonPathStandCallNear();
		break;
	case ATON_ESCAPE_STATE_PATH_STAND_CALL_FAR:
		StateAtonPathStandCallFar();
		break;
	case ATON_ESCAPE_STATE_PATH_STAND_CALL_BACK:
		StateAtonPathStandCallBack();
		break;
	case 0xd:
		bVar10 = ComputeSpeedAndAccelerationAndAnalyseForRun();
		SStack368.field_0x10 = 0.0f;
		SStack368.field_0x8 = 3;
		SStack368.field_0xc = 1;
		SStack368.acceleration = (this->fleeOnPathParams).acceleration;
		SStack368.speed = (this->fleeOnPathParams).speed;
		SStack368.rotationSpeed = 3.141593f;
		pCVar18 = this->pathPlaneArray.GetCurPathPlane();
		fVar23 = SV_MOV_ManageMovOnPath(&pCVar18->pathFollowReader, &SStack368);
		this->field_0x3b0 = fVar23;

		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		if (bVar10 != false) {
			SetState(ATON_ESCAPE_STATE_PATH_BEGIN, -1);
		}

		pCVar8 = CActorHero::_gThis;
		if (this->field_0x3d0 < this->timeInAir) {
			peVar15 = &this->currentLocation;
			pCVar18 = this->pathPlaneArray.GetCurPathPlane();
			pCVar18->InitTargetPos(peVar15, &CStack64);
			pCVar18 = this->pathPlaneArray.GetCurPathPlane();
			pCVar18->ExternComputeTargetPosWithPlane(peVar15, &CStack64);
			peVar15 = &this->currentLocation;
			pCVar18 = this->pathPlaneArray.GetCurPathPlane();
			FUN_00115380(peVar15, &pCVar18->pathFollowReader, CStack64.field_0x0, 0, &eStack864, &fStack8);
			pCVar18 = this->pathPlaneArray.GetCurPathPlane();
			pCVar18->InitTargetPos(peVar15, &CStack80);
			pCVar18 = this->pathPlaneArray.GetCurPathPlane();
			pCVar18->ExternComputeTargetPosWithPlane(peVar15, &CStack80);
			pCVar18 = this->pathPlaneArray.GetCurPathPlane();
			FUN_00115380(&pCVar8->currentLocation, &pCVar18->pathFollowReader, CStack80.field_0x0, 0, &eStack848, &fStack4);

			bVar10 = false;
			if ((CStack64.field_0x0 < CStack80.field_0x0) ||
				((CStack64.field_0x0 == CStack80.field_0x0 && (fStack8 < fStack4)))) {
				bVar10 = true;
			}

			if (!bVar10) {
				edF32Vector4SubHard(&eStack880, &this->currentLocation, &CActorHero::_gThis->currentLocation);
				if (0.0f < this->rotationQuat.x * eStack880.z - eStack880.x * this->rotationQuat.z) {
					SetState(0xe, -1);
				}
				else {
					SetState(0x10, -1);
				}
			}
		}
		break;
	case 0xe:
		bVar10 = ComputeSpeedAndAccelerationAndAnalyseForRun();
		SStack400.field_0x10 = 0;
		SStack400.field_0x8 = 3;
		SStack400.field_0xc = 1;
		SStack400.acceleration = (this->fleeOnPathParams).acceleration;
		SStack400.speed = (this->fleeOnPathParams).speed;
		SStack400.rotationSpeed = 3.141593f;
		pCVar18 = this->pathPlaneArray.GetCurPathPlane();
		fVar23 = SV_MOV_ManageMovOnPath(&pCVar18->pathFollowReader, &SStack400);
		this->field_0x3b0 = fVar23;

		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		if ((bVar10 == false) || (this->field_0x3d4 <= 0.8f)) {
			pCVar3 = this->pAnimationController;
			peVar4 = (pCVar3->anmBinMetaAnimator).aAnimData;
			if ((peVar4->currentAnimDesc).animType == pCVar3->currentAnimType_0x30) {
				bVar10 = false;
				if (peVar4->animPlayState != 0) {
					bVar10 = (peVar4->field_0xcc & 2) != 0;
				}
			}
			else {
				bVar10 = false;
			}

			if (bVar10) {
				this->field_0x480 = this->field_0x480 + 1;
			}

			if (this->field_0x480 == 3) {
				SetState(0xf, -1);
			}
		}
		else {
			SetState(9, -1);
		}
		break;
	case 0xf:
		bVar10 = ComputeSpeedAndAccelerationAndAnalyseForRun();
		SStack432.field_0x10 = 0;
		SStack432.field_0x8 = 3;
		SStack432.field_0xc = 1;
		SStack432.acceleration = (this->fleeOnPathParams).acceleration;
		SStack432.speed = (this->fleeOnPathParams).speed;
		SStack432.rotationSpeed = 3.141593f;
		pCVar18 = this->pathPlaneArray.GetCurPathPlane();
		fVar23 = SV_MOV_ManageMovOnPath(&pCVar18->pathFollowReader, &SStack432);
		this->field_0x3b0 = fVar23;

		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		if (bVar10 == false) {
			if (2.0f < this->timeInAir) {
				SetState(0xd, -1);
			}
		}
		else {
			if (0.8f < this->field_0x3d4) {
				SetState(9, -1);
			}
			else {
				SetState(ATON_ESCAPE_STATE_PATH_BEGIN, -1);
			}
		}
		break;
	case 0x10:
		bVar10 = ComputeSpeedAndAccelerationAndAnalyseForRun();
		SStack464.field_0x10 = 0;
		SStack464.field_0x8 = 3;
		SStack464.field_0xc = 1;
		SStack464.acceleration = (this->fleeOnPathParams).acceleration;
		SStack464.speed = (this->fleeOnPathParams).speed;
		SStack464.rotationSpeed = 3.141593f;
		pCVar18 = this->pathPlaneArray.GetCurPathPlane();
		fVar23 = SV_MOV_ManageMovOnPath(&pCVar18->pathFollowReader, &SStack464);
		this->field_0x3b0 = fVar23;

		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		if (bVar10 != false) {
			SetState(ATON_ESCAPE_STATE_PATH_BEGIN, -1);
		}

		pCVar3 = this->pAnimationController;
		peVar4 = (pCVar3->anmBinMetaAnimator).aAnimData;
		if ((peVar4->currentAnimDesc).animType == pCVar3->currentAnimType_0x30) {
			bVar10 = false;
			if (peVar4->animPlayState != 0) {
				bVar10 = (peVar4->field_0xcc & 2) != 0;
			}
		}
		else {
			bVar10 = false;
		}

		if (bVar10) {
			this->field_0x480 = this->field_0x480 + 1;
		}

		if (this->field_0x480 == 3) {
			SetState(0x11, -1);
		}
		break;
	case 0x11:
		bVar10 = ComputeSpeedAndAccelerationAndAnalyseForRun();
		SStack496.field_0x10 = 0;
		SStack496.field_0x8 = 3;
		SStack496.field_0xc = 1;
		SStack496.acceleration = (this->fleeOnPathParams).acceleration;
		SStack496.speed = (this->fleeOnPathParams).speed;
		SStack496.rotationSpeed = 3.141593f;
		pCVar18 = this->pathPlaneArray.GetCurPathPlane();
		fVar23 = SV_MOV_ManageMovOnPath(&pCVar18->pathFollowReader, &SStack496);
		this->field_0x3b0 = fVar23;

		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		if (bVar10 == false) {
			if (2.0f < this->timeInAir) {
				SetState(0xd, -1);
			}
		}
		else {
			SetState(ATON_ESCAPE_STATE_PATH_BEGIN, -1);
		}
		break;
	case ATON_ESCAPE_STATE_JUMP:
		(this->fleeOnPathParams).acceleration = 6.0f;
		(this->fleeOnPathParams).speed = 5.0f;
		SStack528.field_0x10 = 0;
		SStack528.field_0x8 = 3;
		SStack528.field_0xc = 1;
		SStack528.acceleration = (this->fleeOnPathParams).acceleration;
		SStack528.speed = (this->fleeOnPathParams).speed;
		SStack528.rotationSpeed = 3.141593f;
		pCVar18 = this->pathPlaneArray.GetCurPathPlane();
		fVar23 = SV_MOV_ManageMovOnPath(&pCVar18->pathFollowReader, &SStack528);
		this->field_0x3b0 = fVar23;
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
		pTVar17 = GetTimer();
		fVar23 = 0.5f;
		fVar24 = this->dynamic.linearAcceleration * pTVar17->cutsceneDeltaTime;
		if (0.5f <= fVar24) {
			fVar23 = fVar24;
		}

		if (this->field_0x3b0 < fVar23) {
			iVar13 = this->pathPlaneArray.AtGoal();
			if (iVar13 == 0) {
				this->pathPlaneArray.NextWayPoint();
				ChooseJumpState();
			}
			else {
				SetState(0x32, -1);
				this->field_0x474 = 1;
			}
		}
		break;
	case ATON_ESCAPE_STATE_PATH_BEFORE_WAIT_JUMP:
		StateAtonPathBeforeWaitJump();
		break;
	case 0x17:
		if (iVar13 == -1) {
			uVar12 = 0;
		}
		else {
			pSVar16 = GetStateCfg(iVar13);
			uVar12 = pSVar16->flags_0x4;
		}

		if ((uVar12 & 0x2000) == 0) {
			edF32Vector4SubHard(&eStack896, &pCVar8->currentLocation, &this->currentLocation);
		}
		else {
			edF32Vector4SubHard(&eStack896, &this->currentLocation, &pCVar8->currentLocation);
		}

		eStack896.y = 0.0f;
		edF32Vector4NormalizeHard(&eStack896, &eStack896);
		SV_UpdateOrientation2D(6.283185f, &eStack896, 0);

		ManageDyn(4.0f, 0x2003b, (CActorsTable*)0x0);

		pCVar3 = this->pAnimationController;
		peVar4 = (pCVar3->anmBinMetaAnimator).aAnimData;
		if ((peVar4->currentAnimDesc).animType == pCVar3->currentAnimType_0x30) {
			bVar10 = false;
			if (peVar4->animPlayState != 0) {
				bVar10 = (peVar4->field_0xcc & 2) != 0;
			}
		}
		else {
			bVar10 = false;
		}

		if (bVar10) {
			SetState(0x18, -1);
		}
		break;
	case 0x18:
		if (iVar13 == -1) {
			uVar12 = 0;
		}
		else {
			pSVar16 = GetStateCfg(iVar13);
			uVar12 = pSVar16->flags_0x4;
		}

		if ((uVar12 & 0x2000) == 0) {
			edF32Vector4SubHard(&eStack912, &pCVar8->currentLocation, &this->currentLocation);
		}
		else {
			edF32Vector4SubHard(&eStack912, &this->currentLocation, &pCVar8->currentLocation);
		}

		eStack912.y = 0.0f;

		edF32Vector4NormalizeHard(&eStack912, &eStack912);
		SV_UpdateOrientation2D(6.283185f, &eStack912, 0);

		ManageDyn(4.0, 0x2003b, (CActorsTable*)0x0);

		pCVar9 = CActorHero::_gThis;
		fStack1216 = pCVar8->currentLocation.x - this->currentLocation.x;
		fStack1212 = pCVar8->currentLocation.y - this->currentLocation.y;
		fStack1208 = pCVar8->currentLocation.z - this->currentLocation.z;
		fStack1204 = pCVar8->currentLocation.w - this->currentLocation.w;
		fVar23 = fStack1216 * fStack1216;
		fVar24 = fStack1212 * fStack1212;
		fVar27 = fStack1208 * fStack1208;
		pCVar8 = CActorHero::_gThis;
		uVar12 = ((this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).pPathFollow)->field_0xc;
		if (uVar12 == 3) {
			fVar5 = pCVar9->currentLocation.x - this->currentLocation.x;
			fVar6 = pCVar9->currentLocation.y - this->currentLocation.y;
			fVar7 = pCVar9->currentLocation.z - this->currentLocation.z;
			if ((sqrtf(fVar5 * fVar5 + fVar6 * fVar6 + fVar7 * fVar7) < this->field_0x470) ||
				(this->currentLocation.y < CActorHero::_gThis->currentLocation.y)) {
				iVar13 = (this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).field_0x8;

				peVar15 = this->pathPlaneArray.GetCurPathPlane()->pathFollowReader.GetWayPoint(iVar13);
				if ((fabs(peVar15->y - this->currentLocation.y) < 0.5f) && (bVar10 = true, 0.001f < this->field_0x3e4)) goto LAB_00384948;
			}
		LAB_00384940:
			bVar10 = false;
		}
		else {
			if (uVar12 != 2) goto LAB_00384940;

			peVar15 = &this->currentLocation;
			fVar5 = pCVar9->currentLocation.x - peVar15->x;
			fVar6 = pCVar9->currentLocation.y - this->currentLocation.y;
			fVar7 = pCVar9->currentLocation.z - this->currentLocation.z;

			if (this->field_0x470 <= sqrtf(fVar5 * fVar5 + fVar6 * fVar6 + fVar7 * fVar7)) {

				this->pathPlaneArray.GetCurPathPlane()->InitTargetPos(peVar15, &CStack192);
				this->pathPlaneArray.GetCurPathPlane()->ExternComputeTargetPosWithPlane(peVar15, &CStack192);

				peVar15 = &this->currentLocation;
				FUN_00115380(peVar15, &this->pathPlaneArray.GetCurPathPlane()->pathFollowReader, CStack192.field_0x0, 0, &eStack1248, &fStack40);

				this->pathPlaneArray.GetCurPathPlane()->InitTargetPos(peVar15, &CStack208);
				this->pathPlaneArray.GetCurPathPlane()->ExternComputeTargetPosWithPlane(peVar15, &CStack208);

				FUN_00115380(&pCVar8->currentLocation, &this->pathPlaneArray.GetCurPathPlane()->pathFollowReader, CStack208.field_0x0, 0, &eStack1232, &fStack36);

				bVar10 = false;
				if ((CStack192.field_0x0 < CStack208.field_0x0) ||
					((CStack192.field_0x0 == CStack208.field_0x0 && (fStack40 < fStack36)))) {
					bVar10 = true;
				}

				if (!bVar10) goto LAB_00384940;
			}

			bVar10 = true;
		}
	LAB_00384948:
		if (bVar10) {
			SetState(0x1c, -1);
		}
		else {
			if ((2.5f < this->timeInAir) && (this->field_0x470 * 1.5f < sqrtf(fVar23 + fVar24 + fVar27))) {
				SetState(0x1b, -1);
			}
		}
		break;
	case ATON_ESCAPE_STATE_PATH_JUMP_1_4:
		StateAtonPathJump_1_4();
		break;
	case ATON_ESCAPE_STATE_PATH_JUMP_2_4:
		StateAtonPathJump_2_4();
		break;
	case ATON_ESCAPE_STATE_PATH_JUMP_3_4:
		StateAtonPathJump_3_4();
		break;
	case ATON_ESCAPE_STATE_PATH_JUMP_4_4:
		StateAtonPathJump_4_4();
		break;
	case 0x32:
		if (iVar13 == -1) {
			uVar12 = 0;
		}
		else {
			pSVar16 = GetStateCfg(iVar13);
			uVar12 = pSVar16->flags_0x4;
		}

		if ((uVar12 & 0x2000) == 0) {
			edF32Vector4SubHard(&eStack1120, &pCVar8->currentLocation, &this->currentLocation);
		}
		else {
			edF32Vector4SubHard(&eStack1120, &this->currentLocation, &pCVar8->currentLocation);
		}

		eStack1120.y = 0.0f;

		edF32Vector4NormalizeHard(&eStack1120, &eStack1120);

		SV_UpdateOrientation2D(6.283185f, &eStack1120, 0);
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);
	default:
		IMPLEMENTATION_GUARD();
	}

  	iVar13 = this->actorState;
	uVar12 = 0;

	if (iVar13 != -1) {
		pSVar16 = GetStateCfg(iVar13);
		uVar12 = pSVar16->flags_0x4;
	}

	if ((uVar12 & 0x800) != 0) {
		pTVar17 = GetTimer();
		fVar23 = 0.5f;
		fVar24 = this->dynamic.linearAcceleration * pTVar17->cutsceneDeltaTime;
		if (0.5f <= fVar24) {
			fVar23 = fVar24;
		}

		if (this->field_0x3b0 < fVar23) {
			bVar10 = this->pathPlaneArray.GetCurPathPlane()->pathFollowReader.AtGoal((this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).splinePointIndex,
				(this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).field_0xc);

			if (bVar10 == false) {
				this->pathPlaneArray.NextWayPoint();
			}
			else {
				iVar13 = this->pathPlaneArray.AtGoal();
				if (iVar13 == 0) {
					if (((this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).pPathFollow)->field_0xc != 0xd) {
						this->pathPlaneArray.NextWayPoint();
						AnalysePathType();
					}
				}
				else {
					if (this->field_0x474 == 0) {
						SetState(0x32, -1);
						this->field_0x474 = 1;
					}
				}
			}
		}
	}

	return;
}

void CActorAton::StateAtonPathStandCallNear()
{
	int iVar1;
	CAnimation* pAnim;
	edAnmLayer* peVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	CActorHero* pActorHero;
	bool bVar9;
	CPathPlane* pPathPlane;
	edF32VECTOR4* peVar10;
	StateConfig* pSVar11;
	uint uVar12;
	edF32VECTOR4 eStack32;
	float local_10;
	undefined4 local_c;
	float fStack8;
	float fStack4;

	pActorHero = CActorHero::_gThis;
	bVar9 = ComputeSpeedAndAccelerationAndAnalyseForRun();
	pPathPlane = this->pathPlaneArray.GetCurPathPlane();
	peVar10 = pPathPlane->pathFollowReader.GetWayPoint();
	local_10 = peVar10->x - this->currentLocation.x;
	fStack8 = peVar10->z - this->currentLocation.z;
	fStack4 = peVar10->w - this->currentLocation.w;
	local_c = 0;
	this->field_0x3b0 = sqrtf(local_10 * local_10 + 0.0f + fStack8 * fStack8);

	pActorHero = CActorHero::_gThis;

	iVar1 = this->actorState;
	if (iVar1 == -1) {
		uVar12 = 0;
	}
	else {
		pSVar11 = GetStateCfg(iVar1);
		uVar12 = pSVar11->flags_0x4;
	}

	if ((uVar12 & 0x2000) == 0) {
		edF32Vector4SubHard(&eStack32, &pActorHero->currentLocation, &this->currentLocation);
	}
	else {
		edF32Vector4SubHard(&eStack32, &this->currentLocation, &pActorHero->currentLocation);
	}

	eStack32.y = 0.0f;

	edF32Vector4NormalizeHard(&eStack32, &eStack32);
	SV_UpdateOrientation2D(6.283185f, &eStack32, 0);

	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

	if (bVar9 == false) {
		SetState(0xd, -1);
	}
	else {
		fVar4 = pActorHero->currentLocation.x - this->currentLocation.x;
		fVar5 = pActorHero->currentLocation.y - this->currentLocation.y;
		fVar6 = pActorHero->currentLocation.z - this->currentLocation.z;

		float fVar12 = sqrtf(fVar4 * fVar4 + fVar5 * fVar5 + fVar6 * fVar6);

		pAnim = this->pAnimationController;
		peVar3 = (pAnim->anmBinMetaAnimator).aAnimData;
		if ((peVar3->currentAnimDesc).animType == pAnim->currentAnimType_0x30) {
			bVar9 = false;
			if (peVar3->animPlayState != 0) {
				bVar9 = (peVar3->field_0xcc & 2) != 0;
			}
		}
		else {
			bVar9 = false;
		}

		if (bVar9) {
			if (5.0f < fVar12) {
				SetState(ATON_ESCAPE_STATE_PATH_STAND_CALL_FAR, -1);
			}

			this->field_0x480 = this->field_0x480 + 1;
		}

		if ((this->field_0x480 == 2) || fVar12 < 2.0f) {
			SetState(ATON_ESCAPE_STATE_PATH_BEGIN, -1);
		}
	}

	return;
}

void CActorAton::StateAtonPathStandCallFar()
{
	int iVar1;
	CAnimation* pAnim;
	edAnmLayer* peVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	CActorHero* pActorHero;
	bool bVar9;
	CPathPlane* pPathPlane;
	edF32VECTOR4* peVar10;
	StateConfig* pSVar11;
	uint uVar12;
	edF32VECTOR4 eStack32;
	float local_10;
	undefined4 local_c;
	float fStack8;
	float fStack4;

	pActorHero = CActorHero::_gThis;
	bVar9 = ComputeSpeedAndAccelerationAndAnalyseForRun();
	pPathPlane = this->pathPlaneArray.GetCurPathPlane();
	peVar10 = pPathPlane->pathFollowReader.GetWayPoint();
	local_10 = peVar10->x - this->currentLocation.x;
	fStack8 = peVar10->z - this->currentLocation.z;
	fStack4 = peVar10->w - this->currentLocation.w;
	local_c = 0;
	this->field_0x3b0 = sqrtf(local_10 * local_10 + 0.0f + fStack8 * fStack8);

	pActorHero = CActorHero::_gThis;

	iVar1 = this->actorState;
	if (iVar1 == -1) {
		uVar12 = 0;
	}
	else {
		pSVar11 = GetStateCfg(iVar1);
		uVar12 = pSVar11->flags_0x4;
	}

	if ((uVar12 & 0x2000) == 0) {
		edF32Vector4SubHard(&eStack32, &pActorHero->currentLocation, &this->currentLocation);
	}
	else {
		edF32Vector4SubHard(&eStack32, &this->currentLocation, &pActorHero->currentLocation);
	}

	eStack32.y = 0.0f;

	edF32Vector4NormalizeHard(&eStack32, &eStack32);
	SV_UpdateOrientation2D(6.283185f, &eStack32, 0);

	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

	if (bVar9 == false) {
		SetState(0xd, -1);
	}
	else {
		fVar4 = pActorHero->currentLocation.x - this->currentLocation.x;
		fVar5 = pActorHero->currentLocation.y - this->currentLocation.y;
		fVar6 = pActorHero->currentLocation.z - this->currentLocation.z;

		float fVar12 = sqrtf(fVar4 * fVar4 + fVar5 * fVar5 + fVar6 * fVar6);

		pAnim = this->pAnimationController;
		peVar3 = (pAnim->anmBinMetaAnimator).aAnimData;
		if ((peVar3->currentAnimDesc).animType == pAnim->currentAnimType_0x30) {
			bVar9 = false;
			if (peVar3->animPlayState != 0) {
				bVar9 = (peVar3->field_0xcc & 2) != 0;
			}
		}
		else {
			bVar9 = false;
		}

		if (bVar9) {
			if (5.0f < fVar12) {
				SetState(ATON_ESCAPE_STATE_PATH_STAND_CALL_NEAR, -1);
			}

			this->field_0x480 = this->field_0x480 + 1;
		}

		if ((this->field_0x480 == 2) || fVar12 < 2.0f) {
			SetState(ATON_ESCAPE_STATE_PATH_BEGIN, -1);
		}
	}

	return;
}

void CActorAton::StateAtonPathStandCallBack()
{
	int iVar1;
	CAnimation* pAnim;
	edAnmLayer* peVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	CActorHero* pActorHero;
	bool bVar9;
	CPathPlane* pPathPlane;
	edF32VECTOR4* peVar10;
	StateConfig* pSVar11;
	uint uVar12;
	edF32VECTOR4 eStack32;
	float local_10;
	undefined4 local_c;
	float fStack8;
	float fStack4;

	pActorHero = CActorHero::_gThis;
	bVar9 = ComputeSpeedAndAccelerationAndAnalyseForRun();
	pPathPlane = this->pathPlaneArray.GetCurPathPlane();
	peVar10 = pPathPlane->pathFollowReader.GetWayPoint();
	local_10 = peVar10->x - this->currentLocation.x;
	fStack8 = peVar10->z - this->currentLocation.z;
	fStack4 = peVar10->w - this->currentLocation.w;
	local_c = 0;
	this->field_0x3b0 = sqrtf(local_10 * local_10 + 0.0f + fStack8 * fStack8);

	pActorHero = CActorHero::_gThis;

	iVar1 = this->actorState;
	if (iVar1 == -1) {
		uVar12 = 0;
	}
	else {
		pSVar11 = GetStateCfg(iVar1);
		uVar12 = pSVar11->flags_0x4;
	}

	if ((uVar12 & 0x2000) == 0) {
		edF32Vector4SubHard(&eStack32, &pActorHero->currentLocation, &this->currentLocation);
	}
	else {
		edF32Vector4SubHard(&eStack32, &this->currentLocation, &pActorHero->currentLocation);
	}

	eStack32.y = 0.0f;

	edF32Vector4NormalizeHard(&eStack32, &eStack32);
	SV_UpdateOrientation2D(6.283185f, &eStack32, 0);

	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

	if (bVar9 == false) {
		SetState(0xf, -1);
	}
	else {
		fVar4 = pActorHero->currentLocation.x - this->currentLocation.x;
		fVar5 = pActorHero->currentLocation.y - this->currentLocation.y;
		fVar6 = pActorHero->currentLocation.z - this->currentLocation.z;

		pAnim = this->pAnimationController;
		peVar3 = (pAnim->anmBinMetaAnimator).aAnimData;
		if ((peVar3->currentAnimDesc).animType == pAnim->currentAnimType_0x30) {
			bVar9 = false;
			if (peVar3->animPlayState != 0) {
				bVar9 = (peVar3->field_0xcc & 2) != 0;
			}
		}
		else {
			bVar9 = false;
		}

		if (bVar9) {
			this->field_0x480 = this->field_0x480 + 1;
		}

		if ((this->field_0x480 == 2) || (sqrtf(fVar4 * fVar4 + fVar5 * fVar5 + fVar6 * fVar6) < 2.0f)) {
			SetState(9, -1);
		}
	}

	return;
}

void CActorAton::StateAtonPathBeforeWaitJump()
{
	uint uVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	bool bVar8;
	CActorHero* pCVar9;
	CActorHero* pCVar10;
	bool bVar11;
	int iVar13;
	edF32VECTOR4* peVar14;
	edF32VECTOR4* v2;
	edF32VECTOR4 eStack112;
	edF32VECTOR4 eStack96;
	float local_50;
	float fStack76;
	float fStack72;
	float fStack68;
	edF32VECTOR4 eStack64;
	CPathPlaneOutData local_28;
	CPathPlaneOutData local_18;
	float local_8;
	float local_4;

	v2 = &this->currentLocation;
	
	iVar13 = this->pathPlaneArray.GetCurPathPlane()->pathFollowReader.GetNextPlace((this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).splinePointIndex,
		(this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).field_0xc);
	
	peVar14 = this->pathPlaneArray.GetCurPathPlane()->pathFollowReader.GetWayPoint(iVar13);
	edF32Vector4SubHard(&eStack64, peVar14, v2);

	eStack64.y = 0.0f;

	edF32Vector4NormalizeHard(&eStack64, &eStack64);

	bVar11 = SV_UpdateOrientation(2.70177f, &eStack64);

	ManageDyn(4.0f, 0x2003b, (CActorsTable*)0x0);

	pCVar9 = CActorHero::_gThis;
	local_50 = CActorHero::_gThis->currentLocation.x - v2->x;
	fStack76 = CActorHero::_gThis->currentLocation.y - this->currentLocation.y;
	fStack72 = CActorHero::_gThis->currentLocation.z - this->currentLocation.z;
	fStack68 = CActorHero::_gThis->currentLocation.w - this->currentLocation.w;
	fVar2 = local_50 * local_50;
	fVar4 = fStack76 * fStack76;
	fVar6 = fStack72 * fStack72;
	
	pCVar10 = CActorHero::_gThis;
	uVar1 = ((this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).pPathFollow)->field_0xc;
	if (uVar1 == 3) {
		fVar3 = pCVar9->currentLocation.x - this->currentLocation.x;
		fVar5 = pCVar9->currentLocation.y - this->currentLocation.y;
		fVar7 = pCVar9->currentLocation.z - this->currentLocation.z;
		if ((sqrtf(fVar3 * fVar3 + fVar5 * fVar5 + fVar7 * fVar7) < this->field_0x470) || (this->currentLocation.y < CActorHero::_gThis->currentLocation.y)) {
			
			iVar13 = (this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).field_0x8;
			
			peVar14 = this->pathPlaneArray.GetCurPathPlane()->pathFollowReader.GetWayPoint(iVar13);
			if ((fabs(peVar14->y - this->currentLocation.y) < 0.5f) && (bVar8 = true, 0.001f < this->field_0x3e4)) goto LAB_003820f0;
		}
	}
	else {
		if (uVar1 == 2) {
			peVar14 = &this->currentLocation;
			fVar3 = pCVar9->currentLocation.x - peVar14->x;
			fVar5 = pCVar9->currentLocation.y - this->currentLocation.y;
			fVar7 = pCVar9->currentLocation.z - this->currentLocation.z;
			if (this->field_0x470 <= sqrtf(fVar3 * fVar3 + fVar5 * fVar5 + fVar7 * fVar7)) {
				
				this->pathPlaneArray.GetCurPathPlane()->InitTargetPos(peVar14, &local_18);				
				this->pathPlaneArray.GetCurPathPlane()->ExternComputeTargetPosWithPlane(peVar14, &local_18);
				
				FUN_00115380(peVar14, &this->pathPlaneArray.GetCurPathPlane()->pathFollowReader, local_18.field_0x0, 0, &eStack112, &local_8);
				
				this->pathPlaneArray.GetCurPathPlane()->InitTargetPos(peVar14, &local_28);				
				this->pathPlaneArray.GetCurPathPlane()->ExternComputeTargetPosWithPlane(peVar14, &local_28);
				
				FUN_00115380(&pCVar10->currentLocation, &this->pathPlaneArray.GetCurPathPlane()->pathFollowReader, local_28.field_0x0, 0, &eStack96, &local_4);

				bVar8 = false;
				if ((local_18.field_0x0 < local_28.field_0x0) ||
					((local_18.field_0x0 == local_28.field_0x0 && (local_8 < local_4)))) {
					bVar8 = true;
				}

				if (!bVar8) goto LAB_003820e8;
			}
			bVar8 = true;
			goto LAB_003820f0;
		}
	}
LAB_003820e8:
	bVar8 = false;
LAB_003820f0:
	if (bVar8) {
		SetState(0x1c, -1);
	}
	else {
		if (bVar11 == true) {
			if ((sqrtf(fVar2 + fVar4 + fVar6) <= this->field_0x470 * 1.5f) || (this->timeInAir <= 1.0f)) {
				if (this->currentAnimType != 0) {
					PlayAnim(0x0);
				}
			}
			else {
				ChooseWaitJumpCallState();
			}
		}
	}

	return;
}

void CActorAton::StateAtonPathJump_1_4()
{
	edF32VECTOR4 eStack560;
	float puVar26;
	float fVar27;
	bool bVar10;

	int iVar13 = this->pathPlaneArray.GetCurPathPlane()->pathFollowReader.GetNextPlace((this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).splinePointIndex,
		(this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).field_0xc);
	edF32VECTOR4* peVar15 = this->pathPlaneArray.GetCurPathPlane()->pathFollowReader.GetWayPoint(iVar13);
	edF32Vector4SubHard(&eStack560, peVar15, &this->currentLocation);
	eStack560.y = 0.0f;
	edF32Vector4NormalizeHard(&eStack560, &eStack560);
	float puVar25 = edF32Vector4DotProductHard(&this->rotationQuat, &eStack560);
	if (1.0f < puVar25) {
		puVar26 = 1.0f;
	}
	else {
		puVar26 = -1.0f;
		if (-1.0f <= puVar25) {
			puVar26 = puVar25;
		}
	}

	float fVar23 = acosf(puVar26);
	if (fVar23 < 0.1f) {
		fVar23 = 0.1f;
	}

	CAnimation* pCVar3 = this->pAnimationController;
	float fVar24 = this->timeInAir;
	iVar13 = GetIdMacroAnim(0x14);
	if (iVar13 < 0) {
		fVar27 = 0.0f;
	}
	else {
		fVar27 = pCVar3->GetAnimLength(iVar13, 1);
	}

	fVar27 = fVar27 - fVar24;
	if (0.0f < fVar27) {
		SV_UpdateOrientation(fVar23 / fVar27, &eStack560);
	}

	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

	pCVar3 = this->pAnimationController;
	edAnmLayer* peVar4 = (pCVar3->anmBinMetaAnimator).aAnimData;
	if ((peVar4->currentAnimDesc).animType == pCVar3->currentAnimType_0x30) {
		bVar10 = false;
		if (peVar4->animPlayState != 0) {
			bVar10 = (peVar4->field_0xcc & 2) != 0;
		}
	}
	else {
		bVar10 = false;
	}

	if (bVar10) {
		SetState(ATON_ESCAPE_STATE_PATH_JUMP_2_4, -1);
	}

	return;
}

void CActorAton::StateAtonPathJump_2_4()
{
	bool bVar1;
	CPathPlane* pCVar2;
	int index;
	edF32VECTOR4* v1;
	Timer* pTVar3;
	float fVar4;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	index = this->pathPlaneArray.GetCurPathPlane()->pathFollowReader.GetNextPlace((this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).splinePointIndex,
		(this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).field_0xc);

	v1 = this->pathPlaneArray.GetCurPathPlane()->pathFollowReader.GetWayPoint(index);

	edF32Vector4SubHard(&eStack16, v1, &this->currentLocation);
	eStack16.y = 0.0f;
	edF32Vector4NormalizeHard(&eStack16, &eStack16);
	SV_UpdateOrientation(3.141593f, &eStack16);
	pTVar3 = GetTimer();
	this->vectorDyn.Integrate(pTVar3->cutsceneDeltaTime);
	fVar4 = edF32Vector4NormalizeHard(&local_20, &(this->vectorDyn).field_0x60);
	this->dynamic.rotationQuat = local_20;
	this->dynamic.speed = fVar4;

	if (((this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).pPathFollow)->field_0xc == 2) {
		ManageDyn(4.0f, 9, (CActorsTable*)0x0);
	}

	if (((this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).pPathFollow)->field_0xc == 3) {
		ManageDyn(4.0f, 0x409, (CActorsTable*)0x0);
	}

	if ((this->dynamic.velocityDirectionEuler.y < 0.0f) || (bVar1 = this->vectorDyn.IsFinished(), bVar1 != false)) {
		SetState(ATON_ESCAPE_STATE_PATH_JUMP_3_4, -1);
		this->pathPlaneArray.NextWayPoint();
	}

	return;
}

void CActorAton::StateAtonPathJump_3_4()
{
	uint uVar1;
	CCollision* pCVar2;
	bool bVar3;
	Timer* pTVar4;
	int iVar6;
	float fVar7;
	edF32VECTOR4 local_10;

	bVar3 = this->vectorDyn.IsFinished();
	if (bVar3 == false) {
		pTVar4 = GetTimer();
		this->vectorDyn.Integrate(pTVar4->cutsceneDeltaTime);
		fVar7 = edF32Vector4NormalizeHard(&local_10, &(this->vectorDyn).field_0x60);
		this->dynamic.rotationQuat = local_10;
		this->dynamic.speed = fVar7;
	}

	uVar1 = ((this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).pPathFollow)->field_0xc;
	if (uVar1 == 3) {
		bVar3 = this->vectorDyn.IsFinished();
		if (bVar3 == false) {
			ManageDyn(4.0f, 9, (CActorsTable*)0x0);
		}
		else {
			ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
		}

		pCVar2 = this->pCollisionData;
		if ((pCVar2 != (CCollision*)0x0) && ((pCVar2->flags_0x4 & 2) != 0)) {
			this->dynamic.speed = 0.0f;
			SetState(ATON_ESCAPE_STATE_PATH_JUMP_4_4, -1);
		}
	}
	else {
		if (uVar1 == 2) {
			ManageDyn(4.0f, 9, (CActorsTable*)0x0);
			pCVar2 = this->pCollisionData;

			if ((pCVar2 == (CCollision*)0x0) || ((pCVar2->flags_0x4 & 2) == 0)) {
				bVar3 = this->vectorDyn.IsFinished();
				if (bVar3 != false) {
					bVar3 = this->pathPlaneArray.GetCurPathPlane()->pathFollowReader.AtGoal((this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).splinePointIndex,
						(this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).field_0xc);

					if (bVar3 != false) {
						iVar6 = this->pathPlaneArray.AtGoal();
						if (iVar6 == 0) {
							this->pathPlaneArray.NextWayPoint();
							AnalysePathType();
						}
						else {
							SetState(0x32, -1);
							this->field_0x474 = 1;
						}
					}
				}
			}
			else {
				SetState(ATON_ESCAPE_STATE_PATH_JUMP_4_4, -1);
			}
		}
	}

	return;
}

void CActorAton::StateAtonPathJump_4_4()
{
	bool bVar10;

	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
	CAnimation* pCVar3 = this->pAnimationController;
	edAnmLayer* peVar4 = (pCVar3->anmBinMetaAnimator).aAnimData;
	if ((peVar4->currentAnimDesc).animType == pCVar3->currentAnimType_0x30) {
		bVar10 = false;
		if (peVar4->animPlayState != 0) {
			bVar10 = (peVar4->field_0xcc & 2) != 0;
		}
	}
	else {
		bVar10 = false;
	}

	if (bVar10) {
		bVar10 = this->pathPlaneArray.GetCurPathPlane()->pathFollowReader.AtGoal((this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).splinePointIndex,
			(this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).field_0xc);
		if (bVar10 != false) {
			int iVar13 = this->pathPlaneArray.AtGoal();
			if (iVar13 == 0) {
				this->pathPlaneArray.NextWayPoint();
				AnalysePathType();
			}
			else {
				SetState(0x32, -1);
				this->field_0x474 = 1;
			}
		}
	}

	return;
}

void CActorAton::ChooseJumpState()
{
	float* pfVar1;
	uint uVar2;
	float fVar3;
	float fVar4;
	bool bVar5;
	CActorHero* pCVar6;
	CActorHero* pCVar7;
	CPathPlane* pCVar8;
	int iVar9;
	edF32VECTOR4* peVar10;
	float fVar11;
	edF32VECTOR4 eStack176;
	edF32VECTOR4 eStack160;
	edF32VECTOR4 eStack144;
	edF32VECTOR4 eStack128;
	edF32VECTOR4 local_70;
	edF32VECTOR4 local_60;
	CPathPlaneOutData local_50;
	CPathPlaneOutData local_40;
	CPathPlaneOutData local_30;
	CPathPlaneOutData local_20;
	float local_10;
	float local_c;
	float local_8;
	float local_4;

	iVar9 = (this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).field_0x8;

	pfVar1 = ((this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).pPathFollow)->aDelays;
	if (pfVar1 == (float*)0x0) {
		fVar11 = 0.0f;
	}
	else {
		fVar11 = pfVar1[iVar9];
	}

	this->field_0x470 = fVar11;
	if (fVar11 < 0.001f) {
		this->field_0x470 = 3.0f;
	}

	
	pCVar6 = CActorHero::_gThis;
	uVar2 = ((this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).pPathFollow)->field_0xc;
	if (uVar2 != 3) {
		if (uVar2 != 2) {
			return;
		}
		
		pCVar7 = CActorHero::_gThis;
		uVar2 = ((this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).pPathFollow)->field_0xc;
		if (uVar2 == 3) {
			fVar11 = pCVar6->currentLocation.x - this->currentLocation.x;
			fVar3 = pCVar6->currentLocation.y - this->currentLocation.y;
			fVar4 = pCVar6->currentLocation.z - this->currentLocation.z;
			if ((sqrtf(fVar11 * fVar11 + fVar3 * fVar3 + fVar4 * fVar4) < this->field_0x470) || (this->currentLocation.y < CActorHero::_gThis->currentLocation.y)) {
				
				iVar9 = (this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).field_0x8;
				
				peVar10 = this->pathPlaneArray.GetCurPathPlane()->pathFollowReader.GetWayPoint(iVar9);

				if ((fabs(peVar10->y - this->currentLocation.y) < 0.5f) && (bVar5 = true, 0.001f < this->field_0x3e4)) goto LAB_00387998;
			}
		}
		else {
			if (uVar2 == 2) {
				peVar10 = &this->currentLocation;
				fVar11 = pCVar6->currentLocation.x - peVar10->x;
				fVar3 = pCVar6->currentLocation.y - this->currentLocation.y;
				fVar4 = pCVar6->currentLocation.z - this->currentLocation.z;
				if (this->field_0x470 <= sqrtf(fVar11 * fVar11 + fVar3 * fVar3 + fVar4 * fVar4)) {
					this->pathPlaneArray.GetCurPathPlane()->InitTargetPos(peVar10, &local_20);					
					this->pathPlaneArray.GetCurPathPlane()->ExternComputeTargetPosWithPlane(peVar10, &local_20);

					peVar10 = &this->currentLocation;
					
					FUN_00115380(peVar10, &this->pathPlaneArray.GetCurPathPlane()->pathFollowReader, local_20.field_0x0, 0, &eStack144, &local_8);
					
					this->pathPlaneArray.GetCurPathPlane()->InitTargetPos(peVar10, &local_30);					
					this->pathPlaneArray.GetCurPathPlane()->ExternComputeTargetPosWithPlane(peVar10, &local_30);
					
					FUN_00115380(&pCVar7->currentLocation, &this->pathPlaneArray.GetCurPathPlane()->pathFollowReader, local_30.field_0x0, 0, &eStack128, &local_4);

					bVar5 = false;
					if ((local_20.field_0x0 < local_30.field_0x0) ||
						((local_20.field_0x0 == local_30.field_0x0 && (local_8 < local_4)))) {
						bVar5 = true;
					}

					if (!bVar5) goto LAB_00387990;
				}
				bVar5 = true;
				goto LAB_00387998;
			}
		}
	LAB_00387990:
		bVar5 = false;
	LAB_00387998:
		if (bVar5) {
			SetState(0x1c, -1);
			return;
		}
		
		iVar9 = this->pathPlaneArray.GetCurPathPlane()->pathFollowReader.GetNextPlace((this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).splinePointIndex,
			(this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).field_0xc);
		
		peVar10 = this->pathPlaneArray.GetCurPathPlane()->pathFollowReader.GetWayPoint(iVar9);
		edF32Vector4SubHard(&local_60, peVar10, &this->currentLocation);
		local_60.y = 0.0f;
		edF32Vector4NormalizeHard(&local_60, &local_60);

		fVar11 = this->rotationQuat.x * local_60.z - local_60.x * this->rotationQuat.z;
		if (fVar11 <= 0.01f) {
			if (-0.01f <= fVar11) {
				SetState(ATON_ESCAPE_STATE_PATH_BEFORE_WAIT_JUMP, -1);
				return;
			}

			SetState(ATON_ESCAPE_STATE_PATH_BEFORE_WAIT_JUMP, 0x12);
			return;
		}

		SetState(ATON_ESCAPE_STATE_PATH_BEFORE_WAIT_JUMP, 0x13);
		return;
	}
	
	pCVar7 = CActorHero::_gThis;
	uVar2 = ((this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).pPathFollow)->field_0xc;
	if (uVar2 == 3) {
		fVar11 = pCVar6->currentLocation.x - this->currentLocation.x;
		fVar3 = pCVar6->currentLocation.y - this->currentLocation.y;
		fVar4 = pCVar6->currentLocation.z - this->currentLocation.z;
		if ((sqrtf(fVar11 * fVar11 + fVar3 * fVar3 + fVar4 * fVar4) < this->field_0x470) ||
			(this->currentLocation.y < CActorHero::_gThis->currentLocation.y)) {			
			iVar9 = (this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).field_0x8;
			
			peVar10 = this->pathPlaneArray.GetCurPathPlane()->pathFollowReader.GetWayPoint(iVar9);
			if ((fabs(peVar10->y - this->currentLocation.y) < 0.5f) && (bVar5 = true, 0.001f < this->field_0x3e4)) goto LAB_00387d50;
		}
	}
	else {
		if (uVar2 == 2) {
			peVar10 = &this->currentLocation;
			fVar11 = pCVar6->currentLocation.x - peVar10->x;
			fVar3 = pCVar6->currentLocation.y - this->currentLocation.y;
			fVar4 = pCVar6->currentLocation.z - this->currentLocation.z;
			if (this->field_0x470 <= sqrtf(fVar11 * fVar11 + fVar3 * fVar3 + fVar4 * fVar4)) {
				
				this->pathPlaneArray.GetCurPathPlane()->InitTargetPos(peVar10, &local_40);				
				this->pathPlaneArray.GetCurPathPlane()->ExternComputeTargetPosWithPlane(peVar10, &local_40);
				peVar10 = &this->currentLocation;
				
				FUN_00115380(peVar10, &this->pathPlaneArray.GetCurPathPlane()->pathFollowReader, local_40.field_0x0, 0, &eStack176, &local_10);

				this->pathPlaneArray.GetCurPathPlane()->InitTargetPos(peVar10, &local_50);				
				this->pathPlaneArray.GetCurPathPlane()->ExternComputeTargetPosWithPlane(peVar10, &local_50);
				
				FUN_00115380(&pCVar7->currentLocation, &this->pathPlaneArray.GetCurPathPlane()->pathFollowReader, local_50.field_0x0, 0, &eStack160, &local_c);

				bVar5 = false;
				if ((local_40.field_0x0 < local_50.field_0x0) ||
					((local_40.field_0x0 == local_50.field_0x0 && (local_10 < local_c)))) {
					bVar5 = true;
				}

				if (!bVar5) goto LAB_00387d48;
			}
			bVar5 = true;
			goto LAB_00387d50;
		}
	}
LAB_00387d48:
	bVar5 = false;
LAB_00387d50:
	if (bVar5) {
		SetState(0x1c, -1);
	}
	else {
		
		iVar9 = this->pathPlaneArray.GetCurPathPlane()->pathFollowReader.GetNextPlace((this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).splinePointIndex,
			(this->pathPlaneArray.GetCurPathPlane()->pathFollowReader).field_0xc);
		
		peVar10 = this->pathPlaneArray.GetCurPathPlane()->pathFollowReader.GetWayPoint(iVar9);
		edF32Vector4SubHard(&local_70, peVar10, &this->currentLocation);
		local_70.y = 0.0f;
		edF32Vector4NormalizeHard(&local_70, &local_70);

		fVar11 = this->rotationQuat.x * local_70.z - local_70.x * this->rotationQuat.z;
		if (0.01f < fVar11) {
			SetState(ATON_ESCAPE_STATE_PATH_BEFORE_WAIT_JUMP, 0x13);
		}
		else {
			if (fVar11 < -0.01) {
				SetState(ATON_ESCAPE_STATE_PATH_BEFORE_WAIT_JUMP, 0x12);
			}
			else {
				SetState(ATON_ESCAPE_STATE_PATH_BEFORE_WAIT_JUMP, -1);
			}
		}
	}

	return;
}

void CActorAton::ChooseWaitJumpCallState()

{
	float fVar1;
	float fVar2;
	CActorHero* pCVar3;
	float fVar4;
	edF32VECTOR4 eStack16;

	pCVar3 = CActorHero::_gThis;
	edF32Vector4SubHard(&eStack16, &this->currentLocation, &CActorHero::_gThis->currentLocation);

	fVar4 = edF32Vector4DotProductHard(&eStack16, &this->rotationQuat);
	if (fVar4 < 0.0f) {
		fVar4 = pCVar3->currentLocation.x - this->currentLocation.x;
		fVar1 = pCVar3->currentLocation.y - this->currentLocation.y;
		fVar2 = pCVar3->currentLocation.z - this->currentLocation.z;
		fVar4 = sqrtf(fVar4 * fVar4 + fVar1 * fVar1 + fVar2 * fVar2);
		if (5.0f < fVar4) {
			SetState(0x1a, -1);
		}
		else {
			if (3.0f < fVar4) {
				SetState(0x19, -1);
			}
			else {
				SetState(0x16, -1);
			}
		}
	}
	else {
		SetState(0x17, -1);
	}

	return;
}

CBehaviourAddOnAton::CBehaviourAddOnAton()
{
	this->field_0x10 = 0;
	this->field_0x14 = (int*)0x0;
	this->field_0x1c = 0;
	this->field_0x20 = (int*)0x0;
}

void CBehaviourAddOnAton::Create(ByteCode* pByteCode)
{
	uint uVar1;
	int* piVar2;
	void* pvVar3;
	int iVar4;
	int iVar5;
	int iVar6;
	int iVar7;
	uint* puVar8;
	int iVar9;
	float fVar10;

	uVar1 = pByteCode->GetU32();
	//this->field_0x18 = uVar1;
	uVar1 = pByteCode->GetS32();
	this->field_0x1c = uVar1;
	uVar1 = this->field_0x1c;
	if (uVar1 != 0) {
		//piVar2 = (int*)operator.new.array((long)(int)(uVar1 * 0x18 + 0x10));
		//piVar2 = __construct_new_array(piVar2, (ActorConstructorA*)&LAB_003e1f60, FUN_003e3a70, 0x18, uVar1);
		//this->field_0x20 = piVar2;
		iVar9 = 0;
		if (0 < this->field_0x1c) {
			iVar6 = 0;
			do {
				//puVar8 = (uint*)((int)this->field_0x20 + iVar6);
				uVar1 = pByteCode->GetU32();
				//*puVar8 = uVar1;
				uVar1 = pByteCode->GetS32();
				//puVar8[2] = uVar1;
				//if (0 < (int)puVar8[2]) {
				//	pvVar3 = operator.new.array((long)(int)(puVar8[2] << 2));
				//	puVar8[1] = (uint)pvVar3;
				//}
				iVar7 = 0;
				if (0 < uVar1) {
					iVar5 = 0;
					do {
						iVar4 = pByteCode->GetS32();
						iVar7 = iVar7 + 1;
						//*(int*)(puVar8[1] + iVar5) = iVar4;
						iVar5 = iVar5 + 4;
					} while (iVar7 < uVar1);
				}
				//puVar8[4] = 0xffffffff;
				iVar9 = iVar9 + 1;
				iVar6 = iVar6 + 0x18;
			} while (iVar9 < this->field_0x1c);
		}
	}
	uVar1 = pByteCode->GetS32();
	this->field_0x10 = uVar1;
	uVar1 = this->field_0x10;
	if (uVar1 != 0) {
		//piVar2 = (int*)operator.new.array((long)(int)(uVar1 * 0x1c + 0x10));
		//piVar2 = __construct_new_array(piVar2, (ActorConstructorA*)&LAB_003e3450, FUN_0010df00, 0x1c, uVar1);
		//this->field_0x14 = piVar2;
		iVar9 = 0;
		if (0 < (int)this->field_0x10) {
			iVar6 = 0;
			do {
				//puVar8 = (uint*)((int)this->field_0x14 + iVar6);
				uVar1 = pByteCode->GetU32();
				//puVar8[6] = uVar1;
				uVar1 = pByteCode->GetU32();
				//*puVar8 = uVar1;
				uVar1 = pByteCode->GetS32();
				//puVar8[2] = uVar1;
				//if (0 < (int)puVar8[2]) {
				//	pvVar3 = operator.new.array((long)(int)(puVar8[2] << 2));
				//	puVar8[1] = (uint)pvVar3;
				//}
				iVar7 = 0;
				if (0 < uVar1) {
					iVar5 = 0;
					do {
						iVar4 = pByteCode->GetS32();
						iVar7 = iVar7 + 1;
						//*(int*)(puVar8[1] + iVar5) = iVar4;
						iVar5 = iVar5 + 4;
					} while (iVar7 < uVar1);
				}
				//puVar8[4] = 0xffffffff;
				iVar9 = iVar9 + 1;
				iVar6 = iVar6 + 0x1c;
			} while (iVar9 < this->field_0x10);
		}
	}
	fVar10 = pByteCode->GetF32();
	//this->field_0x24 = fVar10;
	//this->field_0x24 = 10.0;
	return;
}

void CBehaviourAddOnAton::Init(CActor* pActor)
{
	IMPLEMENTATION_GUARD_LOG();
}

void CBehaviourAddOnAton::Manage()
{
	IMPLEMENTATION_GUARD_LOG();
}

void CFxTail::Create(float param_1, int count, int param_4, int materialId)
{
	IMPLEMENTATION_GUARD_LOG();
}

void CFxTail::Init(float param_1, int id)
{
	IMPLEMENTATION_GUARD_LOG();
}

CStaticMeshComponent::CStaticMeshComponent()
{
	//this->field_0x634 = 0;
	//this->field_0x638 = 0;
	//this->staticMeshComponent = &StaticMeshComponent::VTable_0043a7a0;
	//this->field_0x690 = 0;
	//this->field_0x694 = 0;
	//this->field_0x648 = -1;
	//this->field_0x644 = -1;
	//this->staticMeshComponent = &StaticMeshComponent::VTable_0043a6f0;
	//this->staticMeshComponent = &StaticMeshComponent::VTable_0043a6d0;
}

void CStaticMeshComponent::Reset()
{
	IMPLEMENTATION_GUARD_LOG();
}

void CBehaviourAton::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorAton*>(pOwner);
	return;
}

int CBehaviourAton::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CPathPlane* pPathPlane;

	if (msg == 0xe) {
		pPathPlane = this->pOwner->pathPlaneArray.GetCurPathPlane();
		if (((pPathPlane->pathFollowReader).pPathFollow)->field_0xc == 5) {
			this->pOwner->SetState(0x23, -1);
		}
	}
	else {
		if (msg != 0xf) {
			return 0;
		}

		if (this->pOwner->actorState == 0x2b) {
			this->pOwner->field_0x3e0 = 0;
			this->pOwner->SetState(0x2e, -1);
			return 1;
		}
	}

	return 0;
}

void CBehaviourAtonEscape::Manage()
{
	this->pOwner->BehaviourAtonEscape_Manage();
}

void CBehaviourAtonEscape::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorAton* pActorAton;
	CActorHero* pHero;
	int iVar3;
	CPathPlane* pPathPlane;
	edF32VECTOR4* peVar5;
	edF32VECTOR4 local_10;

	this->pOwner = static_cast<CActorAton*>(pOwner);
	this->pOwner->pathPlaneArray.Reset();

	if (newState == -1) {
		iVar3 = this->pOwner->pathPlaneArray.GetNbPathPlane();
		pHero = CActorHero::_gThis;
		if (iVar3 == 0) {
			this->pOwner->SetState(6, -1);
		}
		else {
			pActorAton = this->pOwner;

			if (pActorAton->bWaypointSet == 0) {
				pActorAton->pathPlaneArray.InitPosition(&pActorAton->currentLocation);
			}
			else {
				pActorAton->pathPlaneArray.InitPosition(&pActorAton->waypointLocation);
			}

			this->pOwner->bWaypointSet = 0;

			pPathPlane = this->pOwner->pathPlaneArray.GetCurPathPlane();
			pPathPlane->InitTargetPos(&pHero->currentLocation, &pPathPlane->outData);
			pActorAton = this->pOwner;
			pPathPlane = pActorAton->pathPlaneArray.GetCurPathPlane();
			peVar5 = pPathPlane->pathFollowReader.GetWayPoint();
			pActorAton->UpdatePosition(peVar5, true);
			pPathPlane = this->pOwner->pathPlaneArray.GetCurPathPlane();
			peVar5 = pPathPlane->pathFollowReader.GetWayPointAngles();
			SetVectorFromAngles(&local_10, &peVar5->xyz);
			pActorAton = this->pOwner;
			pActorAton->rotationQuat = local_10;
			pActorAton = this->pOwner;
			pActorAton->dynamic.rotationQuat = local_10;
			this->pOwner->AnalysePathType();
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourAtonEscape::InitState(int newState)
{
	this->pOwner->BehaviourAtonEscape_InitState(newState);
}

void CBehaviourAtonEscape::TermState(int oldState, int newState)
{
	CActorAton* pActorAton;
	CCollision* pCollision;
	int* piVar3;
	StateConfig* pSVar4;
	long lVar5;
	uint uVar6;

	pActorAton = this->pOwner;
	if (oldState == -1) {
		uVar6 = 0;
	}
	else {
		pSVar4 = pActorAton->GetStateCfg(oldState);
		uVar6 = pSVar4->flags_0x4;
	}

	if ((uVar6 & 0x1000) != 0) {
		pActorAton->pAnimationController->UnRegisterBone(pActorAton->field_0x354);
		pActorAton->pAnimationController->UnRegisterBone(pActorAton->field_0x358);
		pActorAton->pAnimationController->UnRegisterBone(pActorAton->field_0x35c);
		pActorAton->pAnimationController->UnRegisterBone(pActorAton->field_0x360);
	}

	if (oldState == 0x2f) {
		pActorAton->flags = pActorAton->flags & 0xffffefff;

		IMPLEMENTATION_GUARD(
		piVar3 = (int*)pActorAton->field_0x638;
		if (((piVar3 != (int*)0x0) && (pActorAton->field_0x634 != 0)) && (pActorAton->field_0x634 == piVar3[6])) {
			(**(code**)(*piVar3 + 0xc))();
		}
		pActorAton->field_0x638 = 0;
		pActorAton->field_0x634 = 0;
		lVar5 = (**(code**)((int)((edF32VECTOR4*)&pActorAton->staticMeshComponent)->x + 0x14))();
		if (lVar5 != 0) {
			StaticMeshComponent::Unload_00114e80((StaticMeshComponent*)&pActorAton->staticMeshComponent, (ed_3D_Scene*)0x0);
		})
	}
	else {
		if ((oldState == ATON_ESCAPE_STATE_PATH_JUMP_2_4) || (oldState == 0x14)) {
			pActorAton->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
			pCollision = pActorAton->pCollisionData;
			if (pCollision != (CCollision*)0x0) {
				pCollision->actorFieldA = (CActor*)0x0;
				CActorHero::_gThis->pCollisionData->actorFieldA = (CActor*)0x0;
			}
		}
		else {
			if (oldState == 0x13) {
				pActorAton->field_0x478 = 0.0f;
			}
		}
	}

	return;
}

int CBehaviourAtonEscape::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CPathPlane* pPathPlane;

	if (msg == 0xe) {
		pPathPlane = this->pOwner->pathPlaneArray.GetCurPathPlane();
		if (((pPathPlane->pathFollowReader).pPathFollow)->field_0xc == 5) {
			this->pOwner->SetState(0x23, -1);
		}
	}
	else {
		if (msg != 0xf) {
			return 0;
		}

		if (this->pOwner->actorState == 0x2b) {
			this->pOwner->field_0x3e0 = 0;
			this->pOwner->SetState(0x2e, -1);
			return 1;
		}
	}

	return 0;
}
