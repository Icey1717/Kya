#include "Fx_Spark.h"
#include "Rendering/DisplayList.h"
#include "MathOps.h"
#include "edDlist.h"
#include "FileManager3D.h"

#ifdef PLATFORM_WIN
#include "displaylist.h"
#endif

bool CFxSpark::IsKindOfObject(ulong kind)
{
	return (kind & 0x90000) != 0;
}

bool CFxSpark::InitDlistPatchable(int)
{
	int iVar1;
	uint uVar2;
	int iVar3;
	int iVar4;
	float x;
	float s;
	_rgba local_4;

	DISPLAY_LIST_PATCH_BEGIN();

	edDListLoadIdentity();

	edDListUseMaterial(CScene::ptable.g_C3DFileManager_00451664->GetMaterialFromId(this->particleID, 0));

	edDListBegin(0.0f, 0.0f, 0.0f, 4, this->count_0xa0 * this->count_0x98 * 2);
	iVar4 = 0;
	if (0 < this->count_0x98) {
		do {
			iVar1 = this->count_0xa0;
			s = -8.0;
			iVar3 = 0;
			if (0 < iVar1) {
				do {
					local_4.LerpRGBA((float)iVar3 / (float)iVar1, this->field_0x90, this->field_0x94);
					uVar2 = local_4.rgba;
					edDListColor4u8(local_4.r, local_4.g, local_4.b, 0);
					edDListTexCoo2f(s, 0.0f);
					if (iVar3 == 0) {
						x = this->field_0xb0;
					}
					else {
						x = this->field_0xb0;
					}
					edDListVertex4f(x, 0.0f, 0.0f, uVar2);
					edDListTexCoo2f(s, 1.0f);
					edDListVertex4f(-this->field_0xb0, 0.0f, 0.0f, uVar2);
					iVar1 = this->count_0xa0;
					iVar3 = iVar3 + 1;
					s = s + this->field_0xc0 / (float)iVar1;
				} while (iVar3 < iVar1);
			}
			iVar4 = iVar4 + 1;
		} while (iVar4 < this->count_0x98);
	}

	edDListEnd();

	DISPLAY_LIST_PATCH_END();

	return true;
}

void CFxSpark::Create(int vectorCount, int floatCount, edF32VECTOR3* pVectorBuffer, float* pFloatBuffer, int particleID)
{
	int iVar5;

	this->count_0x98 = vectorCount;
	this->count_0xa0 = floatCount;
	//this->field_0x9c = 0;
	this->particleID = particleID;
	this->pVector_0xc = pVectorBuffer;
	this->pFloat_0x10 = pFloatBuffer;
	this->field_0x90 = 0x80808080;
	this->field_0x94 = 0x80808080;
	iVar5 = 0;
	if (0 < this->count_0x98) {
		do {
			this->pVector_0xc[iVar5].x = 1.0f;
			this->pVector_0xc[iVar5].y = 0.0f;
			this->pVector_0xc[iVar5].z = 0.0f;
			iVar5 = iVar5 + 1;
		} while (iVar5 < this->count_0x98);
	}
	
	this->pVector_0xcc = this->pVector_0xc;
	
	this->field_0xd0 = gF32Vector4UnitZ;
	this->field_0xe4 = 0;
	//this->pFunc_0xc4 = _DefaultModifyVertex;

	return;
}

void CFxSpark::Init(int id)
{
	int iVar1;
	int iVar2;
	float* pfVar3;
	float* pfVar4;
	float fVar5;
	float fVar6;
	float fVar7;

	iVar2 = 0;
	//if (0 < this->count_0x98) {
	//	do {
	//		fVar7 = gF32Vertex4Zero.w;
	//		fVar6 = gF32Vertex4Zero.z;
	//		fVar5 = gF32Vertex4Zero.y;
	//		pfVar4 = (float*)(this->field_0xe4 + (this->count_0xa0 + iVar2 * this->count_0xa0 + -1) * 0x10);
	//		*pfVar4 = gF32Vertex4Zero.x;
	//		pfVar4[1] = fVar5;
	//		pfVar4[2] = fVar6;
	//		pfVar4[3] = fVar7;
	//		fVar7 = pfVar4[1];
	//		fVar5 = pfVar4[2];
	//		fVar6 = pfVar4[3];
	//		pfVar3 = (float*)(this->field_0xe4 + iVar2 * this->count_0xa0 * 0x10);
	//		*pfVar3 = *pfVar4;
	//		pfVar3[1] = fVar7;
	//		pfVar3[2] = fVar5;
	//		pfVar3[3] = fVar6;
	//		iVar1 = iVar2 * this->count_0xa0;
	//		iVar2 = iVar2 + 1;
	//		*(undefined4*)(this->field_0xe4 + (this->count_0xa0 + iVar1) * 0x10 + -8) = 0x3f800000;
	//	} while (iVar2 < this->count_0x98);
	//}

	this->objectId = id;
	this->field_0xe0 = 0;

	this->vector_0x80 = gF32Vector4Zero;
	this->dlistPatchId = GameDListPatch_Register(this, this->count_0xa0 * this->count_0x98 * 2, 0);

	return;
}

void CFxSpark::SetParameters(float param_1, float param_2, float param_3, float param_4, float param_5, undefined4 param_7)
{
	int iVar2;

	//this->field_0x8 = param_7;
	this->field_0xb0 = param_2;
	this->field_0xc0 = param_5;
	//this->field_0xac = param_1;
	//this->field_0xb8 = param_3;
	//this->field_0xbc = 1.0f / param_3;
	//this->field_0xb4 = param_4;
	//
	//iVar2 = 0;
	//if (0 < this->count_0xa0) {
	//	do {
	//		pFloat_0x10[iVar1] = param_2;
	//		iVar2 = iVar2 + 1;
	//	} while (iVar2 < this->count_0xa0);
	//}

	return;
}
