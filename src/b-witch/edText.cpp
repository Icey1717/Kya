#include "edText.h"
#include "PackedFont.h"
#include "ed3D.h"
#include "Rendering/edCTextFont.h"
#include "Rendering/edCTextFormat.h"
#include "port/pointer_conv.h"
#include "EdenLib/edText/sources/edTextResources.h"

edCTextFont* g_PackedFontPtr_004324d0 = (edCTextFont*)g_PackedFontData_0041f290;

bool Install(edCTextFont* pFont)
{
	bool bInstallSuccess;
	FontPacked_2C* pFVar3;
	ed_g2d_manager* pTVar4;
	char* pcVar5;
	PagePacked* pSVar6;
	int iStack4;
	uint uVar1;

	if (pFont == (edCTextFont*)0x0) {
		bInstallSuccess = false;
	}
	else {
		if ((((pFont->header[0] == 'M') && (pFont->header[1] == 'K')) && (pFont->header[2] == 'F')) && (pFont->header[3] == 'N')) {
			if ((pFont->field_0x4 == 2) && (pFont->field_0x6 == 0)) {
				bInstallSuccess = false;
				if ((FontPacked_2C*)pFont->pSubData == (FontPacked_2C*)0x0) {
					pFVar3 = new FontPacked_2C();

#ifdef PLATFORM_WIN
					pFont->pSubData = STORE_SECTION(pFVar3);
#else
					pFont->pSubData = pFVar3;
#endif
					if ((FontPacked_2C*)pFont->pSubData == (FontPacked_2C*)0x0) {
						bInstallSuccess = false;
					}
					else {
						pFVar3->pTextureInfo = (ed_g2d_manager*)0x0;
						pFVar3->field_0x1c = (void*)0x0;
						pFVar3->field_0x20 = (void*)0x0;
						pFVar3->field_0x4 = (void*)0x0;
						pFVar3->pGlyphData = (void*)0x0;
						pFVar3->pOverrideData = (ushort*)0x0;
						if (((pFont->glyphHeader[0] == 'G') && (pFont->glyphHeader[1] == 'L')) && ((pFont->glyphHeader[2] == 'Y' && (pFont->glyphHeader[3] == 'F')))) {
							pFVar3->pGlyphData = pFont + 1;
							pcVar5 = pFont[1].header + (uint)pFont->count_0x18 * 4;
							pFVar3->field_0x4 = pcVar5;
							pcVar5 = pcVar5 + (uint)pFont->field_0x10 * 0x1c;
							if ((pFont->field_0xc & 0xc) == 0) {
								pFVar3->field_0x1c = (void*)0x0;
								pFVar3->field_0x20 = (void*)0x0;
							}
							else {
								if ((((*pcVar5 != 'K') || (pcVar5[1] != 'E')) || (pcVar5[2] != 'R')) || (pcVar5[3] != 'N')) {
									return false;
								}
								pcVar5 = pcVar5 + 4;
								if ((pFont->field_0xc & 4) != 0) {
									pFVar3->field_0x1c = pcVar5;
								}
								if ((pFont->field_0xc & 8) != 0) {
									pFVar3->field_0x20 = pcVar5;
								}
							}
							if (((*pcVar5 == 'S') && (pcVar5[1] == 'E')) && ((pcVar5[2] == 'G' && (pcVar5[3] == 'M')))) {
								pFVar3->pSegment = (SegmentPacked*)(pcVar5 + 4);
								pSVar6 = (PagePacked*)((SegmentPacked*)(pcVar5 + 4) + pFont->pageOffset);
								if (((pSVar6->header[0] == 'P') && (pSVar6->header[1] == 'A')) && ((pSVar6->header[2] == 'G' && (pSVar6->header[3] == 'E')))) {
#ifdef PLATFORM_PS2
									char* intPtr = (char*)pSVar6 + 0x4 + 0xf;
									pcVar5 = (char*)((int)intPtr & 0xfffffff0);
#else
									char* intPtr = (char*)pSVar6 + 0x4 + 0xf;
									pcVar5 = (char*)((unsigned long long)intPtr & 0xfffffffffffffff0);
#endif
									pTVar4 = ed3DInstallG2D(pcVar5, *(int*)(pcVar5 + 8), &iStack4, (ed_g2d_manager*)0x0, 0);
									pFVar3->pTextureInfo = pTVar4;
									edDListCreatMaterialFromIndex(&pFVar3->materialInfo, 0, pFVar3->pTextureInfo, 2);
									bInstallSuccess = true;
								}
								else {
									bInstallSuccess = false;
								}
							}
							else {
								bInstallSuccess = false;
							}
						}
						else {
							bInstallSuccess = false;
						}
					}
				}
			}
			else {
				bInstallSuccess = false;
			}
		}
		else {
			bInstallSuccess = false;
		}
	}
	return bInstallSuccess;
}

int g_iMaxTextIconEntryCount_0044916c = 0x26;

// Should be in: D:/projects/EdenLib/edText/sources/edTextFont.cpp
bool edTextInstallFont(edCTextFont* pFontData)
{
	bool bInstallSuccess;

	if (pFontData == (edCTextFont*)0x0) {
		bInstallSuccess = false;
	}
	else {
		bInstallSuccess = Install(pFontData);
	}
	return bInstallSuccess;
}

// Should be in: D:/projects/EdenLib/edText/sources/edTextInit.cpp
bool edTextInit(void)
{
	bool bVar1;
	bool uVar1;

	NAME_NEXT_OBJECT("Default Font");

	bVar1 = edTextInstallFont(g_PackedFontPtr_004324d0);
	if (bVar1 == false) {
		uVar1 = false;
	}
	else {
		//edTextEdDebugInit();
		edTextResources.Init(g_iMaxTextIconEntryCount_0044916c);
	}
	return true;
	return uVar1;
}

void edTextDraw(float x, float y, char* text)
{
	bool bVar1;
	uint uVar2;
	Rectangle window;
	edCTextFormat auStack5504;

	bVar1 = auStack5504.FormatString(text);
	if (bVar1 != false) {
		auStack5504.GetRect();
		uVar2 = auStack5504.fontData_0x850->GetDebug();
		if (uVar2 != 0) {
			auStack5504.DisplayDebugInfos(x, y);
		}
		window.width = 640.0f;
		window.height = 512.0f;
		window.x = 0.0f;
		window.y = 0.0f;
		auStack5504.DisplayWindow(x, y, &window);
	}

	return;
}