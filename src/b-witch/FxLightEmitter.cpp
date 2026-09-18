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

	//this->field_0x60 = param_1;
	//this->field_0xc0 = 0;
	//this->nbRayDefs = param_3;
	//this->field_0x50 = 0;
	//this->field_0x54 = 0;
	//this->stallTime = 1;
	//this->field_0x68 = 0;
	//if ((this->flags & 0x200) == 0) {
	//	this->pRayDef = new RAY_DEF[this->nbRayDefs << 6];
	//	iVar11 = 0;
	//	uVar1 = this->nbRayDefs;
	//	this->field_0x54 = 0;
	//	this->field_0x50 = 0;
	//	if (0 < (int)uVar1) {
	//		iVar10 = 0;
	//		do {
	//			fVar7 = gF32Vector4UnitZ.w;
	//			fVar4 = gF32Vector4UnitZ.z;
	//			fVar3 = gF32Vector4UnitZ.y;
	//			iVar11 = iVar11 + 1;
	//			iVar9 = &this->pRayDef->field_0x0 + iVar10;
	//			*(float*)(iVar9 + 0x10) = gF32Vector4UnitZ.x;
	//			*(float*)(iVar9 + 0x14) = fVar3;
	//			*(float*)(iVar9 + 0x18) = fVar4;
	//			*(float*)(iVar9 + 0x1c) = fVar7;
	//			*(undefined2*)(&this->pRayDef->field_0x2c + iVar10) = 0xffff;
	//			*(undefined2*)(&this->pRayDef->field_0x2e + iVar10) = 0x3039;
	//			*(undefined2*)(&this->pRayDef->field_0x30 + iVar10) = 0;
	//			*(undefined*)(&this->pRayDef->field_0x33 + iVar10) = 0xff;
	//			*(undefined4*)(&this->pRayDef->field_0x38 + iVar10) = 0;
	//			*(undefined4*)(&this->pRayDef->field_0x3c + iVar10) = 0;
	//			iVar10 = iVar10 + 0x40;
	//		} while (iVar11 < (int)uVar1);
	//	}
	//}
	//
	//this->flags = this->flags | 1;
	//
	//this->field_0x5c = 1.0f;
	//
	//this->field_0x20 = gF32Vector4Zero;
	//
	//this->field_0xa0 = 0;
	//this->field_0xa4 = 0;
	//this->flags = this->flags | 0x80;
	//this->field_0xb8 = 0.0f;
	//this->field_0xbc = 1;
	//this->field_0xbe = 1;
	//
	//this->uvMapping = edF32VECTOR4_00426890;
	//this->field_0x10 = edF32VECTOR4_004268a0;
	//
	//if ((this->flags & 2) != 0) {
	//	this->field_0x30 = g_xVector;
	//}
	//
	//this->field_0xc4 = 0;
	//this->aColors[0] = 0x80808080;
	//this->aColors[1] = 0x80808080;
	//this->aColors[2] = 0x80808080;
	//this->aColors[3] = 0x80808080;
	//this->flags = this->flags | 1;

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

	//this->field_0x54 = 0;
	//this->field_0x50 = 0;
	//iVar6 = 0;
	//if (0 < nbRays) {
	//	iVar5 = 0;
	//	do {
	//		fVar3 = gF32Vector4UnitZ.w;
	//		fVar2 = gF32Vector4UnitZ.z;
	//		fVar1 = gF32Vector4UnitZ.y;
	//		iVar6 = iVar6 + 1;
	//		iVar4 = &this->pRayDef->field_0x0 + iVar5;
	//		*(float*)(iVar4 + 0x10) = gF32Vector4UnitZ.x;
	//		*(float*)(iVar4 + 0x14) = fVar1;
	//		*(float*)(iVar4 + 0x18) = fVar2;
	//		*(float*)(iVar4 + 0x1c) = fVar3;
	//		*(undefined2*)((int)&this->pRayDef->field_0x2c + iVar5) = 0xffff;
	//		*(undefined2*)(&this->pRayDef->field_0x2e + iVar5) = 0x3039;
	//		*(undefined2*)(&this->pRayDef->field_0x30 + iVar5) = 0;
	//		(&this->pRayDef->field_0x33)[iVar5] = 0xff;
	//		*(undefined4*)(&this->pRayDef->field_0x38 + iVar5) = 0;
	//		*(undefined4*)(&this->pRayDef->field_0x3c + iVar5) = 0;
	//		iVar5 = iVar5 + 0x40;
	//	} while (iVar6 < nbRays);
	//}

	return;
}

void CFxLightEmitter::ResetRays()
{
	uint uVar1;
	RAY_DEF* iVar2;
	int iVar3;
	int iVar4;

	//if (this->pRayDef != (RAY_DEF*)0x0) {
	//	uVar1 = this->nbRayDefs;
	//	iVar4 = 0;
	//	this->field_0x54 = 0;
	//	this->field_0x50 = 0;
	//	if (0 < (int)uVar1) {
	//		iVar3 = 0;
	//		do {
	//			iVar2 = (RAY_DEF*)(&this->pRayDef->field_0x0 + iVar3);
	//			if (this->countId == *(char*)&iVar2->field_0x33) {
	//				*(undefined2*)&iVar2->field_0x30 = 0;
	//				*(undefined*)(&this->pRayDef->field_0x33 + iVar3) = 0xff;
	//				*(undefined2*)(&this->pRayDef->field_0x2c + iVar3) = 0xffff;
	//			}
	//			iVar4 = iVar4 + 1;
	//			iVar3 = iVar3 + 0x40;
	//		} while (iVar4 < (int)uVar1);
	//	}
	//}
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
	int iVar1;
	int iVar2;
	int iVar3;

	IMPLEMENTATION_GUARD_FX(
	this->field_0x54 = 0;
	this->field_0x50 = 0;
	iVar3 = 0;
	if (0 < nbRays) {
		iVar2 = 0;
		do {
			iVar1 = &this->pRayDef->field_0x0 + iVar2;
			if (this->countId == *(char*)(iVar1 + 0x33)) {
				*(undefined2*)(iVar1 + 0x30) = 0;
				(&this->pRayDef->field_0x33)[iVar2] = 0xff;
				*(undefined2*)((int)&this->pRayDef->field_0x2c + iVar2) = 0xffff;
			}
			iVar3 = iVar3 + 1;
			iVar2 = iVar2 + 0x40;
		} while (iVar3 < nbRays);
	})

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
	float* local_120;
	float* local_11c;
	float* local_118;
	float* local_114;
	float local_110;
	float local_10c;
	float local_108;
	float local_104;
	float local_100;
	float local_fc;
	float local_f8;
	float local_f4;
	edF32VECTOR4* local_f0;
	edF32VECTOR4* local_ec;
	edF32VECTOR4* local_e8;
	edF32VECTOR4* local_e4;
	edF32VECTOR4 eStack224;
	edF32VECTOR4 eStack208;
	edF32VECTOR4 eStack192;
	edF32VECTOR4 eStack176;
	byte* local_a0;
	byte* local_9c;
	byte* local_98;
	byte* local_94;
	edF32VECTOR4 local_90;
	edF32VECTOR4 local_80;
	edF32VECTOR4 eStack112;
	edF32MATRIX4 auStack96;
	edF32VECTOR2 local_20;
	edF32VECTOR2 local_18;
	_rgba local_10;
	byte local_c;
	byte local_b;
	byte local_a;
	undefined local_9;
	byte local_8;
	byte local_7;
	byte local_6;
	undefined local_5;
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
									IMPLEMENTATION_GUARD(
									local_8 = local_4.r;
									local_7 = local_4.g;
									local_6 = local_4.b;
									local_a0 = &local_8;
									local_98 = &local_c;
									local_5 = (undefined)static_cast<int>((float)(uint)local_4.a * fVar18);
									local_c = local_4.r;
									local_b = local_4.g;
									local_a = local_4.b;
									local_9 = 0;
									local_9c = local_a0;
									local_94 = local_98;
									edF32Vector4SubHard(&eStack224, &local_90, &local_80);
									edF32Vector4AddHard(&eStack208, &local_90, &local_80);
									edF32Vector4SubHard(&eStack192, &eStack224, &eStack112);
									edF32Vector4SubHard(&eStack176, &eStack208, &eStack112);
									local_f0 = &eStack224;
									local_ec = &eStack208;
									local_e8 = &eStack192;
									local_e4 = &eStack176;
									local_120 = &local_110;
									local_110 = (this->uvMapping).x;
									local_10c = (this->uvMapping).y;
									local_108 = (this->uvMapping).x;
									local_104 = (this->uvMapping).y + (this->uvMapping).w;
									local_100 = (this->uvMapping).x + (this->uvMapping).z;
									local_fc = (this->uvMapping).y;
									local_f8 = (this->uvMapping).x + (this->uvMapping).z;
									local_f4 = (this->uvMapping).y + (this->uvMapping).w;
									local_11c = &local_108;
									local_118 = &local_100;
									local_114 = &local_f8;
									FUN_002d17d0(&local_120, &local_a0, (int*)&local_f0);)
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
