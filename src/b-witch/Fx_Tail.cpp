#include "Fx_Tail.h"
#include "edMem.h"
#include "DlistManager.h"
#include "MathOps.h"
#include "FileManager3D.h"
#include "TimeController.h"
#include "edDlist.h"
#include "edDList/edDList.inl"

#ifdef PLATFORM_WIN
#include "displaylist.h"
#endif

char* gDefaultOrder = "XYZ";

CFxTail::CFxTail()
	: pSegmentAlpha((float*)0x0)
{
}

CFxTail::~CFxTail()
{
	if (this->pSegmentAlpha != (float*)0x0) {
		edMemFree(this->pSegmentAlpha);
	}

	return;
}

bool CFxTail::IsKindOfObject(ulong kind)
{
	return (kind & 0x110000) != 0;
}

bool CFxTail::InitDlistPatchable(int)
{
	int iVar2;
	float fVar3;
	float s;

	edDListLoadIdentity();

	edDListUseMaterial(CScene::ptable.g_C3DFileManager_00451664->GetMaterialFromId(this->materialId, 0));

	edDListBegin(0.0f, 0.0f, 0.0f, 4, (this->nbSegments + 1) * 2);
	s = -8.0f;
	iVar2 = 0;
	if (0 < this->nbSegments + 1) {
		do {
			edDListColor4u8(this->color.r, this->color.g, this->color.b, this->color.a);
			edDListTexCoo2f(s, this->negativeEdgeTextureT);
			edDListVertex4f(0.0f, 0.0f, 0.0f, 49152.0f);
			edDListTexCoo2f(s, this->positiveEdgeTextureT);
			edDListVertex4f(0.0f, 0.0f, 0.0f, 49152.0f);
			fVar3 = s + this->textureSStep;
			s = -8.0f;
			if (fVar3 <= 8.0f) {
				s = fVar3;
			}

			iVar2 = iVar2 + 1;
		} while (iVar2 < this->nbSegments + 1);
	}

	edDListEnd();

	return true;
}

void CFxTail::Create(float param_1, int count, int param_4, int materialId)
{
	undefined* puVar1;

	this->pSegmentAlpha = reinterpret_cast<float*>(edMemAlloc(TO_HEAP(H_MAIN), (count + 1) * sizeof(float)));

	if (param_4 == 0) {
		param_4 = 1;
	}

	this->flags = 0;
	this->nbSegments = count;
	this->headFadeSegmentCount = param_4;
	this->nextSegmentIndex = 0;
	this->pDlistPatch = 0;
	this->materialId = materialId;
	this->previousHalfWidth = this->halfWidth;
	this->halfWidth = param_1 * 0.5f;
	this->previousHalfWidth = param_1 * 0.5f;

	this->previousColor = this->color;
	this->color.rgba = 0x80808080;
	this->previousColor = this->color;

	this->widthAxis = 2;
	this->szOrder[0] = gDefaultOrder[0];
	this->szOrder[1] = gDefaultOrder[1];
	this->szOrder[2] = gDefaultOrder[2];
	this->szOrder[3] = gDefaultOrder[3];

	return;
}

void CFxTail::Init(float param_1, int id)
{
	this->sectorId = id;
	this->negativeEdgeTextureT = 0.0f;
	this->positiveEdgeTextureT = 1.0f;
	this->textureSStep = param_1 / (float)this->nbSegments;

	Reset();

	this->dlistPatchId = GameDListPatch_Register(this, (this->nbSegments + 1) * 2, 0);
	this->alphaDecayPerUpdate = (float)this->color.a / (float)this->nbSegments;
	return;
}

void CFxTail::Reset()
{
	int iVar2;
	int iVar3;

	this->flags = this->flags & 0xfffdefff;
	this->nextTextureS = -8.0f;
	this->nbUsedSegments = 0;
	this->nextSegmentIndex = 0;
	this->pendingBreakSegmentIndex = -1;
	this->rotationEuler.x = 0.0f;
	this->rotationEuler.y = 0.0f;
	this->rotationEuler.z = 0.0f;
	this->rotationEuler.w = 0.0f;

	this->previousRotationEuler = this->rotationEuler;
	
	this->transformMatrix = gF32Matrix4Unit;
	
	iVar3 = 0;
	if (0 < this->nbSegments + 1) {
		do {
			this->pSegmentAlpha[iVar3] = 0.0f;
			iVar3 = iVar3 + 1;
		} while (iVar3 < this->nbSegments + 1);
	}

	return;
}

void CFxTail::ChangeOrder(char* pNewOrder)
{
	this->szOrder[0] = pNewOrder[0];
	this->szOrder[1] = pNewOrder[1];
	this->szOrder[2] = pNewOrder[2];
	this->szOrder[3] = pNewOrder[3];

	return;
}

bool CFxTail::SetPatchActive(int bActive)
{
	CGlobalDListManager* pDlistManager;

	pDlistManager = reinterpret_cast<CGlobalDListManager*>(CScene::GetManager(MO_GlobalDListManager));
	return pDlistManager->SetActive(this->dlistPatchId, bActive);
}

bool CFxTail::Manage(edF32VECTOR4* param_2, edF32VECTOR4* param_3, int param_4)
{
	bool bVar1;
	uint uVar2;
	int iVar3;

	if (param_4 == 0) {
		this->flags = this->flags & 0xfffdffff;
	}
	else {
		this->flags = this->flags | 0x20000;
	}

	if (((this->flags & 0x1000) == 0) && ((this->flags & 0x20000) != 0)) {
		bVar1 = true;
	}
	else {
		this->flags = this->flags | 0x1000;
		this->pDlistPatch = GameDListPatch_BeginCurrent(this->dlistPatchId);
		if (this->pDlistPatch != (CGlobalDListPatch*)0x0) {
			edF32Matrix4CopyHard(&this->pDlistPatch->pCurrentPatch->pDisplayListCommand->matrix, &this->transformMatrix);
			if (param_4 == 0) {
				_CreateSegment(param_2, param_3);
				this->nbUsedSegments = this->nbUsedSegments + 1;
				iVar3 = this->nbSegments + 1;
				if (iVar3 < this->nbUsedSegments) {
					this->nbUsedSegments = iVar3;
				}

				if (this->nbSegments == 0) {
					trap(7);
				}

				this->nextSegmentIndex = (this->nextSegmentIndex + 1) % this->nbSegments;
			}

			_ManageLife();
			GameDListPatch_EndCurrent(this->nbUsedSegments << 1, 1);
		}

		bVar1 = false;
	}

	return bVar1;
}

void CFxTail::_ManageLife()
{
	float* pfVar1;
	int iVar2;
	int iVar3;
	float fVar4;

	fVar4 = 0.0f;
	if (1 < this->nbUsedSegments) {
		this->pSegmentAlpha[this->nbSegments] = *this->pSegmentAlpha;
	}

	iVar2 = this->nbUsedSegments;
	iVar3 = 0;
	if (0 < iVar2) {
		if (8 < iVar2) {
			pfVar1 = this->pSegmentAlpha;
			do {
				iVar3 = iVar3 + 8;
				fVar4 = fVar4 + *pfVar1 + pfVar1[1] + pfVar1[2] + pfVar1[3] + pfVar1[4] + pfVar1[5] + pfVar1[6] + pfVar1[7];
				pfVar1 = pfVar1 + 8;
			} while (iVar3 < iVar2 + -8);
		}

		if (iVar3 < iVar2) {
			pfVar1 = this->pSegmentAlpha + iVar3;
			do {
				iVar3 = iVar3 + 1;
				fVar4 = fVar4 + *pfVar1;
				pfVar1 = pfVar1 + 1;
			} while (iVar3 < iVar2);
		}
	}

	if (fVar4 == 0.0f) {
		this->flags = this->flags & 0xffffefff;
		this->nbUsedSegments = 0;
		this->nextSegmentIndex = 0;
	}
	else {
		iVar3 = 0;
		if (0 < iVar2) {
			do {
				pfVar1 = this->pSegmentAlpha + iVar3;
				if (this->alphaDecayPerUpdate <= *pfVar1) {
					*pfVar1 = *pfVar1 - this->alphaDecayPerUpdate;
				}
				else {
					*pfVar1 = 0.0f;
				}
				iVar3 = iVar3 + 1;
			} while (iVar3 < this->nbUsedSegments);
		}

		_PatchListAlpha();
	}

	return;
}

void CFxTail::_PatchListAlpha()
{
	S_GLOBAL_DLIST_PATCH* pSVar1;
	_rgba _Var2;
	int iVar3;
	uint uVar4;
	uint uVar6;
	uint uVar7;
	int iVar8;
	int iVar10;
	int iVar11;

	iVar10 = 0;
	if (0 < this->nbUsedSegments) {
		uVar7 = 0;
		uVar6 = 1;
		do {
			iVar11 = static_cast<int>(this->pSegmentAlpha[iVar10]);
			pSVar1 = this->pDlistPatch->pCurrentPatch;
			_Var2 = (_rgba)(edDListGetRGBA_Inline(pSVar1->pRgba, uVar7).rgba & 0xffffff | iVar11 << 0x18);
			edDListPatchRGBA_Inline(pSVar1->pRgba, _Var2, uVar7, pSVar1->nbMatrices);

			gpCurPatchRGBABuf = 0;
			_Var2 = (_rgba)(edDListGetRGBA_Inline(pSVar1->pRgba, uVar6).rgba & 0xffffff | iVar11 << 0x18);
			edDListPatchRGBA_Inline(pSVar1->pRgba, _Var2, uVar6, pSVar1->nbMatrices);

			iVar10 = iVar10 + 1;
			uVar7 = uVar7 + 2;
			uVar6 = uVar6 + 2;
		} while (iVar10 < this->nbUsedSegments);
	}

	iVar8 = this->nbSegments;
	iVar10 = (this->nextSegmentIndex + iVar8 + -1) % iVar8;
	if (iVar8 == 0) {
		trap(7);
	}

	iVar8 = this->headFadeSegmentCount;
	iVar11 = 0;
	if (0 < iVar8) {
		do {
			uVar7 = iVar10 * 2;
			iVar8 = static_cast<int>(((float)iVar11 / (float)iVar8) * this->pSegmentAlpha[iVar10]);
			pSVar1 = this->pDlistPatch->pCurrentPatch;
			_Var2 = (_rgba)(edDListGetRGBA_Inline(pSVar1->pRgba, uVar7).rgba & 0xffffff | iVar8 << 0x18);
			edDListPatchRGBA_Inline(pSVar1->pRgba, _Var2, uVar7, pSVar1->nbMatrices);

			uVar4 = uVar7 + 1;
			gpCurPatchRGBABuf = 0;
			_Var2 = (_rgba)(edDListGetRGBA_Inline(pSVar1->pRgba, uVar4).rgba & 0xffffff | iVar8 << 0x18);
			edDListPatchRGBA_Inline(pSVar1->pRgba, _Var2, uVar4, pSVar1->nbMatrices);

			if (iVar10 == 0) {
				uVar7 = this->nbSegments * 2;
				pSVar1 = this->pDlistPatch->pCurrentPatch;
				_Var2 = (_rgba)(edDListGetRGBA_Inline(pSVar1->pRgba, uVar7).rgba & 0xffffff | iVar8 << 0x18);
				edDListPatchRGBA_Inline(pSVar1->pRgba, _Var2, uVar7, pSVar1->nbMatrices);

				iVar3 = this->nbSegments * 2;
				uVar7 = iVar3 + 1;

				gpCurPatchRGBABuf = 0;

				_Var2 = (_rgba)(edDListGetRGBA_Inline(pSVar1->pRgba, uVar7).rgba & 0xffffff | iVar8 << 0x18);
				edDListPatchRGBA_Inline(pSVar1->pRgba, _Var2, uVar7, pSVar1->nbMatrices);
			}

			iVar8 = this->nbSegments;
			iVar10 = (iVar10 + iVar8 + -1) % iVar8;
			if (iVar8 == 0) {
				trap(7);
			}

			iVar8 = this->headFadeSegmentCount;
			iVar11 = iVar11 + 1;
		} while (iVar11 < iVar8);
	}

	return;
}

bool CFxTail::Manage(edF32VECTOR4* param_2, int param_3, int param_4)
{
	bool bVar1;
	uint uVar2;
	int iVar3;

	if (param_4 == 0) {
		this->flags = this->flags & 0xfffdffff;
	}
	else {
		this->flags = this->flags | 0x20000;
	}

	if (((this->flags & 0x1000) == 0) && ((this->flags & 0x20000) != 0)) {
		bVar1 = true;
	}
	else {
		this->flags = this->flags | 0x1000;
		this->pDlistPatch = GameDListPatch_BeginCurrent(this->dlistPatchId);
		if (this->pDlistPatch != (CGlobalDListPatch*)0x0) {
			edF32Matrix4CopyHard(&this->pDlistPatch->pCurrentPatch->pDisplayListCommand->matrix, &this->transformMatrix);
			if (param_4 == 0) {
				_CreateSegment(param_2, param_3);
				this->nbUsedSegments = this->nbUsedSegments + param_3;
				iVar3 = this->nbSegments + 1;
				if (iVar3 < this->nbUsedSegments) {
					this->nbUsedSegments = iVar3;
				}

				iVar3 = 0;
				if (0 < param_3) {
					// Had unrolled segment here, assuming a compiler insertion.

					for (; iVar3 < param_3; iVar3 = iVar3 + 1) {
						if (this->nbSegments == 0) {
							trap(7);
						}

						this->nextSegmentIndex = (this->nextSegmentIndex + 1) % this->nbSegments;
					}
				}
			}

			_ManageLife();
			GameDListPatch_EndCurrent(this->nbUsedSegments << 1, 1);
		}

		bVar1 = false;
	}

	return bVar1;
}

// 001d1aa0
void CFxTail::_CreateSegment(edF32VECTOR4* param_2, edF32VECTOR4* param_3)
{
	CGlobalDListManager* pDlistManager;
	S_GLOBAL_DLIST_PATCH* pPatch;
	edVertex* pVertex;
	edVertex skip;
	int patchId;
	int segment;
	uint nextSegment;

	pDlistManager = reinterpret_cast<CGlobalDListManager*>(CScene::GetManager(MO_GlobalDListManager));
	segment = this->nextSegmentIndex;
	if ((this->pDlistPatch != (CGlobalDListPatch*)0x0) && (Timer::GetTimer()->timeScale != 0.0f)) {
		if ((segment == 0) && (1 < this->nbUsedSegments)) {
			patchId = this->dlistPatchId;
			pVertex = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch->pVertex;
			edDListPatchVertex_Inline(pVertex, &param_3->xyz, &param_3->w, this->nbSegments * 2);
			pDlistManager->_AddCallFuncElement2(patchId, 10, (short)(this->nbSegments * 2), STORE_POINTER(pVertex));
			patchId = this->dlistPatchId;
			pVertex = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch->pVertex;
			edDListPatchVertex_Inline(pVertex, &param_2->xyz, &param_2->w, this->nbSegments * 2 + 1);
			pDlistManager->_AddCallFuncElement2(patchId, 10, (short)(this->nbSegments * 2 + 1), STORE_POINTER(pVertex));
			patchId = this->dlistPatchId;
			pPatch = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch;
			edDListPatchRGBA_Inline(pPatch->pRgba, this->color, this->nbSegments * 2, pPatch->nbMatrices);
			pDlistManager->_AddCallFuncElement2(patchId, 0xb, (short)(this->nbSegments * 2), STORE_POINTER(pPatch->pRgba));
			patchId = this->dlistPatchId;
			pPatch = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch;
			edDListPatchRGBA_Inline(pPatch->pRgba, this->color, this->nbSegments * 2 + 1, pPatch->nbMatrices);
			pDlistManager->_AddCallFuncElement2(patchId, 0xb, (short)(this->nbSegments * 2 + 1), STORE_POINTER(pPatch->pRgba));
		}

		this->pSegmentAlpha[segment] = (float)this->color.a;
		patchId = this->dlistPatchId;
		pVertex = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch->pVertex;
		edDListPatchVertex_Inline(pVertex, &param_3->xyz, &param_3->w, segment * 2);
		pDlistManager->_AddCallFuncElement2(patchId, 10, (short)(segment * 2), STORE_POINTER(pVertex));
		patchId = this->dlistPatchId;
		pVertex = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch->pVertex;
		edDListPatchVertex_Inline(pVertex, &param_2->xyz, &param_2->w, segment * 2 + 1);
		pDlistManager->_AddCallFuncElement2(patchId, 10, (short)(segment * 2 + 1), STORE_POINTER(pVertex));
		patchId = this->dlistPatchId;
		pPatch = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch;
		edDListPatchRGBA_Inline(pPatch->pRgba, this->color, segment * 2, pPatch->nbMatrices);
		pDlistManager->_AddCallFuncElement2(patchId, 0xb, (short)(segment * 2), STORE_POINTER(pPatch->pRgba));
		patchId = this->dlistPatchId;
		pPatch = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch;
		edDListPatchRGBA_Inline(pPatch->pRgba, this->color, segment * 2 + 1, pPatch->nbMatrices);
		pDlistManager->_AddCallFuncElement2(patchId, 0xb, (short)(segment * 2 + 1), STORE_POINTER(pPatch->pRgba));

		_PatchSegmentST(this->textureSStep, segment * 2);
		if (this->nextSegmentIndex == this->pendingBreakSegmentIndex) {
			patchId = this->dlistPatchId;
			pPatch = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch;
			edDListPatchRGBA_Inline(pPatch->pRgba, (_rgba)0, this->pendingBreakSegmentIndex * 2, pPatch->nbMatrices);
			pDlistManager->_AddCallFuncElement2(patchId, 0xb, (short)(this->pendingBreakSegmentIndex * 2), STORE_POINTER(pPatch->pRgba));
			patchId = this->dlistPatchId;
			pPatch = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch;
			edDListPatchRGBA_Inline(pPatch->pRgba, (_rgba)0, this->pendingBreakSegmentIndex * 2 + 1, pPatch->nbMatrices);
			pDlistManager->_AddCallFuncElement2(patchId, 0xb, (short)(this->pendingBreakSegmentIndex * 2 + 1), STORE_POINTER(pPatch->pRgba));
		}

		if (this->nbSegments == 0) {
			trap(7);
		}

		nextSegment = ((uint)this->nextSegmentIndex + 1) % (uint)this->nbSegments;
		skip.uSkip = 0xc000;
		if (nextSegment == 0) {
			pVertex = this->pDlistPatch->pCurrentPatch->pVertex;
			edDListPatchVertex_Inline(pVertex, &pVertex[this->nbSegments * 2].vector.xyz, &skip.fSkip, this->nbSegments * 2);
			edDListGetPatchableVertexEnd_Inline();
			pVertex = this->pDlistPatch->pCurrentPatch->pVertex;
			edDListPatchVertex_Inline(pVertex, &pVertex[this->nbSegments * 2 + 1].vector.xyz, &skip.fSkip, this->nbSegments * 2 + 1);
			edDListGetPatchableVertexEnd_Inline();
		}

		pVertex = this->pDlistPatch->pCurrentPatch->pVertex;
		edDListPatchVertex_Inline(pVertex, &pVertex[nextSegment * 2].vector.xyz, &skip.fSkip, nextSegment * 2);
		edDListGetPatchableVertexEnd_Inline();
		pVertex = this->pDlistPatch->pCurrentPatch->pVertex;
		edDListPatchVertex_Inline(pVertex, &pVertex[nextSegment * 2 + 1].vector.xyz, &skip.fSkip, nextSegment * 2 + 1);
		edDListGetPatchableVertexEnd_Inline();

		if (-1 < this->pendingBreakSegmentIndex) {
			if (this->nbUsedSegments < 2) {
				this->pendingBreakSegmentIndex = -1;
			}
			else {
				pDlistManager = reinterpret_cast<CGlobalDListManager*>(CScene::GetManager(MO_GlobalDListManager));
				patchId = this->dlistPatchId;
				pVertex = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch->pVertex;
				edDListPatchVertex_Inline(pVertex, &pVertex[this->pendingBreakSegmentIndex * 2].vector.xyz, &skip.fSkip, this->pendingBreakSegmentIndex * 2);
				edDListGetPatchableVertexEnd_Inline();
				pDlistManager->_AddCallFuncElement2(patchId, 8, (short)(this->pendingBreakSegmentIndex * 2), 0);
				patchId = this->dlistPatchId;
				pVertex = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch->pVertex;
				edDListPatchVertex_Inline(pVertex, &pVertex[this->pendingBreakSegmentIndex * 2 + 1].vector.xyz, &skip.fSkip, this->pendingBreakSegmentIndex * 2 + 1);
				edDListGetPatchableVertexEnd_Inline();
				pDlistManager->_AddCallFuncElement2(patchId, 8, (short)(this->pendingBreakSegmentIndex * 2 + 1), 0);

				this->pSegmentAlpha[this->nextSegmentIndex] = 0.0f;
				this->nextTextureS = -8.0f;
				this->pendingBreakSegmentIndex = -1;
			}
		}
	}

	return;
}

// Scratchpad layout used by 001d2d80 (0xb0 bytes).
struct TailSegmentScratch
{
	edF32MATRIX4 matrix;
	edF32VECTOR4 axis;
	edF32VECTOR4 angleStep;
	edF32VECTOR4 angles;
	edF32VECTOR4 positiveVertex;
	edF32VECTOR4 negativeVertex;
	_rgba color;
	float width;
	uint pointIndex;
	float stStep;
	uint segment;
	undefined padding[12];
};

static_assert(sizeof(TailSegmentScratch) == 0xb0, "Tail segment scratchpad layout");

// 001d2d80
void CFxTail::_CreateSegment(edF32VECTOR4* param_2, int param_3)
{
	TailSegmentScratch* pScratch;
	CGlobalDListManager* pDlistManager;
	S_GLOBAL_DLIST_PATCH* pPatch;
	edVertex* pVertex;
	edF32VECTOR4* pPositive;
	edF32VECTOR4* pNegative;
	edVertex skip;
	int patchId;
	uint nextSegment;

	pScratch = reinterpret_cast<TailSegmentScratch*>(gSP_Manager.GetFreeBuffer(sizeof(TailSegmentScratch)));
	pDlistManager = reinterpret_cast<CGlobalDListManager*>(CScene::GetManager(MO_GlobalDListManager));
	if ((this->pDlistPatch != (CGlobalDListPatch*)0x0) && (Timer::GetTimer()->timeScale != 0.0f)) {
		if (-1 < this->pendingBreakSegmentIndex) {
			this->previousRotationEuler = this->rotationEuler;
		}

		pScratch->angles = this->previousRotationEuler;
		pScratch->stStep = this->textureSStep / (float)param_3;
		pScratch->segment = this->nextSegmentIndex;
		pScratch->angleStep.x = edF32GetAnglesDelta(this->previousRotationEuler.x, this->rotationEuler.x);
		pScratch->angleStep.y = edF32GetAnglesDelta(this->previousRotationEuler.y, this->rotationEuler.y);
		pScratch->angleStep.z = edF32GetAnglesDelta(this->previousRotationEuler.z, this->rotationEuler.z);
		edF32Vector4ScaleHard(1.0f / (float)param_3, &pScratch->angleStep, &pScratch->angleStep);

		if (this->widthAxis == 2) {
			pScratch->axis = { 0.0f, 0.0f, 1.0f, 0.0f };
		}
		else {
			if (this->widthAxis == 1) {
				pScratch->axis = { 0.0f, 1.0f, 0.0f, 0.0f };
			}
			else {
				if (this->widthAxis == 0) {
					pScratch->axis = { 1.0f, 0.0f, 0.0f, 0.0f };
				}
			}
		}

		pScratch->pointIndex = 0;
		pPositive = &pScratch->positiveVertex;
		pNegative = &pScratch->negativeVertex;
		if (pScratch->pointIndex < (uint)param_3) {
			do {
				pScratch->color.LerpRGBA((float)pScratch->pointIndex / (float)param_3, this->previousColor.rgba, this->color.rgba);
				pScratch->angles.x = edF32Between_0_2Pi(pScratch->angles.x + pScratch->angleStep.x);
				pScratch->angles.y = edF32Between_0_2Pi(pScratch->angles.y + pScratch->angleStep.y);
				pScratch->angles.z = edF32Between_0_2Pi(pScratch->angles.z + pScratch->angleStep.z);
				pScratch->width = this->previousHalfWidth + ((float)pScratch->pointIndex / (float)param_3) * (this->halfWidth - this->previousHalfWidth);
				this->pSegmentAlpha[pScratch->segment] = (float)pScratch->color.a;

				edF32Matrix4FromEulerSoft(&pScratch->matrix, &pScratch->angles.xyz, this->szOrder);
				edF32Matrix4MulF32Vector4Hard(pPositive, &pScratch->matrix, &pScratch->axis);
				edF32Vector4ScaleHard(pScratch->width, pPositive, pPositive);
				edF32Vector4SubHard(pNegative, param_2 + pScratch->pointIndex, pPositive);
				edF32Vector4AddHard(pPositive, param_2 + pScratch->pointIndex, pPositive);

				if ((pScratch->segment == 0) && (1 < this->nbUsedSegments)) {
					patchId = this->dlistPatchId;
					pVertex = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch->pVertex;
					edDListPatchVertex_Inline(pVertex, &pNegative->xyz, &pNegative->w, this->nbSegments * 2);
					pDlistManager->_AddCallFuncElement2(patchId, 10, (short)(this->nbSegments * 2), STORE_POINTER(pVertex));
					patchId = this->dlistPatchId;
					pVertex = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch->pVertex;
					edDListPatchVertex_Inline(pVertex, &pPositive->xyz, &pPositive->w, this->nbSegments * 2 + 1);
					pDlistManager->_AddCallFuncElement2(patchId, 10, (short)(this->nbSegments * 2 + 1), STORE_POINTER(pVertex));
					patchId = this->dlistPatchId;
					pPatch = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch;
					edDListPatchRGBA_Inline(pPatch->pRgba, pScratch->color, this->nbSegments * 2, pPatch->nbMatrices);
					pDlistManager->_AddCallFuncElement2(patchId, 0xb, (short)(this->nbSegments * 2), STORE_POINTER(pPatch->pRgba));
					patchId = this->dlistPatchId;
					pPatch = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch;
					edDListPatchRGBA_Inline(pPatch->pRgba, pScratch->color, this->nbSegments * 2 + 1, pPatch->nbMatrices);
					pDlistManager->_AddCallFuncElement2(patchId, 0xb, (short)(this->nbSegments * 2 + 1), STORE_POINTER(pPatch->pRgba));
				}

				patchId = this->dlistPatchId;
				pVertex = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch->pVertex;
				edDListPatchVertex_Inline(pVertex, &pNegative->xyz, &pNegative->w, pScratch->segment * 2);
				pDlistManager->_AddCallFuncElement2(patchId, 10, (short)(pScratch->segment * 2), STORE_POINTER(pVertex));
				patchId = this->dlistPatchId;
				pVertex = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch->pVertex;
				edDListPatchVertex_Inline(pVertex, &pPositive->xyz, &pPositive->w, pScratch->segment * 2 + 1);
				pDlistManager->_AddCallFuncElement2(patchId, 10, (short)(pScratch->segment * 2 + 1), STORE_POINTER(pVertex));
				patchId = this->dlistPatchId;
				pPatch = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch;
				edDListPatchRGBA_Inline(pPatch->pRgba, pScratch->color, pScratch->segment * 2, pPatch->nbMatrices);
				pDlistManager->_AddCallFuncElement2(patchId, 0xb, (short)(pScratch->segment * 2), STORE_POINTER(pPatch->pRgba));
				patchId = this->dlistPatchId;
				pPatch = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch;
				edDListPatchRGBA_Inline(pPatch->pRgba, pScratch->color, pScratch->segment * 2 + 1, pPatch->nbMatrices);
				pDlistManager->_AddCallFuncElement2(patchId, 0xb, (short)(pScratch->segment * 2 + 1), STORE_POINTER(pPatch->pRgba));

				_PatchSegmentST(pScratch->stStep, pScratch->segment * 2);
				if (this->nbSegments == 0) {
					trap(7);
				}

				pScratch->segment = (pScratch->segment + 1) % (uint)this->nbSegments;
				pScratch->pointIndex = pScratch->pointIndex + 1;
			} while (pScratch->pointIndex < (uint)param_3);
		}

		if (this->nextSegmentIndex == this->pendingBreakSegmentIndex) {
			patchId = this->dlistPatchId;
			pPatch = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch;
			edDListPatchRGBA_Inline(pPatch->pRgba, (_rgba)0, this->pendingBreakSegmentIndex * 2, pPatch->nbMatrices);
			pDlistManager->_AddCallFuncElement2(patchId, 0xb, (short)(this->pendingBreakSegmentIndex * 2), STORE_POINTER(pPatch->pRgba));
			patchId = this->dlistPatchId;
			pPatch = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch;
			edDListPatchRGBA_Inline(pPatch->pRgba, (_rgba)0, this->pendingBreakSegmentIndex * 2 + 1, pPatch->nbMatrices);
			pDlistManager->_AddCallFuncElement2(patchId, 0xb, (short)(this->pendingBreakSegmentIndex * 2 + 1), STORE_POINTER(pPatch->pRgba));
		}

		if (this->nbSegments == 0) {
			trap(7);
		}

		nextSegment = ((uint)this->nextSegmentIndex + (uint)param_3) % (uint)this->nbSegments;
		skip.uSkip = 0xc000;
		if (nextSegment == 0) {
			pVertex = this->pDlistPatch->pCurrentPatch->pVertex;
			edDListPatchVertex_Inline(pVertex, &pVertex[this->nbSegments * 2].vector.xyz, &skip.fSkip, this->nbSegments * 2);
			edDListGetPatchableVertexEnd_Inline();
			pVertex = this->pDlistPatch->pCurrentPatch->pVertex;
			edDListPatchVertex_Inline(pVertex, &pVertex[this->nbSegments * 2 + 1].vector.xyz, &skip.fSkip, this->nbSegments * 2 + 1);
			edDListGetPatchableVertexEnd_Inline();
		}

		pVertex = this->pDlistPatch->pCurrentPatch->pVertex;
		edDListPatchVertex_Inline(pVertex, &pVertex[nextSegment * 2].vector.xyz, &skip.fSkip, nextSegment * 2);
		edDListGetPatchableVertexEnd_Inline();
		pVertex = this->pDlistPatch->pCurrentPatch->pVertex;
		edDListPatchVertex_Inline(pVertex, &pVertex[nextSegment * 2 + 1].vector.xyz, &skip.fSkip, nextSegment * 2 + 1);
		edDListGetPatchableVertexEnd_Inline();

		if (-1 < this->pendingBreakSegmentIndex) {
			if (this->nbUsedSegments < 2) {
				this->pendingBreakSegmentIndex = -1;
			}
			else {
				pDlistManager = reinterpret_cast<CGlobalDListManager*>(CScene::GetManager(MO_GlobalDListManager));
				patchId = this->dlistPatchId;
				pVertex = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch->pVertex;
				edDListPatchVertex_Inline(pVertex, &pVertex[this->pendingBreakSegmentIndex * 2].vector.xyz, &skip.fSkip, this->pendingBreakSegmentIndex * 2);
				edDListGetPatchableVertexEnd_Inline();
				pDlistManager->_AddCallFuncElement2(patchId, 8, (short)(this->pendingBreakSegmentIndex * 2), 0);
				patchId = this->dlistPatchId;
				pVertex = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch->pVertex;
				edDListPatchVertex_Inline(pVertex, &pVertex[this->pendingBreakSegmentIndex * 2 + 1].vector.xyz, &skip.fSkip, this->pendingBreakSegmentIndex * 2 + 1);
				edDListGetPatchableVertexEnd_Inline();
				pDlistManager->_AddCallFuncElement2(patchId, 8, (short)(this->pendingBreakSegmentIndex * 2 + 1), 0);

				this->pSegmentAlpha[this->nextSegmentIndex] = 0.0f;
				this->nextTextureS = -8.0f;
				this->pendingBreakSegmentIndex = -1;
			}
		}
	}

	gSP_Manager.ReleaseBuffer(pScratch);

	return;
}

// 001d1330
void CFxTail::_PatchSegmentST(float param_1, uint param_3)
{
	CGlobalDListManager* pDlistManager;
	S_GLOBAL_DLIST_PATCH* pPatch;
	int patchId;
	float s;
	float t0;
	float t1;

	pDlistManager = reinterpret_cast<CGlobalDListManager*>(CScene::GetManager(MO_GlobalDListManager));
	if (param_3 == 0) {
		s = this->nextTextureS;
		this->nextTextureS = fmodf(s, 1.0f) - 8.0f;
		if (s < 0.0f) {
			this->nextTextureS = this->nextTextureS + 1.0f;
		}
	}

	t0 = this->negativeEdgeTextureT;
	s = this->nextTextureS;
	t1 = this->positiveEdgeTextureT;
	patchId = this->dlistPatchId;
	pPatch = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch;
	edDListPatchST_Inline(pPatch->pSt, s, t0, param_3, pPatch->nbMatrices);
	pDlistManager->_AddCallFuncElement2(patchId, 0xc, (short)(param_3), STORE_POINTER(pPatch->pSt));
	patchId = this->dlistPatchId;
	pPatch = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch;
	edDListPatchST_Inline(pPatch->pSt, s, t1, param_3 + 1, pPatch->nbMatrices);
	pDlistManager->_AddCallFuncElement2(patchId, 0xc, (short)(param_3 + 1), STORE_POINTER(pPatch->pSt));

	this->nextTextureS = this->nextTextureS + param_1;
	if (param_3 == (this->nbSegments - 1) * 2) {
		s = this->nextTextureS;
		patchId = this->dlistPatchId;
		pPatch = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch;
		edDListPatchST_Inline(pPatch->pSt, s, t0, param_3 + 2, pPatch->nbMatrices);
		pDlistManager->_AddCallFuncElement2(patchId, 0xc, (short)(param_3 + 2), STORE_POINTER(pPatch->pSt));
		patchId = this->dlistPatchId;
		pPatch = pDlistManager->ppGlobalDlist[patchId >> 0x10].pDlistPatch->pCurrentPatch;
		edDListPatchST_Inline(pPatch->pSt, s, t1, param_3 + 3, pPatch->nbMatrices);
		pDlistManager->_AddCallFuncElement2(patchId, 0xc, (short)(param_3 + 3), STORE_POINTER(pPatch->pSt));
	}

	return;
}
