#include "SpriteWidget.h"
#include "kya.h"
#include "MathOps.h"


edF32VECTOR2 FLOAT_ARRAY_00448cd0 = { 1.0f, 1.0f };

CSprite::CSprite()
{
	this->flags_0x7c = 0;

	ClearLocalData();

	return;
}

void CSprite::ClearLocalData()
{
	undefined4* puVar1;
	int puVar4;
	undefined4* puVar3;
	edF32VECTOR2 local_10;
	edF32VECTOR2 local_8;

	local_8 = {};

	(this->textBitmap).field_0x4 = 0.0f;
	(this->textBitmap).field_0x8 = -0.5f;
	(this->textBitmap).fWidth = (float)(uint)this->iWidth;
	(this->textBitmap).fHeight = (float)(uint)this->iHeight;
	(this->textBitmap).field_0x14 = 0.0f;
	(this->textBitmap).field_0x18 = 0.0f;
	(this->textBitmap).field_0x1c = 1.0f;
	(this->textBitmap).field_0x20 = 1.0f;
	(this->textBitmap).pMaterial = (edDList_material*)0x0;
	this->texCoordA = local_8;
	local_10 = FLOAT_ARRAY_00448cd0;
	this->texCoordB = local_10;

	this->rgba[0] = 0x80;
	this->rgba[1] = 0x80;
	this->rgba[2] = 0x80;
	this->rgba[3] = 0x80;

	this->bValid = false;
	this->pParent = (CSprite*)0x0;
	this->flags = 0;
	return;
}

void CSprite::Draw(bool bUpdateMaterial)
{
	CSprite* pCurParent = this->pParent;

	while (pCurParent != (CSprite*)0x0) {
		if (pCurParent->bValid == false) {
			pCurParent->Validate();
		}

		pCurParent = pCurParent->pParent;
	}

	if (this->bValid == false) {
		Validate();
	}

	PrepareDraw(bUpdateMaterial);
	DrawSprite();

	return;
}

void CSprite::Draw(float param_2, float param_3, float param_4, uint drawFlags)
{
	IMPLEMENTATION_GUARD();
}

void CSprite::Draw(float ratio, float x1, float y1, float param_4, uint drawFlags)
{
	ushort uVar1;
	float fVar2;
	float fVar3;
	float fVar4;

	if ((drawFlags & 0x4000) == 0) {
		uVar1 = this->iWidth;
	}
	else {
		uVar1 = this->iHeight;
	}

	fVar4 = ratio * (float)(uint)uVar1;
	if ((drawFlags & 0x4000) == 0) {
		uVar1 = this->iHeight;
	}
	else {
		uVar1 = this->iWidth;
	}

	fVar2 = x1 * (float)(uint)uVar1;
	if ((drawFlags & 2) == 0) {
		if ((drawFlags & 4) == 0) {
			fVar3 = y1 + fVar4;
		}
		else {
			fVar3 = y1;
			y1 = y1 - fVar4;
		}
	}
	else {
		y1 = y1 - fVar4 / 2.0;
		fVar3 = y1 + fVar4;
	}

	if ((drawFlags & 0x10) == 0) {
		if ((drawFlags & 0x20) == 0) {
			fVar2 = param_4 + fVar2;
			fVar4 = param_4;
		}
		else {
			fVar4 = param_4 - fVar2;
			fVar2 = param_4;
		}
	}
	else {
		fVar4 = param_4 - fVar2 / 2.0;
		fVar2 = fVar4 + fVar2;
	}

	DrawXYXY(drawFlags, ratio, y1, fVar4, fVar3, fVar2);

	return;
}

edF32VECTOR2 edF32VECTOR2_00448cd8 = { -0.5f, -0.5f };
edF32VECTOR2 edF32VECTOR2_00448ce0 = { 0.5f, 0.5f };

void CSprite::Validate()
{
	edF32VECTOR2 screenDimensions;
	edF32VECTOR2 eStack24;
	edF32VECTOR2 local_10;
	edF32VECTOR2 local_8;

	local_8 = edF32VECTOR2_00448cd8;
	local_10 = edF32VECTOR2_00448ce0;

	if (this->pParent == (CSprite*)0x0) {
		(this->field_0x6c).scale = (this->field_0x5c).scale;
		(this->field_0x6c).position = (this->field_0x5c).position;
	}
	else {
		edF32Vector2Mul(&(this->field_0x6c).scale, &(this->pParent->field_0x6c).scale, &(this->field_0x5c).scale);
		edF32Vector2Mul(&eStack24, &(this->pParent->field_0x6c).scale, &this->field_0x5c.position);
		edF32Vector2Add(&this->field_0x6c.position, &this->pParent->field_0x6c.position, &eStack24);
	}

	edF32Vector2Mul(&local_10, &local_10, &(this->field_0x6c).scale);
	edF32Vector2Mul(&local_8, &local_8, &(this->field_0x6c).scale);

	edF32Vector2Add(&local_10, &local_10, &this->field_0x6c.position);
	edF32Vector2Add(&local_8, &local_8, &this->field_0x6c.position);

	screenDimensions = {};

	screenDimensions.x = (float)gVideoConfig.screenWidth;
	screenDimensions.y = (float)gVideoConfig.screenHeight;

	edF32Vector2Mul(&this->screenCoordsTL, &local_8, &screenDimensions);
	edF32Vector2Mul(&this->screenCoordsBR, &local_10, &screenDimensions);

	this->bValid = true;

	return;
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
	float fVar2;
	float fVar3;
	float local_10;
	float local_c;
	float local_8;
	float local_4;

	local_8 = (this->texCoordA).x;
	local_4 = (this->texCoordA).y;
	fVar2 = (this->texCoordB).x;
	fVar3 = (this->texCoordB).y;

	local_c = fVar3;
	if ((this->flags & 0x2000) != 0) {
		local_c = local_4;
		local_4 = fVar3;
	}

	local_10 = fVar2;
	if ((this->flags & 0x1000) != 0) {
		local_10 = local_8;
		local_8 = fVar2;
	}

	edDListLoadIdentity();

	edDListBegin(1.0f, 1.0f, 1.0f, DISPLAY_LIST_DATA_TYPE_SPRITE, 2);
	edDListTexCoo2f(local_8, local_4);
	edDListVertex4f((this->screenCoordsTL).x, (this->screenCoordsTL).y, 0.0f, 0.0f);
	edDListTexCoo2f(local_10, local_c);
	edDListVertex4f((this->screenCoordsBR).x, (this->screenCoordsBR).y, 0.0f, 0.0f);
	edDListEnd();

	return;
}

void CSprite::Install(char* pFileBuffer)
{
	ed_g2d_material* pMaterial;
	ed_g2d_bitmap* pBitmap;
	int iStack4;

	ed3DInstallG2D(pFileBuffer, *(int*)(pFileBuffer + 8), &iStack4, &this->textureManager, 0);

	this->flags_0x7c = this->flags_0x7c | 2;
	this->bValid = false;

	pMaterial = ed3DG2DGetG2DMaterialFromIndex(&this->textureManager, 0);
	pBitmap = ed3DG2DGetBitmapFromMaterial(pMaterial, 0);

	edDListCreatMaterialFromIndex(&this->materialInfo, 0, &this->textureManager, 2);

	this->flags_0x7c = this->flags_0x7c | 1;
	this->iWidth = pBitmap->width;
	this->iHeight = pBitmap->height;
	this->textBitmap.pMaterial = &this->materialInfo;
	this->textBitmap.fWidth = (float)(uint)this->iWidth;
	this->textBitmap.fHeight = (float)(uint)this->iHeight;

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

	this->bValid = false;

	hashCode = ed3DComputeHashCode(pFileName);
	edDListCreatMaterialFromHashCode(&this->materialInfo, hashCode, pManager, 2);

	this->flags_0x7c = this->flags_0x7c | 1;

	pMaterial = ed3DG2DGetG2DMaterial(pManager, hashCode);
	pBitmap = ed3DG2DGetBitmapFromMaterial(pMaterial, 0);
	this->iWidth = pBitmap->width;
	this->iHeight = pBitmap->height;
	this->textBitmap.pMaterial = &this->materialInfo;
	this->textBitmap.fWidth = (float)(uint)this->iWidth;
	this->textBitmap.fHeight = (float)(uint)this->iHeight;

	return ed3DG2DMaterialGetLayer(pMaterial, 0);
}

edDList_material* CSprite::Install(ed_g2d_manager* pManager, int index)
{
	ed_g2d_material* pMaterial;
	ed_g2d_bitmap* pBitmap;
	edDList_material* pDlistMaterial;

	this->bValid = false;
	pMaterial = ed3DG2DGetG2DMaterialFromIndex(pManager, 0);
	pBitmap = ed3DG2DGetBitmapFromMaterial(pMaterial, 0);
	pDlistMaterial = edDListCreatMaterialFromIndex(&this->materialInfo, index, pManager, 2);
	this->flags_0x7c = this->flags_0x7c | 1;
	this->iWidth = pBitmap->width;
	this->iHeight = pBitmap->height;
	(this->textBitmap).pMaterial = &this->materialInfo;
	(this->textBitmap).fWidth = (float)(uint)this->iWidth;
	(this->textBitmap).fHeight = (float)(uint)this->iHeight;

	return pDlistMaterial;
}

void CSprite::Install(edDList_material* pMaterial)
{
	ed_g2d_bitmap* pBitmap;

	this->bValid = false;

	(this->materialInfo).pManager = pMaterial->pManager;
	(this->materialInfo).pMaterial = pMaterial->pMaterial;
	(this->materialInfo).mode = pMaterial->mode;
	(this->materialInfo).index = pMaterial->index;

	pBitmap = ed3DG2DGetBitmapFromMaterial(pMaterial->pMaterial, 0);
	this->iWidth = pBitmap->width;
	this->iHeight = pBitmap->height;
	this->textBitmap.pMaterial = &this->materialInfo;
	this->textBitmap.fWidth = (float)(uint)this->iWidth;
	this->textBitmap.fHeight = (float)(uint)this->iHeight;

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

void FUN_0038c6f0(edF32VECTOR2* v0, edF32VECTOR2* v1, int param_3)
{
	float local_8;
	float local_4;

	local_8 = v1->x;
	local_4 = v1->y;

	if (param_3 != 0) {
		do {
			if (local_8 == 0.0f) {
				if (local_4 == 1.0f) {
					local_4 = 0.0f;
				}
				else {
					local_8 = 1.0f;
				}
			}
			else {
				if (local_4 == 0.0f) {
					local_4 = 1.0f;
				}
				else {
					local_8 = 0.0f;
				}
			}
			param_3 = param_3 + -1;
		} while (param_3 != 0);
	}

	v0->x = local_8;
	v0->y = local_4;

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
	edF32VECTOR2 local_20;
	edF32VECTOR2 local_18;
	edF32VECTOR2 local_10;
	edF32VECTOR2 local_8;

	if (param_4 == param_6) {
		param_6 = param_4 + (float)(uint)this->iWidth;
	}

	if (param_5 == param_7) {
		param_7 = param_5 + (float)(uint)this->iHeight;
	}

	edDListUseMaterial(&this->materialInfo);
	edDListLoadIdentity();

	local_10.y = this->texCoordB.y;
	local_8.y = this->texCoordA.y;
	if ((this->flags & 0x2000) != 0) {
		local_10.y = this->texCoordA.y;
		local_8.y = this->texCoordB.y;
	}

	local_10.x = this->texCoordB.x;
	local_8.x = this->texCoordA.x;
	if ((drawFlag & 0x1000) != 0) {
		local_10.x = this->texCoordA.x;
		local_8.x = this->texCoordB.x;
	}

	local_18.x = local_10.x;
	local_18.y = local_8.y;

	local_20.x = local_8.x;
	local_20.y = local_10.y;

	if ((drawFlag & 0xc000) != 0) {
		iVar3 = 2;
		if ((drawFlag & 0x8000) == 0) {
			iVar3 = 0;
		}

		iVar3 = (uint)((drawFlag & 0x4000) != 0) + iVar3;

		FUN_0038c6f0(&local_8, &local_8, iVar3);
		FUN_0038c6f0(&local_10, &local_10, iVar3);
		FUN_0038c6f0(&local_18, &local_18, iVar3);
		FUN_0038c6f0(&local_20, &local_20, iVar3);
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
		edDListBegin(1.0f, 1.0f, 1.0f, DISPLAY_LIST_DATA_TYPE_TRIANGLE_LIST, 4);
		edDListTexCoo2f(local_8.x, local_8.y);
		edDListVertex4f(x, y_00, 0.0f, 0.0f);
		edDListTexCoo2f(local_18.x, local_18.y);
		edDListVertex4f(fVar5, y_00, 0.0f, 0.0f);
		edDListTexCoo2f(local_20.x, local_20.y);
		edDListVertex4f(x, y, 0.0f, 0.0f);
		edDListTexCoo2f(local_10.x, local_10.y);
		edDListVertex4f(fVar5, y, 0.0f, 0.0f);
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
		edDListBegin(1.0f, 1.0f, 1.0f, DISPLAY_LIST_DATA_TYPE_TRIANGLE_LIST, 4);
		edDListTexCoo2f(local_8.x, local_8.y);
		edDListVertex4f(param_4, param_5, 0.0f, 0.0f);
		edDListTexCoo2f(local_18.x, local_18.y);
		edDListVertex4f(param_6, param_5, 0.0f, 0.0f);
		edDListTexCoo2f(local_20.x, local_20.y);
		edDListVertex4f(param_4, param_7, 0.0f, 0.0f);
		edDListTexCoo2f(local_10.x, local_10.y);
		edDListVertex4f(param_6, param_7, 0.0f, 0.0f);
		edDListEnd();
	}
	else {
		if (uVar1 == 0x20000) {
			edDListBegin(1.0f, 1.0f, 1.0f, DISPLAY_LIST_DATA_TYPE_TRIANGLE_LIST, 8);
			edDListTexCoo2f(local_8.x, local_8.y);
			edDListVertex4f(param_4, param_5, 0.0f, 0.0f);
			edDListTexCoo2f(local_18.x, local_18.y);
			edDListVertex4f(param_6, param_5, 0.0f, 0.0f);
			edDListTexCoo2f(local_20.x, local_20.y);
			fVar4 = (param_5 + param_7) / 2.0f;
			edDListVertex4f(param_4, fVar4, 0.0f, 0.0f);
			edDListTexCoo2f(local_10.x, local_10.y);
			edDListVertex4f(param_6, fVar4, 0.0f, 0.0f);
			edDListTexCoo2f(local_8.x, local_8.y);
			edDListVertex4f(param_4, param_7, 0.0f, 0.0f);
			edDListTexCoo2f(local_18.x, local_18.y);
			edDListVertex4f(param_6, param_7, 0.0f, 0.0f);
			edDListEnd();
		}
		else {
			if (uVar1 == 0x10000) {
				edDListBegin(1.0f, 1.0f, 1.0f, DISPLAY_LIST_DATA_TYPE_TRIANGLE_LIST, 6);
				edDListTexCoo2f(local_8.x, local_8.y);
				edDListVertex4f(param_4, param_5, 0.0f, 0.0f);
				edDListTexCoo2f(local_20.x, local_20.y);
				edDListVertex4f(param_4, param_7, 0.0f, 0.0f);
				edDListTexCoo2f(local_18.x, local_18.y);
				fVar4 = (param_4 + param_6) / 2.0f;
				edDListVertex4f(fVar4, param_5, 0.0f, 0.0f);
				edDListTexCoo2f(local_10.x, local_10.y);
				edDListVertex4f(fVar4, param_7, 0.0f, 0.0f);
				edDListTexCoo2f(local_8.x, local_8.y);
				edDListVertex4f(param_6, param_5, 0.0f, 0.0f);
				edDListTexCoo2f(local_20.x, local_20.y);
				edDListVertex4f(param_6, param_7, 0.0f, 0.0f);
				edDListEnd();
			}
			else {
				edDListBegin(1.0f, 1.0f, 1.0f, DISPLAY_LIST_DATA_TYPE_TRIANGLE_LIST, 0xb);
				edDListTexCoo2f(local_8.x, local_8.y);
				edDListVertex4f(param_4, param_5, 0.0f, 0.0f);
				edDListTexCoo2f(local_20.x, local_20.y);
				fVar4 = (param_5 + param_7) / 2.0f;
				edDListVertex4f(param_4, fVar4, 0.0f, 0.0f);
				edDListTexCoo2f(local_18.x, local_18.y);
				fVar5 = (param_4 + param_6) / 2.0f;
				edDListVertex4f(fVar5, param_5, 0.0f, 0.0f);
				edDListTexCoo2f(local_10.x, local_10.y);
				edDListVertex4f(fVar5, fVar4, 0.0f, 0.0f);
				edDListTexCoo2f(local_8.x, local_8.y);
				edDListVertex4f(param_6, param_5, 0.0f, 0.0f);
				edDListTexCoo2f(local_20.x, local_20.y);
				edDListVertex4f(param_6, fVar4, 0.0f, 0.0f);
				edDListTexCoo2f(local_8.x, local_8.y);
				edDListVertex4f(param_6, param_7, 0.0f, 0.0f);
				edDListTexCoo2f(local_10.x, local_10.y);
				edDListVertex4f(fVar5, fVar4, 0.0f, 0.0f);
				edDListTexCoo2f(local_18.x, local_18.y);
				edDListVertex4f(fVar5, param_7, 0.0f, 0.0f);
				edDListTexCoo2f(local_20.x, local_20.y);
				edDListVertex4f(param_4, fVar4, 0.0f, 0.0f);
				edDListTexCoo2f(local_8.x, local_8.y);
				edDListVertex4f(param_4, param_7, 0.0f, 0.0f);
				edDListEnd();
			}
		}
	}

	return;
}

void CSprite::SetParent(CSprite* pNewParent)
{
	this->bValid = false;
	this->pParent = pNewParent;

	return;
}

void CSprite::UpdateSlotPosition(float x, float y)
{
	this->bValid = false;

	this->field_0x5c.position.x = x;
	this->field_0x5c.position.y = y;

	return;
}

void CSprite::UpdateSlotScale(float x, float y)
{
	this->bValid = false;

	this->field_0x5c.scale.x = x;
	this->field_0x5c.scale.y = y;

	return;
}

void CSprite::UpdateSlotPositionAndScale(float x, float y, float xScale, float yScale)
{
	this->bValid = false;

	this->field_0x5c.position.x = x;
	this->field_0x5c.position.y = y;

	this->field_0x5c.scale.x = xScale;
	this->field_0x5c.scale.y = yScale;

	return;
}

void CSprite::SetIsValid(bool bNewValid)
{
	this->bValid = bNewValid;

	return;
}

bool CSprite::GetIsValid()
{
	return this->bValid;
}

void CSpriteWindow::Validate()
{
	CSprite::Validate();

	this->texCoordA.x = this->xMin;
	this->texCoordA.y = 0.0f;
	this->texCoordB.x = this->xMax;
	this->texCoordB.y = 1.0f;

	float baseTLx = this->screenCoordsTL.x;
	float baseBRx = this->screenCoordsBR.x;

	this->screenCoordsTL.x = this->xMin * baseBRx + (1.0f - this->xMin) * baseTLx;
	this->screenCoordsBR.x = this->xMax * baseBRx + (1.0f - this->xMax) * baseTLx;

	return;
}

void CSpriteWindow::DrawSprite()
{
	float fVar2;
	float fVar3;
	float local_10;
	float local_c;
	float local_8;
	float local_4;

	if (this->xMin != this->xMax) {
		local_8 = this->texCoordA.x;
		local_4 = this->texCoordA.y;
		fVar2 = this->texCoordB.x;
		fVar3 = this->texCoordB.y;

		local_c = fVar3;
		if ((this->flags & 0x2000) != 0) {
			local_c = local_4;
			local_4 = fVar3;
		}

		local_10 = fVar2;
		if ((this->flags & 0x1000) != 0) {
			local_10 = local_8;
			local_8 = fVar2;
		}

		edDListLoadIdentity();

		edDListBegin(1.0f, 1.0f, 1.0f, 6, 2);
		edDListTexCoo2f(local_8, local_4);
		edDListVertex4f(this->screenCoordsTL.x, this->screenCoordsTL.y, 0.0f, 0.0f);
		edDListTexCoo2f(local_10, local_c);
		edDListVertex4f(this->screenCoordsBR.x, this->screenCoordsBR.y, 0.0f, 0.0f);
		edDListEnd();
	}

	return;
}

void CSpriteWindow::ClampX(float clampMin, float clampMax)
{
	this->xMin = clampMin;
	this->xMax = clampMax;

	this->bValid = false;

	return;
}

void CSpriteWindow::ClampX(float clampMin, float clampMax, bool param_4)
{
	if (param_4 == false) {
		this->xMax = 1.0f - clampMin;
		this->xMin = 1.0f - clampMax;
	}
	else {
		this->xMin = clampMin;
		this->xMax = clampMax;
	}

	this->bValid = false;

	return;
}
