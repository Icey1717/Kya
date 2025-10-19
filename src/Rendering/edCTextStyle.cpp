#include "edCTextStyle.h"
#include "MathOps.h"

extern edCTextFont* g_PackedFontPtr_004324d0;
edCTextStyle* pedTextCurrentStyle;

// Should be in: D:/projects/EdenLib/edText/sources/edTextStyle.cpp
edCTextStyle* edTextStyleGetCurrent()
{
	return pedTextCurrentStyle;
}

// Should be in: D:/projects/EdenLib/edText/sources/edTextStyle.cpp
edCTextStyle* edTextStyleSetCurrent(edCTextStyle* pNewFont)
{
	edCTextStyle* pFVar1;

	pFVar1 = pedTextCurrentStyle;
	pedTextCurrentStyle = pNewFont;
	return pFVar1;
}

void edCTextStyle::TransformMatrix(float x, float y, edF32MATRIX4* outMatrix)
{
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	if ((this->flags_0x84 & 0x400) != 0) {
		local_20.x = this->xScale;
		local_20.y = this->yScale;
		local_20.z = 1.0f;
		local_20.w = 1.0f;
		edF32Matrix4SetIdentityHard(&this->m0);
		edF32Matrix4RotateZHard(this->rotation, &this->m0, &this->m0);
		edF32Matrix4ScaleHard(&this->m0, &this->m0, &local_20);
		this->flags_0x84 = this->flags_0x84 & 0xfffffbff;
	}
	local_10.z = 0.0f;
	local_10.w = 0.0f;
	local_10.x = x;
	local_10.y = y;
	edF32Matrix4TranslateHard(outMatrix, &this->m0, &local_10);
	edF32Matrix4MulF32Matrix4Hard(outMatrix, outMatrix, &this->m1);
	return;
}

// Should be in: D:/projects/EdenLib/edText/sources/edTextStyle.cpp
void edCTextStyle::SetRotation(float angle)
{
	this->rotation = angle;
	this->flags_0x84 = this->flags_0x84 | 0x400;
	return;
}

void edCTextStyle::GetScale(float* outX, float* outY)
{
	*outX = this->xScale;
	*outY = this->yScale;
	return;
}

void edCTextStyle::SetScale(float xScale, float yScale)
{
	this->xScale = xScale;
	this->yScale = yScale;
	this->flags_0x84 = this->flags_0x84 | 0x400;
	return;
}

// Should be in: D:/projects/EdenLib/edText/sources/edTextStyle.cpp
void edCTextStyle::SetVerticalSize(float verticalSize)
{
	this->verticalSize = verticalSize;
	return;
}

// Should be in: D:/projects/EdenLib/edText/sources/edTextStyle.cpp
float edCTextStyle::GetHorizontalSize()
{
	return this->horizontalSize;
}


// Should be in: D:/projects/EdenLib/edText/sources/edTextStyle.cpp
void edCTextStyle::SetHorizontalSize(float horizontalSize)
{
	this->horizontalSize = horizontalSize;
	return;
}

void edCTextStyle::GetShadowShift(float* outX, float* outY)
{
	*outX = this->shadowShiftX;
	*outY = this->shadowShiftY;
	return;
}

void edCTextStyle::SetShadowShift(float x, float y)
{
	this->shadowShiftX = x;
	this->shadowShiftY = y;
	return;
}

// Should be in: D:/projects/EdenLib/edText/sources/edTextStyle.cpp
uint edCTextStyle::GetDebug()
{
	return this->flags_0x84 & 0x200;
}

void edCTextStyle::SetShadow(uint flag)
{
	flags_0x84 = flags_0x84 & 0xfffffeff | flag;
	return;
}

void edCTextStyle::SetEolAutomatic(uint param_2)
{
	this->flags_0x84 = this->flags_0x84 & 0xffffff7f | param_2;
	return;
}

void edCTextStyle::SetHorizontalJustification(uint param_2)
{
	this->flags_0x84 = this->flags_0x84 & 0xffffffef | param_2;
	return;
}

void edCTextStyle::SetVerticalAlignment(uint flag)
{
	flags_0x84 = flags_0x84 & 0xfffffff3 | flag;
	return;
}

void edCTextStyle::SetHorizontalAlignment(uint flag)
{
	flags_0x84 = flags_0x84 & 0xfffffffc | flag;
	return;
}

void edCTextStyle::SetFont(edCTextFont* pPackedFont, bool bUpdateSpacing)
{
	float fVar1;

	if (pPackedFont == (edCTextFont*)0x0) {
		pPackedFont = g_PackedFontPtr_004324d0;
	}
	this->pPackedFont = pPackedFont;
	if (bUpdateSpacing != false) {
#ifndef PLATFORM_PS2
		IMPLEMENTATION_GUARD();
#endif
		//fVar1 = GetSpaceWidth(pPackedFont);
		//pFontFileData->spaceSize = fVar1;
	}
	return;
}

// Should be in: D:/projects/EdenLib/edText/sources/edTextStyle.cpp
void edCTextStyle::Reset()
{
	this->pPackedFont = g_PackedFontPtr_004324d0;

	edF32Matrix4SetIdentityHard(&this->m0);
	edF32Matrix4SetIdentityHard(&this->m1);

	this->flags_0x84 = 0x400;
	this->field_0x8c = 1.0f;
	this->field_0x90 = 1.0f;
	this->spaceSize = 2.0f;
	this->rotation = 0.0f;
	this->xScale = 1.0f;
	this->yScale = 1.0f;
	this->horizontalSize = 0.0f;
	this->verticalSize = 0.0f;
	this->shadowShiftX = 2.0f;
	this->shadowShiftY = 2.0f;
	this->rgbaColour = -1;
	this->altColour = -1;
	this->alpha = 0xff;
	return;
}