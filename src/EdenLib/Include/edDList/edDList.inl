#ifndef ED_DLIST_INL
#define ED_DLIST_INL

#include "Types.h"

inline void edDListPatchRGBASprite_Inline(_rgba* pRgba, _rgba* pColor, uint index)
{
	_rgba* p_Var8 = pRgba + index * 4;
	p_Var8[0] = *pColor;
	p_Var8[1] = *pColor;
	p_Var8[2] = *pColor;
	p_Var8[3] = *pColor;

	return;
}

inline void edDListPatchRGBA_Inline(_rgba* param_1, _rgba newColor, uint param_3, uint param_4)
{
	uint uVar2 = param_3;

	if (0x47 < param_3) {
		uVar2 = param_3 + ((param_3 - 0x48) / 0x46 + 1) * 2;
	}

	if (0x47 < param_4) {
		param_4 = param_4 + ((param_4 - 0x48) / 0x46 + 1) * 2;
	}

	_rgba* p_Var7 = param_1 + uVar2;
	*p_Var7 = newColor;
	if (((1 < param_3) && ((uint)((int)param_3 % 0x46) < 2)) && (uVar2 < param_4)) {
		p_Var7[2] = newColor;
	}

	return;
}

#endif // ED_DLIST_INL