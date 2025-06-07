#include "FxLightEmitter.h"
#include "LargeObject.h"
#include "FileManager3D.h"
#include "TimeController.h"

edF32VECTOR4 edF32VECTOR4_00426890 = { 0.0f, 0.0f, 1.0f, 1.0f };
edF32VECTOR4 edF32VECTOR4_004268a0 = { 0.0f, -0.5f, 0.0f, 1.0f };

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

bool CFxLightEmitter::Manage(edF32VECTOR4*, undefined4)
{
	IMPLEMENTATION_GUARD_FX();
	return true;
}

void CFxLightEmitter::GenerateNewOne(edF32VECTOR4* param_2)
{
	IMPLEMENTATION_GUARD_FX();
}
