#include "CameraFightData.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "CameraGame.h"
#include "TimeController.h"
#include "CollisionRay.h"
#include "InputManager.h"
#include "ActorCommander.h"

CamUpdate CamUpdate_0049c760;

void CCamFigData::Create(ByteCode* pByteCode)
{
	float fVar1;
	float fVar2;
	float* pfVar3;
	int iVar4;
	float fVar5;
	float fVar6;
	float local_40[14];

	iVar4 = 0;
	pfVar3 = local_40;
	do {
		fVar5 = pByteCode->GetF32();
		iVar4 = iVar4 + 1;
		*pfVar3 = fVar5;
		pfVar3 = pfVar3 + 1;
	} while (iVar4 < 0xe);

	this->field_0x4 = local_40[0] * 0.01745329f;

	fVar5 = local_40[2] * 0.01745329f;
	(this->angleBetaDyn).field_0x14 = fVar5;
	(this->angleBetaDyn).field_0x18 = fVar5 * 3.0f;
	(this->angleDynA).field_0x14 = fVar5 * 0.5f;
	(this->angleDynA).field_0x18 = fVar5 * 3.0f * 0.5f;

	this->field_0x0 = local_40[3] * 0.01745329f;
	this->field_0x8 = local_40[1];
	this->field_0xc = local_40[4];
	this->field_0x10 = local_40[5];
	this->field_0x14 = local_40[6];

	(this->distanceDyn).field_0x14 = local_40[7];
	(this->distanceDyn).field_0x18 = local_40[7] * 3.0f;

	(this->boundingSphereDyn).field_0x14 = local_40[7];
	(this->boundingSphereDyn).field_0x18 = local_40[7] * 3.0f;

	(this->distanceDyn).field_0xc = local_40[8];

	this->field_0x18 = local_40[9];
	this->field_0x1c = local_40[10];
	this->field_0x20 = 0.2f;
	this->field_0x24 = 0.25f;

	fVar6 = local_40[11] * 3.0;
	this->vertexDynExtA.Init(0, &gF32Vertex4Zero);
	this->vertexDynExtE.Init(0, &gF32Vertex4Zero);
	this->vertexDynExtD.Init(0, &gF32Vertex4Zero);


	this->boundingSphereDyn.Init(0.0f, 0.0f);

	this->field_0x2e0 = gF32Vertex4Zero;
	this->field_0x250 = gF32Vertex4Zero;

	this->vertexDynExtC.Init(0, &gF32Vertex4Zero);

	this->field_0x28 = 0.5f;

	(this->vertexDynExtA).field_0x14 = local_40[11];
	(this->vertexDynExtA).field_0x18 = fVar6;

	(this->vertexDynExtE).field_0x14 = local_40[11];
	(this->vertexDynExtE).field_0x18 = fVar6;

	(this->vertexDynExtD).field_0x14 = local_40[11];
	(this->vertexDynExtD).field_0x18 = fVar6;

	(this->vertexDynExtC).field_0x14 = local_40[11];
	(this->vertexDynExtC).field_0x18 = fVar6;

	(this->vertexDynExtA).field_0xc = local_40[12];
	(this->vertexDynExtD).field_0xc = local_40[13];

	this->field_0x290 = gF32Vector4Zero;

	return;
}

void CCamFigData::IsValid(undefined4 param_2)
{
	this->field_0x2a0 = param_2;
	return;
}

void CCamFigData::SetBoundingSphere(ed_Bound_Sphere* pBoundSphere)
{
	(this->boundingSphereDyn).field_0x20.xyz = pBoundSphere->xyz;
	(this->boundingSphereDyn).field_0x20.w = 1.0f;

	(this->boundingSphereDyn).field_0x30 = pBoundSphere->radius;

	this->vertexDynExtE.MoveTo(&(this->boundingSphereDyn).field_0x20);
	this->boundingSphereDyn.MoveTo((this->boundingSphereDyn).field_0x30);

	return;
}

void CCamFigData::SetAdversaryPos(edF32VECTOR4* pPosition)
{
	edF32VECTOR4 adjustedPos;

	edF32Vector4AddHard(&adjustedPos, pPosition, &this->field_0x290);
	this->adversaryPos = adjustedPos;
	this->vertexDynExtD.MoveTo(&adjustedPos);

	return;
}

void CCamFigData::SetPosParam(edF32VECTOR4* pPosition)
{
	this->vertexDynExtA.Init(0, pPosition);
	this->vertexDynExtE.Init(0, pPosition);
	this->vertexDynExtD.Init(0, pPosition);

	this->boundingSphereDyn.Init(0.0f, 0.0f);

	this->field_0x2e0 = *pPosition;
	this->field_0x250 = *pPosition;

	this->vertexDynExtC.Init(0, pPosition);

	return;
}

void CCamFigData::Cam_SetDefault(float param_1, float param_2, CCameraGame* pCamera)
{
	CActor* pTarget;
	undefined4* puVar2;
	uint uVar3;
	edF32VECTOR4* pPosition;
	float fVar4;
	float fVar5;
	float fVar6;

	pTarget = pCamera->GetTarget();
	this->pCamera = pCamera;

	pPosition = &pTarget->currentLocation;

	this->distanceDyn.Init(pCamera->GetDistance(), 0.0f);
	this->angleDynA.Init(0.1963495f, 0.0f);

	this->field_0x4c = 0.1963495f;
	this->field_0x2c4 = 0.1963495f;

	this->angleBetaDyn.Init(pCamera->GetAngleBeta(), 0.0f);

	this->speedDyn.Init(0.0f, 6.283185f);

	this->vertexDynExtB.Init(0, &(pCamera->transformationMatrix).rowT);
	(this->vertexDynExtB).field_0x14 = param_1;
	(this->vertexDynExtB).field_0x18 = param_2;

	this->vertexDynExtA.Init(0, pPosition);
	this->vertexDynExtD.Init(0, pPosition);

	this->adversaryPos = *pPosition;

	this->vertexDynExtE.Init(0, pPosition);


	(this->boundingSphereDyn).field_0x20 = *pPosition;
	this->boundingSphereDyn.Init(0.0f, 0.0f);
	(this->boundingSphereDyn).field_0x30 = 0.0f;
	this->field_0x260 = 0;
	//uVar3 = 0;
	//DAT_0049c78a = 0x23;
	//puVar2 = &DAT_0049c760;
	//DAT_0049c89c = 0;
	//DAT_0049c784 = 0;
	//DAT_0049c786 = 0;
	//DAT_0049c788 = 0;
	//do {
	//	puVar2[0x51] = 0x7149f2ca;
	//	puVar2[0x53] = 0x7149f2ca;
	//	uVar3 = uVar3 + 7;
	//	puVar2[0x55] = 0x7149f2ca;
	//	puVar2[0x57] = 0x7149f2ca;
	//	puVar2[0x59] = 0x7149f2ca;
	//	puVar2[0x5b] = 0x7149f2ca;
	//	puVar2[0x5d] = 0x7149f2ca;
	//	puVar2 = puVar2 + 0xe;
	//} while (uVar3 < 0x23);
	return;
}

struct CamUpdateBuffer
{
	int field_0x0;
	float field_0x4;
};

cam_temp_struct* CCamFigData::Cam_Update(edF32VECTOR4* param_2, int param_3)
{
	int iVar1;
	bool bVar2;
	bool bVar3;
	edF32VECTOR3* peVar4;
	CamUpdateBuffer* pBuffer;
	CActor* pCVar6;
	edF32VECTOR3* peVar7;
	CPlayerInput* lVar9;
	edF32VECTOR3* peVar10;
	edF32VECTOR4* v0;
	ushort uVar11;
	short sVar12;
	uint uVar13;
	float fVar14;
	uint uVar15;
	uint uVar16;
	float fVar17;
	float fVar18;
	CCollisionRay CStack512;
	undefined4 local_1e0;
	undefined4 uStack476;
	float fStack472;
	float fStack468;
	undefined auStack464[64];
	edF32VECTOR4 local_190;
	CCollisionRay CStack384;
	undefined4 local_160;
	undefined4 uStack348;
	float fStack344;
	float fStack340;
	undefined auStack336[64];
	edF32VECTOR4 local_110;
	CCollisionRay CStack256;
	edF32VECTOR4 eStack224;
	edF32VECTOR4 local_d0;
	CCollisionRay CStack192;
	edF32VECTOR4 eStack160;
	edF32VECTOR4 local_90;
	edF32MATRIX4 eStack128;
	undefined4 local_40;
	float local_3c;
	float local_38;
	float local_34;
	edF32VECTOR4 eStack48;
	edF32VECTOR3 local_20;
	edF32VECTOR3 local_10;

	pBuffer = (CamUpdateBuffer*)gSP_Manager.GetFreeBuffer(sizeof(CamUpdateBuffer));
	fVar18 = 0.0f;
	pCVar6 = this->pCamera->GetTarget();
	lVar9 = pCVar6->GetInputManager(0, 0);
	if ((lVar9 != 0) && (0.3f < fabs(lVar9->aAnalogSticks[1].x))) {
		this->field_0x2ac = 1;

		if (0.0f <= lVar9->aAnalogSticks[1].x) {
			fVar18 = 1.3f;
		}
		else {
			fVar18 = -1.3f;
		}
	}

	this->speedDyn.Update(fVar18);

	edF32Vector4AddHard(&eStack48, param_2, &this->field_0x290);

	this->vertexDynExtC.Init(0, &eStack48);

	if (param_3 != 0) {
		CamUpdate_0049c760.field_0x14 = (this->angleBetaDyn).field_0x4;
		CamUpdate_0049c760.field_0x10 = (this->angleDynA).field_0x4;
		CamUpdate_0049c760.maxDistance = this->field_0x10;
		CamUpdate_0049c760.field_0x18 = this->field_0xc;
		CamUpdate_0049c760.FUN_003c4a40(&(this->vertexDynExtA).field_0x20);

		this->field_0x2d0 = this->field_0x2d0 + 1 & 1;

		ComputeData();

		this->vertexDynExtA.MoveTo(&this->field_0x2e0);

		if (fVar18 == 0.0f) {
			if (1 < this->field_0x2b4) {
				IMPLEMENTATION_GUARD(
				DetermineBetaAndWay();)
			}
		}
		else {
			this->field_0x2b4 = 0;
			this->field_0x2b0 = 0;
			this->field_0x2b8 = 1;
			this->field_0x2bc = 1;
			this->field_0x2c0 = 0;
		}

		pBuffer->field_0x0 = 0;
		if (this->field_0x2b4 != 0) {
			pBuffer->field_0x0 = 1;
		}

		if ((pBuffer->field_0x0 != 0) && (this->field_0x2bc == 1)) {
			this->field_0x2bc = this->field_0x2b8;
			this->field_0x2b8 = 1;
			this->field_0x2b0 = this->field_0x2b4;
			this->field_0x2b4 = 0;
			this->field_0x2a4 = 1;
			this->field_0x2cc = (this->distanceDyn).field_0x4;
			this->field_0x2c0 = 0;
			IMPLEMENTATION_GUARD(
			CancelCurrentMovements();)
		}

		if (this->field_0x2a4 != 0) {
			this->field_0x2c8 = this->field_0x2f8;
			this->field_0x2a4 = 0;
		}

		if (this->field_0x2d0 == 0) {
			local_d0 = this->adversaryPos;

			edF32Vector4SubHard(&eStack224, &this->pCamera->transformationMatrix.rowT, &local_d0);
			CCollisionRay CStack256 =CCollisionRay(edF32Vector4NormalizeHard(&eStack224, &eStack224), &local_d0, &eStack224);
			this->field_0x28c = CStack256.Intersect(3, this->pCamera->GetTarget(), (CActor*)0x0, 0x40000004, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
		}
		else {
			local_90 = (this->vertexDynExtC).field_0x20;

			edF32Vector4SubHard(&eStack160, &this->pCamera->transformationMatrix.rowT, &local_90);
			CCollisionRay CStack192 = CCollisionRay(edF32Vector4NormalizeHard(&eStack160, &eStack160), &local_90, &eStack160);
			this->field_0x288 = CStack192.Intersect(3, this->pCamera->GetTarget(), (CActor*)0x0, 0x40000004, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
		}

		iVar1 = this->field_0x2bc;
		if ((iVar1 == 3) || (iVar1 == 2)) {
			fVar14 = edF32GetAnglesDelta((this->angleBetaDyn).field_0x4, this->field_0x300);
			pBuffer->field_0x4 = fVar14;
			if (0.0f <= fVar14) {
				fVar14 = 0.2617994f;
			}
			else {
				fVar14 = -0.2617994f;
			}
			fVar14 = edF32Between_2Pi(fVar14 + (this->angleBetaDyn).field_0x4);
			fVar14 = edF32Between_0_2Pi_Incr(fVar14);

			bVar3 = false;
			bVar2 = false;
			if ((this->field_0x2b0 < 2) && (this->field_0xc <= this->field_0x2cc)) {
				bVar2 = true;
			}

			IMPLEMENTATION_GUARD(
			if ((bVar2) && ((float)(&DAT_0049c8a4)[(int)((fVar14 * 35.0f) / 6.283185f) * 2] <= this->field_0xc)) {
				bVar3 = true;
			})

			if (((bVar3) || (fVar18 != 0.0f)) || (fabs(pBuffer->field_0x4) <= 0.01f)) {
				if (fabs(pBuffer->field_0x4) <= 0.01f) {
					this->field_0x2b0 = 0;
				}

				this->field_0x2bc = 1;
			}
		}
		else {
			if ((iVar1 == 1) || (iVar1 == 0)) {
				this->field_0x2a4 = 1;
				this->field_0x300 = (this->angleBetaDyn).field_0x4;
				if (fVar18 == 0.0f) {
					IMPLEMENTATION_GUARD(
					if ((this->field_0x2b4 == 0) || (this->field_0x2b4 == 1)) {
						fVar14 = (this->angleBetaDyn).field_0x4;
						fVar17 = fVar14 + 0.3141593f;
						fVar14 = edF32Between_0_2Pi_Incr(fVar14 - 0.3141593f);
						uVar13 = 0;
						uVar15 = (uint)((fVar14 * 35.0f) / 6.283185f);
						while (true) {
							sVar12 = 0;
							if (((ushort)(&DAT_0049c788)[uVar13 * 4] <= uVar15) && (uVar15 < (ushort)(&DAT_0049c78a)[uVar13 * 4])) {
								sVar12 = 1;
							}
							if (((&DAT_0049c786)[uVar13 * 4] != sVar12) || (DAT_0049c780 <= uVar13)) break;
							uVar13 = uVar13 + 1;
						}
						if (uVar13 == DAT_0049c780) {
							uVar13 = 0;
						}
						fVar14 = edF32Between_0_2Pi_Incr(fVar17);
						uVar15 = 0;
						uVar16 = (uint)((fVar14 * 35.0f) / 6.283185f);
						while (true) {
							sVar12 = 0;
							if (((ushort)(&DAT_0049c788)[uVar15 * 4] <= uVar16) && (uVar16 < (ushort)(&DAT_0049c78a)[uVar15 * 4])) {
								sVar12 = 1;
							}
							if (((&DAT_0049c786)[uVar15 * 4] != sVar12) || (DAT_0049c780 <= uVar15)) break;
							uVar15 = uVar15 + 1;
						}
						if (uVar15 == DAT_0049c780) {
							uVar15 = 0;
						}
						uVar16 = uVar13;
						if (uVar15 < uVar13) {
							uVar16 = uVar15;
							uVar15 = uVar13;
						}
						uVar11 = (&DAT_0049c784)[uVar16 * 4];
						while (uVar16 < uVar15) {
							uVar16 = uVar16 + 1;
							uVar11 = uVar11 | (&DAT_0049c784)[uVar16 * 4];
						}
						if (uVar11 == 0) {
							if ((this->field_0x288 != 1e+30f) || (this->field_0x28c != 1e+30f)) {
								this->field_0x2b4 = 3;
								this->field_0x2c0 = 0.0f;
							}
						}
						else {
							fVar14 = edF32Between_0_2Pi_Incr((this->angleBetaDyn).base.field_0x4);
							uVar13 = 0;
							uVar15 = (uint)((fVar14 * 35.0) / 6.283185);
							while (true) {
								sVar12 = 0;
								if (((ushort)(&DAT_0049c788)[uVar13 * 4] <= uVar15) && (uVar15 < (ushort)(&DAT_0049c78a)[uVar13 * 4])) {
									sVar12 = 1;
								}
								if (((&DAT_0049c786)[uVar13 * 4] != sVar12) || (DAT_0049c780 <= uVar13)) break;
								uVar13 = uVar13 + 1;
							}
							if (uVar13 == DAT_0049c780) {
								uVar13 = 0;
							}
							fVar14 = (float)FUN_00193e30(((float)(uint)(ushort)(&DAT_0049c788)[uVar13 * 4] * 6.283185) / 35.0,
								((float)(uint)(ushort)(&DAT_0049c78a)[uVar13 * 4] * 6.283185) / 35.0);
							if ((&DAT_0049c786)[uVar13 * 4] != 0) {
								fVar14 = 6.283185 - fVar14;
							}
							if (0.6283185 < fVar14) {
								this->field_0x2b4 = 2;
								this->field_0x2c0 = 0.0f;
							}
						}
					}
					if ((this->field_0x2bc == 1) && ((int)this->field_0x2b4 < 2)) {
						if ((fVar18 != 0.0) ||
							((ABS(this->field_0x26c) <= this->field_0x0 || (ABS(this->field_0x270) <= this->field_0x0)))) {
							this->field_0x2b4 = 0;
							this->field_0x2c0 = 0.0f;
						}
						else {
							if ((this->field_0x260 & 3) != 0) {
								local_40 = 1;
								local_3c = (this->angleDynA).base.field_0x4;
								local_34 = this->field_0x264;
								if ((this->field_0x260 & 1) == 0) {
									local_38 = edF32Between_2Pi(this->field_0x270 + (this->angleBetaDyn).base.field_0x4);
								}
								else {
									local_38 = edF32Between_2Pi(this->field_0x26c + (this->angleBetaDyn).base.field_0x4);
								}
								bVar2 = false;
								fVar18 = edF32Between_0_2Pi_Incr(local_38);
								if (DAT_0049c778 <= (float)(&DAT_0049c8a4)[(int)((fVar18 * 35.0) / 6.283185) * 2]) {
									local_110.x = (this->vertexDynExtC).field_0x20.x;
									local_110.y = (this->vertexDynExtC).field_0x20.y;
									local_110.z = (this->vertexDynExtC).field_0x20.z;
									local_110.w = (this->vertexDynExtC).field_0x20.w;
									peVar7 = (edF32VECTOR3*)0xc;
									local_160 = (undefined4)gF32Vector4UnitY._0_8_;
									uStack348 = (undefined4)((ulong)gF32Vector4UnitY._0_8_ >> 0x20);
									fStack344 = gF32Vector4UnitY.z;
									fStack340 = gF32Vector4UnitY.w;
									peVar10 = &local_10;
									peVar4 = peVar10;
									while (peVar4 != (edF32VECTOR3*)0x0) {
										*(undefined*)&peVar10->x = 0;
										peVar10 = (edF32VECTOR3*)((int)&peVar10->x + 1);
										peVar7 = (edF32VECTOR3*)((int)&peVar7[-1].z + 3);
										peVar4 = peVar7;
									}
									local_10.x = local_3c;
									local_10.y = local_38;
									edF32Matrix4FromEulerSoft((edF32MATRIX4*)auStack336, &local_10, (char*)&PTR_DAT_004375c0);
									edF32Vector4SubHard((edF32VECTOR4*)(auStack336 + 0x20), &gF32Vector4Zero,
										(edF32VECTOR4*)(auStack336 + 0x20));
									CCollisionRay::CCollisionRay(local_34, &CStack384, &local_110, (edF32VECTOR4*)(auStack336 + 0x20));
									pCVar6 = (*((this->pCamera->view).camera.objBase.pVTable)->GetTarget)((CCamera*)this->pCamera);
									fVar18 = CCollisionRay::Intersect
									(&CStack384, 3, pCVar6, (CActor*)0x0, 0x40000004, (edF32VECTOR4*)0x0,
										(_ray_info_out*)0x0);
									if (this->field_0xc <= fVar18) {
										local_190.x = (this->adversaryPos).x;
										local_190.y = (this->adversaryPos).y;
										local_190.z = (this->adversaryPos).z;
										local_190.w = (this->adversaryPos).w;
										peVar7 = (edF32VECTOR3*)0xc;
										local_1e0 = (undefined4)gF32Vector4UnitY._0_8_;
										uStack476 = (undefined4)((ulong)gF32Vector4UnitY._0_8_ >> 0x20);
										fStack472 = gF32Vector4UnitY.z;
										fStack468 = gF32Vector4UnitY.w;
										peVar10 = &local_20;
										peVar4 = peVar10;
										while (peVar4 != (edF32VECTOR3*)0x0) {
											*(undefined*)&peVar10->x = 0;
											peVar10 = (edF32VECTOR3*)((int)&peVar10->x + 1);
											peVar7 = (edF32VECTOR3*)((int)&peVar7[-1].z + 3);
											peVar4 = peVar7;
										}
										local_20.x = local_3c;
										local_20.y = local_38;
										edF32Matrix4FromEulerSoft((edF32MATRIX4*)auStack464, &local_20, (char*)&PTR_DAT_004375c0);
										edF32Vector4SubHard((edF32VECTOR4*)(auStack464 + 0x20), &gF32Vector4Zero,
											(edF32VECTOR4*)(auStack464 + 0x20));
										CCollisionRay::CCollisionRay(local_34, &CStack512, &local_190, (edF32VECTOR4*)(auStack464 + 0x20));
										pCVar6 = (*((this->pCamera->view).camera.objBase.pVTable)->GetTarget)((CCamera*)this->pCamera);
										fVar18 = CCollisionRay::Intersect
										(&CStack512, 3, pCVar6, (CActor*)0x0, 0x40000004, (edF32VECTOR4*)0x0,
											(_ray_info_out*)0x0);
										if (this->field_0xc <= fVar18) {
											bVar2 = true;
										}
									}
								}
								if (bVar2) {
									this->field_0x2c0 = this->field_0x2c0 + GetTimer()->lastFrameTime;
								}
							}
						}
					}

					if (this->field_0x8 <= this->field_0x2c0) {
						this->field_0x2b4 = 1;
						this->field_0x2c0 = 0.0f;
						if (this->pCommanderRef_0x230->typeID == 0x13) {
							CActorCommander* pCommander = static_cast<CActorCommander*>(this->pCommanderRef_0x230);
							pCommander->squad.chessboard.field_0x220 = 1;
						}
					})
				}
			}
		}

		this->angleDynA.MoveTo(this->field_0x4c, 0);

		uVar13 = (uint)(this->field_0x2bc == 2);
		if (this->field_0x2bc == 3) {
			uVar13 = 2;
		}

		this->angleBetaDyn.MoveTo(this->field_0x300 + (this->speedDyn).currentAlpha, uVar13);
		this->distanceDyn.MoveTo(this->field_0x2f0);

		_UpdateCameraData();

		if (this->pCommanderRef_0x230->typeID == 0x13) {
			CActorCommander* pCommander = static_cast<CActorCommander*>(this->pCommanderRef_0x230);
			pCommander->field_0x6d0 = (this->angleBetaDyn).field_0x1c != 0 ^ 1;
		}
	}

	(this->camTemp).pVector = &(this->vertexDynExtA).field_0x20;
	(this->camTemp).field_0x4 = &(this->vertexDynExtB).field_0x20;
	(this->camTemp).field_0x8 = (this->angleDynA).field_0x4;

	fVar18 = edF32Between_0_2Pi_Incr((this->angleBetaDyn).field_0x4);
	(this->camTemp).field_0xc = fVar18;
	(this->camTemp).field_0x10 = (this->distanceDyn).field_0x4;
	gSP_Manager.ReleaseBuffer(pBuffer);

	if (this->pCamera->pOtherTarget != (CActor*)0x0) {
		this->pCamera->FillThisFrameExpectedDifferentialMatrix(&eStack128);

		IMPLEMENTATION_GUARD(
		this->vertexDynExtA.FUN_00213b90(&eStack128);
		this->vertexDynExtD.FUN_00213b90(&eStack128);
		this->vertexDynExtE.FUN_00213b90(&eStack128);)
		edF32Matrix4MulF32Vector4Hard(&this->adversaryPos, &eStack128, &this->adversaryPos);
		v0 = &(this->boundingSphereDyn).field_0x20;
		edF32Matrix4MulF32Vector4Hard(v0, &eStack128, v0);
	}
	return &this->camTemp;
}

struct ComputeDataBuffer
{
	edF32VECTOR4 field_0x40;
	edF32VECTOR4 field_0x50;

	float field_0x70;
};

void CCamFigData::ComputeData()
{
	ComputeDataBuffer* pComputeDataBuffer;
	ushort uVar2;
	edF32VECTOR4* v1;
	edF32VECTOR4* v2;
	uint uVar3;
	edF32VECTOR4* v2_00;
	float fVar4;
	float fVar5;
	uint uVar6;
	uint uVar7;
	float fVar8;
	float fVar9;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 local_30;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	pComputeDataBuffer = (ComputeDataBuffer*)gSP_Manager.GetFreeBuffer(sizeof(ComputeDataBuffer));

	v1 = &(this->vertexDynExtC).field_0x20;
	edF32Vector4LERPHard(this->field_0x28, &local_10, v1, &this->adversaryPos);
	edF32Vector4LERPHard(this->field_0x20, &local_10, &local_10, &(this->boundingSphereDyn).field_0x20);
	local_10.w = 1.0f;
	this->field_0x250.xyz = local_10.xyz;
	this->field_0x250.w = 1.0f;

	edF32Vector4SubHard(&pComputeDataBuffer->field_0x50, &this->adversaryPos, v1);
	this->field_0x268 = GetAngleYFromVector(&pComputeDataBuffer->field_0x50);

	fVar9 = (this->boundingSphereDyn).field_0x30;
	edF32Vector4SubHard(&eStack32, &(this->boundingSphereDyn).field_0x20, &this->field_0x250);
	eStack32.y = 0.0f;
	this->field_0x264 = edFIntervalLERP(fVar9 + this->field_0x24 * edF32Vector4GetDistHard(&eStack32), this->field_0x18, this->field_0x1c, this->field_0xc, this->field_0x10);
	
	pComputeDataBuffer->field_0x40 = this->field_0x250;
	pComputeDataBuffer->field_0x40.y = 0.0f;

	this->field_0x26c = edF32GetAnglesDelta((this->angleBetaDyn).field_0x4, this->field_0x268 - this->field_0x4);
	this->field_0x270 = edF32GetAnglesDelta((this->angleBetaDyn).field_0x4, this->field_0x268 + this->field_0x4);

	if (this->field_0x270 < this->field_0x26c) {
		pComputeDataBuffer->field_0x70 = this->field_0x26c;
		this->field_0x26c = this->field_0x270;
		this->field_0x270 = pComputeDataBuffer->field_0x70;
	}

	this->field_0x260 = 0;
	fVar9 = this->field_0x26c;
	fVar4 = (this->angleBetaDyn).field_0x4;
	if (0.0f <= fVar9) {
		fVar8 = 0.3141593f;
	}
	else {
		fVar8 = -0.3141593f;
	}
	if (0.0f <= fVar9) {
		fVar5 = 0.3141593f;
	}
	else {
		fVar5 = -0.3141593f;
	}

	fVar5 = edF32Between_0_2Pi_Incr((fVar9 + fVar4) - fVar5);
	uVar3 = CamUpdate_0049c760.FUN_Count(fVar5);

	fVar4 = edF32Between_0_2Pi_Incr(fVar8 + fVar9 + fVar4);
	uVar6 = CamUpdate_0049c760.FUN_Count(fVar4);

	uVar7 = uVar6;
	if (uVar6 < uVar3) {
		uVar7 = uVar3;
		uVar3 = uVar6;
	}

	uVar2 = CamUpdate_0049c760.field_0x24[uVar3].field_0x0;
	for (; uVar3 < uVar7; uVar3 = uVar3 + 1) {
		uVar2 = uVar2 | CamUpdate_0049c760.field_0x24[uVar3 + 1].field_0x0;
	}

	if (uVar2 == 0) {
		this->field_0x260 = this->field_0x260 | 1;
	}

	fVar9 = this->field_0x270;
	fVar4 = (this->angleBetaDyn).field_0x4;
	if (0.0f <= fVar9) {
		fVar8 = 0.3141593f;
	}
	else {
		fVar8 = -0.3141593f;
	}
	if (0.0f <= fVar9) {
		fVar5 = 0.3141593f;
	}
	else {
		fVar5 = -0.3141593f;
	}

	fVar5 = edF32Between_0_2Pi_Incr((fVar9 + fVar4) - fVar5);
	uVar3 = CamUpdate_0049c760.FUN_Count(fVar5);

	fVar4 = edF32Between_0_2Pi_Incr(fVar8 + fVar9 + fVar4);
	uVar6 = CamUpdate_0049c760.FUN_Count(fVar4);

	uVar7 = uVar6;
	if (uVar6 < uVar3) {
		uVar7 = uVar3;
		uVar3 = uVar6;
	}

	uVar2 = CamUpdate_0049c760.field_0x24[uVar3].field_0x0;
	for (; uVar3 < uVar7; uVar3 = uVar3 + 1) {
		uVar2 = uVar2 | CamUpdate_0049c760.field_0x24[uVar3 + 1].field_0x0;
	}

	if (uVar2 == 0) {
		this->field_0x260 = this->field_0x260 | 2;
	}

	v2_00 = &(this->vertexDynExtD).field_0x20;
	v2 = &(this->vertexDynExtE).field_0x20;
	edF32Vector4LERPHard(this->field_0x28, &local_30, v1, v2_00);
	edF32Vector4LERPHard(this->field_0x20, &local_30, &local_30, v2);
	local_30.w = 1.0f;

	this->field_0x2e0.xyz = local_30.xyz;
	(this->field_0x2e0).w = 1.0f;

	edF32Vector4SubHard(&pComputeDataBuffer->field_0x50, v2_00, v1);
	this->field_0x2f8 = GetAngleYFromVector(&pComputeDataBuffer->field_0x50);
	fVar9 = (this->boundingSphereDyn).field_0x4;
	edF32Vector4SubHard(&eStack64, v2, &this->field_0x2e0);
	eStack64.y = 0.0f;
	this->field_0x2f0 = edFIntervalLERP(fVar9 + this->field_0x24 * edF32Vector4GetDistHard(&eStack64), this->field_0x18, this->field_0x1c, this->field_0xc, this->field_0x10);
	gSP_Manager.ReleaseBuffer(pComputeDataBuffer);

	return;
}

float CCamFigData::SV_TestCollisionSphere(ParCamFigLAB* param_2, edF32VECTOR4* param_3)
{
	edF32VECTOR4 local_a0;
	edF32VECTOR4 local_90;
	edF32VECTOR4 eStack112;
	edF32VECTOR3 local_10;
	edF32MATRIX4 auStack128;

	local_a0 = gF32Vector4UnitY;

	local_10 = {};

	local_10.x = param_2->field_0x4;
	local_10.y = param_2->field_0x8;

	edF32Matrix4FromEulerSoft(&auStack128, &local_10, "XYZ");
	edF32VECTOR4* v0 = &auStack128.rowY;
	edF32Vector4SubHard(v0, &gF32Vector4Zero, v0);

	if (param_2->field_0x0 == 0) {
		local_90 = this->field_0x2e0;
	}
	else {
		local_90 = this->field_0x250;
	}

	edF32Vector4ScaleHard(param_2->field_0xc - 0.8f, &eStack112, v0);
	edF32Vector4AddHard(&local_90, &local_90, &eStack112);

	CCollisionRay CStack192 = CCollisionRay(1.6f, &local_90, v0);

	float fVar4 = CStack192.Intersect(3, this->pCamera->GetTarget(), (CActor*)0x0, 0x40000004, &local_a0, (_ray_info_out*)0x0);

	if (param_3 != (edF32VECTOR4*)0x0) {
		*param_3 = auStack128.rowZ;
	}

	float puVar5 = -1.0f;
	if (fVar4 != 1e+30f) {
		puVar5 = edF32Vector4DotProductHard(&local_a0, v0);
	}

	if (0.0f <= puVar5) {
		fVar4 = 1e+30f;
	}

	return fVar4;
}

void CCamFigData::_UpdateCameraData()
{
	edF32VECTOR3* peVar1;
	edF32VECTOR3* peVar2;
	CActor* pActor;
	edF32VECTOR3* peVar3;
	edF32VECTOR4* v0;
	float fVar4;
	float puVar5;
	float puVar6;
	edF32VECTOR4 local_a0;
	edF32VECTOR4 local_90;
	edF32MATRIX4 auStack128;
	ParCamFigLAB local_40;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR3 local_10;

	local_40.field_0x0 = 0;
	local_40.field_0x4 = (this->angleDynA).field_0x4;
	local_40.field_0x8 = (this->angleBetaDyn).field_0x4;
	local_40.field_0xc = (this->distanceDyn).field_0x4;

	local_a0 = gF32Vector4UnitY;

	local_10 = {};

	local_10.x = local_40.field_0x4;
	local_10.y = local_40.field_0x8;

	edF32Matrix4FromEulerSoft(&auStack128, &local_10, "XYZ");
	v0 = &auStack128.rowY;
	edF32Vector4SubHard(v0, &gF32Vector4Zero, v0);

	if (local_40.field_0x0 == 0) {
		local_90 = this->field_0x2e0;
	}
	else {
		local_90 = this->field_0x250;
	}

	CCollisionRay CStack192 = CCollisionRay(local_40.field_0xc + 0.8f, &local_90, v0);
	pActor = this->pCamera->GetTarget();
	fVar4 = CStack192.Intersect(3, pActor, (CActor*)0x0, 0x40000004, &local_a0, (_ray_info_out*)0x0);
	puVar5 = -1.0f;
	if (fVar4 != 1e+30f) {
		puVar5 = edF32Vector4DotProductHard(&local_a0, v0);
	}

	if (0.0f <= puVar5) {
		fVar4 = 1e+30f;
	}

	this->field_0x284 = fVar4;

	fVar4 = SV_TestCollisionSphere(&local_40, &eStack32);
	this->field_0x280 = fVar4;

	if ((this->field_0x284 != 1e+30f) || (this->field_0x280 != 1e+30f)) {
		if (this->field_0x280 == 1e+30f) {
			puVar5 = 0.8f;
			puVar6 = this->field_0x284 - 0.8f;
			this->field_0x2ac = 1;
			if (0.8f <= puVar6) {
				puVar5 = puVar6;
			}

			if (this->field_0x2b0 < 2) {
				this->distanceDyn.Init(puVar5, 0.0f);
			}
		}
		else {
			puVar5 = 0.8f;
			puVar6 = (this->distanceDyn).field_0x4 - (1.6f - this->field_0x280);

			if (0.8f <= puVar6) {
				puVar5 = puVar6;
			}

			this->distanceDyn.Init(puVar5, 0.0f);
		}
	}

	edF32Vector4ScaleHard((this->distanceDyn).field_0x4, &eStack32, &eStack32);
	edF32Vector4AddHard(&eStack48, &(this->vertexDynExtA).field_0x20, &eStack32);

	this->vertexDynExtB.Init(0, &eStack48);

	return;
}

CamUpdate::CamUpdate()
{
	this->position = gF32Vertex4Zero;

	this->field_0x10 = 0.0f;
	this->field_0x14 = 0.0f;
	this->field_0x18 = 0.0f;
	this->maxDistance = 0.0f;

	this->field_0x20 = 0;

	this->activeIndex = 0;
	this->field_0x140 = 5;

	return;
}

void CamUpdate::FUN_003c4a40(edF32VECTOR4* pPosition)
{
	int iVar1;
	bool bVar2;
	CamUpdateSubObjB* pCVar4;
	CamUpdateSubObjB* pCVar5;
	uint uVar6;
	float fVar7;
	float fVar8;
	float fVar9;

	uVar6 = 0;

	this->position = *pPosition;

	pCVar4 = this->field_0x144;
	do {
		uVar6 = uVar6 + 1;
		pCVar4->field_0x4 = pCVar4->field_0x4 + GetTimer()->cutsceneDeltaTime;
		pCVar4 = pCVar4 + 1;
	} while (uVar6 < 0x23);

	uVar6 = 0;
	if (this->field_0x140 != 0) {
		do {
			CalculateIntersection(this->activeIndex);

			if (this->field_0x140 == 0) {
				trap(7);
			}

			uVar6 = uVar6 + 1;
			this->activeIndex = (this->activeIndex + 0x23 / this->field_0x140) % 0x23;
		} while (uVar6 < this->field_0x140);
	}

	uVar6 = 0;
	this->activeIndex = (this->activeIndex + 1) % 0x23;
	this->field_0x20 = 0;
	pCVar4 = this->field_0x144;
	do {
		bVar2 = pCVar4->intersectionDistance < this->field_0x18;
		this->field_0x24[this->field_0x20].field_0x0 = bVar2;
		this->field_0x24[this->field_0x20].field_0x2 = 0;
		this->field_0x24[this->field_0x20].field_0x4 = uVar6;
		pCVar5 = pCVar4;
		for (; (bVar2 == pCVar5->intersectionDistance < this->field_0x18 && (uVar6 < 0x23)); uVar6 = uVar6 + 1) {
			pCVar5 = pCVar5 + 1;
			pCVar4 = pCVar4 + 1;
		}

		iVar1 = this->field_0x20;
		this->field_0x20 = iVar1 + 1;
		this->field_0x24[iVar1].field_0x6 = uVar6;
	} while (uVar6 < 0x23);

	if ((2 < this->field_0x20) &&
		(this->field_0x24[0].field_0x0 == this->field_0x24[this->field_0x20 - 1].field_0x0)) {
		this->field_0x24[0].field_0x4 = this->field_0x24[0].field_0x6;
		this->field_0x24[0].field_0x6 = this->field_0x24[this->field_0x20 + -1].field_0x4;
		this->field_0x24[0].field_0x2 = 1;

		this->field_0x20 = this->field_0x20 + -1;
	}

	return;
}

void CamUpdate::CalculateIntersection(int index)
{
	edF32VECTOR4* v0;
	float fVar1;
	float puVar2;
	edF32VECTOR4 local_60;
	edF32MATRIX4 auStack80;
	edF32VECTOR3 local_10;

	local_60 = gF32Vector4UnitY;

	local_10.x = this->field_0x10;
	if (index < 0) {
		fVar1 = (float)((uint)index >> 1 | index & 1U);
		fVar1 = fVar1 + fVar1;
	}
	else {
		fVar1 = (float)index;
	}

	local_10.z = 0.0f;
	local_10.y = (fVar1 * 6.283185f) / 35.0f;
	edF32Matrix4FromEulerSoft(&auStack80, &local_10, "XYZ");
	v0 = &auStack80.rowZ;
	edF32Vector4SubHard(v0, &gF32Vector4Zero, v0);
	CCollisionRay CStack128 = CCollisionRay(this->maxDistance, &this->position, v0);
	float intersectDistance = CStack128.Intersect(3, (CActor*)0x0, (CActor*)0x0, 0x40000004, &local_60, (_ray_info_out*)0x0);
	puVar2 = -1.0f;
	if (intersectDistance != 1e+30f) {
		puVar2 = edF32Vector4DotProductHard(&local_60, v0);
	}

	if (puVar2 < 0.0f) {
		if (intersectDistance == 1e+30f) {
			intersectDistance = this->maxDistance;
		}

		this->field_0x144[index].intersectionDistance = intersectDistance;
	}
	else {
		this->field_0x144[index].intersectionDistance = this->maxDistance;
	}

	this->field_0x144[index].field_0x4 = 0.0f;
	return;
}

uint CamUpdate::FUN_Count(float param_1)
{
	uint uVar6 = 0;
	CamUpdateSubObjA* pCamUpdate = CamUpdate_0049c760.field_0x24;
	uint uVar7 = (uint)((param_1 * 35.0f) / 6.283185f);
	while (true) {
		short sVar1 = 0;
		if ((pCamUpdate->field_0x4 <= uVar7) && (uVar7 < pCamUpdate->field_0x6)) {
			sVar1 = 1;
		}

		if ((pCamUpdate->field_0x2 != sVar1) || (CamUpdate_0049c760.field_0x20 <= uVar6)) break;

		pCamUpdate = pCamUpdate + 1;
		uVar6 = uVar6 + 1;
	}

	if (uVar6 == CamUpdate_0049c760.field_0x20) {
		uVar6 = 0;
	}

	return uVar6;
}