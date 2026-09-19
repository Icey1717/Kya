#include "Fx_Spark.h"
#include "DlistManager.h"
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
	int innerLoopIndex;
	int outerLoopIndex;
	float x;
	float fVar4;
	float s;
	_rgba local_4;

	edDListLoadIdentity();

	edDListUseMaterial(CScene::ptable.g_C3DFileManager_00451664->GetMaterialFromId(this->particleID, 0));

	edDListBegin(0.0f, 0.0f, 0.0f, 4, this->count_0xa0 * this->count_0x98 * 2);

	outerLoopIndex = 0;
	if (0 < this->count_0x98) {
		do {
			s = -8.0f;
			innerLoopIndex = 0;
			if (0 < this->count_0xa0) {
				do {
					local_4.LerpRGBA((float)innerLoopIndex / (float)this->count_0xa0, this->field_0x90, this->field_0x94);
					uVar2 = local_4.rgba;
					edDListColor4u8(local_4.r, local_4.g, local_4.b, 0);
					edDListTexCoo2f(s, 0.0f);
					if (innerLoopIndex == 0) {
						fVar4 = 49152.0f;
						x = this->field_0xb0;
					}
					else {
						fVar4 = 1.0f;
						x = this->field_0xb0;
					}
					edDListVertex4f(x, 0.0f, 0.0f, fVar4);
					edDListTexCoo2f(s, 1.0f);

					if (innerLoopIndex == 0) {
						fVar4 = 49152.0f;
					}
					else {
						fVar4 = 1.0f;
					}

					edDListVertex4f(-this->field_0xb0, 0.0f, 0.0f, fVar4);
					innerLoopIndex = innerLoopIndex + 1;
					s = s + this->field_0xc0 / (float)this->count_0xa0;
				} while (innerLoopIndex < this->count_0xa0);
			}

			outerLoopIndex = outerLoopIndex + 1;
		} while (outerLoopIndex < this->count_0x98);
	}

	edDListEnd();

	return true;
}

edF32VECTOR4* _DefaultModifyVertex(edF32VECTOR4* param_1, edF32VECTOR4* param_2)
{
	edF32VECTOR4 local_10;

	edF32Vector4ScaleHard(cosf(param_1->z * 3.141593f - 1.570796f), &local_10, param_2);
	param_1->x = local_10.x;
	param_1->y = local_10.y;
	return param_1;
}

void CFxSpark::Create(int vectorCount, int floatCount, DATA* pVectorBuffer, float* pFloatBuffer, int particleID)
{
	int iVar5;

	this->count_0x98 = vectorCount;
	this->count_0xa0 = floatCount;
	this->field_0x9c = 0;
	this->particleID = particleID;
	this->pVector_0xc = pVectorBuffer;
	this->pFloat_0x10 = pFloatBuffer;
	this->field_0x90 = 0x80808080;
	this->field_0x94 = 0x80808080;
	iVar5 = 0;
	if (0 < this->count_0x98) {
		do {
			this->pVector_0xc[iVar5].field_0x0 = 1.0f;
			this->pVector_0xc[iVar5].field_0x8 = 0;
			this->pVector_0xc[iVar5].field_0x9 = 0;
			iVar5 = iVar5 + 1;
		} while (iVar5 < this->count_0x98);
	}
	
	this->pVector_0xcc = this->pVector_0xc;
	
	this->field_0xd0 = gF32Vector4UnitZ;
	this->field_0xe4 = (edF32VECTOR4*)0x0;
	this->pFunc_0xc4 = _DefaultModifyVertex;

	return;
}

void CFxSpark::Init(int id)
{
	int iVar1;
	int iVar2;
	edF32VECTOR4* peVar3;
	edF32VECTOR4* peVar4;
	float fVar5;
	float fVar6;
	float fVar7;

	iVar2 = 0;
	if (0 < this->count_0x98) {
		do {
			peVar4 = this->field_0xe4 + this->count_0xa0 + iVar2 * this->count_0xa0 + -1;
			*peVar4 = gF32Vertex4Zero;
			peVar3 = this->field_0xe4 + iVar2 * this->count_0xa0;
			*peVar3 = *peVar4;
			iVar1 = iVar2 * this->count_0xa0;
			iVar2 = iVar2 + 1;
			this->field_0xe4[this->count_0xa0 + iVar1 + -1].z = 1.0f;
		} while (iVar2 < this->count_0x98);
	}

	this->sectorId = id;
	this->field_0xe0 = 0;

	this->vector_0x80 = gF32Vector4Zero;
	this->dlistPatchId = GameDListPatch_Register(this, this->count_0xa0 * this->count_0x98 * 2, 0);

	return;
}

void CFxSpark::SetParameters(float param_1, float param_2, float param_3, float param_4, float param_5, undefined4 param_7)
{
	int iVar2;

	this->field_0x8 = param_7;
	this->field_0xb0 = param_2;
	this->field_0xc0 = param_5;
	this->field_0xac = param_1;
	this->field_0xb8 = param_3;
	this->field_0xbc = 1.0f / param_3;
	this->field_0xb4 = param_4;
	
	iVar2 = 0;
	if (0 < this->count_0xa0) {
		do {
			this->pFloat_0x10[iVar2] = param_2;
			iVar2 = iVar2 + 1;
		} while (iVar2 < this->count_0xa0);
	}

	return;
}

void CFxSpark::Reset()
{
	int iVar2;

	iVar2 = 0;
	if (0 < this->count_0x98) {
		do {
			this->pVector_0xc[iVar2].field_0x0 = 1.0f;
			this->pVector_0xc[iVar2].field_0x8 = 0;
			this->pVector_0xc[iVar2].field_0x9 = 0;
			iVar2 = iVar2 + 1;
		} while (iVar2 < this->count_0x98);
	}

	this->field_0x9c = 0;
	this->pVector_0xcc = this->pVector_0xc;
	this->field_0xbc = 0.0f;
	CScene::ptable.g_GlobalDListManager_004516bc->SetActive(this->dlistPatchId, 0);

	return;
}

void CFxSpark::Manage(edF32VECTOR4* param_1, edF32VECTOR4* param_2)
{
	float fVar1;
	float fVar2;
	float fVar3;

	this->field_0x60 = *param_2;
	this->field_0x70 = *param_1;

	edF32Vector4SubHard(&this->field_0xd0, &this->field_0x70, &this->field_0x60);
	fVar1 = edF32Vector4NormalizeHard(&this->field_0xd0, &this->field_0xd0);
	this->field_0xe0 = fVar1;
	edF32Matrix4BuildFromVectorUnitSoft(&this->field_0x20, &this->field_0xd0);
	edF32Matrix4MulF32Hard(this->field_0xe0, &this->field_0x20, &this->field_0x20);
	this->field_0x20.rowT = this->field_0x60;

	return;
}

void CFxSpark::Draw(bool param_2)
{}
