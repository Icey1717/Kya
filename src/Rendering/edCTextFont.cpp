#include "edCTextFont.h"
#include "port/pointer_conv.h"

FontSymbolData* edCTextFont::GetSymbol(uint character)
{
	FontPacked_2C* pFVar1;
	uint uVar2;
	uint uVar3;
	int iVar4;
	SegmentPacked* pSVar5;

	pFVar1 = RESOLVE_FONT_SUB_DATA(this->pSubData);
	pSVar5 = pFVar1->pSegment;
	iVar4 = 0;
	if (pFVar1 != (FontPacked_2C*)0x0) {
		if (((pFVar1->pOverrideData != (ushort*)0x0) && (character < 0x100)) && (uVar2 = (uint)pFVar1->pOverrideData[character & 0xff], uVar2 != 0)) {
			character = uVar2;
		}
	}
	uVar2 = 0;
	if (this->pageOffset != 0) {
		do {
			uVar3 = (uint)pSVar5->header;
			if (character < uVar3) {
				return (FontSymbolData*)0x0;
			}
			if (character < uVar3 + pSVar5->field_0x2) {
				return (FontSymbolData*)((char*)pFVar1->field_0x4 + (iVar4 + (character - uVar3)) * 0x1c);
			}
			uVar2 = uVar2 + 1;
			iVar4 = iVar4 + (uint)pSVar5->field_0x2;
			pSVar5 = pSVar5 + 1;
		} while (uVar2 < this->pageOffset);
	}
	return (FontSymbolData*)0x0;
}

float edCTextFont::GetRelativeAlignment(uint param_2, uint param_3)
{
	int* piVar1;
	ushort* puVar2;
	ushort uVar3;
	int iVar4;

	piVar1 = (int*)(RESOLVE_FONT_SUB_DATA(this->pSubData))->field_0x1c;
	if (piVar1 != (int*)0x0) {
		puVar2 = (ushort*)piVar1[1];
		iVar4 = *piVar1;
		if (param_2 != 0) {
			for (; (iVar4 != 0 && (*puVar2 != param_2)); puVar2 = puVar2 + 4) {
				iVar4 = iVar4 + -1;
			}
			if (iVar4 != 0) {
				uVar3 = puVar2[1];
				for (puVar2 = *(ushort**)(puVar2 + 2); (uVar3 != 0 && (*puVar2 != param_3)); puVar2 = puVar2 + 2) {
					uVar3 = uVar3 - 1;
				}
				if (uVar3 != 0) {
					return (float)(int)(short)puVar2[1];
				}
			}
		}
	}
	return 0.0f;
}