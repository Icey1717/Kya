#include "Fx_Spark.h"
#include "DlistManager.h"
#include "MathOps.h"
#include "edDlist.h"
#include "FileManager3D.h"
#include "TimeController.h"
#include "edDList/edDList.inl"

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
{
	CGlobalDListManager* pManager;
	CGlobalDListPatch* pPatch;
	DATA* pData;
	uint sparkIndex;
	uint vertexIndex;
	uint colorIndex;
	int vertexOffset;
	int pointIndex;
	int alpha;
	byte bVar1;
	_rgba color;

	pManager = CScene::ptable.g_GlobalDListManager_004516bc;
	if (this->particleID != -1) {
		if (param_2 != false) {
			pPatch = GameDListPatch_BeginCurrent(this->dlistPatchId);
			this->field_0xc8 = pPatch;
			if (pPatch != (CGlobalDListPatch*)0x0) {
				pManager->SetActive(this->dlistPatchId, 1);
				vertexOffset = 0;
				edF32Matrix4CopyHard(&this->field_0xc8->pCurrentPatch->pDisplayListCommand->matrix, &this->field_0x20);
				sparkIndex = 0;

				if (this->count_0x98 != 0) {
					do {
						pData = this->pVector_0xc + sparkIndex;
						if ((pData->field_0x8 == 0) || (pData->field_0x0 == 0.0f)) {
							pointIndex = 0;
							if (0 < this->count_0xa0) {
								do {
									vertexIndex = vertexOffset + pointIndex * 2;
									colorIndex = vertexIndex;
									if (0x47 < vertexIndex) {
										colorIndex = vertexIndex + ((vertexIndex - 0x48) / 0x46 + 1) * 2;
									}
									color.rgba = this->field_0xc8->pCurrentPatch->pRgba[colorIndex].rgba & 0xffffff;
									edDListPatchRGBA_Inline(this->field_0xc8->pCurrentPatch->pRgba, color, vertexIndex, this->field_0xc8->pCurrentPatch->nbMatrices);
									vertexIndex = vertexIndex + 1;
									colorIndex = vertexIndex;
									if (0x47 < vertexIndex) {
										colorIndex = vertexIndex + ((vertexIndex - 0x48) / 0x46 + 1) * 2;
									}
									gpCurPatchRGBABuf = 0;
									color.rgba = this->field_0xc8->pCurrentPatch->pRgba[colorIndex].rgba & 0xffffff;
									edDListPatchRGBA_Inline(this->field_0xc8->pCurrentPatch->pRgba, color, vertexIndex, this->field_0xc8->pCurrentPatch->nbMatrices);
									pointIndex = pointIndex + 1;
								} while (pointIndex < this->count_0xa0);
							}
						}
						else {
							pointIndex = 0;
							alpha = (int)((float)(this->field_0x90 >> 0x18) * pData->field_0x0);
							if (0 < this->count_0xa0) {
								do {
									vertexIndex = vertexOffset + pointIndex * 2;
									colorIndex = vertexIndex;
									if (0x47 < vertexIndex) {
										colorIndex = vertexIndex + ((vertexIndex - 0x48) / 0x46 + 1) * 2;
									}
									color.rgba = (this->field_0xc8->pCurrentPatch->pRgba[colorIndex].rgba & 0xffffff) | (uint)alpha << 0x18;
									edDListPatchRGBA_Inline(this->field_0xc8->pCurrentPatch->pRgba, color, vertexIndex, this->field_0xc8->pCurrentPatch->nbMatrices);
									vertexIndex = vertexIndex + 1;
									colorIndex = vertexIndex;
									if (0x47 < vertexIndex) {
										colorIndex = vertexIndex + ((vertexIndex - 0x48) / 0x46 + 1) * 2;
									}
									gpCurPatchRGBABuf = 0;
									color.rgba = (this->field_0xc8->pCurrentPatch->pRgba[colorIndex].rgba & 0xffffff) | (uint)alpha << 0x18;
									edDListPatchRGBA_Inline(this->field_0xc8->pCurrentPatch->pRgba, color, vertexIndex, this->field_0xc8->pCurrentPatch->nbMatrices);
									pointIndex = pointIndex + 1;
								} while (pointIndex < this->count_0xa0);
							}
						}

						vertexOffset = vertexOffset + this->count_0xa0 * 2;
						pData->field_0x0 = pData->field_0x0 - (this->field_0xb8 / (float)this->count_0x98) * GetTimer()->cutsceneDeltaTime;
						if (pData->field_0x0 < 0.0f) {
							pData->field_0x0 = 0.0f;
							pData->field_0x8 = 0;
						}

						sparkIndex = sparkIndex + 1;
					} while (sparkIndex < (uint)this->count_0x98);
				}

				if ((this->field_0xbc <= 0.0f) || (this->pVector_0xcc->field_0x9 != 0)) {
					this->field_0xbc = 1.0f / this->field_0xb8;
					UpdateVertices();

					bVar1 = this->pVector_0xcc->field_0x9;
					this->pVector_0xcc->field_0x9 = bVar1 ^ 1;
					if (bVar1 != 0) {
						this->field_0x9c = (this->field_0x9c + 1) % this->count_0x98;
						this->pVector_0xcc = this->pVector_0xc + this->field_0x9c;
					}
				}

				this->field_0xbc = this->field_0xbc - GetTimer()->cutsceneDeltaTime;
				GameDListPatch_EndCurrent(-1, 0);
				return;
			}
		}

		pManager->SetActive(this->dlistPatchId, 0);
	}

	return;
}

// 001d5ab0
void CFxSpark::UpdateVertices()
{
	edF32VECTOR4 vertex;
	edF32VECTOR4* pPoint;
	uint vertexIndex;
	int pointIndex;
	float width;

	*this->pFloat_0x10 = this->field_0xb0;
	this->pFloat_0x10[this->count_0xa0 - 1] = this->field_0xb0;
	vertexIndex = this->count_0xa0 * this->field_0x9c * 2;
	vertex = { this->field_0xb0 / this->field_0xe0, 0.0f, 0.0f, 0.0f };
	// The first pair starts a new strip: the skip word is 0x0000c000.
	uint skip = 0xc000;
	memcpy(&vertex.w, &skip, sizeof(skip));
	edDListPatchVertex_Inline(this->field_0xc8->pCurrentPatch->pVertex, (edF32VECTOR3*)&vertex, &vertex.w, vertexIndex);
	vertex.x = -this->field_0xb0 / this->field_0xe0;
	edDListPatchVertex_Inline(this->field_0xc8->pCurrentPatch->pVertex, (edF32VECTOR3*)&vertex, &vertex.w, vertexIndex + 1);
	vertex.x = this->field_0xb0 / this->field_0xe0;
	vertex.z = 1.0f;
	vertex.w = 1.0f;
	edDListPatchVertex_Inline(this->field_0xc8->pCurrentPatch->pVertex, (edF32VECTOR3*)&vertex, &vertex.w, vertexIndex + this->count_0xa0 * 2 - 2);
	vertex.x = -this->field_0xb0 / this->field_0xe0;
	edDListPatchVertex_Inline(this->field_0xc8->pCurrentPatch->pVertex, (edF32VECTOR3*)&vertex, &vertex.w, vertexIndex + this->count_0xa0 * 2 - 1);
	if (this->pVector_0xcc->field_0x9 == 0) {
		SubdivideVertices(0, this->count_0xa0 - 1);
	}
	else {
		pointIndex = 1;
		if (1 < this->count_0xa0 - 1) {
			do {
				width = this->pFloat_0x10[pointIndex];
				pPoint = this->field_0xe4 + pointIndex + this->field_0x9c * this->count_0xa0;
				vertex = *pPoint;
				vertex.y = pPoint->y - width;
				edDListPatchVertex_Inline(this->field_0xc8->pCurrentPatch->pVertex, (edF32VECTOR3*)&vertex, &vertex.w, vertexIndex + pointIndex * 2);
				vertex.y = pPoint->y + width;
				edDListPatchVertex_Inline(this->field_0xc8->pCurrentPatch->pVertex, (edF32VECTOR3*)&vertex, &vertex.w, vertexIndex + pointIndex * 2 + 1);
				pointIndex = pointIndex + 1;
			} while (pointIndex < this->count_0xa0 - 1);
		}
	}
	this->pVector_0xcc->field_0x0 = 1.0f;
	this->pVector_0xcc->field_0x8 = 1;
	return;
}

// 001d4d60
void CFxSpark::SubdivideVertices(int first, int last)
{
	int middle;
	int vertexOffset;
	float spread;
	float width;
	edF32VECTOR4 difference;
	edF32VECTOR4 leftVertex;
	edF32VECTOR4 midpoint;
	edF32VECTOR4 lastPoint;
	edF32VECTOR4 firstPoint;

	middle = (first + last) >> 1;
	if (middle != first) {
		vertexOffset = this->count_0xa0 * this->field_0x9c * 2;
		firstPoint = this->field_0xe4[first + this->field_0x9c * this->count_0xa0];
		lastPoint = this->field_0xe4[last + this->field_0x9c * this->count_0xa0];
		edF32Vector4SubHard(&difference, &lastPoint, &firstPoint);
		edF32Vector4AddHard(&midpoint, &lastPoint, &firstPoint);
		edF32Vector4ScaleHard(0.5f, &midpoint, &midpoint);
		this->pFunc_0xc4(&midpoint, &this->vector_0x80);
		spread = difference.z + difference.z;
		difference.z = -difference.z;
		midpoint.x = midpoint.x + this->field_0xb4 * (difference.z + spread * ((float)rand() / 2.147484e+09f));
		midpoint.y = midpoint.y + this->field_0xb4 * (difference.z + spread * ((float)rand() / 2.147484e+09f));
		midpoint.z = midpoint.z + this->field_0xb4 * (difference.z + spread * ((float)rand() / 2.147484e+09f));
		this->field_0xe4[middle + this->field_0x9c * this->count_0xa0] = midpoint;
		width = (this->field_0xac * (2.0f - ((float)rand() / 2.147484e+09f) * 1.5f) +
			this->pFloat_0x10[first] + this->pFloat_0x10[last]) / (this->field_0xe0 * 3.0f);
		this->pFloat_0x10[middle] = width;
		midpoint.w = 1.0f;
		leftVertex = midpoint;
		leftVertex.x = midpoint.x - width;
		midpoint.x = midpoint.x + width;
		edDListPatchVertex_Inline(this->field_0xc8->pCurrentPatch->pVertex, (edF32VECTOR3*)&midpoint, &midpoint.w, vertexOffset + middle * 2);
		edDListPatchVertex_Inline(this->field_0xc8->pCurrentPatch->pVertex, (edF32VECTOR3*)&leftVertex, &leftVertex.w, vertexOffset + middle * 2 + 1);
		SubdivideVertices(first, middle);
		SubdivideVertices(middle, last);
	}
	return;
}
