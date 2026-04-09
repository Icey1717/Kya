#include "ActorShadows.h"
#include "FileManager3D.h"
#include "edDlist.h"
#include "DlistManager.h"
#include "MathOps.h"

CShadowShared::CShadowShared()
{
	this->position = gF32Vertex4Zero;
	this->field_0x20 = gF32Vector4UnitY;

	this->shadowColor = _rgba(0x50808080);

	this->displayable = 1;
	this->field_0x38 = 1;
	this->field_0x3c = 1.0f;
	this->globalIntensity = 1.0f;
	this->field_0x44 = 1.0f;
	this->field_0x48 = 0.25f;
	this->field_0x4c = 0.0f;

	return;
}

bool CShadowShared::IsKindOfObject(ulong kind)
{
	return (kind & 0x70000) != 0;
}

void CShadowShared::SetDisplayable(int displayable)
{
	this->displayable = displayable;

	return;
}

void CShadowShared::SetDisplayableAlt(int param_2)
{
	this->field_0x38 = param_2;

	return;
}

byte CShadowShared::Compute(edF32MATRIX4 * param_2)
{
	edF32VECTOR4* v0;
	edF32MATRIX4 local_80;
	edF32MATRIX4 auStack64;

	v0 = &auStack64.rowZ;
	auStack64.rowY = this->field_0x20;
	local_80.bb = this->field_0x44 + (1.0f - this->field_0x44) * this->field_0x3c * this->globalIntensity;
	SetVectorFromAngleY(this->field_0x4c, v0);
	edF32Vector4CrossProductHard(&auStack64.rowX, &auStack64.rowY, v0);
	edF32Vector4NormalizeHard(&auStack64.rowX, &auStack64.rowX);
	edF32Vector4CrossProductHard(v0, &auStack64.rowX, &auStack64.rowY);
	edF32Vector4NormalizeHard(v0, v0);
	local_80.aa = local_80.bb * this->field_0x48;
	edF32Matrix4CopyHard(&local_80, &gF32Matrix4Unit);
	local_80.cc = local_80.aa;
	edF32Matrix4MulF32Matrix4Hard(&auStack64, &local_80, &auStack64);
	auStack64.rowT = this->position;
	edF32Matrix4CopyHard(param_2, &auStack64);

	return (byte)((float)(this->shadowColor).a) * this->field_0x3c * this->globalIntensity;
}

bool CShadow::IsKindOfObject(ulong kind)
{
	return (kind & 0x30000) != 0;
}

void CShadow::Init(int sectorId)
{
	CShadowShared::Init(sectorId);

	if (this->materialId != -1) {
		CScene::ptable.g_C3DFileManager_00451664->InstanciateG2D(this->materialId);
	}

	this->dlistPatchId = GameDListPatch_Register(this, 4, 0);

	return;
}

void CShadow::Draw()
{
	byte newAlpha;
	CGlobalDListPatch* pCVar2;
	CGlobalDListManager* pDlistManager;
	edF32MATRIX4 eStack80;
	_rgba newColor;

	if (this->materialId != -1) {
		if (((this->field_0x3c <= 0.0f) || (this->displayable == 0)) ||
			(this->field_0x38 == 0)) {
			pDlistManager = reinterpret_cast<CGlobalDListManager*>(CScene::GetManager(MO_GlobalDListManager));
			pDlistManager->SetActive(this->dlistPatchId, 0);
		}
		else {
			newAlpha = Compute(&eStack80);
			pCVar2 = GameDListPatch_BeginCurrent(this->dlistPatchId);
			if (pCVar2 != (CGlobalDListPatch*)0x0) {
				newColor.rgba = (this->shadowColor.rgba & 0xffffff | static_cast<uint>(newAlpha) << 0x18);
				edF32Matrix4CopyHard(&pCVar2->pCurrentPatch->pDisplayListCommand->matrix, &eStack80);

				pCVar2->pCurrentPatch->pRgba[0] = newColor;
				pCVar2->pCurrentPatch->pRgba[1] = newColor;
				pCVar2->pCurrentPatch->pRgba[2] = newColor;
				pCVar2->pCurrentPatch->pRgba[3] = newColor;

				GameDListPatch_EndCurrent(-1, 0);
			}
		}
	}

	return;
}

byte CShadow::Compute(edF32MATRIX4* param_2)
{
	edF32VECTOR4* v0;
	edF32VECTOR4* v1;
	float fVar1;
	edF32MATRIX4 local_80;
	edF32MATRIX4 auStack64;

	v0 = &auStack64.rowZ;
	local_80.bb = this->field_0x44;
	auStack64.rowY = this->field_0x20;
	local_80.bb = local_80.bb + (1.0f - local_80.bb) * this->field_0x3c * this->globalIntensity;

	SetVectorFromAngleY(this->field_0x4c, v0);
	v1 = &auStack64.rowY;
	edF32Vector4CrossProductHard(&auStack64.rowX, v1, v0);
	fVar1 = edF32Vector4DotProductHard(&auStack64.rowX, &auStack64.rowX);

	if (fVar1 < 1e-06f) {
		edF32Vector4CrossProductHard(&auStack64.rowX, v1, &gF32Vector4UnitY);
	}

	edF32Vector4NormalizeHard(&auStack64.rowX, &auStack64.rowX);
	edF32Vector4CrossProductHard(v0, &auStack64.rowX, v1);
	edF32Vector4NormalizeHard(v0, v0);
	local_80.aa = local_80.bb * this->field_0x48;
	local_80.cc = local_80.bb * this->field_0x50;
	edF32Matrix4CopyHard(&local_80, &gF32Matrix4Unit);
	edF32Matrix4MulF32Matrix4Hard(&auStack64, &local_80, &auStack64);
	auStack64.rowT = this->position;
	edF32Matrix4CopyHard(param_2, &auStack64);

	return (byte)((float)(this->shadowColor).a) * this->field_0x3c * this->globalIntensity;
}

void CShadow::SetDisplayableAlt(int param_2)
{
	bool bVar1;

	if (param_2 != this->field_0x38) {
		bVar1 = param_2 != 0;
		this->field_0x38 = param_2;

		if (bVar1) {
			bVar1 = this->displayable != 0;
		}

		GameDListPatch_SetActive(this->dlistPatchId, bVar1);
	}

	return;
}

void CShadow::SetDisplayable(int displayable)
{
	bool bActive;

	if (displayable != this->displayable) {
		bActive = displayable != 0;
		this->displayable = displayable;

		if (bActive) {
			bActive = this->field_0x38 != 0;
		}

		GameDListPatch_SetActive(this->dlistPatchId, bActive);
	}

	return;
}

void CShadow::SetMaterialId(int newMaterialId)
{
	this->materialId = newMaterialId;

	if (this->materialId != -1) {
		CScene::ptable.g_C3DFileManager_00451664->InstanciateG2D(this->materialId);
	}

	return;
}

bool CShadow::InitDlistPatchable(int patchId)
{
	edDList_material* pMaterialInfo;

	edDListLoadIdentity();
	pMaterialInfo = CScene::ptable.g_C3DFileManager_00451664->GetMaterialFromId(this->materialId, 0);
	edDListUseMaterial(pMaterialInfo);
	edDListBegin(0.0f, 0.0f, 0.0f, 8, 4);
	edDListColor4u8(this->shadowColor.r, this->shadowColor.g, this->shadowColor.b, 0);
	edDListTexCoo2f(0.0f, 0.0f);
	edDListVertex4f(-1.0f, 0.01f, -1.0f, 1.0f);
	edDListTexCoo2f(0.0f, 1.0f);
	edDListVertex4f(-1.0f, 0.01f, 0.5f, 1.0f);
	edDListTexCoo2f(1.0f, 1.0f);
	edDListVertex4f(0.5f, 0.01f, 0.5f, 1.0f);
	edDListEnd();

	return true;
}

void CShadow::Create(int newMaterialId)
{
	this->materialId = newMaterialId;

	return;
}

CShadow::CShadow()
{
	this->materialId = -1;
	this->field_0x50 = 0.25f;

	return;
}

void CShadowShared::Init(int sectorId)
{
	this->field_0x20.x = 0.0f;
	this->field_0x20.y = -1.0f;
	this->field_0x20.z = 0.0f;
	this->field_0x20.w = 0.0f;

	this->sectorId = sectorId;

	return;
}

void CShadowShared::Draw(CGlobalDListPatch* pPatch, uint index)
{
	byte bVar1;
	uint uVar2;
	_rgba* p_Var5;
	uint uVar6;
	edF32VECTOR4 local_a0;
	edF32MATRIX4 eStack144;
	_rgba local_20;
	_rgba local_18;
	_rgba local_10;
	_rgba local_8;

	edVertex* pVertex = pPatch->pCurrentPatch->pVertex;

	if (((this->field_0x3c <= 0.0f) || (this->displayable == 0)) || (this->field_0x38 == 0)) {
		edDListGetPatchableVertexBegin_Inline(pVertex);
		float* pXyz;
		float* pSkip;
		edDListGetPatchableVertex_Inline(index + 2, &pXyz, &pSkip);
		edF32VECTOR3 xyz;
		float skip;
		edDListGetVertexFromDataPatch_Inline(pXyz, pSkip, &xyz, &skip);

		edDListPatchVertex_Inline(pVertex, &xyz, &skip, index + 2);

		edDListGetPatchableVertex_Inline(index + 3, &pXyz, &pSkip);
		edDListGetVertexFromDataPatch_Inline(pXyz, pSkip, &xyz, &skip);

		edDListPatchVertex_Inline(pVertex, &xyz, &skip, index + 3);

		edDListGetPatchableVertexReset_Inline();
	}
	else {
		bVar1 = Compute(&eStack144);

		local_a0.x = -0.5f;
		local_a0.y = 0.01f;
		local_a0.z = -0.5f;
		local_a0.w = 1.0f;

		float* pSkip = &local_a0.w;

		edF32Matrix4MulF32Vector4Hard(&local_a0, &eStack144, &local_a0);
		edDListPatchVertex_Inline(pVertex, &local_a0.xyz, pSkip, index);

		local_a0.x = -0.5f;
		local_a0.y = 0.01f;
		local_a0.z = 0.5f;
		local_a0.w = 1.0f;
		edF32Matrix4MulF32Vector4Hard(&local_a0, &eStack144, &local_a0);
		edDListPatchVertex_Inline(pVertex, &local_a0.xyz, pSkip, index + 1);

		local_a0.x = 0.5f;
		local_a0.y = 0.01f;
		local_a0.z = -0.5f;
		local_a0.w = 1.0f;
		edF32Matrix4MulF32Vector4Hard(&local_a0, &eStack144, &local_a0);
		edDListPatchVertex_Inline(pVertex, &local_a0.xyz, pSkip, index + 2);

		local_a0.x = 0.5f;
		local_a0.y = 0.01f;
		local_a0.z = 0.5f;
		local_a0.w = 1.0f;
		edF32Matrix4MulF32Vector4Hard(&local_a0, &eStack144, &local_a0);
		edDListPatchVertex_Inline(pVertex, &local_a0.xyz, pSkip, index + 3);

		uVar6 = index;
		if (0x47 < index) {
			uVar6 = index + ((index - 0x48) / 0x46 + 1) * 2;
		}
		local_8 = (_rgba)(static_cast<uint>(bVar1) << 0x18);
		local_8 = (_rgba)(local_8.rgba | pPatch->pCurrentPatch->pRgba[uVar6].rgba & 0xffffff);
		uVar6 = pPatch->pCurrentPatch->nbMatrices;
		uVar2 = index;

		if (0x47 < index) {
			uVar2 = index + ((index - 0x48) / 0x46 + 1) * 2;
		}

		if (0x47 < uVar6) {
			uVar6 = uVar6 + ((uVar6 - 0x48) / 0x46 + 1) * 2;
		}

		p_Var5 = pPatch->pCurrentPatch->pRgba + uVar2;
		*p_Var5 = local_8;
		if (((1 < index) && (static_cast<uint>((int)index % 0x46) < 2)) && (uVar2 < uVar6)) {
			p_Var5[2] = local_8;
		}

		uVar6 = index + 1;
		if (0x47 < uVar6) {
			uVar6 = uVar6 + ((index - 0x47) / 0x46 + 1) * 2;
		}

		uVar2 = index + 1;
		local_10 = (_rgba)(static_cast<uint>(bVar1) << 0x18);
		local_10 = (_rgba)(local_10.rgba | pPatch->pCurrentPatch->pRgba[uVar6].rgba & 0xffffff);
		uVar6 = pPatch->pCurrentPatch->nbMatrices;
		if (0x47 < uVar2) {
			uVar2 = uVar2 + ((index - 0x47) / 0x46 + 1) * 2;
		}

		if (0x47 < uVar6) {
			uVar6 = uVar6 + ((uVar6 - 0x48) / 0x46 + 1) * 2;
		}

		p_Var5 = pPatch->pCurrentPatch->pRgba + uVar2;
		*p_Var5 = local_10;
		if (((1 < index + 1) && (static_cast<uint>((int)(index + 1) % 0x46) < 2)) && (uVar2 < uVar6)) {
			p_Var5[2] = local_10;
		}

		uVar6 = index + 2;
		if (0x47 < uVar6) {
			uVar6 = uVar6 + ((index - 0x46) / 0x46 + 1) * 2;
		}

		uVar2 = index + 2;
		local_18 = (_rgba)(static_cast<uint>(bVar1) << 0x18);
		local_18 = (_rgba)(local_18.rgba | pPatch->pCurrentPatch->pRgba[uVar6].rgba & 0xffffff);
		uVar6 = pPatch->pCurrentPatch->nbMatrices;
		if (0x47 < uVar2) {
			uVar2 = uVar2 + ((index - 0x46) / 0x46 + 1) * 2;
		}

		if (0x47 < uVar6) {
			uVar6 = uVar6 + ((uVar6 - 0x48) / 0x46 + 1) * 2;
		}

		p_Var5 = pPatch->pCurrentPatch->pRgba + uVar2;
		*p_Var5 = local_18;
		if (((1 < index + 2) && (static_cast<uint>((int)(index + 2) % 0x46) < 2)) && (uVar2 < uVar6)) {
			p_Var5[2] = local_18;
		}

		uVar6 = index + 3;
		if (0x47 < uVar6) {
			uVar6 = uVar6 + ((index - 0x45) / 0x46 + 1) * 2;
		}

		uVar2 = index + 3;
		local_20 = (_rgba)(static_cast<uint>(bVar1) << 0x18);
		gpCurPatchRGBABuf = 0;
		local_20 = (_rgba)(local_20.rgba | pPatch->pCurrentPatch->pRgba[uVar6].rgba & 0xffffff);
		uVar6 = pPatch->pCurrentPatch->nbMatrices;

		if (0x47 < uVar2) {
			uVar2 = uVar2 + ((index - 0x45) / 0x46 + 1) * 2;
		}

		if (0x47 < uVar6) {
			uVar6 = uVar6 + ((uVar6 - 0x48) / 0x46 + 1) * 2;
		}

		p_Var5 = pPatch->pCurrentPatch->pRgba + uVar2;
		*p_Var5 = local_20;
		if (((1 < index + 3) && (static_cast<uint>((int)(index + 3) % 0x46) < 2)) && (uVar2 < uVar6)) {
			p_Var5[2] = local_20;
		}
	}

	return;
}

void CShadowShared::SetIntensity(float newIntensity)
{
	float fVar1;

	if (newIntensity < 0.0f) {
		newIntensity = 0.0f;
	}

	fVar1 = 1.0f;
	if (newIntensity <= 1.0f) {
		fVar1 = newIntensity;
	}

	this->field_0x3c = fVar1;

	return;
}

void CShadowShared::SetGlobalIntensity(float newGlobalIntensity)
{
	float fVar1;

	if (newGlobalIntensity < 0.0) {
		newGlobalIntensity = 0.0;
	}
	fVar1 = 1.0;
	if (newGlobalIntensity <= 1.0) {
		fVar1 = newGlobalIntensity;
	}

	this->globalIntensity = fVar1;

	return;
}

