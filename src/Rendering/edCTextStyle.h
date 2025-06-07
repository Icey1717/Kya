#ifndef _EDCTEXTSTYLE_H
#define _EDCTEXTSTYLE_H

#include "Types.h"

class edCTextStyle
{
public:
	void TransformMatrix(float x, float y, edF32MATRIX4* outMatrix);
	void SetRotation(float angle);
	void GetScale(float* outX, float* outY);
	void SetScale(float xScale, float yScale);
	void SetVerticalSize(float verticalSize);
	float GetHorizontalSize();
	void SetHorizontalSize(float horizontalSize);
	void GetShadowShift(float* outX, float* outY);
	void SetShadowShift(float x, float y);
	uint GetDebug();
	void SetShadow(uint flag);
	void SetEolAutomatic(uint param_2);
	void SetHorizontalJustification(uint param_2);
	void SetVerticalAlignment(uint flag);
	void SetHorizontalAlignment(uint flag);
	void SetFont(struct edCTextFont* pPackedFont, bool bUpdateSpacing);
	void Reset();

	edF32MATRIX4 m0;
	edF32MATRIX4 m1;
	undefined4 pFunction;
	uint flags_0x84;
	struct edCTextFont* pPackedFont;
	float field_0x8c;
	float field_0x90;
	float spaceSize;
	float rotation;
	float xScale;
	float yScale;
	float horizontalSize;
	float verticalSize;
	float shadowShiftX;
	float shadowShiftY;
	int rgbaColour;
	int alpha;
	int altColour;
};

edCTextStyle* edTextStyleGetCurrent();
edCTextStyle* edTextStyleSetCurrent(edCTextStyle* pNewFont);

extern edCTextStyle* pedTextCurrentStyle;

#endif // _EDCTEXTSTYLE_H
