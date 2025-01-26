#include "Sprite.h"


float FLOAT_ARRAY_00448cd0[2] = { 1.0f, 1.0f };

CSprite::CSprite()
{
	(this->field_0x5c).scale.y = 0.0f;
	(this->field_0x5c).scale.x = 0.0f;
	(this->field_0x5c).position.y = 0.0f;
	(this->field_0x5c).position.x = 0.0f;

	this->field_0x78 = 0.0f;
	this->field_0x74 = 0.0f;

	this->field_0x70 = (undefined*)0x0;
	this->field_0x6c = 0;
	this->flags_0x7c = 0;

	ClearLocalData();

	return;
}

void CSprite::ClearLocalData()
{
	undefined4* puVar1;
	int puVar4;
	undefined4* puVar3;
	float local_10;
	float fStack12;
	undefined4 local_8;
	undefined4 local_4;

	//puVar4 = 8;
	//puVar3 = &local_8;
	//puVar1 = puVar3;
	//while (puVar1 != (undefined4*)0x0) {
	//	*(undefined*)puVar3 = 0;
	//	puVar3 = (undefined4*)((int)puVar3 + 1);
	//	puVar4 = puVar4 + -1;
	//	puVar1 = (undefined4*)puVar4;
	//}
	this->field_0x10 = 0;
	this->field_0x14 = -0.5f;
	this->fWidth = (float)(uint)this->iWidth;
	this->fHeight = (float)(uint)this->iHeight;
	this->field_0x20 = 0;
	this->field_0x24 = 0;
	this->field_0x28 = 1.0f;
	this->field_0x2c = 1.0f;
	this->pMaterialInfo = (edDList_material*)0x0;
	this->field_0x4c = local_8;
	this->field_0x50 = local_4;
	local_10 = FLOAT_ARRAY_00448cd0[0];
	this->field_0x54 = local_10;
	fStack12 = FLOAT_ARRAY_00448cd0[1];
	this->field_0x58 = fStack12;

	this->rgba[0] = 0x80;
	this->rgba[1] = 0x80;
	this->rgba[2] = 0x80;
	this->rgba[3] = 0x80;

	this->bValid = 0;
	this->pParent = (CSprite*)0x0;
	this->field_0x34 = 0;
	return;
}

void CSprite::Draw(bool bUpdateMaterial)
{
	IMPLEMENTATION_GUARD();
}

void CSprite::Draw(uint drawFlag, float param_2, float param_3, float param_4)
{
	IMPLEMENTATION_GUARD();
}

void CSprite::Draw(uint drawFlag, float param_2, float param_3, float param_4, float param_5)
{
	IMPLEMENTATION_GUARD();
}

void CSprite::Validate()
{
	IMPLEMENTATION_GUARD();
}

void CSprite::PrepareDraw(bool bUpdateMaterial)
{
	if (bUpdateMaterial != false) {
		edDListUseMaterial(&this->materialInfo);
	}

	edDListColor4u8(this->rgba[0], this->rgba[1], this->rgba[2], this->rgba[3]);

	return;
}

void CSprite::DrawSprite()
{
	IMPLEMENTATION_GUARD();
}

void CSprite::Install(char* pFileBuffer)
{
	ed_g2d_material* pMaterial;
	ed_g2d_bitmap* pBitmap;
	int iStack4;

	ed3DInstallG2D(pFileBuffer, *(int*)(pFileBuffer + 8), &iStack4, &this->textureManager, 0);

	this->flags_0x7c = this->flags_0x7c | 2;
	this->bValid = 0;

	pMaterial = ed3DG2DGetG2DMaterialFromIndex(&this->textureManager, 0);
	pBitmap = ed3DG2DGetBitmapFromMaterial(pMaterial, 0);

	edDListCreatMaterialFromIndex(&this->materialInfo, 0, &this->textureManager, 2);

	this->flags_0x7c = this->flags_0x7c | 1;
	this->iWidth = pBitmap->width;
	this->iHeight = pBitmap->height;
	this->pMaterialInfo = &this->materialInfo;
	this->fWidth = (float)(uint)this->iWidth;
	this->fHeight = (float)(uint)this->iHeight;

	return;
}

ed_g2d_layer* CSprite::Install(ed_g2d_manager* pManager, char* pFileName)
{
	ed_g2d_material* pMaterial;
	ed_g2d_bitmap* pBitmap;
	ulong hashCode;

	if ((this->flags_0x7c & 1) != 0) {
		edDListTermMaterial(&this->materialInfo);
		this->flags_0x7c = this->flags_0x7c & 0xfffffffe;
	}

	if ((this->flags_0x7c & 2) != 0) {
		ed3DUnInstallG2D(&this->textureManager);
		this->flags_0x7c = this->flags_0x7c & 0xfffffffd;
	}

	this->bValid = 0;

	hashCode = ed3DComputeHashCode(pFileName);
	edDListCreatMaterialFromHashCode(&this->materialInfo, hashCode, pManager, 2);

	this->flags_0x7c = this->flags_0x7c | 1;

	pMaterial = ed3DG2DGetG2DMaterial(pManager, hashCode);
	pBitmap = ed3DG2DGetBitmapFromMaterial(pMaterial, 0);
	this->iWidth = pBitmap->width;
	this->iHeight = pBitmap->height;
	this->pMaterialInfo = &this->materialInfo;
	this->fWidth = (float)(uint)this->iWidth;
	this->fHeight = (float)(uint)this->iHeight;

	return ed3DG2DMaterialGetLayer(pMaterial, 0);
}

void CSprite::Install(edDList_material* pMaterial)
{
	ed_g2d_bitmap* pBitmap;

	this->bValid = 0;

	(this->materialInfo).pManager = pMaterial->pManager;
	(this->materialInfo).pMaterial = pMaterial->pMaterial;
	(this->materialInfo).mode = pMaterial->mode;
	(this->materialInfo).index = pMaterial->index;

	pBitmap = ed3DG2DGetBitmapFromMaterial(pMaterial->pMaterial, 0);
	this->iWidth = pBitmap->width;
	this->iHeight = pBitmap->height;
	this->pMaterialInfo = &this->materialInfo;
	this->fWidth = (float)(uint)this->iWidth;
	this->fHeight = (float)(uint)this->iHeight;

	return;
}

void CSprite::Remove()
{
	if ((this->flags_0x7c & 1) != 0) {
		edDListTermMaterial(&this->materialInfo);
		this->flags_0x7c = this->flags_0x7c & 0xfffffffe;
	}

	if ((this->flags_0x7c & 2) != 0) {
		ed3DUnInstallG2D(&this->textureManager);
		this->flags_0x7c = this->flags_0x7c & 0xfffffffd;
	}

	ClearLocalData();

	return;
}

void CSprite::DrawXYXY(uint drawFlag, float param_3, float param_4, float param_5, float param_6, float param_7)
{
	uint uVar1;
	byte b;
	byte bVar2;
	int iVar3;
	byte r;
	byte g;
	float fVar4;
	float fVar5;
	float y;
	float x;
	float y_00;
	float local_20;
	float local_1c;
	float local_18;
	float local_14;
	float local_10;
	float local_c;
	float local_8;
	float local_4;

	if (param_4 == param_6) {
		param_6 = param_4 + (float)(uint)this->iWidth;
	}

	if (param_5 == param_7) {
		param_7 = param_5 + (float)(uint)this->iHeight;
	}

	edDListUseMaterial(&this->materialInfo);
	edDListLoadIdentity();

	local_c = this->field_0x58;
	local_4 = this->field_0x50;
	if ((this->field_0x34 & 0x2000) != 0) {
		local_c = this->field_0x50;
		local_4 = this->field_0x58;
	}

	local_10 = this->field_0x54;
	local_8 = this->field_0x4c;
	if ((drawFlag & 0x1000) != 0) {
		local_10 = this->field_0x4c;
		local_8 = this->field_0x54;
	}

	local_18 = local_10;
	local_14 = local_4;
	local_20 = local_8;
	local_1c = local_c;
	if ((drawFlag & 0xc000) != 0) {
		IMPLEMENTATION_GUARD(
			iVar3 = 2;
		if ((drawFlag & 0x8000) == 0) {
			iVar3 = 0;
		}
		iVar3 = (uint)((drawFlag & 0x4000) != 0) + iVar3;
		FUN_0038c6f0(&local_8, &local_8, iVar3);
		FUN_0038c6f0(&local_10, &local_10, iVar3);
		FUN_0038c6f0(&local_18, &local_18, iVar3);
		FUN_0038c6f0(&local_20, &local_20, iVar3);)
	}
	if ((drawFlag & 0x3c0) != 0) {
		fVar4 = param_3 * 3.0f;
		x = param_4 + fVar4;
		y_00 = param_5 + fVar4;
		fVar5 = param_6 - fVar4;
		y = param_7 - fVar4;
		if ((drawFlag & 0x40) != 0) {
			x = x - fVar4 * 2.0f;
		}
		if ((drawFlag & 0x80) != 0) {
			fVar5 = fVar5 + fVar4 * 2.0f;
		}
		if ((drawFlag & 0x100) != 0) {
			y_00 = y_00 - fVar4 * 2.0f;
		}
		if ((drawFlag & 0x200) != 0) {
			y = y + fVar4 * 2.0f;
		}

		edDListBlendSet(1);
		edDListBlendFunc50();
		edDListColor4u8(0, 0, 0, 0x80);
		iVar3 = 4;
		edDListBegin(1.0f, 1.0f, 1.0f, PRIM_TYPE_TRIANGLE_LIST, 4);
		edDListTexCoo2f(local_8, local_4);
		edDListVertex4f(x, y_00, 0.0f, iVar3);
		edDListTexCoo2f(local_18, local_14);
		edDListVertex4f(fVar5, y_00, 0.0f, iVar3);
		edDListTexCoo2f(local_20, local_1c);
		edDListVertex4f(x, y, 0.0f, iVar3);
		edDListTexCoo2f(local_10, local_c);
		edDListVertex4f(fVar5, y, 0.0f, iVar3);
		edDListEnd();
	}

	if ((drawFlag & 0x800) == 0) {
		edDListBlendSet(1);
	}
	else {
		edDListBlendSet(0);
	}

	edDListBlendFuncNormal();

	if ((drawFlag & 0x400) == 0) {
		edDListColor4u8(this->rgba[0], this->rgba[1], this->rgba[2], this->rgba[3]);
	}
	else {
		fVar4 = (float)(uint)this->rgba[0] * param_3;
		if (fVar4 < 2.147484e+09f) {
			r = (byte)(int)fVar4;
			bVar2 = this->rgba[1];
		}
		else {
			r = (byte)(int)(fVar4 - 2.147484e+09f);
			bVar2 = this->rgba[1];
		}
		fVar4 = (float)(uint)bVar2 * param_3;
		if (fVar4 < 2.147484e+09f) {
			g = (byte)(int)fVar4;
			bVar2 = this->rgba[2];
		}
		else {
			g = (byte)(int)(fVar4 - 2.147484e+09f);
			bVar2 = this->rgba[2];
		}
		fVar4 = (float)(uint)bVar2 * param_3;
		if (fVar4 < 2.147484e+09f) {
			b = (byte)(int)fVar4;
			bVar2 = this->rgba[3];
		}
		else {
			b = (byte)(int)(fVar4 - 2.147484e+09f);
			bVar2 = this->rgba[3];
		}
		edDListColor4u8(r, g, b, bVar2);
	}

	uVar1 = drawFlag & 0x30000;
	if (uVar1 == 0) {
		iVar3 = 4;
		edDListBegin(1.0f, 1.0f, 1.0f, PRIM_TYPE_TRIANGLE_LIST, 4);
		edDListTexCoo2f(local_8, local_4);
		edDListVertex4f(param_4, param_5, 0.0f, iVar3);
		edDListTexCoo2f(local_18, local_14);
		edDListVertex4f(param_6, param_5, 0.0f, iVar3);
		edDListTexCoo2f(local_20, local_1c);
		edDListVertex4f(param_4, param_7, 0.0f, iVar3);
		edDListTexCoo2f(local_10, local_c);
		edDListVertex4f(param_6, param_7, 0.0f, iVar3);
		edDListEnd();
	}
	else {
		if (uVar1 == 0x20000) {
			iVar3 = 4;
			edDListBegin(1.0f, 1.0f, 1.0f, PRIM_TYPE_TRIANGLE_LIST, 8);
			edDListTexCoo2f(local_8, local_4);
			edDListVertex4f(param_4, param_5, 0.0f, iVar3);
			edDListTexCoo2f(local_18, local_14);
			edDListVertex4f(param_6, param_5, 0.0f, iVar3);
			edDListTexCoo2f(local_20, local_1c);
			fVar4 = (param_5 + param_7) / 2.0f;
			edDListVertex4f(param_4, fVar4, 0.0f, iVar3);
			edDListTexCoo2f(local_10, local_c);
			edDListVertex4f(param_6, fVar4, 0.0f, iVar3);
			edDListTexCoo2f(local_8, local_4);
			edDListVertex4f(param_4, param_7, 0.0f, iVar3);
			edDListTexCoo2f(local_18, local_14);
			edDListVertex4f(param_6, param_7, 0.0f, iVar3);
			edDListEnd();
		}
		else {
			if (uVar1 == 0x10000) {
				iVar3 = 4;
				edDListBegin(1.0f, 1.0f, 1.0f, PRIM_TYPE_TRIANGLE_LIST, 6);
				edDListTexCoo2f(local_8, local_4);
				edDListVertex4f(param_4, param_5, 0.0f, iVar3);
				edDListTexCoo2f(local_20, local_1c);
				edDListVertex4f(param_4, param_7, 0.0f, iVar3);
				edDListTexCoo2f(local_18, local_14);
				fVar4 = (param_4 + param_6) / 2.0f;
				edDListVertex4f(fVar4, param_5, 0.0f, iVar3);
				edDListTexCoo2f(local_10, local_c);
				edDListVertex4f(fVar4, param_7, 0.0f, iVar3);
				edDListTexCoo2f(local_8, local_4);
				edDListVertex4f(param_6, param_5, 0.0f, iVar3);
				edDListTexCoo2f(local_20, local_1c);
				edDListVertex4f(param_6, param_7, 0.0f, iVar3);
				edDListEnd();
			}
			else {
				iVar3 = 4;
				edDListBegin(1.0f, 1.0f, 1.0f, PRIM_TYPE_TRIANGLE_LIST, 0xb);
				edDListTexCoo2f(local_8, local_4);
				edDListVertex4f(param_4, param_5, 0.0f, iVar3);
				edDListTexCoo2f(local_20, local_1c);
				fVar4 = (param_5 + param_7) / 2.0f;
				edDListVertex4f(param_4, fVar4, 0.0f, iVar3);
				edDListTexCoo2f(local_18, local_14);
				fVar5 = (param_4 + param_6) / 2.0f;
				edDListVertex4f(fVar5, param_5, 0.0f, iVar3);
				edDListTexCoo2f(local_10, local_c);
				edDListVertex4f(fVar5, fVar4, 0.0f, iVar3);
				edDListTexCoo2f(local_8, local_4);
				edDListVertex4f(param_6, param_5, 0.0f, iVar3);
				edDListTexCoo2f(local_20, local_1c);
				edDListVertex4f(param_6, fVar4, 0.0f, iVar3);
				edDListTexCoo2f(local_8, local_4);
				edDListVertex4f(param_6, param_7, 0.0f, iVar3);
				edDListTexCoo2f(local_10, local_c);
				edDListVertex4f(fVar5, fVar4, 0.0f, iVar3);
				edDListTexCoo2f(local_18, local_14);
				edDListVertex4f(fVar5, param_7, 0.0f, iVar3);
				edDListTexCoo2f(local_20, local_1c);
				edDListVertex4f(param_4, fVar4, 0.0f, iVar3);
				edDListTexCoo2f(local_8, local_4);
				edDListVertex4f(param_4, param_7, 0.0f, iVar3);
				edDListEnd();
			}
		}
	}

	return;
}

void CSprite::SetParent(CSprite* pNewParent)
{
	this->bValid = 0;
	this->pParent = pNewParent;

	return;
}

void CSprite::UpdateSlotPosition(float x, float y)
{
	this->bValid = 0;

	this->field_0x5c.position.x = x;
	this->field_0x5c.position.y = y;

	return;
}

void CSprite::UpdateSlotScale(float x, float y)
{
	this->bValid = 0;

	this->field_0x5c.scale.x = x;
	this->field_0x5c.scale.y = y;

	return;
}

void CSprite::UpdateSlotPositionAndScale(float x, float y, float xScale, float yScale)
{
	this->bValid = 0;

	this->field_0x5c.position.x = x;
	this->field_0x5c.position.y = y;

	this->field_0x5c.scale.x = xScale;
	this->field_0x5c.scale.y = yScale;

	return;
}

void CSpriteWindow::Validate()
{
	IMPLEMENTATION_GUARD();
}

void CSpriteWindow::DrawSprite()
{
	IMPLEMENTATION_GUARD();
}

void CSpriteWindow::ClampX(float clampMin, float clampMax)
{
	this->xMin = clampMin;
	this->xMax = clampMax;

	this->bValid = 0;

	return;
}

void CSpriteWindow::ClampX(float clampMin, float clampMax, bool param_4)
{
	if (param_4 == false) {
		this->xMax = 1.0 - clampMin;
		this->xMin = 1.0 - clampMax;
	}
	else {
		this->xMin = clampMin;
		this->xMax = clampMax;
	}

	this->bValid = 0;

	return;
}
