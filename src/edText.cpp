#include "edText.h"
#include "PackedFont.h"
#include "ed3D.h"

FontPacked* g_PackedFontPtr_004324d0 = (FontPacked*)g_PackedFontData_0041f290;


bool FontSetup_Internal(FontPacked* pFont)
{
	bool bVar2;
	FontPacked_2C* pFVar3;
	TextureInfoSmall* pTVar4;
	char* pcVar5;
	PagePacked* pSVar6;
	int iStack4;
	uint uVar1;

	if (pFont == (FontPacked*)0x0) {
		bVar2 = false;
	}
	else {
		if ((((pFont->header[0] == 'M') && (pFont->header[1] == 'K')) && (pFont->header[2] == 'F')) && (pFont->header[3] == 'N')) {
			if ((pFont->field_0x4 == 2) && (pFont->field_0x6 == 0)) {
				bVar2 = false;
				if (pFont->pSubData == (FontPacked_2C*)0x0) {
					pFVar3 = new FontPacked_2C();
					pFont->pSubData = pFVar3;
					if (pFont->pSubData == (FontPacked_2C*)0x0) {
						bVar2 = false;
					}
					else {
						pFont->pSubData->pTextureInfo = (TextureInfoSmall*)0x0;
						pFont->pSubData->field_0x1c = (void*)0x0;
						pFont->pSubData->field_0x20 = (void*)0x0;
						pFont->pSubData->field_0x4 = (void*)0x0;
						pFont->pSubData->pGlyphData = (void*)0x0;
						pFont->pSubData->pOverrideData = (ushort*)0x0;
						if (((pFont->glyphHeader[0] == 'G') && (pFont->glyphHeader[1] == 'L')) && ((pFont->glyphHeader[2] == 'Y' && (pFont->glyphHeader[3] == 'F')))) {
							pFont->pSubData->pGlyphData = pFont + 1;
							pcVar5 = pFont[1].header + (uint)pFont->count_0x18 * 4;
							pFont->pSubData->field_0x4 = pcVar5;
							pcVar5 = pcVar5 + (uint)pFont->field_0x10 * 0x1c;
							if ((pFont->field_0xc & 0xc) == 0) {
								pFont->pSubData->field_0x1c = (void*)0x0;
								pFont->pSubData->field_0x20 = (void*)0x0;
							}
							else {
								if ((((*pcVar5 != 'K') || (pcVar5[1] != 'E')) || (pcVar5[2] != 'R')) || (pcVar5[3] != 'N')) {
									return false;
								}
								pcVar5 = pcVar5 + 4;
								if ((pFont->field_0xc & 4) != 0) {
									pFont->pSubData->field_0x1c = pcVar5;
								}
								if ((pFont->field_0xc & 8) != 0) {
									pFont->pSubData->field_0x20 = pcVar5;
								}
							}
							if (((*pcVar5 == 'S') && (pcVar5[1] == 'E')) && ((pcVar5[2] == 'G' && (pcVar5[3] == 'M')))) {
								pFont->pSubData->pSegment = (SegmentPacked*)(pcVar5 + 4);
								pSVar6 = (PagePacked*)((SegmentPacked*)(pcVar5 + 4) + pFont->pageOffset);
								if (((pSVar6->header[0] == 'P') && (pSVar6->header[1] == 'A')) && ((pSVar6->header[2] == 'G' && (pSVar6->header[3] == 'E')))) {
#ifdef PLATFORM_PS2
									pcVar5 = (char*)((uint)(char*)&pSVar6->field_0x13 & 0xfffffff0);
#else
									char* intPtr = (char*)pSVar6 + 0x4 + 0xf;
									pcVar5 = (char*)((unsigned long long)intPtr & 0xfffffffffffffff0);
#endif
									pTVar4 = ed3D::LoadTextureFromBuffer(pcVar5, *(int*)(pcVar5 + 8), &iStack4, (TextureInfoSmall*)0x0, 0);
									pFont->pSubData->pTextureInfo = pTVar4;
									ed3D::GetMaterialInfoFromTexture(&pFont->pSubData->materialInfo, 0, pFont->pSubData->pTextureInfo, 2);
									bVar2 = true;
								}
								else {
									bVar2 = false;
								}
							}
							else {
								bVar2 = false;
							}
						}
						else {
							bVar2 = false;
						}
					}
				}
			}
			else {
				bVar2 = false;
			}
		}
		else {
			bVar2 = false;
		}
	}
	return bVar2;
}


bool FontSetup(FontPacked* pFontData)
{
	bool uVar1;

	if (pFontData == (FontPacked*)0x0) {
		uVar1 = false;
	}
	else {
		uVar1 = FontSetup_Internal(pFontData);
	}
	return uVar1;
}

bool Init_edText(void)
{
	bool bVar1;
	bool uVar1;

	bVar1 = FontSetup(g_PackedFontPtr_004324d0);
	if (bVar1 == false) {
		uVar1 = false;
	}
	else {
		//edText::LoadDebugLib_0028c9b0();
		//uVar1 = FUN_0028db80((int**)&g_saveSizeString, DAT_0044916c);
	}
	return true;
	return uVar1;
}
