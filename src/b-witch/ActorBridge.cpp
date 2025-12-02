#include "ActorBridge.h"

#include <math.h>

#include "MemoryStream.h"
#include "MathOps.h"
#include "CollisionManager.h"
#include "WayPoint.h"
#include "TimeController.h"
#include "ActorMovingPlatform.h"
#include "EventManager.h"

int CVerletBridge::GetMemRequirement(CVerletBridge::Config* pConfig)
{
	int iVar1;
	bool cVar2;
	bool cVar3;
	int nbLinkConstraints;
	int iVar3;

	if (pConfig->field_0xe == false) {
		nbLinkConstraints = pConfig->stepCount;
		cVar3 = pConfig->field_0xd;
		iVar3 = (nbLinkConstraints + 1) * 2;

		if (cVar3 != false) {
			iVar3 = iVar3 + 4;
		}

		cVar2 = pConfig->field_0xc;
		if (cVar2 != false) {
			iVar3 = iVar3 + 2;
		}
	}
	else {
		nbLinkConstraints = pConfig->stepCount;
		cVar3 = pConfig->field_0xd;
		iVar3 = nbLinkConstraints + 1;
		if (cVar3 != false) {
			iVar3 = nbLinkConstraints + 3;
		}

		cVar2 = pConfig->field_0xc;
		if (cVar2 != false) {
			iVar3 = iVar3 + 1;
		}
	}

	if (pConfig->field_0xe == false) {
		iVar1 = nbLinkConstraints * 4;
		nbLinkConstraints = iVar1 + -3;
		if (cVar3 != false) {
			nbLinkConstraints = iVar1 + 1;
		}

		if (cVar2 != false) {
			nbLinkConstraints = nbLinkConstraints + 1;
		}
	}
	else {
		if (cVar3 != false) {
			nbLinkConstraints = nbLinkConstraints + 2;
		}
	}

	return (pConfig->nbLinkConstraints + 1) * sizeof(CLinkConstraint*) + iVar3 * 0x40 + pConfig->nbMaxForces * sizeof(CVerletForceGen*) + nbLinkConstraints * sizeof(S_LINK_SUB_OBJ);
}

CVerletBridge::CVerletBridge()
{

}

void CVerletBridge::Reset(CVerletBridge::Config* pConfig)
{
	bool bVar1;
	edF32VECTOR4* v2;
	int iVar2;
	edF32VECTOR4* pfVar3;
	CVParticleInfo* puVar4;
	edF32VECTOR4* peVar5;
	edF32VECTOR4* puVar6;
	int iVar7;
	void* pvVar8;
	int iVar9;
	S_ACTOR_STREAM_REF* pStreamRef;
	edF32VECTOR4* pfVar10;
	S_LINK_SUB_OBJ* piVar11;
	int iVar12;
	edF32VECTOR4* peVar13;
	S_LINK_SUB_OBJ* piVar14;
	int iVar15;
	S_LINK_SUB_OBJ* piVar16;
	edF32VECTOR4* v1;
	int unaff_s7_lo;
	undefined4 uVar17;
	float fVar18;
	float fVar19;
	float fVar20;
	float fVar21;
	undefined4 uVar22;
	float fVar23;
	undefined4 uVar24;
	float fVar25;
	float fVar26;
	edF32VECTOR4 eStack336;
	edF32VECTOR4 eStack320;
	edF32VECTOR4 eStack304;
	edF32VECTOR4 eStack288;
	edF32VECTOR4 eStack272;
	edF32VECTOR4 eStack256;
	edF32VECTOR4 eStack240;
	edF32VECTOR4 eStack224;
	edF32VECTOR4 local_d0;
	edF32VECTOR4 local_c0;
	edF32VECTOR4 local_b0;
	edF32VECTOR4 local_a0;
	edF32VECTOR4 local_90;
	edF32VECTOR4 local_80;
	edF32VECTOR4 local_70;
	edF32VECTOR4 local_60;
	edF32VECTOR4 local_50;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	v2 = &pConfig->field_0x10;
	v1 = &pConfig->field_0x20;

	edF32Vector4SubHard(&eStack16, v1, v2);

	piVar11 = this->linkConstraint.aLinkInfo;

	fVar18 = edF32Vector4GetLengthSoft(&eStack16);

	local_20 = gF32Vector4Zero;

	fVar25 = fVar18 + fVar18 * pConfig->field_0x38;

	edF32Vector4ScaleHard(1.0f / (float)this->altStepCount, &local_20, &eStack16);

	peVar13 = this->pBridgeMem;
	pfVar10 = this->field_0x1c;
	puVar4 = this->aParticleInfo;

	local_30 = gF32Vector4Zero;

	edF32Vector4AddHard(&local_30, v2, v1);
	edF32Vector4ScaleHard(0.5f, &local_30, &local_30);
	fVar18 = fVar18 / fVar25;
	fVar26 = sqrtf(6.0f - fVar18 * 6.0f);

	iVar12 = 5;
	do {
		fVar19 = sinf(fVar26);
		fVar19 = fVar19 - fVar18 * fVar26;
		fVar20 = cosf(fVar26);
		bVar1 = 0 < iVar12;
		fVar26 = fVar26 - fVar19 / (fVar20 - fVar18);
		iVar12 = iVar12 + -1;
	} while (bVar1);

	fVar26 = fVar26 + fVar26;
	fVar18 = cosf(fVar26 / 2.0f);
	local_30.y = local_30.y + (fVar25 / fVar26) * fVar18;
	iVar12 = this->altStepCount;
	fVar19 = 0.0f;

	edF32Vector4SubHard(&local_40, v2, &local_30);
	fVar18 = fVar19;
	fVar25 = fVar19;

	if (this->field_0x68 == false) {
		fVar25 = pConfig->field_0x30;
		fVar18 = fVar25 * 0.5f;
		fVar25 = -fVar25 * 0.5f;
	}

	iVar15 = 0;
	if (0 < this->altStepCount) {
		do {
			fVar20 = cosf(fVar19);
			fVar21 = sinf(fVar19);
			local_50.w = gF32Vector4Zero.w;
			local_50.x = local_40.x;
			local_50.y = local_40.y * fVar20 + local_40.z * fVar21;
			local_50.z = local_40.z * fVar20 - local_40.y * fVar21;

			edF32Vector4AddHard(&local_50, &local_50, &local_30);

			local_60.y = 0.0f;
			local_60.z = 0.0f;
			local_60.w = 0.0f;
			local_60.x = fVar25;

			edF32Vector4AddHard(peVar13, &local_50, &local_60);

			*pfVar10 = *peVar13;

			if (this->field_0x68 == false) {
				local_70.y = 0.0f;
				local_70.z = 0.0f;
				local_70.w = 0.0f;
				local_70.x = fVar18;
				edF32Vector4AddHard(peVar13 + this->altStepCount + 1, &local_50, &local_70);
				iVar2 = this->altStepCount + 1;
				peVar5 = peVar13 + iVar2;
				pfVar3 = pfVar10 + iVar2;
				*pfVar3 = *peVar5;
			}

			iVar15 = iVar15 + 1;
			puVar4->inverseMass = 0.03333334f;
			fVar19 = fVar19 + fVar26 / (float)iVar12;
			pfVar10 = pfVar10 + 1;
			puVar4 = puVar4 + 1;
			peVar13 = peVar13 + 1;
		} while (iVar15 < this->altStepCount);
	}

	local_80.y = 0.0f;
	local_80.z = 0.0f;
	local_80.w = 0.0f;
	local_80.x = fVar25;

	edF32Vector4AddHard(peVar13, v1, &local_80);

	*pfVar10 = *peVar13;

	if (this->field_0x68 == false) {
		local_90.y = 0.0f;
		local_90.z = 0.0f;
		local_90.w = 0.0f;
		local_90.x = fVar18;

		edF32Vector4AddHard(peVar13 + this->altStepCount + 1, v1, &local_90);

		iVar12 = this->altStepCount + 1;
		peVar13 = peVar13 + iVar12;
		pfVar10 = pfVar10 + iVar12;
		*pfVar10 = *peVar13;
	}

	if (this->field_0x69 != false) {
		if (this->field_0x68 == false) {
			unaff_s7_lo = (this->altStepCount + 1) * 2;
		}
		else {
			unaff_s7_lo = this->altStepCount + 1;
		}

		edF32VECTOR4* pVec = reinterpret_cast<edF32VECTOR4*>(this->pBridgeMem);

		peVar13 = pVec + unaff_s7_lo;
		pfVar10 = this->field_0x1c + unaff_s7_lo;
		local_a0.y = pConfig->field_0x34;
		local_a0.z = 0.0f;
		local_a0.w = 0.0f;
		local_a0.x = fVar25;

		edF32Vector4AddHard(peVar13, v2, &local_a0);

		*pfVar10 = *peVar13;

		local_b0.y = pConfig->field_0x34;
		local_b0.z = 0.0f;
		local_b0.w = 0.0f;
		local_b0.x = fVar25;

		edF32Vector4AddHard(peVar13 + 1, v1, &local_b0);

		pfVar10[1] = peVar13[1];

		if (this->field_0x68 == false) {
			local_c0.y = pConfig->field_0x34;
			local_c0.z = 0.0f;
			local_c0.w = 0.0f;
			local_c0.x = fVar18;
			edF32Vector4AddHard(peVar13 + 2, v2, &local_c0);

			pfVar10[2] = peVar13[2];

			local_d0.y = pConfig->field_0x34;
			local_d0.z = 0.0f;
			local_d0.w = 0.0f;
			local_d0.x = fVar18;
			edF32Vector4AddHard(peVar13 + 3, v1, &local_d0);

			pfVar10[3] = peVar13[3];
		}
	}

	iVar12 = this->altStepCount;
	iVar15 = 0;
	if (0 < iVar12) {
		piVar14 = piVar11;

		do {
			pvVar8 = this->pBridgeMem;

			edF32VECTOR4* pVec = this->pBridgeMem;

			piVar14->particleIndexA = iVar15;
			piVar14->particleIndexB = iVar15 + 1;

			if (pvVar8 != (void*)0x0) {
				edF32Vector4SubHard(&eStack224, pVec + iVar15, pVec + (iVar15 + 1));
				fVar18 = edF32Vector4DotProductHard(&eStack224, &eStack224);
				piVar14->squaredRestDistance = fVar18;
			}

			piVar14->iterationCount = 3;

			if (this->field_0x68 == false) {
				iVar12 = this->altStepCount;
				pvVar8 = this->pBridgeMem;
				iVar7 = iVar12 + 2 + iVar15;
				iVar9 = iVar12 + 1 + iVar15;
				piVar16 = piVar11 + (iVar15 + iVar12);
				piVar16->particleIndexA = iVar9;
				piVar16->particleIndexB = iVar7;

				if (pvVar8 != (void*)0x0) {
					edF32Vector4SubHard(&eStack240, pVec + iVar9, pVec + iVar7);
					fVar18 = edF32Vector4DotProductHard(&eStack240, &eStack240);
					piVar16->squaredRestDistance = fVar18;
				}

				piVar16->iterationCount = 3;

				if (iVar15 != 0) {
					iVar12 = this->altStepCount;
					pvVar8 = this->pBridgeMem;
					iVar7 = iVar12 + 1 + iVar15;
					piVar16 = piVar11 + (iVar15 + iVar12 * 2 + -1);
					piVar16->particleIndexA = iVar15;
					piVar16->particleIndexB = iVar7;

					if (pvVar8 != (void*)0x0) {
						edF32Vector4SubHard(&eStack256, pVec + iVar15, pVec + iVar7);
						fVar18 = edF32Vector4DotProductHard(&eStack256, &eStack256);
						piVar16->squaredRestDistance = fVar18;
					}

					piVar16->iterationCount = 3;
				}

				if (0 < iVar15) {
					iVar12 = this->altStepCount;

					if (iVar15 < iVar12 + -1) {
						iVar7 = iVar12 + 2 + iVar15;
						pvVar8 = this->pBridgeMem;
						piVar16 = piVar11 + (iVar15 + iVar12 * 3 + -2);
						piVar16->particleIndexA = iVar15;
						piVar16->particleIndexB = iVar7;

						if (pvVar8 != (void*)0x0) {
							edF32Vector4SubHard(&eStack272, pVec + iVar15, pVec + iVar7);
							fVar18 = edF32Vector4DotProductHard(&eStack272, &eStack272);
							piVar16->squaredRestDistance = fVar18;
						}

						piVar16->iterationCount = 3;
					}
				}
			}

			iVar12 = this->altStepCount;
			iVar15 = iVar15 + 1;
			piVar14 = piVar14 + 1;
		} while (iVar15 < iVar12);
	}

	if (this->field_0x69 != false) {
		if (this->field_0x68 == false) {
			iVar12 = iVar12 * 4 + -3;
			pvVar8 = this->pBridgeMem;
		}
		else {
			pvVar8 = this->pBridgeMem;
		}

		edF32VECTOR4* pVec = this->pBridgeMem;

		piVar14 = piVar11 + iVar12;
		piVar14->particleIndexA = unaff_s7_lo;
		piVar14->particleIndexB = 1;

		if (pvVar8 != (void*)0x0) {
			edF32Vector4SubHard(&eStack288, pVec + unaff_s7_lo, pVec + 1);
			fVar18 = edF32Vector4DotProductHard(&eStack288, &eStack288);
			piVar14->squaredRestDistance = fVar18;
		}

		piVar14->iterationCount = 3;

		pvVar8 = this->pBridgeMem;
		piVar14 = piVar11 + (iVar12 + 1);
		iVar15 = this->altStepCount + -1;
		piVar14->particleIndexA = unaff_s7_lo + 1;
		piVar14->particleIndexB = iVar15;

		if (pvVar8 != (void*)0x0) {
			edF32Vector4SubHard(&eStack304, pVec + (unaff_s7_lo + 1), pVec + iVar15);
			fVar18 = edF32Vector4DotProductHard(&eStack304, &eStack304);
			piVar14->squaredRestDistance = fVar18;
		}

		piVar14->iterationCount = 3;

		if (this->field_0x68 == false) {
			pvVar8 = this->pBridgeMem;
			piVar14 = piVar11 + (iVar12 + 2);
			iVar15 = this->altStepCount + 2;
			piVar14->particleIndexA = unaff_s7_lo + 2;
			piVar14->particleIndexB = iVar15;
			if (pvVar8 != (void*)0x0) {
				edF32Vector4SubHard(&eStack320, pVec + (unaff_s7_lo + 2), pVec + iVar15);
				fVar18 = edF32Vector4DotProductHard(&eStack320, &eStack320);
				piVar14->squaredRestDistance = fVar18;
			}

			piVar14->iterationCount = 3;
			pvVar8 = this->pBridgeMem;
			iVar15 = this->altStepCount;
			piVar11 = piVar11 + (iVar12 + 3);
			piVar11->particleIndexA = unaff_s7_lo + 3;
			piVar11->particleIndexB = iVar15 * 2;

			if (pvVar8 != (void*)0x0) {
				edF32Vector4SubHard(&eStack336, pVec + (unaff_s7_lo + 3), pVec + iVar15 * 2);
				fVar18 = edF32Vector4DotProductHard(&eStack336, &eStack336);
				piVar14->squaredRestDistance = fVar18;
			}

			piVar14->iterationCount = 3;
		}
	}

	edF32VECTOR4* pVec = reinterpret_cast<edF32VECTOR4*>(this->pBridgeMem);
	puVar6 = this->field_0x1c;

	this->aParticleInfo[0].inverseMass = 0.0f;
	puVar6[0] = pVec[0];

	iVar12 = this->altStepCount;
	this->aParticleInfo[iVar12].inverseMass = 0.0f;
	puVar6[iVar12] = pVec[iVar12];

	if (this->field_0x69 != false) {
		this->aParticleInfo[unaff_s7_lo].inverseMass = 0.0f;
		puVar6[unaff_s7_lo] = pVec[unaff_s7_lo];

		this->aParticleInfo[unaff_s7_lo + 1].inverseMass = 0.0f;
		puVar6[unaff_s7_lo + 1] = pVec[unaff_s7_lo + 1];
	}

	if (this->field_0x68 == false) {
		iVar12 = this->altStepCount + 1;

		this->aParticleInfo[iVar12].inverseMass = 0.0f;
		puVar6[iVar12] = pVec[iVar12];
;
		iVar12 = this->altStepCount * 2 + 1;
	
		this->aParticleInfo[iVar12].inverseMass = 0.0f;
		puVar6[iVar12] = pVec[iVar12];

		if (this->field_0x69 != false) {
			this->aParticleInfo[(unaff_s7_lo + 2)].inverseMass = 0.0f;
			puVar6[unaff_s7_lo + 2] = pVec[unaff_s7_lo + 2];

			this->aParticleInfo[(unaff_s7_lo + 3)].inverseMass = 0.0f;
			puVar6[unaff_s7_lo + 3] = pVec[unaff_s7_lo + 3];
		}
	}
}

void CActorBridge::Create(ByteCode* pByteCode)
{
	S_ACTOR_STREAM_REF* pSVar1;
	CWayPointManager* pCVar2;
	int iVar3;
	CWayPoint* pMVar4;
	uint uVar5;
	float fVar6;

	CActor::Create(pByteCode);

	pSVar1 = (S_ACTOR_STREAM_REF*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(&pSVar1->aEntries);
	if (pSVar1->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pSVar1->entryCount * sizeof(S_STREAM_REF<CActor>);
	}
	this->pActorStream = pSVar1;

	pCVar2 = CScene::ptable.g_WayPointManager_0045169c;
	iVar3 = pByteCode->GetS32();
	if (iVar3 == -1) {
		pMVar4 = (CWayPoint*)0x0;
	}
	else {
		pMVar4 = pCVar2->aWaypoints + iVar3;
	}
	this->field_0x160 = pMVar4;

	iVar3 = pByteCode->GetS32();
	if (iVar3 == -1) {
		pMVar4 = (CWayPoint*)0x0;
	}
	else {
		pMVar4 = pCVar2->aWaypoints + iVar3;
	}
	this->field_0x164 = pMVar4;

	this->field_0x330 = pByteCode->GetF32();
	this->field_0x334 = pByteCode->GetF32();
	this->field_0x33c = pByteCode->GetF32();
	this->field_0x340 = pByteCode->GetF32();
	this->field_0x338 = pByteCode->GetF32();
	this->field_0x344 = pByteCode->GetU32();
	this->bridgeConfigFlags = pByteCode->GetU32();
	this->field_0x348 = pByteCode->GetS32();
	this->stepCollisionScale = pByteCode->GetF32();

	//this->field_0x2e0 = 0;
	//this->field_0x350 = 0;

	return;
}

void CActorBridge::Init()
{
	S_ACTOR_STREAM_REF* pStreamRef;
	int iVar12;

	CActor::Init();

	pStreamRef = this->pActorStream;
	S_STREAM_REF<CActor>* pStreamEntry = pStreamRef->aEntries;
	for (iVar12 = pStreamRef->entryCount; iVar12 != 0; iVar12 = iVar12 + -1) {
		pStreamEntry->Init();
		pStreamEntry = pStreamEntry + 1;
	}

	Initialize();
	verletBridge.Reset(&verletBridgeConfig);
	BehaviourManage(1, (CWindForce*)0x0);
	this->field_0x354 = -1;
}

void CActorBridge::Term()
{
	if (this->pBridgeMem != (void*)0x0) {
		delete(this->pBridgeMem);
		this->pBridgeMem = NULL;
	}
	if (this->aHeights != (float*)0x0) {
		delete(this->aHeights);
		this->aHeights = (float*)0x0;
	}

	CActor::Term();

	return;
}

struct S_SAVE_CLASS_BRIDGE
{
	int field_0x0;
};

void CActorBridge::SaveContext(void* pData, uint mode, uint maxSize)
{
	S_SAVE_CLASS_BRIDGE* pSaveData = reinterpret_cast<S_SAVE_CLASS_BRIDGE*>(pData);

	pSaveData->field_0x0 = this->field_0x354;

	return;
}

void CActorBridge::LoadContext(void* pData, uint mode, uint maxSize)
{
	S_SAVE_CLASS_BRIDGE* pSaveData = reinterpret_cast<S_SAVE_CLASS_BRIDGE*>(pData);

	if ((mode == 1) && (this->field_0x354 = pSaveData->field_0x0, this->field_0x354 != -1)) {
		SetState(BRIDGE_BASIC_STATE_SPLIT, -1);
	}

	return;
}

StateConfig CActorBridge::gStateCfg_BRG[3] =
{
	StateConfig(0, 0),
	StateConfig(0, 0),
	StateConfig(0, 0),
};

StateConfig* CActorBridge::GetStateCfg(int state)
{
	StateConfig* pAVar1;

	if (state < 5) {
		pAVar1 = CActor::GetStateCfg(state);
	}
	else {
		pAVar1 = CActorBridge::gStateCfg_BRG + state + -5;
	}
	return pAVar1;
}

int CActorBridge::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int result;

	if (((msg == 0x10) || (msg == 0xf)) || (msg == 0xe)) {
		if (this->actorState == BRIDGE_BASIC_STATE_STAND) {
			this->field_0x354 = (int)pMsgParam;
			SetState(BRIDGE_BASIC_STATE_SPLIT, -1);
		}

		result = 1;
	}
	else {
		result = 0;
	}

	return result;
}

CBehaviour* CActorBridge::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == BRIDGE_BEHAVIOUR_WIND_AWARE) {
		pBehaviour = &this->behaviourWindAware;
	}
	else {
		if (behaviourType == BRIDGE_BEHAVIOUR_BASIC) {
			pBehaviour = &this->behaviourBasic;
		}
		else {
			pBehaviour = CActor::BuildBehaviour(behaviourType);
		}
	}
	return pBehaviour;
}

void CActorBridge::Initialize()
{
	bool bVar1;
	uint configFlags;
	int iVar3;
	edColPRIM_OBJECT* peVar4;
	S_ACTOR_STREAM_REF* pSVar5;
	CEventManager* pCVar6;
	int iVar7;
	void* pvVar8;
	int piVar8;
	ed_zone_3d* peVar9;
	edF32MATRIX4* peVar10;
	float* pfVar11;
	int piVar12;
	int iVar12;
	edF32VECTOR4* peVar13;
	bool cVar13;
	bool cVar14;
	edF32MATRIX4* peVar14;
	int nbSteps;
	undefined4* puVar16;
	undefined4* puVar17;
	float fVar18;
	float fVar19;
	float fVar20;
	edF32MATRIX4 local_e0;
	edF32VECTOR4 gravityForce;
	edF32VECTOR4 local_90;
	edF32VECTOR4 local_80;
	edF32MATRIX4 eStack112;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	e_ed_event_prim3d_type eStack4;
	CActor* pCurActor;

	local_20.xyz = this->field_0x160->location;
	local_20.w = 1.0f;

	local_30.xyz = this->field_0x164->location;
	local_30.w = 1.0f;

	edF32Vector4SubHard(&this->field_0x2f0.rowZ, &local_30, &local_20);
	fVar18 = edF32Vector4GetLengthSoft(&this->field_0x2f0.rowZ);
	peVar13 = &this->field_0x2f0.rowZ;
	edF32Vector4ScaleHard(1.0f / fVar18, peVar13, peVar13);
	edF32Vector4CrossProductHard(&this->field_0x2f0.rowX, &g_xVector, &this->field_0x2f0.rowZ);
	edF32Vector4NormalizeHard(&this->field_0x2f0.rowX, &this->field_0x2f0.rowX);
	edF32Vector4CrossProductHard(&this->field_0x2f0.rowY, &this->field_0x2f0.rowZ, &this->field_0x2f0.rowX);
	(this->field_0x2f0).rowT = local_20;
	edF32Matrix4GetInverseOrthoHard(&eStack112, &this->field_0x2f0);
	edF32Matrix4MulF32Vector4Hard(&local_90, &eStack112, &local_20);
	edF32Matrix4MulF32Vector4Hard(&local_80, &eStack112, &local_30);

	configFlags = this->bridgeConfigFlags;
	fVar19 = this->field_0x338;
	fVar18 = this->field_0x334;
	fVar20 = this->field_0x330;

	if (this->pActorStream == (S_ACTOR_STREAM_REF*)0x0) {
		iVar7 = 0;
	}
	else {
		iVar7 = this->pActorStream->entryCount;
	}

	this->verletBridgeConfig = CVerletBridge::Config(iVar7, 1, 2, (configFlags & 1) != 0, (configFlags & 2) != 0, (configFlags & 4) != 0, &local_90, &local_80, fVar20, fVar18, fVar19);

	this->pBridgeMem = new char[CVerletBridge::GetMemRequirement(&this->verletBridgeConfig)];

	static const edF32VECTOR4 edF32VECTOR4_0040ed60 = { 0.0f, -9.81f, 0.0f, 0.0f };
	gravityForce = edF32VECTOR4_0040ed60;

	edF32Matrix4MulF32Vector4Hard(&gravityForce, &eStack112, &gravityForce);
	(this->verletBridge).gravityForce = gravityForce;
	(this->verletBridge).gravityForce.w = 0.0f;

	this->verletBridgeConfig.ComputeNbParticles(&iVar7, &nbSteps);
	this->verletBridgeConfig.ComputeNbLinks(&piVar12, &piVar8);

	(this->verletBridge).altStepCount = (this->verletBridgeConfig).stepCount;
	(this->verletBridge).field_0x68 = (this->verletBridgeConfig).field_0xe;
	(this->verletBridge).field_0x69 = (this->verletBridgeConfig).field_0xd;

	void* pMem = this->verletBridge.Initialize(pBridgeMem, nbSteps, (this->verletBridgeConfig).nbMaxForces, (this->verletBridgeConfig).nbLinkConstraints);

	this->verletBridge.nbParticleInfo = iVar7;

	(this->verletBridge).linkConstraint.field_0x10 = piVar8;
	(this->verletBridge).linkConstraint.nbLinkInfo = 0x0;
	(this->verletBridge).linkConstraint.aLinkInfo = (S_LINK_SUB_OBJ*)pMem;
	(this->verletBridge).linkConstraint.nbLinkInfo = piVar12;

	
	(this->verletBridge).linkConstraint.iterationPriority = 3;
	(this->verletBridge).aVerletConstraints[(this->verletBridge).nbVerletConstraints] = &(this->verletBridge).linkConstraint;
	(this->verletBridge).nbVerletConstraints = (this->verletBridge).nbVerletConstraints + 1;

	(this->verletBridge).linkConstraint.iterationPriority = this->field_0x344;

	(this->openBoxConstraint).bUseTransform = 0;
	(this->openBoxConstraint).bEdgeSliding = 0;

	(this->openBoxConstraint).transformMatrix = gF32Matrix4Unit;

	(this->openBoxConstraint).minBounds.x = -0.5f;
	(this->openBoxConstraint).minBounds.y = -0.5f;
	(this->openBoxConstraint).minBounds.z = -0.5f;
	(this->openBoxConstraint).minBounds.w = 0.0f;

	(this->openBoxConstraint).maxBounds.x = 0.5f;
	(this->openBoxConstraint).maxBounds.y = 0.5f;
	(this->openBoxConstraint).maxBounds.z = 0.5f;
	(this->openBoxConstraint).maxBounds.w = 0.0f;

	(this->openBoxConstraint).bEdgeSliding = 0;

	pCVar6 = CScene::ptable.g_EventManager_006f5080;
	configFlags = this->field_0x348;
	if (-1 < (int)configFlags) {
		peVar9 = (ed_zone_3d*)0x0;
		if (configFlags != 0xffffffff) {
			peVar9 = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, configFlags);
		}

		peVar10 = edEventGetChunkZonePrimitive(pCVar6->activeChunkId, peVar9, 0, &eStack4);
		edF32Matrix4MulF32Matrix4Hard(&local_e0, &this->field_0x2f0, peVar10);

		(this->openBoxConstraint).bUseTransform = 1;
		(this->openBoxConstraint).transformMatrix = local_e0;

		edF32Matrix4GetInverseGaussSoft(&(this->openBoxConstraint).inverseTransformMatrix, &(this->openBoxConstraint).transformMatrix);
	}


	if (this->pActorStream == (S_ACTOR_STREAM_REF*)0x0) {
		iVar7 = 0;
	}
	else {
		iVar7 = this->pActorStream->entryCount;
	}

	this->aHeights = new float[iVar7];
	nbSteps = 0;
	iVar7 = 0;
	while (true) {
		pSVar5 = this->pActorStream;
		iVar12 = 0;

		if (pSVar5 != (S_ACTOR_STREAM_REF*)0x0) {
			iVar12 = pSVar5->entryCount;
		}

		if (iVar12 <= nbSteps) break;

		pCurActor = pSVar5->aEntries[nbSteps].Get();

		if (((pCurActor != (CActor*)0x0) && (pCurActor->pCollisionData != (CCollision*)0x0)) &&
			(peVar4 = pCurActor->pCollisionData->pObbPrim, peVar4 != (edColPRIM_OBJECT*)0x0)) {
			this->aHeights[nbSteps] = (peVar4->field_0x90).z;
		}
		iVar7 = iVar7 + 4;
		nbSteps = nbSteps + 1;
	}
	return;
}

void CActorBridge::ApplyStepsCollisionScale(int bEnable)
{
	edColPRIM_OBJECT* peVar1;
	S_ACTOR_STREAM_REF* pSVar2;
	int iVar4;
	int iVar6;
	CActor* pCurActor;
	CCollision* pCollision;

	if (this->aHeights != (float*)0x0) {
		iVar6 = 0;

		while (true) {
			pSVar2 = this->pActorStream;
			iVar4 = 0;

			if (pSVar2 != (S_ACTOR_STREAM_REF*)0x0) {
				iVar4 = pSVar2->entryCount;
			}

			if (iVar4 <= iVar6) break;

			pCurActor = pSVar2->aEntries[iVar6].Get();

			if (((pCurActor != (CActor*)0x0) && (pCollision = pCurActor->pCollisionData, pCollision != (CCollision*)0x0)) &&
				(peVar1 = pCollision->pObbPrim, peVar1 != (edColPRIM_OBJECT*)0x0)) {
				if (bEnable == 0) {
					(peVar1->field_0x90).z = this->aHeights[iVar6];
				}
				else {
					(peVar1->field_0x90).z = this->stepCollisionScale * this->aHeights[iVar6];
				}

				pCollision->InvalidatePrims();
			}

			iVar6 = iVar6 + 1;
		}
	}
	return;
}

void CActorBridge::BehaviourManage(byte param_2, CWindForce* pWindForce)
{
	void* pvVar1;
	CActorMovingPlatform* pMovingPlatform;
	CCollision* pMovingPlatformCollision;
	bool bVar2;
	Timer* pTVar3;
	StateConfig* pAVar4;
	int iVar5;
	uint uVar6;
	float* pfVar7;
	undefined4* puVar8;
	int* piVar9;
	int iVar10;
	int iVar11;
	float* pfVar12;
	undefined4* puVar13;
	float* pfVar14;
	int* piVar15;
	int unaff_s0_lo;
	float* pfVar16;
	int iVar17;
	float* pfVar18;
	undefined4 uVar19;
	undefined4 uVar20;
	float fVar21;
	undefined4 uVar22;
	float fVar23;
	float fVar24;
	float fVar25;
	float fVar26;
	float fVar27;
	float fVar28;
	float fVar29;
	float fVar30;
	float fVar31;
	CActorsTable local_1a0;
	edF32VECTOR3 local_90;
	edF32VECTOR4 local_80;
	edF32VECTOR4 local_70;
	edF32VECTOR4 local_60;
	float local_50;
	float local_4c;
	float local_48;
	undefined4 local_44;
	edF32MATRIX4 auStack64;

	this->verletBridge.aVerletConstraints[0] = &(this->verletBridge).linkConstraint;
	this->verletBridge.nbVerletConstraints = 1;

	iVar5 = this->actorState;
	if (((iVar5 == BRIDGE_BASIC_STATE_SPLIT) || (iVar5 == 7)) && (this->field_0x348 != -1)) {
		(this->openBoxConstraint).bEdgeSliding = 0;

		this->verletBridge.aVerletConstraints[1] = &this->openBoxConstraint;
		this->verletBridge.nbVerletConstraints = 2;
	}

	if (this->actorState != BRIDGE_BASIC_STATE_SPLIT) goto LAB_001e1120;

	ApplyStepsCollisionScale(1);

	this->verletBridge.Split(this->field_0x354);
	SetState(7, -1);

LAB_001e1120:
	if (param_2 == 0) {
		if (pWindForce == (CWindForce*)0x0) {
			this->verletBridge.nbForces = 0;
		}
		else {
			this->verletBridge.aForces[0] = pWindForce;
			this->verletBridge.nbForces = 1;
		}

		this->verletBridge.ApplyForces();
		this->verletBridge.Verlet(Timer::GetTimer()->cutsceneDeltaTime);
		this->verletBridge.ResolveConstraints();
	}

	for (iVar5 = 0; iVar5 < (this->verletBridge).altStepCount; iVar5 = iVar5 + 1) {
		pMovingPlatform = (CActorMovingPlatform*)this->pActorStream->aEntries[iVar5].Get();
		if (pMovingPlatform != (CActorMovingPlatform*)0x0) {
			if ((GetStateFlags(this->actorState) & 1) == 0) {
				this->flags = this->flags | 0x1000;
				if ((this->verletBridge).field_0x68 == false) {
					IMPLEMENTATION_GUARD(
					iVar17 = (this->verletBridge).linkConstraint.field_0x4;
					piVar15 = (int*)(iVar17 + iVar5 * 0x14);
					piVar9 = (int*)(iVar17 + (iVar5 + (this->verletBridge).altStepCount) * 0x14);
					pfVar16 = (float*)((int)this->verletBridge.pBridgeMem + *piVar15 * 0x10);
					local_80.x = *pfVar16;
					local_80.y = pfVar16[1];
					local_80.z = pfVar16[2];
					local_80.w = 0.0;
					pfVar16 = (float*)((int)this->verletBridge.pBridgeMem + piVar15[1] * 0x10);
					local_70.x = *pfVar16;
					local_70.y = pfVar16[1];
					local_70.z = pfVar16[2];
					local_70.w = 0.0;
					pfVar16 = (float*)((int)this->verletBridge.pBridgeMem + *piVar9 * 0x10);
					local_60.x = *pfVar16;
					local_60.y = pfVar16[1];
					local_60.z = pfVar16[2];
					local_60.w = 0.0;
					pfVar16 = (float*)((int)this->verletBridge.pBridgeMem + piVar9[1] * 0x10);
					local_50 = *pfVar16;
					local_4c = pfVar16[1];
					local_48 = pfVar16[2];
					local_44 = 0;
					edF32Vector4SubHard(&auStack64.rowZ, &local_70, &local_80);
					edF32Vector4NormalizeHard(&auStack64.rowZ, &auStack64.rowZ);
					edF32Vector4SubHard(&auStack64.rowX, &local_60, &local_80);
					edF32Vector4CrossProductHard(&auStack64.rowY, &auStack64.rowZ, &auStack64.rowX);
					edF32Vector4NormalizeHard(&auStack64.rowY, &auStack64.rowY);
					edF32Vector4CrossProductHard
					(&auStack64.rowX, &auStack64.rowY, &auStack64.rowZ);
					auStack64.da = (local_80.x + local_70.x + local_60.x + local_50) * 0.25;
					auStack64.db = (local_80.y + local_70.y + local_60.y + local_4c) * 0.25;
					auStack64.dc = (local_80.z + local_70.z + local_60.z + local_48) * 0.25;)
				}
				else {
					S_LINK_SUB_OBJ* pLinkSubObj = (this->verletBridge).linkConstraint.aLinkInfo + iVar5;

					edF32VECTOR4* pVectorData = this->verletBridge.pBridgeMem;

					edF32VECTOR4* pVecA = pVectorData + pLinkSubObj->particleIndexA;
					edF32VECTOR4* pVecB = pVectorData + pLinkSubObj->particleIndexB;

					auStack64.ca = pVecB->x - pVecA->x;
					auStack64.cb = pVecB->y - pVecA->y;
					auStack64.cc = pVecB->z - pVecA->z;
					auStack64.cd = 0.0f;

					edF32Vector4NormalizeHard(&auStack64.rowZ, &auStack64.rowZ);
					edF32Vector4CrossProductHard(&auStack64.rowY, &auStack64.rowZ, &gF32Vector4UnitX);
					edF32Vector4NormalizeHard(&auStack64.rowY, &auStack64.rowY);
					edF32Vector4CrossProductHard(&auStack64.rowX, &auStack64.rowY, &auStack64.rowZ);

					auStack64.rowT.x = (pVecA->x + pVecB->x) * 0.5f;
					auStack64.rowT.y = (pVecA->y + pVecB->y) * 0.5f;
					auStack64.rowT.z = (pVecA->z + pVecB->z) * 0.5f;
				}

				auStack64.dd = 1.0f;
				edF32Matrix4MulF32Matrix4Hard(&auStack64, &auStack64, &this->field_0x2f0);
				edF32Matrix4ToEulerSoft(&auStack64, &local_90, "XYZ");

				this->rotationEuler.xyz = local_90;

				if (param_2 == 0) {
					local_1a0.nbEntries = 0;

					if (this->typeID == MOVING_PLATFORM) {
						pMovingPlatform->Platform_UpdateMatrix(&auStack64, 1, &local_1a0);
						pMovingPlatform->GenericManage(1, 1, -1, -1);
					}
					else {
						pMovingPlatformCollision = this->pCollisionData;
						if (pMovingPlatformCollision == (CCollision*)0x0) {
							pMovingPlatform->UpdatePosition(&auStack64, 1);
						}
						else {
							pMovingPlatformCollision->CheckCollisions_MoveActor(pMovingPlatform, &auStack64, &local_1a0, (CActor*)0x0, 1);
						}
					}

					if (this->actorState != 7) {
						if (local_1a0.nbEntries == 0) {
							fVar23 = 1.0f / this->field_0x33c;
							if ((1 < iVar5) &&
								(this->verletBridge.aParticleInfo[iVar5].inverseMass = fVar23, (this->verletBridge).field_0x68 == false)) {
								(this->verletBridge).aParticleInfo[iVar5 + (this->verletBridge).altStepCount + 1].inverseMass = fVar23;
							}

							if ((iVar5 < (this->verletBridge).altStepCount + -1) && (this->verletBridge.aParticleInfo[iVar5 + 1].inverseMass = fVar23, (this->verletBridge).field_0x68 == false)) {
								this->verletBridge.aParticleInfo[iVar5 + 2 + (this->verletBridge).altStepCount].inverseMass = fVar23;
							}
						}
						else {
							IMPLEMENTATION_GUARD(
							fVar23 = this->field_0x33c;
							for (iVar17 = 0; iVar17 < local_1a0.nbEntries; iVar17 = iVar17 + 1) {
								bVar2 = (*local_1a0.aEntries[iVar17]->pVTable->IsKindOfObject)(local_1a0.aEntries[iVar17], 2);
								if (bVar2 == false) {
									fVar21 = this->field_0x340 * 10.0;
								}
								else {
									fVar21 = CActorMovable::GetSubjectiveCumulatedWeight((CActorMovable*)local_1a0.aEntries[iVar17]);
									fVar21 = this->field_0x340 * fVar21;
								}
								fVar23 = fVar23 + fVar21;
							}

							fVar23 = 1.0f / fVar23;

							if ((1 < iVar5) &&
								(this->verletBridge.aParticleInfo[iVar5 * 8] = fVar23,
									(this->verletBridge).field_0x68 == false)) {
								this->verletBridge.aParticleInfo[(iVar5 + (this->verletBridge).altStepCount) * 8 + 8] = fVar23
									;
							}
							if ((iVar5 < (this->verletBridge).altStepCount + -1) &&
								(this->verletBridge.aParticleInfo[iVar5 * 8 + 8] = fVar23,
									(this->verletBridge).field_0x68 == false)) {
								this->verletBridge.aParticleInfo[(iVar5 + 2 + (this->verletBridge).altStepCount) * 8] = fVar23
									;
							})
						}
					}
				}
				else {
					pMovingPlatform->UpdatePosition(&auStack64, 1);
				}
			}
		}
	}
	return;
}

edF32VECTOR3 edF32VECTOR3_0040ed70 = { -1.5707964f, 0.0f, 0.0f };

void CActorBridge::CBhvBasic::Create(ByteCode* pByteCode)
{
	float fVar1;
	edF32MATRIX4 eStack80;
	edF32VECTOR3 local_10;

	(this->field_0x30).x = pByteCode->GetF32();
	(this->field_0x30).y = pByteCode->GetF32();
	(this->field_0x30).z = pByteCode->GetF32();
	(this->field_0x30).w = 0.0f;

	local_10 = edF32VECTOR3_0040ed70;

	edF32Matrix4FromEulerSoft(&eStack80, &local_10, "XYZ");
	edF32Matrix4MulF32Vector4Hard(&this->field_0x30, &eStack80, &this->field_0x30);
	edF32Vector4SafeNormalize1Hard(&this->field_0x30, &this->field_0x30);

	this->field_0x40 = pByteCode->GetF32();
	this->field_0x44 = pByteCode->GetF32();
	this->field_0x48 = pByteCode->GetF32();
	this->field_0x4c = pByteCode->GetF32();
	this->field_0x50 = pByteCode->GetF32();

	return;
}

void CActorBridge::CBhvBasic::Manage()
{
	float fVar1;
	float fVar2;
	float unaff_f20;

	fVar2 = this->field_0x54;
	if ((fVar2 <= this->field_0x58) && (fVar2 != 0.0f)) {
		fVar2 = fmodf(this->field_0x58, fVar2);
		this->field_0x58 = fVar2;
	}

	if (this->field_0x58 < this->field_0x48) {
		unaff_f20 = this->field_0x40;
	}
	else {
		fVar2 = this->field_0x4c;
		fVar1 = this->field_0x58 - this->field_0x48;
		if (fVar1 < fVar2) {
			if (fVar2 != 0.0f) {
				unaff_f20 = this->field_0x40 + (this->field_0x44 - this->field_0x40) * (fVar1 / fVar2);
			}
		}
		else {
			if (fVar1 - fVar2 < this->field_0x50) {
				unaff_f20 = this->field_0x44;
			}
			else {
				if (fVar2 != 0.0f) {
					unaff_f20 = this->field_0x40 +
						(this->field_0x44 - this->field_0x40) * (((fVar1 - fVar2) - this->field_0x50) / fVar2);
				}
			}
		}
	}

	edF32Vector4ScaleHard(unaff_f20, &this->windForce.force, &this->field_0x30);

	this->field_0x58 = this->field_0x58 + 0.0166666f;

	this->pOwner->BehaviourManage(0, &this->windForce);

	return;
}

void CActorBridge::CBhvBasic::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = reinterpret_cast<CActorBridge*>(pOwner);

	this->field_0x54 = this->field_0x4c + this->field_0x50 + this->field_0x48 + this->field_0x4c;
	this->field_0x58 = 0.0f;

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CActorBridge::CBhvBasic::End(int newBehaviourId)
{
	this->pOwner->ApplyStepsCollisionScale(0);

	return;
}

void CActorBridge::CBhvWindAware::Create(ByteCode* pByteCode)
{
	this->field_0x40 = pByteCode->GetF32();
	this->field_0x44 = pByteCode->GetF32();
	this->field_0x48 = pByteCode->GetF32();
	this->field_0x4c = pByteCode->GetF32();
	this->field_0x50 = pByteCode->GetF32();

	return;
}

void CActorBridge::CBhvWindAware::Manage()
{
	IMPLEMENTATION_GUARD();
}

void CActorBridge::CBhvWindAware::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	IMPLEMENTATION_GUARD();
}

void CActorBridge::CBhvWindAware::End(int newBehaviourId)
{
	IMPLEMENTATION_GUARD();
}

int CActorBridge::CBhvWindAware::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

CVerletBridge::Config::Config(int stepCount, int nbMaxForces, int param_3, bool param_4, bool param_5, bool param_6, edF32VECTOR4* param_7, edF32VECTOR4* param_8, float param_9, float param_10, float param_11)
{
	this->stepCount = stepCount;
	this->nbMaxForces = nbMaxForces;
	this->nbLinkConstraints = param_3;
	this->field_0xc = param_4;
	this->field_0xd = param_5;
	this->field_0xe = param_6;

	this->field_0x10 = *param_7;
	this->field_0x20 = *param_8;

	this->field_0x30 = param_9;
	this->field_0x34 = param_10;
	this->field_0x38 = param_11;
}

void CVerletBridge::Config::ComputeNbParticles(int* param_1, int* param_2)
{
	bool bVar1 = this->field_0xe;
	if (bVar1 == false) {
		int piVar19 = this->stepCount;
		bool cVar14 = this->field_0xd;
		*param_1 = (piVar19 + 1) * 2;
		if (cVar14 != false) {
			*param_1 = *param_1 + 4;
		}
		bool cVar13 = this->field_0xc;
		*param_2 = *param_1;
		if (cVar13 != false) {
			*param_2 = *param_1 + 2;
		}
	}
	else {
		int piVar19 = this->stepCount;
		bool cVar14 = this->field_0xd;
		*param_1 = piVar19 + 1;
		if (cVar14 != false) {
			*param_1 = piVar19 + 3;
		}
		bool cVar13 = this->field_0xc;
		*param_2 = *param_1;
		if (cVar13 != false) {
			*param_2 = *param_1 + 1;
		}
	}
}

void CVerletBridge::Config::ComputeNbLinks(int* param_1, int* param_2)
{
	int piVar19 = this->stepCount;
	bool bVar1 = this->field_0xe;
	bool cVar13 = this->field_0xc;
	bool cVar14 = this->field_0xd;

	if (bVar1 == false) {
		*param_1 = piVar19 * 4 + -3;
		if (cVar14 != false) {
			*param_1 = piVar19 * 4 + 1;
		}
		*param_2 = *param_1;
		if (cVar13 != false) {
			*param_2 = *param_1 + 1;
		}
	}
	else {
		*param_1 = piVar19;
		*param_2 = piVar19;
		if (cVar14 != false) {
			*param_1 = piVar19 + 2;
			*param_2 = *param_1;
		}
	}
}

// Should be in: D:/Projects/b-witch/ActorBridge.h
void CVerletBridge::Split(int splitLinkIndex)
{
	int maxParticleInfo;
	int altParticleIndex;

	if (splitLinkIndex < 0) {
		maxParticleInfo = this->altStepCount;
		splitLinkIndex = maxParticleInfo >> 1;
		if (maxParticleInfo < 0) {
			splitLinkIndex = maxParticleInfo + 1 >> 1;
		}
	}

	if (this->field_0x69 == false) {
		if (splitLinkIndex < 0) {
			splitLinkIndex = 0;
		}
		else {
			maxParticleInfo = this->altStepCount + -1;
			if (maxParticleInfo < splitLinkIndex) {
				splitLinkIndex = maxParticleInfo;
			}
		}
	}
	else {
		if (splitLinkIndex < 1) {
			splitLinkIndex = 1;
		}
		else {
			maxParticleInfo = this->altStepCount + -2;
			if (maxParticleInfo < splitLinkIndex) {
				splitLinkIndex = maxParticleInfo;
			}
		}
	}

	if (this->field_0x68 == false) {
		maxParticleInfo = this->nbParticleInfo;
		if (maxParticleInfo + 2 <= this->stepCount) {
			this->nbParticleInfo = maxParticleInfo + 2;
			altParticleIndex = maxParticleInfo + 1;
			goto LAB_001e0e08;
		}
	}
	else {
		maxParticleInfo = this->nbParticleInfo;

		if (maxParticleInfo + 1 <= this->stepCount) {
			this->nbParticleInfo = maxParticleInfo + 1;
		LAB_001e0e08:
			edF32VECTOR4* pPos = this->field_0x1c;
			edF32VECTOR4* pSplitPos = pPos + splitLinkIndex;
			pPos = pPos + maxParticleInfo;

			*pPos = *pSplitPos;

			pPos = this->pBridgeMem;
			pSplitPos = pPos + splitLinkIndex;
			pPos = pPos + maxParticleInfo;

			*pPos = *pSplitPos;

			CVParticleInfo* pCVar16 = this->aParticleInfo;
			CVParticleInfo*  pCVar9 = pCVar16 + splitLinkIndex + 1;
			pCVar16 = pCVar16 + maxParticleInfo;
			pCVar16->inverseMass = pCVar9->inverseMass;
			pCVar16->accumulatedForce = pCVar9->accumulatedForce;

			if (this->field_0x68 == false) {
				pPos = this->pBridgeMem;
				pSplitPos = pPos + altParticleIndex;
				pPos = pPos + splitLinkIndex + this->altStepCount + 1;

				*pSplitPos = *pPos;

				pPos = this->field_0x1c;
				pSplitPos = pPos + altParticleIndex;
				pPos = pPos + splitLinkIndex + this->altStepCount + 1;
				*pSplitPos = *pPos;

				pCVar16 = this->aParticleInfo;
				pCVar9 = pCVar16 + altParticleIndex;
				pCVar16 = pCVar16 + splitLinkIndex + this->altStepCount + 1;
				pCVar9->inverseMass = pCVar16->inverseMass;
				pCVar9->accumulatedForce = pCVar16->accumulatedForce;
			}

			S_LINK_SUB_OBJ* pSplitLink = this->linkConstraint.aLinkInfo + splitLinkIndex;
			if (pSplitLink->particleIndexA == splitLinkIndex) {
				pSplitLink->particleIndexA = maxParticleInfo;
			}

			if (pSplitLink->particleIndexB == splitLinkIndex) {
				pSplitLink->particleIndexB = maxParticleInfo;
			}

			if (this->field_0x68 == false) {
				int iVar10 = splitLinkIndex + this->altStepCount;
				int iVar12 = iVar10 + 1;

				pSplitLink = this->linkConstraint.aLinkInfo + iVar10;
				if (pSplitLink->particleIndexA == iVar12) {
					pSplitLink->particleIndexA = altParticleIndex;
				}

				if (pSplitLink->particleIndexB == iVar12) {
					pSplitLink->particleIndexB = altParticleIndex;
				}

				pSplitLink = this->linkConstraint.aLinkInfo + splitLinkIndex + this->altStepCount * 3 + -2;
				if (pSplitLink->particleIndexA == splitLinkIndex) {
					pSplitLink->particleIndexA = maxParticleInfo;
				}

				if (pSplitLink->particleIndexB == splitLinkIndex) {
					pSplitLink->particleIndexB = maxParticleInfo;
				}

				S_LINK_SUB_OBJ* pSrcLink = this->linkConstraint.aLinkInfo;
				S_LINK_SUB_OBJ* pDestLink = pSrcLink + this->linkConstraint.nbLinkInfo;
				pSrcLink = pSrcLink + splitLinkIndex + this->altStepCount * 2 + -1;
				pDestLink->particleIndexA = pSrcLink->particleIndexA;
				pDestLink->particleIndexB = pSrcLink->particleIndexB;
				pDestLink->squaredRestDistance = pSrcLink->squaredRestDistance;
				pDestLink->field_0xc = pSrcLink->field_0xc;
				pDestLink->iterationCount = pSrcLink->iterationCount;

				S_LINK_SUB_OBJ* pLastLink = this->linkConstraint.aLinkInfo + this->linkConstraint.nbLinkInfo;

				if (pLastLink->particleIndexA == splitLinkIndex) {
					pLastLink->particleIndexA = maxParticleInfo;
				}

				if (pLastLink->particleIndexB == splitLinkIndex) {
					pLastLink->particleIndexB = maxParticleInfo;
				}

				splitLinkIndex = splitLinkIndex + this->altStepCount + 1;
				pLastLink = this->linkConstraint.aLinkInfo + this->linkConstraint.nbLinkInfo;

				if (pLastLink->particleIndexA == splitLinkIndex) {
					pLastLink->particleIndexA = altParticleIndex;
				}

				if (pLastLink->particleIndexB == splitLinkIndex) {
					pLastLink->particleIndexB = altParticleIndex;
				}

				this->linkConstraint.nbLinkInfo = this->linkConstraint.nbLinkInfo + 1;
			}
		}
	}

	return;
}

void CVerletBridge::ApplyForces()
{
	CVParticleInfo* pSubObjIt = this->aParticleInfo;
	CVParticleInfo* pSubObjEnd = pSubObjIt + this->nbParticleInfo;
	for (;pSubObjIt < pSubObjEnd; pSubObjIt = pSubObjIt + 1) {
		float fVar23 = pSubObjIt->inverseMass;
		if (fVar23 == 0.0f) {
			pSubObjIt->accumulatedForce = gF32Vertex4Zero;
		}
		else {
			if (fVar23 == 0.0f) {
				pSubObjIt->accumulatedForce = gF32Vertex4Zero;
			}
			else {
				edF32Vector4ScaleHard(1.0f / fVar23, &pSubObjIt->accumulatedForce, &this->gravityForce);
			}
		}
	}

	for (int i = 0; i < this->nbForces; i = i + 1) {
		this->aForces[i]->ApplyForce(this, this->aParticleInfo, this->nbParticleInfo);
	}

	return;
}

void CVerletBridge::Verlet(float param_1)
{
	edF32VECTOR4* pMemIt = this->pBridgeMem;
	edF32VECTOR4* pVecBIt = this->field_0x1c;
	CVParticleInfo* pSubObjIt = this->aParticleInfo;
	edF32VECTOR4* pMemEnd = pMemIt + this->nbParticleInfo;
	edF32VECTOR4* pCurVecB = pVecBIt;

	for (; pMemIt < pMemEnd; pMemIt = pMemIt + 1) {
		if (pSubObjIt->inverseMass == 0.0f) {
			*pCurVecB = *pMemIt;
		}
		else {
			*pCurVecB = (*pMemIt - *pVecBIt) + *pMemIt;

			float fVar20 = pSubObjIt->inverseMass * param_1 * param_1;
			*pCurVecB = *pCurVecB + pSubObjIt->accumulatedForce * fVar20;
			pCurVecB->w = 1.0f;
		}

		pVecBIt = pVecBIt + 1;
		pCurVecB = pCurVecB + 1;
		pSubObjIt = pSubObjIt + 1;
	}

	pMemIt = this->pBridgeMem;
	this->pBridgeMem = this->field_0x1c;
	this->field_0x1c = pMemIt;

	return;
}

void CVerletBridge::ResolveConstraints()
{
	int iVar6 = 1;

	for (int i = 0; i < iVar6; i = i + 1) {
		for (int j = 0; j < this->nbVerletConstraints; j = j + 1)
		{
			int iVar13 = this->aVerletConstraints[j]->iterationPriority;
			if (iVar6 < iVar13) {
				iVar6 = iVar13;
			}

			this->aVerletConstraints[j]->Resolve(this, this->pBridgeMem, this->nbParticleInfo);
		}
	}

	return;
}

CVerletParticles::CVerletParticles()
{
	this->aVerletConstraints = (CVerletConstraint**)0x0;
	this->aForces = 0;
	this->stepCount = 0;
	this->nbParticleInfo = 0;

	this->gravityForce.x = 0.0f;
	this->gravityForce.y = -9.81f;
	this->gravityForce.z = 0.0f;
	this->gravityForce.w = 0.0f;

	return;
}

void* CVerletParticles::Initialize(void* pBridgeMem, int stepCount, int nbMaxForces, int nbLinkConstraints)
{
	CVParticleInfo* pEnd;
	CVParticleInfo* pCurParticleSubObj;

	this->stepCount = stepCount;
	this->nbParticleInfo = 0;

	this->pBridgeMem = reinterpret_cast<edF32VECTOR4*>(pBridgeMem);
	this->field_0x1c = this->pBridgeMem + stepCount;

	this->aParticleInfo = reinterpret_cast<CVParticleInfo*>(this->field_0x1c + stepCount);
	this->aForces = reinterpret_cast<CVerletForceGen**>(this->aParticleInfo + stepCount);

	this->aVerletConstraints = reinterpret_cast<CVerletConstraint**>(this->aForces + nbMaxForces);

	this->nbForces = 0;
	this->nbVerletConstraints = 0;

	pCurParticleSubObj = this->aParticleInfo;
	pEnd = pCurParticleSubObj + stepCount;

	// #UnrolledLoop
	if (pCurParticleSubObj < pEnd) {
		while (pCurParticleSubObj < pEnd) {
			pCurParticleSubObj->inverseMass = 1.0f;
			pCurParticleSubObj = pCurParticleSubObj + 1;
		}
	}

	return reinterpret_cast<void*>(this->aVerletConstraints + nbLinkConstraints);
}

COpenBoxConstraint::COpenBoxConstraint()
{
	this->bUseTransform = 0;
	this->bEdgeSliding = 0;

	this->minBounds.x = -0.5f;
	this->minBounds.y = -0.5f;
	this->minBounds.z = -0.5f;
	this->minBounds.w = 0.0f;

	this->maxBounds.x = 0.5f;
	this->maxBounds.y = 0.5f;
	this->maxBounds.z = 0.5f;
	this->maxBounds.w = 0.0f;

	this->transformMatrix = gF32Matrix4Unit;

	return;
}

void COpenBoxConstraint::Resolve(CVerletParticles* pParticles, edF32VECTOR4* pPositions, int nbPositions)
{
	// Flag indicating whether position was clamped to bounds
	bool wasPositionClamped;

	// Pointer to the end of the positions array
	edF32VECTOR4* pPositionsEnd;

	// Temporary values for clamping coordinates
	float minBoundValue;
	float maxBoundValue;
	float currentCoordValue;

	// Position in local/transformed space
	edF32VECTOR4 transformedPosition;

	// Calculate end pointer for positions array iteration
	pPositionsEnd = pPositions + nbPositions;

	if (this->bEdgeSliding == 0) {
		if (this->bUseTransform == 0) {
			// Simple AABB clamp without transformation
			for (; pPositions < pPositionsEnd; pPositions = pPositions + 1) {
				// Only process positions below the upper Y bound
				if (pPositions->y < (this->maxBounds).y) {
					// Clamp X coordinate to bounds
					minBoundValue = (this->minBounds).x;
					currentCoordValue = pPositions->x;
					if ((minBoundValue <= currentCoordValue) && (minBoundValue = (this->maxBounds).x, currentCoordValue <= minBoundValue)) {
						minBoundValue = currentCoordValue;
					}
					pPositions->x = minBoundValue;

					// Clamp Y coordinate to lower bound only
					minBoundValue = (this->minBounds).y;
					if (pPositions->y < minBoundValue) {
						pPositions->y = minBoundValue;
					}

					// Clamp Z coordinate to bounds
					minBoundValue = (this->minBounds).z;
					currentCoordValue = pPositions->z;
					if ((minBoundValue <= currentCoordValue) && (minBoundValue = (this->maxBounds).z, currentCoordValue <= minBoundValue)) {
						minBoundValue = currentCoordValue;
					}
					pPositions->z = minBoundValue;
				}
			}
		}
		else {
			// Transform-based AABB clamp (position space to local space)
			if (pPositions < pPositionsEnd) {
				do {
					// Ensure homogeneous coordinate is set for matrix multiplication
					pPositions->w = 1.0f;

					// Transform position to local constraint space
					edF32Matrix4MulF32Vector4Hard(&transformedPosition, &this->transformMatrix, pPositions);

					// Only process positions below the upper Y bound in local space
					if (transformedPosition.y < (this->maxBounds).y) {
						// Initialize clamp flag (true if any coordinate was clamped)
						minBoundValue = (this->minBounds).x;
						wasPositionClamped = false;

						// Clamp X coordinate
						if (transformedPosition.x < minBoundValue) {
							wasPositionClamped = true;
							transformedPosition.x = minBoundValue;
						}
						else {
							minBoundValue = (this->maxBounds).x;
							if (minBoundValue < transformedPosition.x) {
								wasPositionClamped = true;
								transformedPosition.x = minBoundValue;
							}
						}

						// Clamp Y coordinate
						minBoundValue = (this->minBounds).y;
						if (transformedPosition.y < minBoundValue) {
							wasPositionClamped = true;
							transformedPosition.y = minBoundValue;
						}

						// Clamp Z coordinate
						minBoundValue = (this->minBounds).z;
						if (transformedPosition.z < minBoundValue) {
							wasPositionClamped = true;
							transformedPosition.z = minBoundValue;
						}
						else {
							minBoundValue = (this->maxBounds).z;
							if (minBoundValue < transformedPosition.z) {
								wasPositionClamped = true;
								transformedPosition.z = minBoundValue;
							}
						}

						// If any coordinate was clamped, transform back to world space
						if (wasPositionClamped) {
							edF32Matrix4MulF32Vector4Hard(pPositions, &this->inverseTransformMatrix, &transformedPosition);
							pPositions->w = 1.0f;
						}
					}

					pPositions = pPositions + 1;
				} while (pPositions < pPositionsEnd);
			}
		}
	}
	else {
		// Edge sliding mode: smoothly interpolate near edges instead of hard clamps
		if (pPositions < pPositionsEnd) {
			do {
				// Only process positions below the upper Y bound
				if (pPositions->y < (this->maxBounds).y) {
					// Handle Z coordinate with edge sliding
					currentCoordValue = (this->minBounds).z;
					float zCoordinate = pPositions->z;

					if (zCoordinate < currentCoordValue) {
						// Below minimum Z: hard clamp
						pPositions->z = currentCoordValue;
					}
					else {
						maxBoundValue = (this->maxBounds).z;
						if (maxBoundValue < zCoordinate) {
							// Above maximum Z: hard clamp
							pPositions->z = maxBoundValue;
						}
						else {
							// Within bounds: check if near edges for smooth interpolation
							float distanceToMinZ = zCoordinate - currentCoordValue;
							if (distanceToMinZ < 0.01f) {
								// Near minimum Z edge: smoothly push inward
								pPositions->z = currentCoordValue + distanceToMinZ * 0.9f;
							}
							else {
								float distanceToMaxZ = maxBoundValue - zCoordinate;
								if (distanceToMaxZ < 0.01f) {
									// Near maximum Z edge: smoothly push inward
									pPositions->z = maxBoundValue - distanceToMaxZ * 0.9f;
								}
							}
						}
					}

					// Clamp Y coordinate to lower bound only
					minBoundValue = (this->minBounds).y;
					if (pPositions->y < minBoundValue) {
						pPositions->y = minBoundValue;
					}

					// Clamp X coordinate to bounds (standard hard clamp)
					minBoundValue = (this->minBounds).x;
					currentCoordValue = pPositions->x;
					if ((minBoundValue <= currentCoordValue) && (minBoundValue = (this->maxBounds).x, currentCoordValue <= minBoundValue)) {
						minBoundValue = currentCoordValue;
					}
					pPositions->x = minBoundValue;
				}

				pPositions = pPositions + 1;
			} while (pPositions < pPositionsEnd);
		}
	}

	return;
}

CVerletConstraint::CVerletConstraint()
{
	this->iterationPriority = 1;

	return;
}

CLinkConstraint::CLinkConstraint()
{
	this->aLinkInfo = (S_LINK_SUB_OBJ*)0x0;
	this->nbLinkInfo = 0x0;
	this->field_0x10 = 0;
}

void CLinkConstraint::Resolve(CVerletParticles* pParticles, edF32VECTOR4* pPositions, int nbPositions)
{
	// Pointers to particle positions for the two ends of the current link
	edF32VECTOR4* pPosA;
	edF32VECTOR4* pPosB;

	// Iterators for traversing the link constraint array
	S_LINK_SUB_OBJ* pLinkEnd;
	S_LINK_SUB_OBJ* pLinkIt;

	// Mass (inverse mass) values for particles A and B
	// squaredRestDistance in S_LINK_SUB_OBJ stores the squared rest distance
	float invMassA;
	float invMassB;

	// Correction factor that determines how much to move each particle
	// to enforce the distance constraint
	float correctionFactor;

	// Vector representing the current distance offset between particles
	edF32VECTOR4 deltaPos;

	// Temporary accumulator for scaled correction vectors
	edF32VECTOR4 correction;

	// Initialize correction accumulator to zero
	correction = gF32Vector4Zero;

	// Set up iteration bounds for all link constraints
	pLinkIt = this->aLinkInfo;
	pLinkEnd = pLinkIt + this->nbLinkInfo;

	// Process each link constraint
	for (; pLinkIt < pLinkEnd; pLinkIt = pLinkIt + 1) {
		// Get the two particle indices and retrieve their positions
		pPosA = pPositions + pLinkIt->particleIndexA;
		invMassA = pParticles->aParticleInfo[pLinkIt->particleIndexA].inverseMass;

		pPosB = pPositions + pLinkIt->particleIndexB;
		invMassB = pParticles->aParticleInfo[pLinkIt->particleIndexB].inverseMass;

		// Calculate the vector from particle A to particle B
		deltaPos = *pPosB - *pPosA;

		// Verlet constraint resolution formula:
		// correctionFactor = (1 - restDistance² / currentDistance²) / (invMassA + invMassB)
		// where:
		//   - pLinkIt->squaredRestDistance is the squared rest distance
		//   - deltaPos squared is the current distance squared
		// This ensures the particles stay at the correct distance apart
		correctionFactor = (1.0f - (pLinkIt->squaredRestDistance + pLinkIt->squaredRestDistance) /
			(deltaPos.x * deltaPos.x + deltaPos.y * deltaPos.y + deltaPos.z * deltaPos.z +
				pLinkIt->squaredRestDistance)) / (invMassA + invMassB);

		// Apply constraint correction to particle A (if it's not static)
		if (invMassA != 0.0f) {
			edF32Vector4ScaleHard(correctionFactor * invMassA, &correction, &deltaPos);
			*pPosA = *pPosA + correction;
		}

		// Apply constraint correction to particle B (if it's not static)
		if (invMassB != 0.0f) {
			edF32Vector4ScaleHard(correctionFactor * invMassB, &correction, &deltaPos);
			*pPosB = *pPosB - correction;
		}
	}

	return;
}

void CWindForce::ApplyForce(CVerletParticlesBase* pParticles, CVParticleInfo* pInfo, int nbInfo)
{
	CVParticleInfo* pInfoEnd;

	pInfoEnd = pInfo + nbInfo;
	if (pInfo < pInfoEnd) {
		do {
			edF32Vector4AddHard(&pInfo->accumulatedForce, &pInfo->accumulatedForce, &this->force);
			pInfo = pInfo + 1;
		} while (pInfo < pInfoEnd);
	}

	return;
}
