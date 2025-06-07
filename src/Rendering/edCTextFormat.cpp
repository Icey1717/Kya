#include "edCTextFormat.h"
#include "edDlist.h"
#include "ed3D.h"
#include "edCTextFont.h"

#include "port/pointer_conv.h"

#ifdef PLATFORM_WIN
#include "renderer.h"
#else
#include <libvu0.h>
#include <libdma.h>
#endif
#include "MathOps.h"
#include "EdenLib/edText/sources/edTextResources.h"
#include "Font.h"

struct CharacterData {
	uint colour;
	float pos_x1;
	float pos_y1;
	float pos_x2;
	float pos_y2;
	float tex_x1;
	float tex_y1;
	float tex_x2;
	float tex_y2;
};

struct DrawText16 {
	struct CharacterData* pCharacterData;
	int characterCount;
	struct edDList_material* pMaterialInfoA;
	struct edDList_material* pMaterialInfoB;
};


void edCTextFormat::DisplayWindow(float x, float y, Rectangle* pRect)
{
	edF32MATRIX4 textMatrix;
	float shadowShiftY;
	float shadowShiftX;

	if ((((this->field_0x30 != 0) || (this->nbBitmaps != 0)) && (pRect->width != 0.0f)) && (pRect->height != 0.0f)) {
		(this->vector_0x10).x = pRect->x + this->offsetX_0x0;
		(this->vector_0x10).y = pRect->y + this->offsetY_0x4;
		(this->vector_0x10).width = pRect->width;
		(this->vector_0x10).height = pRect->height;

		this->fontData_0x850->TransformMatrix(x, y, &textMatrix);
		edDListLoadMatrix(&textMatrix);

		if ((this->flags_0x28 & TEXT_FLAG_SHADOW) != 0) {
			this->fontData_0x850->GetShadowShift(&shadowShiftX, &shadowShiftY);
		}

		if ((this->flags_0x28 & TEXT_FLAG_SHADOW) != 0) {
			DrawString(shadowShiftX + this->offsetX_0x0, shadowShiftY + this->offsetY_0x4, false);
		}

		DrawString(this->offsetX_0x0, this->offsetY_0x4, true);
	}

	return;
}

void edCTextFormat::Display(float x, float y)
{
	Rectangle window;

	window.width = 640.0f;
	window.x = 0.0f;
	window.height = 512.0f;
	window.y = 0.0f;
	DisplayWindow(x, y, &window);

	return;
}

void edCTextFormat::DisplayDebugInfos(float x, float y)
{
	TextLine* pTVar2;
	uint uVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	edF32MATRIX4 MStack64;

	this->fontData_0x850->TransformMatrix(x, y, &MStack64);
	edDListLoadMatrix(&MStack64);
	fVar4 = this->offsetY_0x4;
	fVar5 = this->offsetX_0x0;
	fVar7 = fVar4 + this->field_0xc;
	fVar6 = fVar5 + this->field_0x8;
	edDListUseMaterial((edDList_material*)0x0);
	fVar6 = fVar6 + 1.0f;
	fVar7 = fVar7 + 1.0f;
	edDListColor4u8(0xff, 0xff, 0xff, 0x40);
	edDListBegin(1.0f, 1.0f, 1.0f, DISPLAY_LIST_DATA_TYPE_TRIANGLE_LIST, 4);
	edDListVertex4f(fVar5, fVar4, 0.0f, 0.0f);
	edDListVertex4f(fVar6, fVar4, 0.0f, 0.0f);
	edDListVertex4f(fVar5, fVar7, 0.0f, 0.0f);
	edDListVertex4f(fVar6, fVar7, 0.0f, 0.0f);
	edDListEnd();
	pTVar2 = this->aTextLines;
	uVar3 = 0;
	if (this->lineCount != 0) {
		do {
			fVar6 = this->offsetX_0x0;
			fVar4 = this->field_0x8;
			fVar7 = pTVar2->field_0x14 + this->offsetY_0x4;
			fVar5 = pTVar2->field_0x18 + this->offsetY_0x4;
			edDListUseMaterial((edDList_material*)0x0);
			fVar4 = fVar6 + fVar4 + 1.0f;
			fVar5 = fVar5 + 1.0f;
			edDListColor4u8(0x7f, (char)((uVar3 & 1) << 7) + 0x7f, 0xff, 0x40);
			edDListBegin(1.0f, 1.0f, 1.0f, DISPLAY_LIST_DATA_TYPE_TRIANGLE_LIST, 4);
			edDListVertex4f(fVar6, fVar7, 0.0f, 0.0f);
			edDListVertex4f(fVar4, fVar7, 0.0f, 0.0f);
			edDListVertex4f(fVar6, fVar5, 0.0f, 0.0f);
			edDListVertex4f(fVar4, fVar5, 0.0f, 0.0f);
			edDListEnd();
#ifdef PLATFORM_WIN
			Renderer::Draw();
#endif
			uVar3 = uVar3 + 1;
			pTVar2 = pTVar2 + 1;
		} while (uVar3 < this->lineCount);
	}
	return;
}

void edCTextFormat::DrawString(float x, float y, bool bFlag)
{
	byte curCharacter;
	ushort uVar2;
	edTextBitmap* pTextBitmap;
	edCTextFont* pFontPacked;
	bool bVar4;
	bool bVar5;
	void* pvVar6;
	FontSymbolData* pSymbolData;
	CharacterData* puVar7;
	uint uVar7;
	char* pcVar8;
	edCTextStyle* pTextStyle;
	ulong uVar10;
	TextLine* pCurTextLine;
	char* pCurCharacter;
	uint unaff_s7_lo;
	byte* pbVar12;
	float characterWidth;
	float fVar14;
	float fVar15;
	float fVar16;
	float fVar17;
	float fVar18;
	float fVar19;
	float in_f21;
	float in_f23;
	float drawX2;
	float in_f25;
	float drawX1;
	float in_f27;
	float unaff_f26;
	float curX;
	float unaff_f28;
	float spaceBetweenCharacters;
	float local_68;
	uint local_44;
	uint local_30;
	DrawText16 drawCommandParams;

	pvVar6 = edSystemFastRamGetAddr();
	drawCommandParams.pCharacterData = (CharacterData*)((char*)pvVar6 + 0x800);
	drawCommandParams.characterCount = 0;
	drawCommandParams.pMaterialInfoA = (edDList_material*)0x0;
	drawCommandParams.pMaterialInfoB = (edDList_material*)0x0;

	local_30 = 0;
	pCurTextLine = this->aTextLines;

	if (this->lineCount != 0) {
		do {
			spaceBetweenCharacters = 0.0f;
			local_68 = pCurTextLine->pTextStyle->field_0x90;
			curX = x;

			if (pCurTextLine->field_0x24 == false) {
				if (((this->flags_0x28 & 0x80) == 0) || ((this->flags_0x28 & 0x10) == 0)) {
					if ((this->flags_0x28 & 2) == 0) {
						if ((this->flags_0x28 & 1) != 0) {
							curX = x + (this->field_0x8 - pCurTextLine->field_0x10);
						}
					}
					else {
						curX = x + (this->field_0x8 - pCurTextLine->field_0x10) / 2.0f;
					}
				}
				else {
					pcVar8 = pCurTextLine->pTextEnd + (-(uint)pCurTextLine->field_0x22 - (int)pCurTextLine->pTextStart);
					characterWidth = pCurTextLine->pTextStyle->GetHorizontalSize();
					uVar2 = pCurTextLine->field_0x20;
					uVar7 = (uint)uVar2;
					characterWidth = characterWidth - pCurTextLine->field_0x10;
					if ((uVar7 == 0) || (pcVar8 == (char*)0x0)) {
						if (uVar7 == 0) {
							if (pcVar8 != (char*)0x0) {
								if ((int)pcVar8 < 0) {
									fVar14 = (float)((uint)pcVar8 >> 1 | (uint)pcVar8 & 1);
									fVar14 = fVar14 + fVar14;
								}
								else {
									fVar14 = (float)(int)pcVar8;
								}

								local_68 = local_68 + characterWidth / fVar14;
							}
						}
						else {
							if (false) {
								spaceBetweenCharacters = (float)((uint)(uVar2 >> 1) | uVar7 & 1);
								spaceBetweenCharacters = spaceBetweenCharacters + spaceBetweenCharacters;
							}
							else {
								spaceBetweenCharacters = (float)uVar7;
							}

							spaceBetweenCharacters = characterWidth / spaceBetweenCharacters + 0.0f;
						}
					}
					else {
						if (false) {
							spaceBetweenCharacters = (float)((uint)(uVar2 >> 1) | uVar7 & 1);
							spaceBetweenCharacters = spaceBetweenCharacters + spaceBetweenCharacters;
						}
						else {
							spaceBetweenCharacters = (float)uVar7;
						}

						spaceBetweenCharacters = (characterWidth / spaceBetweenCharacters) / 2.0f + 0.0f;
						if (false) {
							fVar14 = (float)((uint)(uVar2 >> 1) | uVar7 & 1);
							fVar14 = fVar14 + fVar14;
						}
						else {
							fVar14 = (float)uVar7;
						}

						if ((int)pcVar8 < 0) {
							fVar15 = (float)((uint)pcVar8 >> 1 | (uint)pcVar8 & 1);
							fVar15 = fVar15 + fVar15;
						}
						else {
							fVar15 = (float)(int)pcVar8;
						}

						local_68 = local_68 + (characterWidth - spaceBetweenCharacters * fVar14) / fVar15;
					}
				}
			}
			else {
				if ((this->flags_0x28 & 2) == 0) {
					if ((this->flags_0x28 & 1) != 0) {
						curX = x + (this->field_0x8 - pCurTextLine->field_0x10);
					}
				}
				else {
					curX = x + (this->field_0x8 - pCurTextLine->field_0x10) / 2.0f;
				}
			}

			bVar4 = true;
			pCurCharacter = pCurTextLine->pTextStart;
			pTextStyle = pCurTextLine->pTextStyle;
			pbVar12 = (byte*)pCurTextLine->pText;
			uVar10 = 0xffffffffffffffff;
			if (pCurCharacter <= pCurTextLine->pTextEnd) {
				do {
					if (bVar4) {
						local_44 = uVar10;
					}
					else {
						bVar4 = true;
					}

					curCharacter = *pCurCharacter;
					uVar10 = curCharacter;
					characterWidth = 0.0f;
					bVar5 = false;
					pCurCharacter = pCurCharacter + 1;

					if (uVar10 == 2) {
						pTextStyle = pTextStyle + 1;
						bVar4 = false;
					}
					else {
						if (uVar10 == 1) {
							bVar5 = true;
							curCharacter = *pbVar12;
							pbVar12 = pbVar12 + 1;
							pTextBitmap = this->aBitmaps[curCharacter];
							drawCommandParams.pMaterialInfoA = pTextBitmap->pMaterial;
							characterWidth = pTextBitmap->fWidth;
							unaff_f28 = pTextBitmap->field_0x14;
							in_f27 = pTextBitmap->field_0x18;
							unaff_f26 = pTextBitmap->field_0x1c;
							in_f25 = pTextBitmap->field_0x20;
							in_f23 = y + (pCurTextLine->field_0x14 + pCurTextLine->field_0x18) / 2.0f + pTextBitmap->field_0x8 * (pTextBitmap->fHeight - 1.0f);
							in_f21 = in_f23 + pTextBitmap->fHeight;
							drawX2 = curX + characterWidth;
							drawX1 = curX;

							if (bFlag == false) {
								unaff_s7_lo = pTextStyle->alpha;
							}
							else {
								unaff_s7_lo = pTextStyle->altColour;
							}
						}
						else {
							if (uVar10 == 0x20) {
								characterWidth = spaceBetweenCharacters + pTextStyle->spaceSize;
							}
							else {
								if ((uVar10 != 10) && (uVar10 != 0)) {
									pFontPacked = pTextStyle->pPackedFont;
									pSymbolData = pFontPacked->GetSymbol(curCharacter);

									if (pSymbolData != (FontSymbolData*)0x0) {
										drawCommandParams.pMaterialInfoA = &(RESOLVE_FONT_SUB_DATA(pFontPacked->pSubData))->materialInfo;

										if (bFlag == false) {
											unaff_s7_lo = pTextStyle->alpha;
										}
										else {
											unaff_s7_lo = pTextStyle->rgbaColour;
										}

										fVar14 = pSymbolData->field_0x10;
										characterWidth = pFontPacked->GetRelativeAlignment(local_44, uVar10);
										characterWidth = fVar14 + characterWidth;
										drawX2 = curX + fVar14;
										bVar5 = true;
										unaff_f28 = pSymbolData->field_0x0;
										in_f27 = pSymbolData->field_0x4;
										unaff_f26 = pSymbolData->field_0x8;
										in_f25 = pSymbolData->field_0xc;
										in_f23 = pSymbolData->field_0x18 + ((y + pCurTextLine->field_0x1c) - pSymbolData->field_0x14);
										in_f21 = in_f23 + pSymbolData->field_0x14;
										drawX1 = curX;
									}
								}
							}
						}
					}

					if (bVar4) {
						if (bVar5) {
							fVar14 = (this->vector_0x10).x;
							fVar15 = (this->vector_0x10).width;
							fVar18 = fVar14 + fVar15;

							if ((drawX1 <= fVar18) && (fVar14 <= drawX2)) {
								fVar16 = (this->vector_0x10).y;
								fVar17 = (this->vector_0x10).height;
								fVar19 = fVar16 + fVar17;

								if ((in_f23 <= fVar19) && (fVar16 <= in_f21)) {
									if (drawX1 < fVar14) {
										unaff_f28 = unaff_f26 - ((unaff_f26 - unaff_f28) * (drawX2 - fVar14)) / (drawX2 - drawX1);
										drawX1 = fVar14;
									}

									if (fVar18 < drawX2) {
										fVar14 = fVar14 + fVar15;
										unaff_f26 = unaff_f28 + ((unaff_f26 - unaff_f28) * (fVar14 - drawX1)) / (drawX2 - drawX1);
										drawX2 = fVar14;
									}

									if (in_f23 < fVar16) {
										in_f27 = in_f25 - ((in_f25 - in_f27) * (in_f21 - fVar16)) / (in_f21 - in_f23);
										in_f23 = fVar16;
									}

									if (fVar19 < in_f21) {
										fVar16 = fVar16 + fVar17;
										in_f25 = in_f27 + ((in_f25 - in_f27) * (fVar16 - in_f23)) / (in_f21 - in_f23);
										in_f21 = fVar16;
									}

									if ((drawCommandParams.characterCount != 0) && (drawCommandParams.pMaterialInfoB != drawCommandParams.pMaterialInfoA)) {
										SendTextRenderCommands_0028b0e0(&drawCommandParams);
									}

									drawCommandParams.pMaterialInfoB = drawCommandParams.pMaterialInfoA;
									puVar7 = drawCommandParams.pCharacterData + drawCommandParams.characterCount;
									puVar7->colour = unaff_s7_lo;
									puVar7->pos_x1 = drawX1;
									puVar7->pos_y1 = in_f23;
									puVar7->pos_x2 = drawX2;
									puVar7->pos_y2 = in_f21;
									puVar7->tex_x1 = unaff_f28;
									puVar7->tex_y1 = in_f27;
									puVar7->tex_x2 = unaff_f26;
									puVar7->tex_y2 = in_f25;
									drawCommandParams.characterCount = drawCommandParams.characterCount + 1;
								}
							}
						}

						if (characterWidth != 0.0f) {
							curX = curX + characterWidth + local_68;
						}
					}
				} while (pCurCharacter <= pCurTextLine->pTextEnd);
			}

			local_30 = local_30 + 1;
			pCurTextLine = pCurTextLine + 1;
		} while (local_30 < this->lineCount);
	}

	SendTextRenderCommands_0028b0e0(&drawCommandParams);

	return;
}

void edCTextFormat::SendTextRenderCommands_0028b0e0(DrawText16* pTextRenderCommands)
{
	int iVar1;
	uint uVar2;
	CharacterData* pCharacterData;

	if (pTextRenderCommands->characterCount != 0) {
		edDListUseMaterial(pTextRenderCommands->pMaterialInfoB);
		edDListAlphaTestAndZTest(1, 7, 0, 0, 0, 0, 1, 1);
		edDListBlendFuncNormal();
		edDListBlendSet(1);
		edDListBegin(0.0f, 0.0f, 0.0f, 8, pTextRenderCommands->characterCount << 2);
		pCharacterData = pTextRenderCommands->pCharacterData;
		iVar1 = pTextRenderCommands->characterCount;
		while (iVar1 != 0) {
			uVar2 = pCharacterData->colour;
			edDListColor4u8((byte)((uVar2 >> 0x18) + 1 >> 1), 
				(byte)((uVar2 >> 0x10 & 0xff) + 1 >> 1), 
				(byte)((uVar2 >> 8 & 0xff) + 1 >> 1),
				(byte)((uVar2 & 0xff) + 1 >> 1));

			edDListTexCoo2f(pCharacterData->tex_x1, pCharacterData->tex_y1);
			edDListVertex4f(pCharacterData->pos_x1, pCharacterData->pos_y1, 0.0f, 0.0f);
			edDListTexCoo2f(pCharacterData->tex_x1, pCharacterData->tex_y2);
			edDListVertex4f(pCharacterData->pos_x1, pCharacterData->pos_y2, 0.0f, 0.0f);
			edDListTexCoo2f(pCharacterData->tex_x2, pCharacterData->tex_y1);
			edDListVertex4f(pCharacterData->pos_x2, pCharacterData->pos_y1, 0.0f, 0.0f);
			edDListTexCoo2f(pCharacterData->tex_x2, pCharacterData->tex_y2);
			edDListVertex4f(pCharacterData->pos_x2, pCharacterData->pos_y2, 0.0f, 0.0f);

			pTextRenderCommands->characterCount = pTextRenderCommands->characterCount + -1;
			pCharacterData = pCharacterData + 1;
			iVar1 = pTextRenderCommands->characterCount;
		}
		edDListEnd();
#ifdef PLATFORM_WIN
		Renderer::Draw();
#endif
	}
	return;
}

void edCTextFormat::GetRect()
{
	ushort uVar1;
	bool bVar2;
	bool bVar3;
	bool bVar4;
	bool bVar5;
	FontSymbolData* pSVar6;
	uint uVar7;
	byte* pbVar8;
	byte* pbVar9;
	edCTextFont* pFontPacked;
	ulong uVar10;
	TextLine* pTVar11;
	edCTextStyle* pFVar12;
	byte* pbVar13;
	float fVar14;
	float fVar15;
	float fVar16;
	float fVar17;
	float unaff_f20;
	float fVar18;
	float fVar19;
	byte* local_28;
	byte* local_24;
	byte* local_20;
	edCTextStyle* local_8;
	int local_4;

	offsetX_0x0 = 0.0f;
	offsetY_0x4 = 0.0f;
	field_0xc = 0.0f;
	pFVar12 = fontData_0x850;
	if ((flags_0x28 & 0x10) == 0) {
		field_0x8 = 0.0f;
	}
	else {
		fVar14 = pFVar12->GetHorizontalSize();
		field_0x8 = fVar14;
	}
	lineCount = 0;
	pFontPacked = fontData_0x850[0].pPackedFont;
	fVar14 = 0.0f;
	pTVar11 = aTextLines;
	pbVar9 = (byte*)pScratchpadB;
	pbVar13 = (byte*)pScratchpadA;
	local_28 = pbVar13;
	local_8 = pFVar12;
	do {
		pTVar11->pTextStart = (char*)pbVar9;
		pTVar11->pTextEnd = (char*)0x0;
		fVar18 = 0.0f;
		pTVar11->pTextStyle = pFVar12;
		pTVar11->pText = (char*)pbVar13;
		pTVar11->field_0x10 = 0.0f;
		pTVar11->field_0x20 = 0;
		pTVar11->field_0x22 = 0;
		bVar3 = false;
		bVar5 = false;
		bVar2 = false;
		pTVar11->field_0x24 = false;
		pTVar11->field_0x14 = 9999.0f;
		fVar19 = 0.0f;
		pTVar11->field_0x18 = -9999.0f;
		uVar10 = 0xffffffffffffffff;
		pTVar11->field_0x1c = -9999.0f;
		bVar4 = true;
		fVar16 = 0.0f;
		local_24 = pbVar9;
		do {
			pbVar8 = pbVar9;
			if (bVar4) {
				local_4 = (int)uVar10;
				local_20 = pbVar8;
			}
			else {
				bVar4 = true;
			}
			uVar10 = (ulong)*pbVar8;
			pbVar9 = pbVar8 + 1;
			fVar15 = fVar16;
			if (uVar10 == 2) {
				bVar4 = false;
				pFVar12 = pFVar12 + 1;
				pTVar11->field_0x22 = pTVar11->field_0x22 + 2;
			}
			else {
				if (uVar10 == 1) {
					fVar15 = aBitmaps[*pbVar13]->fHeight;
					unaff_f20 = aBitmaps[*pbVar13]->fWidth;

					pbVar13 = pbVar13 + 1;
					if (fVar15 <= fVar16) {
						fVar15 = fVar16;
					}
				}
				else {
					if (uVar10 == 0x20) {
						local_24 = local_20;
						pTVar11->field_0x20 = pTVar11->field_0x20 + 1;
						unaff_f20 = pFVar12->spaceSize;
						fVar19 = fVar18;
						local_28 = pbVar13;
						local_8 = pFVar12;
					}
					else {
						if ((uVar10 == 10) || (uVar10 == 0)) {
							unaff_f20 = 0.0f;
							pTVar11->field_0x24 = true;
							bVar3 = true;
						}
						else {
							pFontPacked = pFVar12->pPackedFont;
							pSVar6 = pFontPacked->GetSymbol((uint)*pbVar8);
							if (pSVar6 == (FontSymbolData*)0x0) {
								unaff_f20 = 0.0f;
							}
							else {
								fVar15 = pFontPacked->GetRelativeAlignment((long)local_4, uVar10);
								uVar1 = pFontPacked->field_0x16;
								unaff_f20 = pSVar6->field_0x10 + fVar15;
								if (false) {
									fVar15 = (float)((uint)(uVar1 >> 1) | uVar1 & 1);
									fVar15 = fVar15 + fVar15;
								}
								else {
									fVar15 = (float)(uint)uVar1;
								}
								fVar15 = fVar14 + fVar15;
								if (pTVar11->field_0x1c < fVar15) {
									pTVar11->field_0x1c = fVar15;
								}
								fVar17 = pSVar6->field_0x18 + (fVar15 - pSVar6->field_0x14);
								fVar15 = fVar17 + pSVar6->field_0x14;
								if (fVar17 < pTVar11->field_0x14) {
									pTVar11->field_0x14 = fVar17;
								}
								if (pTVar11->field_0x18 < fVar15) {
									pTVar11->field_0x18 = fVar15;
								}
							}
							bVar2 = true;
							fVar15 = fVar16;
						}
					}
				}
			}
			if (bVar4) {
				if (((flags_0x28 & 0x80) != 0) && (fVar16 = pFVar12->GetHorizontalSize(), fVar16 <= fVar18 + unaff_f20)) {
					bVar5 = true;
					bVar3 = true;
				}
				if (!bVar3) {
					fVar18 = fVar18 + unaff_f20 + pFVar12->field_0x90;
				}
			}
			fVar16 = fVar15;
		} while (!bVar3);
		if (!bVar2) {
			pTVar11->field_0x14 = fVar14;
			pTVar11->field_0x18 = fVar14 + fVar15;
			pTVar11->field_0x1c = fVar14 + fVar15;
		}
		if (bVar5) {
			if (pTVar11->field_0x20 == 0) {
				pTVar11->field_0x10 = fVar18;
				local_20 = local_20 + -1;
				pTVar11->pTextEnd = (char*)local_20;
				pbVar9 = pbVar8;
				if (*pbVar8 == 0x20) {
					pbVar9 = pbVar8 + 1;
				}
			}
			else {
				pTVar11->field_0x10 = fVar19;
				pTVar11->pTextEnd = (char*)(local_24 + -1);
				pTVar11->field_0x20 = pTVar11->field_0x20 - 1;
				pFVar12 = local_8;
				pbVar9 = local_24 + 1;
				pbVar13 = local_28;
			}
			uVar10 = 0xffffffffffffffff;
		}
		else {
			pTVar11->field_0x10 = fVar18;
			pTVar11->pTextEnd = (char*)local_20;
		}
		if (field_0x8 < pTVar11->field_0x10) {
			field_0x8 = pTVar11->field_0x10;
		}
		uVar1 = pFontPacked->field_0x16;
		uVar7 = (uint)uVar1;
		if (false) {
			fVar16 = (float)((uint)(uVar1 >> 1) | uVar7 & 1);
			fVar16 = fVar16 + fVar16;
		}
		else {
			fVar16 = (float)uVar7;
		}
		if (0.0f < fVar15 - fVar16) {
			pTVar11->field_0x18 = pTVar11->field_0x14 + fVar15;
			pTVar11->field_0x1c = pTVar11->field_0x1c + (fVar15 - fVar16) / 2.0f;
		}
		else {
			if (false) {
				fVar15 = (float)((uint)(uVar1 >> 1) | uVar7 & 1);
				fVar15 = fVar15 + fVar15;
			}
			else {
				fVar15 = (float)uVar7;
			}
		}
		fVar16 = pFVar12->field_0x8c;
		pTVar11 = pTVar11 + 1;
		lineCount = lineCount + 1;
		fVar14 = fVar14 + fVar15 + fVar16;
	} while ((lineCount + 1 < 0x32) && (uVar10 != 0));
	fVar16 = 0.0f;
	uVar7 = flags_0x28 & 3;
	if ((uVar7 != 0) && (fVar16 = 0.0f - field_0x8, uVar7 == 2)) {
		fVar16 = fVar16 * 0.5;
	}
	offsetX_0x0 = fVar16;
	field_0xc = fVar14;
	fVar14 = 0.0f;
	uVar7 = flags_0x28 & 0xc;
	if ((uVar7 != 0) && (fVar14 = 0.0f - field_0xc, uVar7 == 8)) {
		fVar14 = fVar14 * 0.5;
	}
	offsetY_0x4 = fVar14;
	return;
}

bool edCTextFormat::FormatString(char* pText, va_list param_3)
{
	byte bVar1;
	int numberFormatBase;
	char* pcVar4;
	bool bVar9;
	char* pcVar10;
	int iVar11;
	char** pCurFormatValueA;
	float fVar12;
	float fVar13;
	astruct_10* piVar13;
	int* piVar14;
	float fVar15;
	float fVar16;
	uint uVar17;
	FormatSpec* pFormatSpecA;
	//double* param_3;
	int iVar25;
	byte* pbVar26;
	ulong uVar27;
	byte bVar33;
	edTextBitmap* pTextBitmap;
	char* pOutputIt;
	char* curFormatValue;
	edCTextStyle* pTextStyle;
	byte* pbVar36;
	char** pCurFormatValueB;
	char* pcVar38;
	FormatSpec* pFormatSpecB;
	uint unaff_s5_lo;
	astruct_10* paVar37;
	FormatSpec* pFormatSpecC;
	double* pdVar39;
	uint uVar40;
	char* pcVar41;
	char local_3b0;
	int local_3a0;
	ulong local_390;
	int local_380;
	uint local_370;
	int local_360;
	char aOutputA[128];
	FormatSpec aFormatSpecs[16];
	astruct_10 local_90[2];
	char* aFormatValues[19];
	char* local_4;
	char* pcVar3;

	uVar40 = 0;
	bVar9 = true;
	pTextStyle = fontData_0x850;
	local_360 = 0;
	pFormatSpecB = aFormatSpecs;
	local_370 = 0;
	local_380 = 0;
	do {
		while (true) {
			while (true) {
				if (bVar9) {
					pFormatSpecB->pText = pText;
					pFormatSpecB->switchMode = 0;
					pFormatSpecB->characterCount = 0;
					bVar9 = false;
					pFormatSpecB->field_0x14 = 0;
					pFormatSpecB->field_0x4 = 0;
					pFormatSpecB->field_0x15 = '\x01';
					pFormatSpecB->numberFormatBase = 10;
					pFormatSpecB->caseMode = 0;
				}
				if (*pText == 0) {
					uVar17 = 0;
					if (uVar40 != 0) {
						if (8 < uVar40) {
							piVar13 = local_90;
							do {
								piVar13->field_0x0 = 0;
								piVar13->field_0x4 = 0;
								uVar17 = uVar17 + 8;
								piVar13->field_0x8 = 0;
								piVar13->field_0xc = 0;
								piVar13->field_0x10 = 0;
								piVar13->field_0x14 = 0;
								piVar13->field_0x18 = 0;
								piVar13->field_0x1c = 0;
								piVar13 = piVar13 + 1;
							} while (uVar17 < uVar40 - 8);
						}
						if (uVar17 < uVar40) {
							piVar14 = &local_90[0].field_0x0 + uVar17;
							do {
								uVar17 = uVar17 + 1;
								*piVar14 = 0;
								piVar14 = piVar14 + 1;
							} while (uVar17 < uVar40);
						}
					}
					uVar17 = 0;
					if (local_370 != 0) {
						pFormatSpecA = aFormatSpecs;
						do {
							if (pFormatSpecA->field_0x15 != '\0') {
								(&local_90[0].field_0x0)[pFormatSpecA->field_0x18] = pFormatSpecA->switchMode;
							}
							uVar17 = uVar17 + 1;
							pFormatSpecA = pFormatSpecA + 1;
						} while (uVar17 < local_370);
					}
					uVar17 = 0;
					if (uVar40 != 0) {
						paVar37 = local_90;
						pCurFormatValueB = aFormatValues;
						do {
							//pdVar39 = param_3;
							if (true) {
								switch (paVar37->field_0x0) {
								case 0:
								case 1:
								case 2:
									IMPLEMENTATION_GUARD(
									pdVar39 = param_3 + 1;
									*pCurFormatValueB = *(char**)param_3;)
									break;
								case 3:
									*pCurFormatValueB = (char*)va_arg(param_3, uint);
									//pdVar39 = param_3 + 1;
									//*pCurFormatValueB = *(char**)param_3;
									break;
								case 4:
									IMPLEMENTATION_GUARD(
									pdVar39 = param_3 + 1;
									pcVar10 = (char*)dptofp(*param_3);
									*pCurFormatValueB = pcVar10;)
									break;
								case 5:
								case 7:
								case 8:
									IMPLEMENTATION_GUARD(
									pdVar39 = param_3 + 1;
									*pCurFormatValueB = *(char**)param_3;)
								}
							}
							uVar17 = uVar17 + 1;
							paVar37 = (astruct_10*)&paVar37->field_0x4;
							pCurFormatValueB = pCurFormatValueB + 1;
							//param_3 = pdVar39;
						} while (uVar17 < uVar40);
					}

					pcVar10 = (char*)edSystemFastRamGetAddr();
					pScratchpadA = pcVar10;
					pFormatSpecC = aFormatSpecs;
					uVar40 = 0;
					pScratchpadB = pScratchpadA + local_360;
					pcVar10 = pScratchpadA;
					pcVar38 = pScratchpadB;
					field_0x30 = 0;
					nbBitmaps = 0;
					pFormatSpecA = pFormatSpecC;
					do {
						pbVar36 = (byte*)pFormatSpecC->pText;
						for (iVar11 = pFormatSpecC->characterCount; iVar11 != 0; iVar11 = iVar11 + -1) {
							bVar33 = *pbVar36;
							pbVar26 = pbVar36 + 1;
							if ((bVar33 == 0x25) && (*pbVar26 == 0x25)) {
								pbVar26 = pbVar36 + 2;
								field_0x30 = field_0x30 + 1;
							}
							else {
								if ((false) || ((0x1f < bVar33 || (bVar33 == 10)))) {
									if (bVar33 != 0x20) {
										field_0x30 = field_0x30 + 1;
									}
								}
								else {
									bVar33 = 0x20;
								}
							}

							*pcVar38 = bVar33;
							pcVar38 = (char*)((byte*)pcVar38 + 1);
							pbVar36 = pbVar26;
						}

						if (pFormatSpecA->field_0x15 == '\0') {
							if (true) {
								switch (pFormatSpecA->switchMode) {
								case 5:
								case 7:
								case 8:
									local_4 = pFormatSpecC->field_0x10;
								}
							}

							pCurFormatValueA = &local_4;
						}
						else {
							pCurFormatValueA = aFormatValues + pFormatSpecC->field_0x18;
						}

						iVar11 = pFormatSpecC->switchMode;
						if (true) {
							switch (iVar11) {
							case 2:
							case 3:
							case 4:
								iVar25 = pFormatSpecC->field_0x4;
								numberFormatBase = pFormatSpecC->numberFormatBase;
								bVar33 = pFormatSpecC->field_0x14;

								char* pcVar34;
								if (pFormatSpecC->caseMode == 1) {
									pcVar34 = s_0123456789abcdef_00432380;
								}
								else {
									pcVar34 = s_0123456789ABCDEF_004323a0;
								}

								if (iVar11 == 4) {
									IMPLEMENTATION_GUARD(
									pcVar41 = *pCurFormatValueA;
									local_3b0 = (float)pcVar41 < 0.0f;
									if ((bool)local_3b0) {
										pcVar41 = (char*)-(float)pcVar41;
									}
									if ((float)pcVar41 < 2.147484e+09f) {
										curFormatValue = (char*)(int)(float)pcVar41;
									}
									else {
										curFormatValue = (char*)((int)((float)pcVar41 - 2.147484e+09f) | 0x80000000);
									}

									if ((int)curFormatValue < 0) {
										fVar12 = (float)((uint)curFormatValue >> 1 | (uint)curFormatValue & 1);
										fVar12 = fVar12 + fVar12;
									}
									else {
										fVar12 = (float)(int)curFormatValue;
									}

									fVar12 = ((float)pcVar41 - fVar12) * 100000.0f;
									if (fVar12 < 2.147484e+09f) {
										unaff_s5_lo = (uint)fVar12;
									}
									else {
										unaff_s5_lo = (int)(fVar12 - 2.147484e+09f) | 0x80000000;
									})
								}
								else {
									if (iVar11 == 2) {
										IMPLEMENTATION_GUARD(
										curFormatValue = *pCurFormatValueA;
										local_3b0 = (int)curFormatValue < 0;
										if ((bool)local_3b0) {
											curFormatValue = (char*)-(int)curFormatValue;
										}
										unaff_s5_lo = 0;)
									}
									else {
										if (iVar11 == 3) {
											curFormatValue = *pCurFormatValueA;
											unaff_s5_lo = 0;
											local_3b0 = '\0';
										}
									}
								}

								pOutputIt = aOutputA;
								if (iVar25 == 0) {
									iVar25 = 1;
								}

								if (local_3b0 != '\0') {
									*pcVar38 = 0x2d;
									pcVar38 = (char*)((byte*)pcVar38 + 1);
									field_0x30 = field_0x30 + 1;
								}

								if (unaff_s5_lo != 0) {
									uVar17 = 100000;
									while (unaff_s5_lo != 0) {
										uVar17 = uVar17 / 10;
										if (numberFormatBase == 0) {
											trap(7);
										}
										iVar25 = iVar25 + -1;
										*pOutputIt = pcVar34[unaff_s5_lo - ((int)unaff_s5_lo / numberFormatBase) * numberFormatBase];
										pOutputIt = pOutputIt + 1;
										field_0x30 = field_0x30 + 1;
										unaff_s5_lo = (int)unaff_s5_lo / numberFormatBase;
									}

									for (; 1 < uVar17; uVar17 = uVar17 / 10) {
										*pOutputIt = '0';
										pOutputIt = pOutputIt + 1;
										field_0x30 = field_0x30 + 1;
									}

									*pOutputIt = '.';
									pOutputIt = pOutputIt + 1;
									field_0x30 = field_0x30 + 1;
								}

								if (curFormatValue == (char*)0x0) {
									iVar25 = iVar25 + -1;
									*pOutputIt = 0x30;
									pOutputIt = pOutputIt  + 1;
									field_0x30 = field_0x30 + 1;
								}
								else {
									while (curFormatValue != (char*)0x0) {
										if (numberFormatBase == 0) {
											trap(7);
										}
										iVar25 = iVar25 + -1;
										*pOutputIt = curFormatValue[(ulong)(pcVar34 + -((int)(char*)((int)curFormatValue / numberFormatBase) * numberFormatBase))];
										pOutputIt = pOutputIt  + 1;
										field_0x30 = field_0x30 + 1;
										curFormatValue = (char*)((int)curFormatValue / numberFormatBase);
									}
								}

								for (; 0 < iVar25; iVar25 = iVar25 + -1) {
									if (bVar33 == 0) {
										*pOutputIt = 0x20;
									}
									else {
										*pOutputIt = 0x30;
										field_0x30 = field_0x30 + 1;
									}

									pOutputIt = pOutputIt  + 1;
								}

								while (pOutputIt != aOutputA) {
									pOutputIt = pOutputIt - 1;
									*pcVar38 = *pOutputIt;
									pcVar38 = pcVar38 + 1;
								}
								break;
							case 5:
								IMPLEMENTATION_GUARD();
#if 0
								pbVar36 = (byte*)*ppcVar10;
								bVar33 = *pbVar36;
								while (bVar33 != 0) {
									pbVar36 = pbVar36 + 1;
									if (bVar33 != 0x20) {
										field_0x30 = field_0x30 + 1;
									}
									*pcVar38 = bVar33;
									pcVar38 = (char*)((byte*)pcVar38 + 1);
									bVar33 = *pbVar36;
								}
#endif
								break;
							case 7:
								pTextBitmap = reinterpret_cast<edTextBitmap*>(*pCurFormatValueA);
								*pcVar38 = 1;
								pcVar38 = (char*)((byte*)pcVar38 + 1);
								*pcVar10 = (char)this->nbBitmaps;
								pcVar10 = pcVar10 + 1;
								this->aBitmaps[nbBitmaps] = pTextBitmap;
								this->nbBitmaps = this->nbBitmaps + 1;
								break;
							case 8:
								IMPLEMENTATION_GUARD();
#if 0
								pcVar4 = (code*)*ppcVar10;
								*pcVar38 = 2;
								pcVar38 = (char*)((byte*)pcVar38 + 1);
								if (paVar38->field_0x20 == 1) {
									pFVar35[1] = pFVar35[0];
								}
								else {
									pFVar35[1] = fontData_0x850[0];
								}

								pFVar35 = pFVar35 + 1;
								if (pcVar4 != (code*)0x0) {
									(*pcVar4)();
								}
#endif
							}
						}
						uVar40 = uVar40 + 1;
						pFormatSpecC = pFormatSpecC + 1;
						pFormatSpecA = pFormatSpecA + 1;
					} while (uVar40 <= local_370);
					*pcVar38 = 0;
					if ((field_0x30 != 0) || (bVar9 = false, nbBitmaps != 0)) {
						bVar9 = true;
					}
					return bVar9;
				}
				if (*pText == 0x25) break;
				pFormatSpecB->characterCount = pFormatSpecB->characterCount + 1;
				pText = (char*)((byte*)pText + 1);
			}
			uVar17 = (uint)((byte*)pText)[1];
			pbVar36 = (byte*)pText + 2;
			if (uVar17 != 0x25) break;
			pFormatSpecB->characterCount = pFormatSpecB->characterCount + 1;
			pText = (char*)pbVar36;
		}

		iVar11 = local_380;
		if (uVar17 == 0x5b) {
			uVar27 = (ulong)*pbVar36;
			iVar11 = 0;
			pbVar36 = (byte*)pText + 3;
			if ((uVar27 < 0x30) || (0x39 < uVar27)) {
				iVar11 = 0;
				local_390 = 0;
				while (uVar27 != 0x5d) {
					iVar11 = iVar11 + 1;
					local_390 = local_390 * 0x100 + uVar27;
					if (8 < iVar11) break;
					bVar33 = *pbVar36;
					pbVar36 = pbVar36 + 1;
					uVar27 = (ulong)bVar33;
				}

				uVar17 = (uint)*pbVar36;
				pFormatSpecB->field_0x15 = '\0';
				pbVar36 = pbVar36 + 1;
				iVar11 = 0;
			}
			else {
				while (((uVar27 != 0x5d && (0x2f < uVar27)) && (uVar27 < 0x3a))) {
					bVar33 = *pbVar36;
					iVar11 = (int)uVar27 + -0x30 + iVar11 * 10;
					pbVar36 = pbVar36 + 1;
					uVar27 = (ulong)bVar33;
				}

				uVar17 = (uint)*pbVar36;
				pbVar36 = pbVar36 + 1;
			}
		}

		pText = (char*)pbVar36;
		if ((0x2f < uVar17) && (uVar17 < 0x3a)) {
			bVar33 = 0;
			iVar25 = 0;
			if (uVar17 == 0x30) {
				uVar17 = (uint)(byte)*pText;
				bVar33 = 1;
				pText = (char*)((byte*)pText + 1);
			}

			while ((0x2f < uVar17 && (uVar17 < 0x3a))) {
				bVar1 = *pText;
				iVar25 = (uVar17 - 0x30) + iVar25 * 10;
				pText = (char*)((byte*)pText + 1);
				uVar17 = (uint)bVar1;
			}

			pFormatSpecB->field_0x14 = bVar33;
			pFormatSpecB->field_0x4 = iVar25;
		}

		int switchMode;

		if (uVar17 == 0x4b) {
			switchMode = 8;
			pFormatSpecB->caseMode = 2;
		}
		else {
			if (uVar17 == 0x6b) {
				switchMode = 8;
				pFormatSpecB->caseMode = 1;
			}
			else {
				if ((uVar17 == 0x43) || (uVar17 == 99)) {
					switchMode = 6;
				}
				else {
					if (uVar17 == 0x73) {
						switchMode = 5;
					}
					else {
						if (uVar17 == 0x66) {
							switchMode = 4;
						}
						else {
							if (uVar17 == 0x58) {
								pFormatSpecB->numberFormatBase = 0x10;
								switchMode = 3;
								pFormatSpecB->caseMode = 2;
							}
							else {
								if (uVar17 == 0x78) {
									pFormatSpecB->numberFormatBase = 0x10;
									switchMode = 3;
									pFormatSpecB->caseMode = 1;
								}
								else {
									if (uVar17 == 0x6f) {
										switchMode = 2;
										pFormatSpecB->numberFormatBase = 8;
									}
									else {
										if (uVar17 == 0x75) {
											switchMode = 3;
										}
										else {
											if ((uVar17 == 0x69) || (uVar17 == 100)) {
												switchMode = 2;
											}
											else {
												if (uVar17 == 0x6d) {
													switchMode = 3;
													pFormatSpecB->numberFormatBase = 2;
												}
												else {
													if (uVar17 == 0x62) {
														switchMode = 7;
													}
													else {
														switchMode = 0;
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		pFormatSpecB->field_0x18 = iVar11;
		if (pFormatSpecB->field_0x15 == '\0') {
			if (switchMode == 8) {
				local_3a0 = 3;
			}
			else {
				if (switchMode == 7) {
					local_3a0 = 2;
					local_360 = local_360 + 1;
				}
				else {
					if (switchMode == 5) {
						local_3a0 = 1;
					}
					else {
						local_3a0 = 0;
					}
				}
			}

			pcVar10 = (char*)edTextResources.GetResourcePtr(local_390, local_3a0);
			if ((pcVar10 == (char*)0x0) && (local_3a0 != 3)) {
				switchMode = 5;
				pFormatSpecB->field_0x10 = "(???)";
			}
			else {
				pFormatSpecB->field_0x10 = pcVar10;
			}
		}
		else {
			if (uVar40 < iVar11 + 1U) {
				uVar40 = iVar11 + 1U;
			}

			local_380 = local_380 + 1;
		}

		pFormatSpecB->switchMode = switchMode;
		pFormatSpecB = pFormatSpecB + 1;
		bVar9 = true;
		local_370 = local_370 + 1;
	} while (true);
}

bool edCTextFormat::FormatString(char* pText, ...)
{
	edCTextStyle* pFVar3;
	bool bVar4;
	int iVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	float fVar15;
	float fVar16;
	va_list args;

	pFVar3 = pedTextCurrentStyle;

	edF32Matrix4CopyHard(&fontData_0x850[0].m0, &pedTextCurrentStyle->m0);
	edF32Matrix4CopyHard(&fontData_0x850[0].m1, &pedTextCurrentStyle->m1);

	fontData_0x850[0].pFunction = pFVar3->pFunction;
	fontData_0x850[0].flags_0x84 = pFVar3->flags_0x84;
	fontData_0x850[0].pPackedFont = pFVar3->pPackedFont;
	fontData_0x850[0].field_0x8c = pFVar3->field_0x8c;
	fontData_0x850[0].field_0x90 = pFVar3->field_0x90;
	fontData_0x850[0].spaceSize = pFVar3->spaceSize;
	fontData_0x850[0].rotation = pFVar3->rotation;
	fontData_0x850[0].xScale = pFVar3->xScale;
	fontData_0x850[0].yScale = pFVar3->yScale;
	fontData_0x850[0].horizontalSize = pFVar3->horizontalSize;
	fontData_0x850[0].verticalSize = pFVar3->verticalSize;
	fontData_0x850[0].shadowShiftX = pFVar3->shadowShiftX;
	fontData_0x850[0].shadowShiftY = pFVar3->shadowShiftY;
	fontData_0x850[0].rgbaColour = pFVar3->rgbaColour;
	fontData_0x850[0].alpha = pFVar3->alpha;
	fontData_0x850[0].altColour = pFVar3->altColour;
	flags_0x28 = pFVar3->flags_0x84;
	if (true) {
		iVar5 = 0x30;
	}
	else {
		iVar5 = 0;
	}

	va_start(args, pText);
	bVar4 = FormatString(pText, args);
	va_end(args);
	if (bVar4 != false) {
		GetRect();
	}
	return bVar4;
}

edCTextFormat::edCTextFormat()
{
	edCTextStyle* pCurStyle;

	pCurStyle = pedTextCurrentStyle;
	fontData_0x850[0].m0 = pedTextCurrentStyle->m0;
	fontData_0x850[0].m1 = pedTextCurrentStyle->m1;

	fontData_0x850[0].pFunction = pCurStyle->pFunction;
	fontData_0x850[0].flags_0x84 = pCurStyle->flags_0x84;
	fontData_0x850[0].pPackedFont = pCurStyle->pPackedFont;
	fontData_0x850[0].field_0x8c = pCurStyle->field_0x8c;
	fontData_0x850[0].field_0x90 = pCurStyle->field_0x90;
	fontData_0x850[0].spaceSize = pCurStyle->spaceSize;
	fontData_0x850[0].rotation = pCurStyle->rotation;
	fontData_0x850[0].xScale = pCurStyle->xScale;
	fontData_0x850[0].yScale = pCurStyle->yScale;
	fontData_0x850[0].horizontalSize = pCurStyle->horizontalSize;
	fontData_0x850[0].verticalSize = pCurStyle->verticalSize;
	fontData_0x850[0].shadowShiftX = pCurStyle->shadowShiftX;
	fontData_0x850[0].shadowShiftY = pCurStyle->shadowShiftY;
	fontData_0x850[0].rgbaColour = pCurStyle->rgbaColour;
	fontData_0x850[0].alpha = pCurStyle->alpha;
	fontData_0x850[0].altColour = pCurStyle->altColour;

	flags_0x28 = pCurStyle->flags_0x84;
	return;
}

edCTextFormat::edCTextFormat(edCTextStyle* pStyle)
{
	fontData_0x850[0].m0 = pedTextCurrentStyle->m0;
	fontData_0x850[0].m1 = pedTextCurrentStyle->m1;

	fontData_0x850[0].pFunction = pStyle->pFunction;
	fontData_0x850[0].flags_0x84 = pStyle->flags_0x84;
	fontData_0x850[0].pPackedFont = pStyle->pPackedFont;
	fontData_0x850[0].field_0x8c = pStyle->field_0x8c;
	fontData_0x850[0].field_0x90 = pStyle->field_0x90;
	fontData_0x850[0].spaceSize = pStyle->spaceSize;
	fontData_0x850[0].rotation = pStyle->rotation;
	fontData_0x850[0].xScale = pStyle->xScale;
	fontData_0x850[0].yScale = pStyle->yScale;
	fontData_0x850[0].horizontalSize = pStyle->horizontalSize;
	fontData_0x850[0].verticalSize = pStyle->verticalSize;
	fontData_0x850[0].shadowShiftX = pStyle->shadowShiftX;
	fontData_0x850[0].shadowShiftY = pStyle->shadowShiftY;
	fontData_0x850[0].rgbaColour = pStyle->rgbaColour;
	fontData_0x850[0].alpha = pStyle->alpha;
	fontData_0x850[0].altColour = pStyle->altColour;

	flags_0x28 = pStyle->flags_0x84;
}
