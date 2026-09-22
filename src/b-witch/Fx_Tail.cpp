#include "Fx_Tail.h"
#include "edMem.h"
#include "DlistManager.h"
#include "MathOps.h"
#include "FileManager3D.h"
#include "edDList/edDList.inl"

#ifdef PLATFORM_WIN
#include "displaylist.h"
#endif

char* gDefaultOrder = "XYZ";

CFxTail::CFxTail()
	: pData_0x18((void*)0x0)
{
}

CFxTail::~CFxTail()
{
	if (this->pData_0x18 != (undefined*)0x0) {
		edMemFree(this->pData_0x18);
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

	edDListUseMaterial(CScene::ptable.g_C3DFileManager_00451664->GetMaterialFromId(this->particleID_0x14, 0));

	edDListBegin(0.0f, 0.0f, 0.0f, 4, (this->count_0x34 + 1) * 2);
	s = -8.0f;
	iVar2 = 0;
	if (0 < this->count_0x34 + 1) {
		do {
			edDListColor4u8(this->field_0x24.r, this->field_0x24.g, this->field_0x24.b, this->field_0x24.a);
			edDListTexCoo2f(s, this->field_0xc0);
			edDListVertex4f(0.0f, 0.0f, 0.0f, 49152.0f);
			edDListTexCoo2f(s, this->field_0xc4);
			edDListVertex4f(0.0f, 0.0f, 0.0f, 49152.0f);
			fVar3 = s + this->field_0xb8;
			s = -8.0f;
			if (fVar3 <= 8.0f) {
				s = fVar3;
			}

			iVar2 = iVar2 + 1;
		} while (iVar2 < this->count_0x34 + 1);
	}

	edDListEnd();

	return true;
}

void CFxTail::Create(float param_1, int count, int param_4, int materialId)
{
	undefined* puVar1;

	this->pData_0x18 = edMemAlloc(TO_HEAP(H_MAIN), (count + 1) * 4);

	if (param_4 == 0) {
		param_4 = 1;
	}

	this->flags = 0;
	this->count_0x34 = count;
	//this->field_0x2c = param_4;
	//this->field_0x3c = 0;
	//this->field_0x8 = 0;
	this->particleID_0x14 = materialId;
	//this->field_0x20 = this->field_0x1c;
	//this->field_0x1c = param_1 * 0.5;
	//this->field_0x20 = param_1 * 0.5;

	this->field_0x28 = this->field_0x24;
	this->field_0x24.rgba = 0x80808080;
	this->field_0x28 = this->field_0x24;

	this->field_0xb0 = 2;
	this->szOrder[0] = gDefaultOrder[0];
	this->szOrder[1] = gDefaultOrder[1];
	this->szOrder[2] = gDefaultOrder[2];
	this->szOrder[3] = gDefaultOrder[3];

	return;
}

void CFxTail::Init(float param_1, int id)
{
	this->sectorId = id;
	this->field_0xc0 = 0.0f;
	this->field_0xc4 = 1.0f;
	this->field_0xb8 = param_1 / (float)this->count_0x34;
	Reset();
	this->dlistPatchId = GameDListPatch_Register(this, (this->count_0x34 + 1) * 2, 0);
	this->field_0x30 = (float)this->field_0x24.a / (float)this->count_0x34;
	return;
}

void CFxTail::Reset()
{
	int iVar2;
	int iVar3;

	this->flags = this->flags & 0xfffdefff;
	this->field_0xbc = -8.0;
	//this->field_0x38 = 0;
	//this->field_0x3c = 0;
	//this->field_0x40 = -1;
	//this->field_0x50 = 0;
	//this->field_0x54 = 0;
	//this->field_0x58 = 0;
	//this->field_0x5c = 0;
	//this->field_0x60 = this->field_0x50;
	//this->field_0x64 = this->field_0x54;
	//this->field_0x68 = this->field_0x58;
	//this->field_0x6c = this->field_0x5c;
	//
	//this->field_0x70 = gF32Matrix4Unit;
	//
	//iVar3 = 0;
	//if (0 < this->count_0x34 + 1) {
	//	iVar2 = 0;
	//	do {
	//		iVar3 = iVar3 + 1;
	//		*(undefined4*)(this->pData_0x18 + iVar2) = 0;
	//		iVar2 = iVar2 + 4;
	//	} while (iVar3 < this->count_0x34 + 1);
	//}

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
		this->field_0x8 = GameDListPatch_BeginCurrent(this->dlistPatchId);
		if (this->field_0x8 != (CGlobalDListPatch*)0x0) {
			edF32Matrix4CopyHard(&this->field_0x8->pCurrentPatch->pDisplayListCommand->matrix, &this->field_0x70);
			if (param_4 == 0) {
				_CreateSegment(param_2, param_3);
				this->field_0x38 = this->field_0x38 + 1;
				iVar3 = this->count_0x34 + 1;
				if (iVar3 < this->field_0x38) {
					this->field_0x38 = iVar3;
				}

				if (this->count_0x34 == 0) {
					trap(7);
				}

				this->field_0x3c = (this->field_0x3c + 1) % this->count_0x34;
			}

			_ManageLife();
			GameDListPatch_EndCurrent(this->field_0x38 << 1, 1);
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
	if (1 < this->field_0x38) {
		this->pData_0x18[this->count_0x34] = *this->pData_0x18;
	}

	iVar2 = this->field_0x38;
	iVar3 = 0;
	if (0 < iVar2) {
		if (8 < iVar2) {
			pfVar1 = this->pData_0x18;
			do {
				iVar3 = iVar3 + 8;
				fVar4 = fVar4 + *pfVar1 + pfVar1[1] + pfVar1[2] + pfVar1[3] + pfVar1[4] + pfVar1[5] + pfVar1[6] + pfVar1[7];
				pfVar1 = pfVar1 + 8;
			} while (iVar3 < iVar2 + -8);
		}

		if (iVar3 < iVar2) {
			pfVar1 = this->pData_0x18 + iVar3;
			do {
				iVar3 = iVar3 + 1;
				fVar4 = fVar4 + *pfVar1;
				pfVar1 = pfVar1 + 1;
			} while (iVar3 < iVar2);
		}
	}

	if (fVar4 == 0.0f) {
		this->flags = this->flags & 0xffffefff;
		this->field_0x38 = 0;
		this->field_0x3c = 0;
	}
	else {
		iVar3 = 0;
		if (0 < iVar2) {
			do {
				pfVar1 = this->pData_0x18 + iVar3;
				if (this->field_0x30 <= *pfVar1) {
					*pfVar1 = *pfVar1 - this->field_0x30;
				}
				else {
					*pfVar1 = 0.0f;
				}
				iVar3 = iVar3 + 1;
			} while (iVar3 < this->field_0x38);
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
	if (0 < this->field_0x38) {
		uVar7 = 0;
		uVar6 = 1;
		do {
			iVar11 = static_cast<int>(this->pData_0x18[iVar10]);
			uVar4 = uVar7;
			if (0x47 < uVar7) {
				uVar4 = uVar7 + ((uVar7 - 0x48) / 0x46 + 1) * 2;
			}

			pSVar1 = this->field_0x8->pCurrentPatch;
			_Var2 = (_rgba)(pSVar1->pRgba[uVar4].rgba & 0xffffff | iVar11 << 0x18);
			edDListPatchRGBA_Inline(pSVar1->pRgba, _Var2, uVar7, pSVar1->nbMatrices);

			uVar4 = uVar6;
			if (0x47 < uVar6) {
				uVar4 = uVar6 + ((uVar6 - 0x48) / 0x46 + 1) * 2;
			}

			gpCurPatchRGBABuf = 0;
			_Var2 = (_rgba)(pSVar1->pRgba[uVar4].rgba & 0xffffff | iVar11 << 0x18);
			edDListPatchRGBA_Inline(pSVar1->pRgba, _Var2, uVar6, pSVar1->nbMatrices);

			iVar10 = iVar10 + 1;
			uVar7 = uVar7 + 2;
			uVar6 = uVar6 + 2;
		} while (iVar10 < this->field_0x38);
	}

	iVar8 = this->count_0x34;
	iVar10 = (this->field_0x3c + iVar8 + -1) % iVar8;
	if (iVar8 == 0) {
		trap(7);
	}

	iVar8 = this->field_0x2c;
	iVar11 = 0;
	if (0 < iVar8) {
		do {
			uVar7 = iVar10 * 2;
			iVar8 = static_cast<int>(((float)iVar11 / (float)iVar8) * this->pData_0x18[iVar10]);
			uVar6 = uVar7;
			if (0x47 < uVar7) {
				uVar6 = uVar7 + ((uVar7 - 0x48) / 0x46 + 1) * 2;
			}
			pSVar1 = this->field_0x8->pCurrentPatch;
			_Var2 = (_rgba)(pSVar1->pRgba[uVar6].rgba & 0xffffff | iVar8 << 0x18);
			edDListPatchRGBA_Inline(pSVar1->pRgba, _Var2, uVar7, pSVar1->nbMatrices);

			uVar4 = uVar7 + 1;
			uVar6 = uVar4;
			if (0x47 < uVar4) {
				uVar6 = uVar4 + ((uVar7 - 0x47) / 0x46 + 1) * 2;
			}

			gpCurPatchRGBABuf = 0;
			_Var2 = (_rgba)(pSVar1->pRgba[uVar6].rgba & 0xffffff | iVar8 << 0x18);
			edDListPatchRGBA_Inline(pSVar1->pRgba, _Var2, uVar4, pSVar1->nbMatrices);

			if (iVar10 == 0) {
				uVar7 = this->count_0x34 * 2;
				uVar6 = uVar7;
				if (0x47 < uVar7) {
					uVar6 = uVar7 + ((uVar7 - 0x48) / 0x46 + 1) * 2;
				}

				pSVar1 = this->field_0x8->pCurrentPatch;
				_Var2 = (_rgba)(pSVar1->pRgba[uVar6].rgba & 0xffffff | iVar8 << 0x18);
				edDListPatchRGBA_Inline(pSVar1->pRgba, _Var2, uVar7, pSVar1->nbMatrices);

				iVar3 = this->count_0x34 * 2;
				uVar7 = iVar3 + 1;
				uVar6 = uVar7;
				if (0x47 < uVar7) {
					uVar6 = uVar7 + ((iVar3 - 0x47U) / 0x46 + 1) * 2;
				}

				gpCurPatchRGBABuf = 0;

				_Var2 = (_rgba)(pSVar1->pRgba[uVar6].rgba & 0xffffff | iVar8 << 0x18);
				edDListPatchRGBA_Inline(pSVar1->pRgba, _Var2, uVar7, pSVar1->nbMatrices);
			}

			iVar8 = this->count_0x34;
			iVar10 = (iVar10 + iVar8 + -1) % iVar8;
			if (iVar8 == 0) {
				trap(7);
			}

			iVar8 = this->field_0x2c;
			iVar11 = iVar11 + 1;
		} while (iVar11 < iVar8);
	}

	return;
}

bool CFxTail::Manage(edF32VECTOR4* param_2, int param_3, int param_4)
{
	IMPLEMENTATION_GUARD_FX();
	return true;
}
