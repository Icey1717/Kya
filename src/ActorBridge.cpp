#include "ActorBridge.h"

#include <math.h>

#include "MemoryStream.h"
#include "MathOps.h"
#include "CollisionManager.h"
#include "WayPoint.h"
#include "TimeController.h"
#include "ActorMovingPlatform.h"

int CVerletBridge::GetMemRequirement(CVerletBridge::Config* pConfig)
{
	int iVar1;
	bool cVar2;
	bool cVar3;
	int iVar2;
	int iVar3;

	if (pConfig->field_0xe == false) {
		iVar2 = pConfig->stepCount;
		cVar3 = pConfig->field_0xd;
		iVar3 = (iVar2 + 1) * 2;
		if (cVar3 != false) {
			iVar3 = iVar3 + 4;
		}
		cVar2 = pConfig->field_0xc;
		if (cVar2 != false) {
			iVar3 = iVar3 + 2;
		}
	}
	else {
		iVar2 = pConfig->stepCount;
		cVar3 = pConfig->field_0xd;
		iVar3 = iVar2 + 1;
		if (cVar3 != false) {
			iVar3 = iVar2 + 3;
		}
		cVar2 = pConfig->field_0xc;
		if (cVar2 != false) {
			iVar3 = iVar3 + 1;
		}
	}
	if (pConfig->field_0xe == false) {
		iVar1 = iVar2 * 4;
		iVar2 = iVar1 + -3;
		if (cVar3 != false) {
			iVar2 = iVar1 + 1;
		}
		if (cVar2 != false) {
			iVar2 = iVar2 + 1;
		}
	}
	else {
		if (cVar3 != false) {
			iVar2 = iVar2 + 2;
		}
	}
	return (pConfig->field_0x8 + 1) * 4 + iVar3 * 0x40 + pConfig->field_0x4 * 4 + iVar2 * 0x14;
}

void CVerletBridge::Reset(CVerletBridge::Config* pConfig)
{
	bool bVar1;
	edF32VECTOR4* v2;
	int iVar2;
	edF32VECTOR4* pfVar3;
	S_PARTICLE_SUB_OBJ* puVar4;
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

	piVar11 = this->linkConstraint.field_0x4;

	fVar18 = edF32Vector4GetLengthSoft(&eStack16);

	local_20 = gF32Vector4Zero;

	fVar25 = fVar18 + fVar18 * pConfig->field_0x38;

	edF32Vector4ScaleHard(1.0f / (float)this->altStepCount, &local_20, &eStack16);

	peVar13 = (edF32VECTOR4*)this->pBridgeMem;
	pfVar10 = this->field_0x1c;
	puVar4 = this->field_0x20;

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
			puVar4->field_0x0 = 0.03333334f;
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
		iVar2 = 0;
		piVar14 = piVar11;

		do {
			pvVar8 = this->pBridgeMem;

			edF32VECTOR4* pVec = this->pBridgeMem;

			piVar14->field_0x0 = iVar15;
			piVar14->field_0x4 = iVar15 + 1;

			if (pvVar8 != (void*)0x0) {
				edF32Vector4SubHard(&eStack224, pVec + iVar15, pVec + (iVar15 + 1));
				fVar18 = edF32Vector4DotProductHard(&eStack224, &eStack224);
				piVar14->field_0x8 = fVar18;
			}

			piVar14->field_0x10 = 3;

			if (this->field_0x68 == false) {
				iVar12 = this->altStepCount;
				pvVar8 = this->pBridgeMem;
				iVar7 = iVar12 + 2 + iVar15;
				iVar9 = iVar12 + 1 + iVar15;
				piVar16 = piVar11 + (iVar15 + iVar12);
				piVar16->field_0x0 = iVar9;
				piVar16->field_0x4 = iVar7;

				if (pvVar8 != (void*)0x0) {
					edF32Vector4SubHard(&eStack240, pVec + iVar9, pVec + iVar7);
					fVar18 = edF32Vector4DotProductHard(&eStack240, &eStack240);
					piVar16->field_0x8 = fVar18;
				}

				piVar16->field_0x10 = 3;

				if (iVar15 != 0) {
					iVar12 = this->altStepCount;
					pvVar8 = this->pBridgeMem;
					iVar7 = iVar12 + 1 + iVar15;
					piVar16 = piVar11 + (iVar15 + iVar12 * 2 + -1);
					piVar16->field_0x0 = iVar15;
					piVar16->field_0x4 = iVar7;

					if (pvVar8 != (void*)0x0) {
						edF32Vector4SubHard(&eStack256, pVec + iVar15, pVec + iVar7);
						fVar18 = edF32Vector4DotProductHard(&eStack256, &eStack256);
						piVar16->field_0x8 = fVar18;
					}

					piVar16->field_0x10 = 3;
				}

				if (0 < iVar15) {
					iVar12 = this->altStepCount;

					if (iVar15 < iVar12 + -1) {
						iVar7 = iVar12 + 2 + iVar15;
						pvVar8 = this->pBridgeMem;
						piVar16 = piVar11 + (iVar15 + iVar12 * 3 + -2);
						piVar16->field_0x0 = iVar15;
						piVar16->field_0x4 = iVar7;

						if (pvVar8 != (void*)0x0) {
							edF32Vector4SubHard(&eStack272, pVec + iVar15, pVec + iVar7);
							fVar18 = edF32Vector4DotProductHard(&eStack272, &eStack272);
							piVar16->field_0x8 = fVar18;
						}

						piVar16->field_0x10 = 3;
					}
				}
			}

			iVar12 = this->altStepCount;
			iVar15 = iVar15 + 1;
			piVar14 = piVar14 + 1;
			iVar2 = iVar2 + 0x10;
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

		edF32VECTOR4* pVec = reinterpret_cast<edF32VECTOR4*>(this->pBridgeMem);

		piVar14 = piVar11 + iVar12;
		piVar14->field_0x0 = unaff_s7_lo;
		piVar14->field_0x4 = 1;

		if (pvVar8 != (void*)0x0) {
			edF32Vector4SubHard(&eStack288, pVec + unaff_s7_lo, pVec + 1);
			fVar18 = edF32Vector4DotProductHard(&eStack288, &eStack288);
			piVar14->field_0x8 = fVar18;
		}

		piVar14->field_0x10 = 3;

		pvVar8 = this->pBridgeMem;
		piVar14 = piVar11 + (iVar12 + 1);
		iVar15 = this->altStepCount + -1;
		piVar14->field_0x0 = unaff_s7_lo + 1;
		piVar14->field_0x4 = iVar15;

		if (pvVar8 != (void*)0x0) {
			edF32Vector4SubHard(&eStack304, pVec + (unaff_s7_lo + 1), pVec + iVar15);
			fVar18 = edF32Vector4DotProductHard(&eStack304, &eStack304);
			piVar14->field_0x8 = fVar18;
		}

		piVar14->field_0x10 = 3;

		if (this->field_0x68 == false) {
			pvVar8 = this->pBridgeMem;
			piVar14 = piVar11 + (iVar12 + 2);
			iVar15 = this->altStepCount + 2;
			piVar14->field_0x0 = unaff_s7_lo + 2;
			piVar14->field_0x4 = iVar15;
			if (pvVar8 != (void*)0x0) {
				edF32Vector4SubHard(&eStack320, pVec + (unaff_s7_lo + 2), pVec + iVar15);
				fVar18 = edF32Vector4DotProductHard(&eStack320, &eStack320);
				piVar14->field_0x8 = fVar18;
			}

			piVar14->field_0x10 = 3;
			pvVar8 = this->pBridgeMem;
			iVar15 = this->altStepCount;
			piVar11 = piVar11 + (iVar12 + 3);
			piVar11->field_0x0 = unaff_s7_lo + 3;
			piVar11->field_0x4 = iVar15 * 2;

			if (pvVar8 != (void*)0x0) {
				edF32Vector4SubHard(&eStack336, pVec + (unaff_s7_lo + 3), pVec + iVar15 * 2);
				fVar18 = edF32Vector4DotProductHard(&eStack336, &eStack336);
				piVar14->field_0x8 = fVar18;
			}

			piVar14->field_0x10 = 3;
		}
	}

	edF32VECTOR4* pVec = reinterpret_cast<edF32VECTOR4*>(this->pBridgeMem);
	puVar6 = this->field_0x1c;

	this->field_0x20[0].field_0x0 = 0.0f;
	puVar6[0] = pVec[0];

	iVar12 = this->altStepCount;
	this->field_0x20[iVar12].field_0x0 = 0.0f;
	puVar6[iVar12] = pVec[iVar12];

	if (this->field_0x69 != false) {
		this->field_0x20[unaff_s7_lo].field_0x0 = 0.0f;
		puVar6[unaff_s7_lo] = pVec[unaff_s7_lo];

		this->field_0x20[unaff_s7_lo + 1].field_0x0 = 0.0f;
		puVar6[unaff_s7_lo + 1] = pVec[unaff_s7_lo + 1];
	}

	if (this->field_0x68 == false) {
		iVar12 = this->altStepCount + 1;

		this->field_0x20[iVar12].field_0x0 = 0.0f;
		puVar6[iVar12] = pVec[iVar12];
;
		iVar12 = this->altStepCount * 2 + 1;
	
		this->field_0x20[iVar12].field_0x0 = 0.0f;
		puVar6[iVar12] = pVec[iVar12];

		if (this->field_0x69 != false) {
			this->field_0x20[(unaff_s7_lo + 2)].field_0x0 = 0.0f;
			puVar6[unaff_s7_lo + 2] = pVec[unaff_s7_lo + 2];

			this->field_0x20[(unaff_s7_lo + 3)].field_0x0 = 0.0f;
			puVar6[unaff_s7_lo + 3] = pVec[unaff_s7_lo + 3];
		}
	}
}

CActorBridge::CActorBridge()
{
	float fVar1;
	float fVar2;
	float fVar3;
	int iVar4;
	float* pfVar5;
	edF32MATRIX4* peVar6;

	CActor();

	IMPLEMENTATION_GUARD_LOG(
	this->pVTable = (CActorFighterVTable*)&_vt;
	(this->openBoxConstraint).pVTable = &CVerletConstraint::_vt;
	this->field_0x170 = 1;
	peVar6 = &gF32Matrix4Unit;
	(this->openBoxConstraint).pVTable = &COpenBoxConstraint::_vt;
	pfVar5 = (float*)&(this->openBoxConstraint).field_0x3c;
	(this->openBoxConstraint).field_0x2c = 0;
	iVar4 = 8;
	(this->openBoxConstraint).field_0x2d = 0;
	do {
		iVar4 = iVar4 + -1;
		fVar1 = peVar6->ab;
		*pfVar5 = peVar6->aa;
		peVar6 = (edF32MATRIX4*)&peVar6->ac;
		pfVar5[1] = fVar1;
		pfVar5 = pfVar5 + 2;
	} while (0 < iVar4);
	(this->openBoxConstraint).field_0xc = (float)&DAT_bf000000;
	(this->openBoxConstraint).field_0x10 = (float)&DAT_bf000000;
	(this->openBoxConstraint).field_0x14 = (float)&DAT_bf000000;
	(this->openBoxConstraint).field_0x18 = 0.0;
	(this->openBoxConstraint).field_0x1c = 0.5;
	(this->openBoxConstraint).field_0x20 = 0.5;
	(this->openBoxConstraint).field_0x24 = 0.5;
	(this->openBoxConstraint).field_0x28 = 0.0;
	(this->verletBridge).pVTable = (int)CVerletParticles::_vt;
	*(undefined4*)&this->field_0x270 = 0;
	*(undefined4*)&this->field_0x278 = 0;
	*(undefined4*)&this->field_0x284 = 0;
	*(undefined4*)&this->field_0x280 = 0;
	*(undefined4*)&this->field_0x2a0 = 0;
	this->field_0x2a4 = -9.81;
	*(undefined4*)&this->field_0x2a8 = 0;
	*(undefined4*)&this->field_0x2ac = 0;
	(this->verletBridge).pVTable = (int)CVerletBridge::_vt;
	(this->verletBridge).linkConstraint.pVTable = &CVerletConstraint::_vt;
	(this->verletBridge).field_0x10 = 1;
	(this->verletBridge).linkConstraint.pVTable = &CLinkConstraint::_vt;
	(this->verletBridge).linkConstraint.field_0x4 = 0;
	(this->verletBridge).linkConstraint.field_0x8 = (int*)0x0;
	(this->verletBridge).linkConstraint.field_0xc = 0;
	(this->behaviourBasic).pVTable = &CBehaviour::_vt;
	(this->behaviourBasic).pVTable = &CBhvBasic::_vt;
	(this->field_0x370).pVTable = &CVerletForceGen::_vt;
	(this->field_0x370).pVTable = &CWindForce::_vt;
	(this->field_0x370).pVTable = &CPulsedWindForce::_vt;
	*(undefined4*)&this->field_0x3a0 = 0;
	*(undefined4*)&this->field_0x3a4 = 0;
	*(undefined4*)&this->field_0x3a8 = 0x3f800000;
	*(undefined4*)&this->field_0x3ac = 0x3f800000;
	*(undefined4*)&this->field_0x3b0 = 0x3f800000;
	*(undefined4*)&this->field_0x3b8 = 0;
	fVar3 = gF32Vector4Zero.w;
	fVar2 = gF32Vector4Zero.z;
	fVar1 = gF32Vector4Zero.y;
	(this->field_0x390).x = gF32Vector4Zero.x;
	(this->field_0x390).y = fVar1;
	(this->field_0x390).z = fVar2;
	(this->field_0x390).w = fVar3;
	(this->behaviourWindAware).pVTable = &CBehaviour::_vt;
	(this->behaviourWindAware).pVTable = &CBhvWindAware::_vt;
	(this->field_0x3e0).pVTable = &CVerletForceGen::_vt;
	(this->field_0x3e0).pVTable = &CWindForce::_vt;
	(this->field_0x3e0).pVTable = &CPulsedWindForce::_vt;
	*(undefined4*)&this->field_0x410 = 0;
	*(undefined4*)&this->field_0x414 = 0;
	*(undefined4*)&this->field_0x418 = 0x3f800000;
	*(undefined4*)&this->field_0x41c = 0x3f800000;
	*(undefined4*)&this->field_0x420 = 0x3f800000;
	*(undefined4*)&this->field_0x428 = 0;
	fVar3 = gF32Vector4Zero.w;
	fVar2 = gF32Vector4Zero.z;
	fVar1 = gF32Vector4Zero.y;
	(this->field_0x400).x = gF32Vector4Zero.x;
	(this->field_0x400).y = fVar1;
	(this->field_0x400).z = fVar2;
	(this->field_0x400).w = fVar3;)
	return;
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

	fVar6 = pByteCode->GetF32();
	this->field_0x330 = fVar6;
	fVar6 = pByteCode->GetF32();
	this->field_0x334 = fVar6;
	fVar6 = pByteCode->GetF32();
	this->field_0x33c = fVar6;
	fVar6 = pByteCode->GetF32();
	this->field_0x340 = fVar6;
	fVar6 = pByteCode->GetF32();
	this->field_0x338 = fVar6;
	uVar5 = pByteCode->GetU32();
	this->field_0x344 = uVar5;
	uVar5 = pByteCode->GetU32();
	this->field_0x358 = uVar5;
	iVar3 = pByteCode->GetS32();
	this->field_0x348 = iVar3;
	fVar6 = pByteCode->GetF32();
	this->stepCollisionScale = fVar6;

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

AnimResult CActorBridge::gStateCfg_BRG[3] =
{
	AnimResult(0, 0),
	AnimResult(0, 0),
	AnimResult(0, 0),
};

AnimResult* CActorBridge::GetStateCfg(int state)
{
	AnimResult* pAVar1;

	if (state < 5) {
		pAVar1 = CActor::GetStateCfg(state);
	}
	else {
		pAVar1 = CActorBridge::gStateCfg_BRG + state + -5;
	}
	return pAVar1;
}

CBehaviour* CActorBridge::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 3) {
		pBehaviour = &this->behaviourWindAware;
	}
	else {
		if (behaviourType == 2) {
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
	uint uVar2;
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
	int iVar15;
	undefined4* puVar16;
	undefined4* puVar17;
	float fVar18;
	float fVar19;
	float fVar20;
	edF32MATRIX4 local_e0;
	edF32VECTOR4 local_a0;
	edF32VECTOR4 local_90;
	edF32VECTOR4 local_80;
	edF32MATRIX4 eStack112;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	//e_ed_event_prim3d_type eStack4;
	CActor* pCurActor;

	local_20.xyz = this->field_0x160->field_0x0;
	local_20.w = 1.0f;

	local_30.xyz = this->field_0x164->field_0x0;
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

	uVar2 = this->field_0x358;
	fVar19 = this->field_0x338;
	fVar18 = this->field_0x334;
	fVar20 = this->field_0x330;

	if (this->pActorStream == (S_ACTOR_STREAM_REF*)0x0) {
		iVar7 = 0;
	}
	else {
		iVar7 = this->pActorStream->entryCount;
	}

	this->verletBridgeConfig = CVerletBridge::Config(iVar7, 1, 2, (uVar2 & 1) != 0, (uVar2 & 2) != 0, (uVar2 & 4) != 0, &local_90, &local_80, fVar20, fVar18, fVar19);

	iVar7 = CVerletBridge::GetMemRequirement(&this->verletBridgeConfig);
	this->pBridgeMem = new char[iVar7];

	static const edF32VECTOR4 edF32VECTOR4_0040ed60 = { 0.0f, 0.0f, -9.81f, 0.0f };
	local_a0 = edF32VECTOR4_0040ed60;

	edF32Matrix4MulF32Vector4Hard(&local_a0, &eStack112, &local_a0);
	(this->verletBridge).field_0x30 = local_a0;
	(this->verletBridge).field_0x30.w = 0.0f;

	this->verletBridgeConfig.ComputeNbParticles(&iVar7, &iVar15);
	this->verletBridgeConfig.ComputeNbLinks(&piVar12, &piVar8);

	(this->verletBridge).altStepCount = (this->verletBridgeConfig).stepCount;
	(this->verletBridge).field_0x68 = (this->verletBridgeConfig).field_0xe;
	(this->verletBridge).field_0x69 = (this->verletBridgeConfig).field_0xd;

	void* pMem = this->verletBridge.Initialize(pBridgeMem, iVar15, (this->verletBridgeConfig).field_0x4, (this->verletBridgeConfig).field_0x8);

	IMPLEMENTATION_GUARD_LOG(
	this->field_0x280 = iVar7;)

	(this->verletBridge).linkConstraint.field_0xc = piVar8;
	(this->verletBridge).linkConstraint.field_0x8 = 0x0;
	(this->verletBridge).linkConstraint.field_0x4 = (S_LINK_SUB_OBJ*)pMem;
	(this->verletBridge).linkConstraint.field_0x8 = piVar12;

	IMPLEMENTATION_GUARD_LOG(
	peVar14 = &gF32Matrix4Unit;
	(this->verletBridge).field_0x10 = 3;
	peVar10 = &this->field_0x1b0;
	iVar7 = 8;
	*(int**)(*(int*)&this->field_0x270 + this->field_0x274 * 4) = &(this->verletBridge).field_0x10;
	this->field_0x274 = this->field_0x274 + 1;
	(this->verletBridge).field_0x10 = this->field_0x344;
	this->field_0x1a0 = 0;
	this->field_0x1a1 = 0;
	do {
		iVar7 = iVar7 + -1;
		fVar18 = peVar14->ab;
		peVar10->aa = peVar14->aa;
		peVar14 = (edF32MATRIX4*)&peVar14->ac;
		peVar10->ab = fVar18;
		peVar10 = (edF32MATRIX4*)&peVar10->ac;
	} while (0 < iVar7);
	(this->field_0x180).x = (float)&DAT_bf000000;
	(this->field_0x180).y = (float)&DAT_bf000000;
	(this->field_0x180).z = (float)&DAT_bf000000;
	(this->field_0x180).w = 0.0;
	(this->field_0x190).x = 0.5;
	(this->field_0x190).y = 0.5;
	(this->field_0x190).z = 0.5;
	(this->field_0x190).w = 0.0;
	this->field_0x1a1 = 0;
	pCVar6 = CScene::ptable.g_EventManager_006f5080;
	uVar2 = this->field_0x348;
	if (-1 < (int)uVar2) {
		peVar9 = (ed_zone_3d*)0x0;
		if (uVar2 != 0xffffffff) {
			peVar9 = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, uVar2);
		}
		peVar10 = (edF32MATRIX4*)edEventGetChunkZonePrimitive(pCVar6->activeChunkId, peVar9, 0, &eStack4);
		edF32Matrix4MulF32Matrix4Hard(&local_e0, &this->field_0x2f0, peVar10);
		peVar14 = &local_e0;
		this->field_0x1a0 = 1;
		peVar10 = &this->field_0x1b0;
		iVar7 = 8;
		do {
			iVar7 = iVar7 + -1;
			fVar18 = peVar14->ab;
			peVar10->aa = peVar14->aa;
			peVar14 = (edF32MATRIX4*)&peVar14->ac;
			peVar10->ab = fVar18;
			peVar10 = (edF32MATRIX4*)&peVar10->ac;
		} while (0 < iVar7);
		edF32Matrix4GetInverseGaussSoft(&this->field_0x1f0, &this->field_0x1b0);
	})


	if (this->pActorStream == (S_ACTOR_STREAM_REF*)0x0) {
		iVar7 = 0;
	}
	else {
		iVar7 = this->pActorStream->entryCount;
	}

	this->aHeights = new float[iVar7];
	iVar15 = 0;
	iVar7 = 0;
	while (true) {
		pSVar5 = this->pActorStream;
		iVar12 = 0;

		if (pSVar5 != (S_ACTOR_STREAM_REF*)0x0) {
			iVar12 = pSVar5->entryCount;
		}

		if (iVar12 <= iVar15) break;

		pCurActor = pSVar5->aEntries[iVar15].Get();

		if (((pCurActor != (CActor*)0x0) && (pCurActor->pCollisionData != (CCollision*)0x0)) &&
			(peVar4 = pCurActor->pCollisionData->pObbPrim, peVar4 != (edColPRIM_OBJECT*)0x0)) {
			this->aHeights[iVar15] = (peVar4->field_0x90).z;
		}
		iVar7 = iVar7 + 4;
		iVar15 = iVar15 + 1;
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
	AnimResult* pAVar4;
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

	//*(int*)this->verletBridge.field_0x0 = &(this->verletBridge).field_0x50;
	this->verletBridge.field_0x4 = 1;
	iVar5 = this->actorState;
	if (((iVar5 == 6) || (iVar5 == 7)) && (this->field_0x348 != -1)) {
		(this->openBoxConstraint).field_0x31 = 0;

		//((this->verletBridge.field_0x0)->linkConstraint).pVTable = (CLinkConstraintVTable*)&this->openBoxConstraint;
		this->verletBridge.field_0x4 = 2;
	}

	if (this->actorState != 6) goto LAB_001e1120;

	this->verletBridge.Split(this->field_0x354);
	ApplyStepsCollisionScale(1);
	SetState(7, -1);

LAB_001e1120:
	if (param_2 == 0) {
		if (pWindForce == (CWindForce*)0x0) {
			this->verletBridge.field_0xc = 0;
		}
		else {
			//((CWindForce*)this->verletBridge.field_0x8)->field_0x0 = (undefined*)pWindForce;
			this->verletBridge.field_0xc = 1;
		}

		pTVar3 = Timer::GetTimer();
		fVar23 = pTVar3->cutsceneDeltaTime;
		IMPLEMENTATION_GUARD_LOG(
		CVerletParticles::ApplyForces((CVerletParticles*)(this + 0x270));
		CVerletParticles::Verlet((CVerletParticles*)(this + 0x270), fVar10);
		CVerletParticles::ResolveConstraints((CVerletParticles*)(this + 0x270));)
	}

	for (iVar5 = 0; iVar5 < (this->verletBridge).altStepCount; iVar5 = iVar5 + 1) {
		pMovingPlatform = (CActorMovingPlatform*)this->pActorStream->aEntries[iVar5].Get();
		if (pMovingPlatform != (CActorMovingPlatform*)0x0) {
			iVar17 = this->actorState;
			uVar6 = 0;

			if (iVar17 != -1) {
				pAVar4 = pMovingPlatform->GetStateCfg(iVar17);
				uVar6 = pAVar4->flags_0x4;
			}

			if ((uVar6 & 1) == 0) {
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
					S_LINK_SUB_OBJ* pLinkSubObj = (this->verletBridge).linkConstraint.field_0x4 + iVar5;

					edF32VECTOR4* pVectorData = reinterpret_cast<edF32VECTOR4*>(this->verletBridge.pBridgeMem);

					edF32VECTOR4* pVecA = pVectorData + pLinkSubObj->field_0x0;
					edF32VECTOR4* pVecB = pVectorData + pLinkSubObj->field_0x4;

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
					local_1a0.entryCount = 0;

					if (this->typeID == 0xd) {
						IMPLEMENTATION_GUARD(
						pMovingPlatform->Platform_UpdateMatrix(&auStack64, 1, &local_1a0);
						pMovingPlatform->GenericManage(1, 1, -1, -1);)
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
						if (local_1a0.entryCount == 0) {
							fVar23 = 1.0f / this->field_0x33c;
							if ((1 < iVar5) &&
								(this->verletBridge.field_0x20[iVar5].field_0x0 = fVar23, (this->verletBridge).field_0x68 == false)) {
								(this->verletBridge).field_0x20[iVar5 + (this->verletBridge).altStepCount + 1].field_0x0 = fVar23;
							}

							if ((iVar5 < (this->verletBridge).altStepCount + -1) && (this->verletBridge.field_0x20[iVar5 + 1].field_0x0 = fVar23, (this->verletBridge).field_0x68 == false)) {
								this->verletBridge.field_0x20[iVar5 + 2 + (this->verletBridge).altStepCount].field_0x0 = fVar23;
							}
						}
						else {
							IMPLEMENTATION_GUARD(
							fVar23 = this->field_0x33c;
							for (iVar17 = 0; iVar17 < local_1a0.entryCount; iVar17 = iVar17 + 1) {
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
								(this->verletBridge.field_0x20[iVar5 * 8] = fVar23,
									(this->verletBridge).field_0x68 == false)) {
								this->verletBridge.field_0x20[(iVar5 + (this->verletBridge).altStepCount) * 8 + 8] = fVar23
									;
							}
							if ((iVar5 < (this->verletBridge).altStepCount + -1) &&
								(this->verletBridge.field_0x20[iVar5 * 8 + 8] = fVar23,
									(this->verletBridge).field_0x68 == false)) {
								this->verletBridge.field_0x20[(iVar5 + 2 + (this->verletBridge).altStepCount) * 8] = fVar23
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

	fVar1 = pByteCode->GetF32();
	(this->field_0x30).x = fVar1;
	fVar1 = pByteCode->GetF32();
	(this->field_0x30).y = fVar1;
	fVar1 = pByteCode->GetF32();
	(this->field_0x30).z = fVar1;
	(this->field_0x30).w = 0.0f;

	local_10 = edF32VECTOR3_0040ed70;

	edF32Matrix4FromEulerSoft(&eStack80, &local_10, "XYZ");
	edF32Matrix4MulF32Vector4Hard(&this->field_0x30, &eStack80, &this->field_0x30);
	edF32Vector4SafeNormalize1Hard(&this->field_0x30, &this->field_0x30);
	fVar1 = pByteCode->GetF32();
	this->field_0x40 = fVar1;
	fVar1 = pByteCode->GetF32();
	this->field_0x44 = fVar1;
	fVar1 = pByteCode->GetF32();
	this->field_0x48 = fVar1;
	fVar1 = pByteCode->GetF32();
	this->field_0x4c = fVar1;
	fVar1 = pByteCode->GetF32();
	this->field_0x50 = fVar1;
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

	edF32Vector4ScaleHard(unaff_f20, &this->field_0x20, &this->field_0x30);

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

void CActorBridge::CBhvWindAware::Create(ByteCode* pByteCode)
{
	float fVar1;

	fVar1 = pByteCode->GetF32();
	this->field_0x40 = fVar1;
	fVar1 = pByteCode->GetF32();
	this->field_0x44 = fVar1;
	fVar1 = pByteCode->GetF32();
	this->field_0x48 = fVar1;
	fVar1 = pByteCode->GetF32();
	this->field_0x4c = fVar1;
	fVar1 = pByteCode->GetF32();
	this->field_0x50 = fVar1;
}

CVerletBridge::Config::Config(int stepCount, int param_2, int param_3, bool param_4, bool param_5, bool param_6, edF32VECTOR4* param_7, edF32VECTOR4* param_8, float param_9, float param_10, float param_11)
{
	this->stepCount = stepCount;
	this->field_0x4 = param_2;
	this->field_0x8 = param_3;
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

void CVerletBridge::Split(int param_1)
{
	IMPLEMENTATION_GUARD();
}

void* CVerletParticles::Initialize(void* pBridgeMem, int stepCount, int param_3, int param_4)
{
	char* iVar1;
	undefined4* puVar2;
	undefined4* puVar3;

	this->stepCount = stepCount;
	//this->field_0x10 = 0;
	this->pBridgeMem = reinterpret_cast<edF32VECTOR4*>(pBridgeMem);
	this->field_0x1c = this->pBridgeMem + stepCount;
	this->field_0x20 = reinterpret_cast<S_PARTICLE_SUB_OBJ*>(this->field_0x1c + stepCount);
	this->field_0x8 = reinterpret_cast<char*>(this->field_0x20 + stepCount);
	this->field_0x0 = this->field_0x8 + param_3 * 4;
	iVar1 = this->field_0x0;
	//this->field_0xc = 0;
	//this->field_0x4 = 0;
	//puVar3 = (undefined4*)this->field_0x20;
	//puVar2 = puVar3 + stepCount * 8;
	//if (puVar3 < puVar2) {
	//	if (8 < (int)puVar2 + (0x1f - (int)puVar3) >> 5) {
	//		do {
	//			*puVar3 = 0x3f800000;
	//			puVar3[8] = 0x3f800000;
	//			puVar3[0x10] = 0x3f800000;
	//			puVar3[0x18] = 0x3f800000;
	//			puVar3[0x20] = 0x3f800000;
	//			puVar3[0x28] = 0x3f800000;
	//			puVar3[0x30] = 0x3f800000;
	//			puVar3[0x38] = 0x3f800000;
	//			puVar3 = puVar3 + 0x40;
	//		} while (puVar3 < puVar2 + -0x40);
	//	}
	//	for (; puVar3 < puVar2; puVar3 = puVar3 + 8) {
	//		*puVar3 = 0x3f800000;
	//	}
	//}
	return iVar1 + (param_4 + 1) * 4;
}
