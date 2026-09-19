#include "FxLightEmitter.h"
#include "LargeObject.h"
#include "FileManager3D.h"
#include "TimeController.h"
#include "DlistManager.h"
#include "MathOps.h"
#include "CameraViewManager.h"

edF32VECTOR4 edF32VECTOR4_00426890 = { 0.0f, 0.0f, 1.0f, 1.0f };
edF32VECTOR4 edF32VECTOR4_004268a0 = { 0.0f, -0.5f, 0.0f, 1.0f };

byte CFxLightEmitter::_gs8CountId = 0;

CFxLightEmitter::CFxLightEmitter()
{
	this->pRayDef = (RAY_DEF*)0x0;
	this->countId = _gs8CountId;
	_gs8CountId = _gs8CountId + 1;

	return;
}

CFxLightEmitter::~CFxLightEmitter()
{
	_gs8CountId = _gs8CountId - 1;

	return;
}

void CFxLightEmitter::Create(float param_1, uint param_3, uint param_4)
{
	uint uVar1;
	ulong uVar2;
	float fVar3;
	float fVar4;
	undefined4 uVar5;
	undefined4 uVar6;
	float fVar7;
	int iVar9;
	int iVar10;
	int iVar11;

	this->pRayDef = (RAY_DEF*)0x0;
	this->flags = param_4;

	this->field_0x60 = param_1;
	this->field_0xc0 = 0;
	this->nbRayDefs = param_3;
	this->field_0x50 = 0;
	this->field_0x54 = 0;
	this->stallTime = 1;
	this->field_0x68 = 0;

	if ((this->flags & 0x200) == 0) {
		this->pRayDef = new RAY_DEF[this->nbRayDefs];
		iVar11 = 0;
		uVar1 = this->nbRayDefs;
		this->field_0x54 = 0;
		this->field_0x50 = 0;
		if (0 < (int)uVar1) {
			do {
				RAY_DEF* pRayDef = &this->pRayDef[iVar11];
				iVar11 = iVar11 + 1;

				pRayDef->field_0x10 = gF32Vector4UnitZ;
				pRayDef->field_0x2c = 0xffff;
				pRayDef->field_0x2e = 0x3039;
				pRayDef->field_0x30 = 0;
				pRayDef->field_0x33 = 0xff;
				pRayDef->field_0x38 = 0;
				pRayDef->field_0x3c = 0;
			} while (iVar11 < (int)uVar1);
		}
	}
	
	this->flags = this->flags | 1;
	
	this->field_0x5c = 1.0f;
	
	this->field_0x20 = gF32Vector4Zero;
	
	this->field_0xa0 = 0;
	this->field_0xa4 = 0;
	this->flags = this->flags | 0x80;
	this->field_0xb8 = 0.0f;
	this->field_0xbc = 1;
	this->field_0xbe = 1;
	
	this->uvMapping = edF32VECTOR4_00426890;
	this->field_0x10 = edF32VECTOR4_004268a0;
	
	if ((this->flags & 2) != 0) {
		this->direction = g_xVector;
	}
	
	this->field_0xc4 = 0;
	this->aColors[0] = 0x80808080;
	this->aColors[1] = 0x80808080;
	this->aColors[2] = 0x80808080;
	this->aColors[3] = 0x80808080;
	this->flags = this->flags | 1;

	return;
}

void CFxLightEmitter::InitRays(int nbRays)
{
	float fVar1;
	float fVar2;
	float fVar3;
	int iVar4;
	int iVar5;
	int iVar6;

	this->field_0x54 = 0;
	this->field_0x50 = 0;
	iVar6 = 0;
	if (0 < nbRays) {
		iVar5 = 0;
		do {
			RAY_DEF* pRayDef = &this->pRayDef[iVar6];
			iVar6 = iVar6 + 1;
			pRayDef->field_0x10 = gF32Vector4UnitZ;
			pRayDef->field_0x2c = 0xffff;
			pRayDef->field_0x2e = 0x3039;
			pRayDef->field_0x30 = 0;
			pRayDef->field_0x33 = 0xff;
			pRayDef->field_0x38 = 0;
			pRayDef->field_0x3c = 0;
			iVar5 = iVar5 + 0x40;
		} while (iVar6 < nbRays);
	}

	return;
}

void CFxLightEmitter::ResetRays()
{
	uint uVar1;
	RAY_DEF* iVar2;
	int iVar3;
	int iVar4;

	if (this->pRayDef != (RAY_DEF*)0x0) {
		uVar1 = this->nbRayDefs;
		iVar4 = 0;
		this->field_0x54 = 0;
		this->field_0x50 = 0;

		if (0 < (int)uVar1) {
			do {
				RAY_DEF* pRayDef = &this->pRayDef[iVar4];
				if (this->countId == *(char*)&pRayDef->field_0x33) {
					pRayDef->field_0x30 = 0;
					pRayDef->field_0x33 = 0xff;
					pRayDef->field_0x2c = 0xffff;
				}
				iVar4 = iVar4 + 1;
			} while (iVar4 < (int)uVar1);
		}
	}

	return;
}

void CFxLightEmitter::ChangeUvMapping(edF32VECTOR4* pUvMapping)
{
	this->uvMapping = *pUvMapping;

	return;
}

void CFxLightEmitter::ChangeUvSpeed(float param_1, short param_3, short param_4)
{
	this->flags = this->flags | 0x80;
	this->field_0xb8 = param_1;
	this->field_0xbc = param_3;
	this->field_0xbe = param_4;

	return;
}

void CFxLightEmitter::ChangeColor(int index, uint newColor)
{
	this->aColors[index & 0xffff] = newColor;
	return;
}

void CFxLightEmitter::SetSharedRays(RAY_DEF* pSharedRays)
{
	this->pRayDef = pSharedRays;
	return;
}

void CFxLightEmitter::ChangeMaterialId(int mainMaterialId, int param_3)
{
	this->materialId = mainMaterialId;
	this->field_0x98 = param_3;

	if (mainMaterialId == -1) {
		this->nbMaterials = 0;
	}
	else {
		this->nbMaterials = CScene::ptable.g_C3DFileManager_00451664->GetNbMaterialInG2D(mainMaterialId);
	}

	if (this->nbMaterials <= this->field_0x98) {
		this->field_0x98 = 0;
	}

	return;
}

void CFxLightEmitter::ChangeRadius(float rMin, float rMax)
{
	this->radiusMin = rMin;
	this->radiusMax = rMax;

	return;
}

void CFxLightEmitter::ChangeLengthAndWidth(float param_1, float param_2, float param_3, float param_4)
{
	this->field_0x64 = param_1;
	this->field_0x88 = param_2;
	this->field_0x8c = param_3;
	this->field_0x90 = param_4;

	return;
}

void CFxLightEmitter::ChangeSpeedAndAccel(float param_1, float param_2, float param_3, float param_4)
{
	this->field_0x78 = param_1;
	this->field_0x7c = param_2;
	this->field_0x80 = param_3;
	this->field_0x84 = param_4;

	return;
}

void CFxLightEmitter::ChangeColors(uint newColor)
{
	this->aColors[0] = newColor;
	this->aColors[1] = newColor;
	this->aColors[2] = newColor;
	this->aColors[3] = newColor;

	return;
}

void CFxLightEmitter::ChangeStallTime(float stalltime)
{
	if (GetTimer()->frameTime == 0.0f) {
		this->stallTime = 1;
	}
	else {
		this->stallTime = (int)(stalltime / GetTimer()->frameTime);
	}

	if (this->stallTime < 1) {
		this->stallTime = 1;
	}

	return;
}

void CFxLightEmitter::ChangeDirection(edF32VECTOR4* pDirection)
{
	this->direction = *pDirection;

	return;
}

void CFxLightEmitter::ChangeAlphaFactor(float alphaFactor)
{
	if (alphaFactor == 0.0f) {
		this->flags = this->flags & 0xfffffffe;
	}
	else {
		this->flags = this->flags | 1;
	}

	this->alphaFactor = alphaFactor;

	return;
}

bool CFxLightEmitter::Manage(edF32VECTOR4*, undefined4)
{
	IMPLEMENTATION_GUARD_FX();
	return true;
}

void CFxLightEmitter::GenerateNewOne(edF32VECTOR4* param_2)
{
	IMPLEMENTATION_GUARD_FX();
}

void CFxLightEmitter::Enable(int bEnable)
{
	if (bEnable == 0) {
		this->flags = this->flags & 0xfffffffe;
	}
	else {
		this->flags = this->flags | 1;
	}

	return;
}

void CFxLightEmitter::ReleaseRays(int nbRays)
{
	RAY_DEF* iVar1;
	int iVar2;
	int iVar3;

	this->field_0x54 = 0;
	this->field_0x50 = 0;
	iVar3 = 0;
	if (0 < nbRays) {
		do {
			iVar1 = this->pRayDef + iVar3;
			if (this->countId == iVar1->field_0x33) {
				iVar1->field_0x30 = 0;
				iVar1->field_0x33 = 0xff;
				iVar1->field_0x2c = 0xffff;
			}

			iVar3 = iVar3 + 1;
		} while (iVar3 < nbRays);
	}

	return;
}

bool CFxLightEmitter::Begin_SharedDraw()
{
	bool bVar1;
	edDList_material* pMaterialInfo;

	if ((this->flags & 0x140) == 0) {
		if (((this->flags & 0x400) == 0) || (bVar1 = GameDList_BeginCurrent(), bVar1 == false)) {
			bVar1 = false;
		}
		else {
			pMaterialInfo = CScene::ptable.g_C3DFileManager_00451664->GetMaterialFromId(this->materialId, this->field_0x98);
			edDListUseMaterial(pMaterialInfo);
			bVar1 = true;
		}
	}
	else {
		bVar1 = false;
	}

	return bVar1;
}

bool CFxLightEmitter::End_SharedDraw()
{
	bool bVar1;

	bVar1 = false;
	if ((this->flags & 0x140) == 0) {
		if ((this->flags & 0x400) == 0) {
			bVar1 = false;
		}
		else {
			GameDList_EndCurrent();
			bVar1 = true;
		}
	}

	return bVar1;
}

int CFxLightEmitter::ManageSlice(RAY_DEF* pDefs, int param_3, edF32VECTOR4* param_4, uint param_5)
{
	byte bVar1;
	byte bVar2;
	short sVar3;
	int iVar5;
	int iVar6;
	int iVar7;
	int iVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	edF32VECTOR4 eStack16;

	iVar8 = 0;
	if (static_cast<int>(param_5) < 0) {
		param_5 = this->nbRayDefs;
	}

	if (this->field_0x54 + param_5 == 0) {
		this->flags = this->flags & 0xfffffffe;
	}
	else {
		this->flags = this->flags | 1;
	}

	if (param_4 != (edF32VECTOR4*)0x0) {
		this->field_0x10 = *param_4;
	}

	iVar7 = 0;
	if ((this->flags & 1) != 0) {
		if ((this->flags & 0x80) != 0) {
			fVar9 = this->field_0xb8;
			fVar9 = this->field_0xc0 + fVar9 * GetTimer()->cutsceneDeltaTime;
			this->field_0xc0 = fVar9;
			if (1.0f <= fVar9) {
				this->field_0xc0 = 0;
				iVar8 = 1;
			}
		}
		iVar6 = 0;
		iVar7 = 0;
		if (0 < param_3) {
			do {
				bVar1 = this->countId;
				bVar2 = pDefs->field_0x33;
				if ((bVar2 == bVar1) || (bVar2 == 0xff)) {
					if (bVar2 == bVar1) {
						iVar7 = iVar7 + 1;
						if ((pDefs->field_0x30 & 1) == 0) {
							if ((this->flags & 8) == 0) {
								sVar3 = pDefs->field_0x2c;
								if (sVar3 < 1) {
									GenerateRay(pDefs);
								}
								else {
									if (0 < sVar3) {
										pDefs->field_0x2c = sVar3 + -1;
									}
								}
							}
						}
						else {
							if ((this->flags & 4) != 0) {
								edF32Vector4SubHard(&pDefs->field_0x10, &this->field_0x10, &pDefs->field_0x0);
								edF32Vector4NormalizeHard(&pDefs->field_0x10, &pDefs->field_0x10);
							}

							if (this->field_0x60 <= pDefs->field_0x28) {
								pDefs->field_0x30 = pDefs->field_0x30 & 0xfffe;
								pDefs->field_0x33 = 0xff;
								this->field_0x54 = this->field_0x54 + -1;
								this->field_0x50 = this->field_0x50 + -1;
							}

							pDefs->field_0x28 = pDefs->field_0x28 + GetTimer()->cutsceneDeltaTime;
							fVar9 = this->field_0x84;
							pDefs->field_0x20 = pDefs->field_0x20 + fVar9 * GetTimer()->cutsceneDeltaTime;
							if (((pDefs->field_0x30 & 2) == 0) && ((this->flags & 0x20) != 0)) {
								fVar9 = this->field_0xa4;
								fVar9 = edF32Between_0_2Pi(pDefs->field_0x38 + fVar9 * GetTimer()->cutsceneDeltaTime);
								pDefs->field_0x38 = fVar9;
							}

							if (pDefs->field_0x24 < pDefs->field_0x20) {
								pDefs->field_0x20 = pDefs->field_0x24;
							}

							fVar9 = pDefs->field_0x20;
							edF32Vector4ScaleHard(fVar9 * GetTimer()->cutsceneDeltaTime, &eStack16, &pDefs->field_0x10);
							edF32Vector4AddHard(&pDefs->field_0x0, &pDefs->field_0x0, &eStack16);
							(pDefs->field_0x0).w = 1.0f;

							if (((this->flags & 0x80) != 0) && (pDefs->field_0x3c = pDefs->field_0x3c + iVar8, pDefs->field_0x3c == this->field_0xbc * this->field_0xbe)) {
								pDefs->field_0x3c = 0;
							}
						}
					}
					else {
						if (((this->flags & 8) == 0) && ((int)this->field_0x50 < static_cast<int>(param_5))) {
							pDefs->field_0x33 = bVar1;
							if (pDefs->field_0x2c == -1) {
								iVar5 = rand();
								if (this->stallTime == 0) {
									trap(7);
								}

								pDefs->field_0x2c = (short)(iVar5 % this->stallTime);
							}

							iVar7 = iVar7 + 1;
							this->field_0x50 = this->field_0x50 + 1;
						}
					}
				}

				iVar6 = iVar6 + 1;
				pDefs = pDefs + 1;
			} while (iVar6 < param_3);
		}
	}

	return iVar7;
}


void CFxLightEmitter::GenerateRay(RAY_DEF* pRayDef)
{
	int iVar1;
	int iVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	edF32VECTOR4 local_60;
	edF32MATRIX4 eStack80;
	edF32VECTOR3 local_10;

	fVar4 = this->field_0x80;
	fVar5 = this->field_0x7c;
	local_60.z = 0.0f;
	iVar1 = rand();
	pRayDef->field_0x24 = fVar5 + (fVar4 - fVar5) * (static_cast<float>(iVar1) / 2.147484e+09f);
	pRayDef->field_0x20 = this->field_0x78;
	iVar1 = rand();
	pRayDef->field_0x28 = this->field_0x60 * (static_cast<float>(iVar1) / 2.147484e+09f) * 0.1f;

	if ((this->flags & 0x800) != 0) {
		local_60.z = this->field_0x68;
		iVar1 = rand();
		local_60.z = local_60.z * (static_cast<float>(iVar1) / 2.147484e+09f);
		pRayDef->field_0x28 = pRayDef->field_0x28 + local_60.z / this->field_0x78;
	}

	fVar4 = (this->field_0x20).x;
	iVar1 = rand();
	local_10.x = (this->field_0x40 + fVar4 * (static_cast<float>(iVar1) / 2.147484e+09f)) - (this->field_0x20).x * 0.5f;
	fVar4 = (this->field_0x20).y;
	iVar1 = rand();
	local_10.y = (this->field_0x44 + fVar4 * (static_cast<float>(iVar1) / 2.147484e+09f)) - (this->field_0x20).y * 0.5f;
	local_10.z = (this->field_0x20).z;
	iVar1 = rand();
	local_60.x = 0.0f;
	local_10.z = local_10.z * (static_cast<float>(iVar1) / 2.147484e+09f);
	local_60.y = this->radiusMin;
	fVar4 = this->radiusMax;
	iVar1 = rand();
	local_60.w = 1.0f;
	local_60.y = local_60.y + (fVar4 - local_60.y) * (static_cast<float>(iVar1) / 2.147484e+09f);

	edF32Matrix4FromEulerSoft(&eStack80, &local_10, "ZXY");
	edF32Matrix4MulF32Vector4Hard(&local_60, &eStack80, &local_60);
	edF32Vector4AddHard(&pRayDef->field_0x0, &this->field_0x10, &local_60);

	(pRayDef->field_0x0).w = 1.0f;
	pRayDef->field_0x33 = this->countId;
	pRayDef->field_0x32 = this->field_0xc4;
	this->field_0xc4 = this->field_0xc4 + 1 & 3;

	if ((pRayDef->field_0x30 & 2) == 0) {
		fVar4 = this->field_0x88;
		fVar5 = this->field_0x8c;
		iVar1 = rand();
		pRayDef->field_0x34 = fVar4 + (fVar5 - fVar4) * (static_cast<float>(iVar1) / 2.147484e+09f);
		if ((this->flags & 0x20) != 0) {
			fVar4 = (float)this->field_0xa0;
			iVar1 = rand();
			pRayDef->field_0x38 = fVar4 * (static_cast<float>(iVar1) / 2.147484e+09f);
		}
	}

	if ((this->flags & 2) == 0) {
		edF32Vector4SubHard(&pRayDef->field_0x10, &pRayDef->field_0x0, &this->field_0x10);
		edF32Vector4NormalizeHard(&pRayDef->field_0x10, &pRayDef->field_0x10);
	}
	else {
		pRayDef->field_0x10 = this->direction;
	}

	if ((this->flags & 0x80) != 0) {
		iVar2 = rand();
		iVar1 = this->field_0xbc * this->field_0xbe;
		if (iVar1 == 0) {
			trap(7);
		}
		pRayDef->field_0x3c = iVar2 % iVar1;
	}

	pRayDef->field_0x30 = pRayDef->field_0x30 | 1;
	this->field_0x54 = this->field_0x54 + 1;

	return;
}

byte BYTE_00448d1c = 1;
byte BYTE_00448d20 = 1;
byte BYTE_00448d24 = 1;
byte BYTE_00448d28 = 1;

byte BYTE_004497e4 = 0;

void CFxLightEmitter::Draw(uint param_2, edF32MATRIX4* param_3, edF32MATRIX4* param_4)
{
	ushort uVar1;
	uint uVar2;
	undefined8 uVar3;
	undefined8 uVar4;
	undefined8 uVar5;
	undefined8 uVar6;
	CCameraManager* pCameraManager;
	edDList_material* pMaterialInfo;
	RAY_DEF* pScratchRayDef;
	undefined4 uVar7;
	undefined4 uVar8;
	int iVar9;
	float fVar10;
	RAY_DEF* pRVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	float fVar15;
	float fVar16;
	float fVar17;
	float fVar18;
	uint local_1f0;
	edF32VECTOR4 local_1e0;
	edF32VECTOR4 local_1d0;
	edF32VECTOR4 local_1c0;
	edF32VECTOR4 local_1b0;
	edF32MATRIX4 eStack416;
	edF32MATRIX4 eStack352;
	edF32VECTOR2* local_120[4];
	edF32VECTOR2 local_110;
	edF32VECTOR2 local_10c;
	edF32VECTOR2 local_108;
	edF32VECTOR2 local_100;
	edF32VECTOR2 local_f8;
	edF32VECTOR4* local_f0[4];
	edF32VECTOR4 eStack224;
	edF32VECTOR4 eStack208;
	edF32VECTOR4 eStack192;
	edF32VECTOR4 eStack176;
	_rgba* local_a0[4];
	edF32VECTOR4 local_90;
	edF32VECTOR4 local_80;
	edF32VECTOR4 eStack112;
	edF32MATRIX4 auStack96;
	edF32VECTOR2 local_20;
	edF32VECTOR2 local_18;
	_rgba local_10;
	_rgba local_c;
	_rgba local_8;
	_rgba local_4;
	C3DFileManager* pFileManager;

	pFileManager = CScene::ptable.g_C3DFileManager_00451664;
	pCameraManager = static_cast<CCameraManager*>(CScene::GetManager(MO_Camera));
	if ((this->field_0x54 != 0) && (uVar2 = this->flags, (uVar2 & 0x100) == 0)) {
		if ((uVar2 & 0x40) == 0) {
			if ((uVar2 & 0x400) == 0) {
				GameDList_BeginCurrent();
				pMaterialInfo = pFileManager->GetMaterialFromId(this->materialId, this->field_0x98);
				edDListUseMaterial(pMaterialInfo);
			}

			if (BYTE_004497e4 != 0) {
				edDListUseMaterial((edDList_material*)0x0);
			}

			if (param_3 == (edF32MATRIX4*)0x0) {
				edDListLoadIdentity();
			}
			else {
				edDListLoadMatrix(param_3);
			}

			if (((this->flags & 0x10) == 0) || ((this->flags & 0x20) != 0)) {
				fVar17 = 0.0f;
				edDListBegin(0.0f, 0.0f, 0.0f, 8, this->field_0x54 << 2);
			}
			else {
				fVar17 = 0.0f;
				edDListBegin(0.0f, 0.0f, 0.0f, 0xb, this->field_0x54);
			}

			if (param_4 == (edF32MATRIX4*)0x0) {
				auStack96.dd = 9.876f;
			}
			else {
				edF32Matrix4MulF32Matrix4Hard(&auStack96, &pCameraManager->transMatrix_0x390, param_4);
			}

			pRVar11 = this->pRayDef;
			local_1f0 = param_2;
			if (param_2 == 0xffffffff) {
				local_1f0 = this->nbRayDefs;
			}

			pScratchRayDef = static_cast<RAY_DEF*>(gSP_Manager.GetFreeBuffer(sizeof(RAY_DEF)));
			iVar9 = 0;
			if (0 < static_cast<int>(local_1f0)) {
				do {
					if ((pRVar11->field_0x33 == this->countId) && ((pRVar11->field_0x30 & 1) != 0)) {
						*pScratchRayDef = *pRVar11;
						
						fVar14 = this->field_0x60;
						fVar15 = pScratchRayDef->field_0x28;
						if (fVar15 < fVar14 * 0.25f) {
							fVar14 = edFIntervalLERP(fVar15, 0.0f, fVar14 * 0.25f, 0.0f, 1.0f);
							fVar18 = fVar18 * fVar14;
						}
						else {
							if (fVar14 * 0.75f < fVar15) {
								fVar14 = edFIntervalLERP(fVar15, fVar14 * 0.75f, fVar14, 1.0f, fVar17);
								fVar18 = fVar18 * fVar14;
							}
						}

						local_4 = this->aColors[pScratchRayDef->field_0x32];
						if ((this->flags & 0x10) == 0) {
							local_90 = pScratchRayDef->field_0x0;

							edF32Vector4ScaleHard(this->field_0x64, &eStack112, &pScratchRayDef->field_0x10);
							edF32Vector4SubHard(&local_80, &local_90, &auStack96.rowT);
							edF32Vector4CrossProductHard(&local_80, &eStack112, &local_80);
							edF32Vector4NormalizeHard(&local_80, &local_80);
							edF32Vector4ScaleHard(pScratchRayDef->field_0x34, &local_80, &local_80);
							if (BYTE_00448d1c != 0) {
								if (BYTE_00448d20 == 0) {
									edDListColor4u8(local_4.r, local_4.g, local_4.b, (byte)static_cast<int>((float)(uint)local_4.a * fVar18));
									edDListTexCoo2f((this->uvMapping).x, (this->uvMapping).y);
									edDListVertex4f(local_90.x - local_80.x, local_90.y - local_80.y, local_90.z - local_80.z, 0.0f);
									edDListTexCoo2f((this->uvMapping).x, (this->uvMapping).y + (this->uvMapping).w);
									edDListVertex4f(local_90.x + local_80.x, local_90.y + local_80.y, local_90.z + local_80.z, 0.0f);
									edF32Vector4SubHard(&local_90, &local_90, &eStack112);
									edDListColor4u8(local_4.r, local_4.g, local_4.b, 0);
									edDListTexCoo2f((this->uvMapping).x + (this->uvMapping).z, (this->uvMapping).y);
									edDListVertex4f(local_90.x - local_80.x, local_90.y - local_80.y, local_90.z - local_80.z, 0.0f);
									edDListTexCoo2f((this->uvMapping).x + (this->uvMapping).z, (this->uvMapping).y + (this->uvMapping).w);
									edDListVertex4f(local_90.x + local_80.x, local_90.y + local_80.y, local_90.z + local_80.z, 0.0f);
								}
								else {
									local_8.r = local_4.r;
									local_8.g = local_4.g;
									local_8.b = local_4.b;
									local_a0[0] = &local_8;
									local_a0[2] = &local_c;
									local_8.a = (byte)(int)((float)(uint)local_4.a * fVar18);
									local_c.r = local_4.r;
									local_c.g = local_4.g;
									local_c.b = local_4.b;
									local_c.a = 0;
									local_a0[1] = local_a0[0];
									local_a0[3] = local_a0[2];
									edF32Vector4SubHard(&eStack224, &local_90, &local_80);
									edF32Vector4AddHard(&eStack208, &local_90, &local_80);
									edF32Vector4SubHard(&eStack192, &eStack224, &eStack112);
									edF32Vector4SubHard(&eStack176, &eStack208, &eStack112);
									local_f0[0] = &eStack224;
									local_f0[1] = &eStack208;
									local_f0[2] = &eStack192;
									local_f0[3] = &eStack176;
									local_120[0] = &local_110;
									local_110.x = (this->uvMapping).x;
									local_110.y = (this->uvMapping).y;
									local_108.x = (this->uvMapping).x;
									local_108.y = (this->uvMapping).y + (this->uvMapping).w;
									local_100.x = (this->uvMapping).x + (this->uvMapping).z;
									local_100.y = (this->uvMapping).y;
									local_f8.x = (this->uvMapping).x + (this->uvMapping).z;
									local_f8.y = (this->uvMapping).y + (this->uvMapping).w;
									local_120[1] = &local_108;
									local_120[2] = &local_100;
									local_120[3] = &local_f8;
									edDListAddTexturedQuad(local_120, local_a0, local_f0);
								}
							}
						}
						else {
							if ((this->flags & 0x20) == 0) {
								fVar14 = edFIntervalLERP(pScratchRayDef->field_0x28, fVar17, this->field_0x60, pScratchRayDef->field_0x34, this->field_0x90);
								if (BYTE_00448d24 != 0) {
									if (BYTE_00448d28 == 0) {
										uVar1 = this->field_0xbc;
										if (uVar1 == 0) {
											trap(7);
										}
										fVar15 = (this->uvMapping).z * static_cast<float>(pScratchRayDef->field_0x3c % uVar1);
										if (uVar1 == 0) {
											trap(7);
										}
										fVar16 = (this->uvMapping).w * static_cast<float>(pScratchRayDef->field_0x3c / uVar1);
										edDListTexCoo2f(fVar15, fVar16);
										edDListTexCoo2f(fVar15 + (this->uvMapping).z, fVar16 + (this->uvMapping).w);
										edDListWidthHeight2f(fVar14, fVar14);
										edDListColor4u8(local_4.r, local_4.g, local_4.b, local_4.a * fVar18);
										edDListVertex4f((pScratchRayDef->field_0x0).x, (pScratchRayDef->field_0x0).y, (pScratchRayDef->field_0x0).z, fVar17);
									}
									else {
										if (this->field_0xbc == 0) {
											trap(7);
										}
										local_18.x = (this->uvMapping).z * static_cast<float>(pScratchRayDef->field_0x3c % this->field_0xbc);
										if (this->field_0xbc == 0) {
											trap(7);
										}
										local_18.y = (this->uvMapping).w * static_cast<float>(pScratchRayDef->field_0x3c / this->field_0xbc);
										local_20.x = local_18.x + (this->uvMapping).z;
										local_20.y = local_18.y + (this->uvMapping).w;
										local_10.r = local_4.r;
										local_10.g = local_4.g;
										local_10.b = local_4.b;
										local_10.a = local_4.a * fVar18;
										edDListLightVertex(fVar14, fVar14, &local_18, &local_20, &local_10, &pScratchRayDef->field_0x0);
									}
								}
							}
							else {
								edF32Matrix4CopyHard(&eStack352, &pCameraManager->transMatrix_0x390);
								eStack352.da = (pScratchRayDef->field_0x0).x;
								eStack352.db = (pScratchRayDef->field_0x0).y;
								eStack352.dc = (pScratchRayDef->field_0x0).z;
								eStack352.dd = (pScratchRayDef->field_0x0).w;
								edF32Matrix4RotateZHard(pScratchRayDef->field_0x38, &eStack416, &gF32Matrix4Unit);
								edF32Matrix4MulF32Matrix4Hard(&eStack352, &eStack416, &eStack352);
								local_1e0.x = edFIntervalLERP(pScratchRayDef->field_0x28, 0.0f, this->field_0x60, pScratchRayDef->field_0x34, this->field_0x90);
								local_1d0.x = -local_1e0.x;
								local_1b0.z = 0.0f;
								local_1b0.w = 1.0f;
								local_1c0.z = 0.0f;
								local_1c0.w = 1.0f;
								local_1d0.z = 0.0f;
								local_1d0.w = 1.0f;
								local_1e0.z = 0.0f;
								local_1e0.w = 1.0f;
								local_1e0.y = local_1e0.x;
								local_1d0.y = local_1e0.x;
								local_1c0.x = local_1e0.x;
								local_1c0.y = local_1d0.x;
								local_1b0.x = local_1d0.x;
								local_1b0.y = local_1d0.x;
								edF32Matrix4MulF32Vector4Hard(&local_1b0, &eStack352, &local_1b0);
								edF32Matrix4MulF32Vector4Hard(&local_1c0, &eStack352, &local_1c0);
								edF32Matrix4MulF32Vector4Hard(&local_1d0, &eStack352, &local_1d0);
								edF32Matrix4MulF32Vector4Hard(&local_1e0, &eStack352, &local_1e0);
								edDListColor4u8(local_4.r, local_4.g, local_4.b, local_4.a * fVar18);
								edDListTexCoo2f((this->uvMapping).x + (this->uvMapping).z, (this->uvMapping).y);
								edDListVertex4f(local_1b0.x, local_1b0.y, local_1b0.z, 0.0f);
								edDListTexCoo2f((this->uvMapping).x, (this->uvMapping).y);
								edDListVertex4f(local_1c0.x, local_1c0.y, local_1c0.z, 0.0f);
								edDListTexCoo2f((this->uvMapping).x + (this->uvMapping).z, (this->uvMapping).y + (this->uvMapping).w);
								edDListVertex4f(local_1d0.x, local_1d0.y, local_1d0.z, 0.0f);
								edDListTexCoo2f((this->uvMapping).x, (this->uvMapping).y + (this->uvMapping).w);
								edDListVertex4f(local_1e0.x, local_1e0.y, local_1e0.z, 0.0f);
							}
						}
					}

					iVar9 = iVar9 + 1;
					pRVar11 = pRVar11 + 1;
				} while (iVar9 < static_cast<int>(local_1f0));
			}

			gSP_Manager.ReleaseBuffer(pScratchRayDef);

			edDListEnd();
			if ((this->flags & 0x400) == 0) {
				GameDList_EndCurrent();
			}
		}
		else {
			pRVar11 = this->pRayDef;
			iVar9 = 0;
			if (0 < (int)this->nbRayDefs) {
				do {
					IMPLEMENTATION_GUARD(
					fVar17 = pRVar11->field_0x38;
					iVar9 = iVar9 + 1;
					fVar16 = (pRVar11->field_0x0).y;
					fVar14 = (pRVar11->field_0x0).z;
					fVar15 = (pRVar11->field_0x0).w;
					*static_cast<float*>((int)fVar17 + 0x30) = (pRVar11->field_0x0).x;
					*static_cast<float*>((int)fVar17 + 0x34) = fVar16;
					*static_cast<float*>((int)fVar17 + 0x38) = fVar14;
					*static_cast<float*>((int)fVar17 + 0x3c) = fVar15;
					pRVar11 = pRVar11 + 1;)
				} while (iVar9 < (int)this->nbRayDefs);
			}
		}
	}

	return;
}
