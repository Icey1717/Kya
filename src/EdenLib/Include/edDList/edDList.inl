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

#endif // ED_DLIST_INL