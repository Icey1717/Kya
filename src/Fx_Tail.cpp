#include "Fx_Tail.h"
#include "edMem.h"
#include "Rendering/DisplayList.h"
#include "DlistManager.h"
#include "FileManager3D.h"

#ifdef PLATFORM_WIN
#include "displaylist.h"
#endif

char* gDefaultOrder = "XYZ";

bool CFxTail::IsKindOfObject(ulong kind)
{
	return (kind & 0x110000) != 0;
}

bool CFxTail::InitDlistPatchable(int)
{
	uint uVar1;
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
			uVar1 = this->field_0x24.rgba;
			edDListColor4u8(this->field_0x24.r, this->field_0x24.g, this->field_0x24.b, this->field_0x24.a);
			edDListTexCoo2f(s, this->field_0xc0);
			edDListVertex4f(0.0f, 0.0f, 0.0f, uVar1);
			edDListTexCoo2f(s, this->field_0xc4);
			edDListVertex4f(0.0f, 0.0f, 0.0f, uVar1);
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
	this->objectId = id;
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
	ulong uVar2;
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
		IMPLEMENTATION_GUARD_LOG(
		uVar2 = (long)(int)this->flags | 0x1000;
		this->flags = (uint)uVar2;
		GameDListPatch_BeginCurrent((long)this->dlistPatchId);
		this->field_0x8 = (int)uVar2;
		if (uVar2 != 0) {
			edF32Matrix4CopyHard
			(*(edF32MATRIX4**)(*(int*)(this->field_0x8 + 0x4d0) + 0x20), (edF32MATRIX4*)&this->field_0x70);
			if (param_4 == 0) {
				CFxTail::_CreateSegment((int)this, (undefined4*)param_2, (undefined4*)param_3);
				this->field_0x38 = this->field_0x38 + 1;
				iVar3 = this->count_0x34 + 1;
				if (iVar3 < (int)this->field_0x38) {
					this->field_0x38 = iVar3;
				}
				if (this->count_0x34 == 0) {
					trap(7);
				}
				this->field_0x3c = (this->field_0x3c + 1) % this->count_0x34;
			}
			_ManageLife(this);
			GameDListPatch_EndCurrent(this->field_0x38 << 1, 1);
		})

		bVar1 = false;
	}

	return bVar1;
}
