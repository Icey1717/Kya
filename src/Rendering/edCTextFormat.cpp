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
#include "../MathOps.h"



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


void edCTextFormat::DisplayWindow(float x, float y, edF32VECTOR4* param_4)
{
	edF32MATRIX4 textMatrix;
	float shadowShiftY;
	float shadowShiftX;

	if ((((this->field_0x30 != 0) || (this->field_0x34 != 0)) && (param_4->z != 0.0f)) && (param_4->w != 0.0f)) {
		(this->vector_0x10).x = param_4->x + this->offsetX_0x0;
		(this->vector_0x10).y = param_4->y + this->offsetY_0x4;
		(this->vector_0x10).z = param_4->z;
		(this->vector_0x10).w = param_4->w;
		this->fontData_0x850->TransformMatrix(x, y, &textMatrix);
		edDListLoadMatrix(&textMatrix);
		if ((this->flags_0x28 & 0x100) != 0) {
			this->fontData_0x850->GetShadowShift(&shadowShiftX, &shadowShiftY);
		}
		if ((this->flags_0x28 & 0x100) != 0) {
			DrawString(shadowShiftX + this->offsetX_0x0, shadowShiftY + this->offsetY_0x4, false);
		}
		DrawString(this->offsetX_0x0, this->offsetY_0x4, true);
	}
	return;
}

void edCTextFormat::Display(float x, float y)
{
	edF32VECTOR4 local_10;

	local_10.z = 640.0f;
	local_10.x = 0.0f;
	local_10.w = 512.0f;
	local_10.y = 0.0f;
	DisplayWindow(x, y, &local_10);
	return;
}

void edCTextFormat::DisplayDebugInfos(float x, float y)
{
	int iVar1;
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
	iVar1 = 4;
	edDListBegin(1.0f, 1.0f, 1.0f, 4, 4);
	edDListVertex4f(fVar5, fVar4, 0.0f, iVar1);
	edDListVertex4f(fVar6, fVar4, 0.0f, iVar1);
	edDListVertex4f(fVar5, fVar7, 0.0f, iVar1);
	edDListVertex4f(fVar6, fVar7, 0.0f, iVar1);
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
			iVar1 = 4;
			edDListBegin(1.0f, 1.0f, 1.0f, 4, 4);
			edDListVertex4f(fVar6, fVar7, 0.0f, iVar1);
			edDListVertex4f(fVar4, fVar7, 0.0f, iVar1);
			edDListVertex4f(fVar6, fVar5, 0.0f, iVar1);
			edDListVertex4f(fVar4, fVar5, 0.0f, iVar1);
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
	byte bVar1;
	ushort uVar2;
	edDList_material* pMVar3;
	edCTextFont* pFontPacked;
	bool bVar4;
	bool bVar5;
	void* pvVar6;
	Segment_1C_Packed* ppcVar7;
	CharacterData* puVar7;
	uint uVar7;
	char* pcVar8;
	edCTextStyle* pFVar9;
	ulong uVar10;
	TextLine* ppbVar13;
	byte* pbVar11;
	uint unaff_s7_lo;
	byte* pbVar12;
	float fVar13;
	float fVar14;
	float fVar15;
	float fVar16;
	float fVar17;
	float fVar18;
	float fVar19;
	float in_f21;
	float in_f23;
	float unaff_f22;
	float in_f25;
	float unaff_f24;
	float in_f27;
	float unaff_f26;
	float fVar20;
	float unaff_f28;
	float fVar21;
	float local_68;
	int local_44;
	uint local_30;
	DrawText16 local_10;

	pvVar6 = edSystemFastRamGetAddr();
	local_10.pCharacterData = (CharacterData*)((char*)pvVar6 + 0x800);
	local_10.characterCount = 0;
	local_10.pMaterialInfoA = (edDList_material*)0x0;
	local_10.pMaterialInfoB = (edDList_material*)0x0;
	local_30 = 0;
	ppbVar13 = this->aTextLines;
	if (this->lineCount != 0) {
		do {
			fVar21 = 0.0f;
			local_68 = ppbVar13->pFontFileData->field_0x90;
			fVar20 = x;
			if (ppbVar13->field_0x24 == '\0') {
				uVar7 = this->flags_0x28;
				if (((uVar7 & 0x80) == 0) || ((uVar7 & 0x10) == 0)) {
					if ((uVar7 & 2) == 0) {
						if ((uVar7 & 1) != 0) {
							fVar20 = x + (this->field_0x8 - ppbVar13->field_0x10);
						}
					}
					else {
						fVar20 = x + (this->field_0x8 - ppbVar13->field_0x10) / 2.0f;
					}
				}
				else {
					pcVar8 = ppbVar13->pTextEnd + (-(uint)ppbVar13->field_0x22 - (int)ppbVar13->pTextStart);
					fVar13 = ppbVar13->pFontFileData->GetHorizontalSize();
					uVar2 = ppbVar13->field_0x20;
					uVar7 = (uint)uVar2;
					fVar13 = fVar13 - ppbVar13->field_0x10;
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
								local_68 = local_68 + fVar13 / fVar14;
							}
						}
						else {
							if (false) {
								fVar21 = (float)((uint)(uVar2 >> 1) | uVar7 & 1);
								fVar21 = fVar21 + fVar21;
							}
							else {
								fVar21 = (float)uVar7;
							}
							fVar21 = fVar13 / fVar21 + 0.0f;
						}
					}
					else {
						if (false) {
							fVar21 = (float)((uint)(uVar2 >> 1) | uVar7 & 1);
							fVar21 = fVar21 + fVar21;
						}
						else {
							fVar21 = (float)uVar7;
						}
						fVar21 = (fVar13 / fVar21) / 2.0f + 0.0f;
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
						local_68 = local_68 + (fVar13 - fVar21 * fVar14) / fVar15;
					}
				}
			}
			else {
				if ((this->flags_0x28 & 2) == 0) {
					if ((this->flags_0x28 & 1) != 0) {
						fVar20 = x + (this->field_0x8 - ppbVar13->field_0x10);
					}
				}
				else {
					fVar20 = x + (this->field_0x8 - ppbVar13->field_0x10) / 2.0f;
				}
			}
			bVar4 = true;
			pbVar11 = (byte*)ppbVar13->pTextStart;
			pFVar9 = ppbVar13->pFontFileData;
			pbVar12 = (byte*)ppbVar13->pText;
			uVar10 = 0xffffffffffffffff;
			if (pbVar11 <= (byte*)ppbVar13->pTextEnd) {
				do {
					if (bVar4) {
						local_44 = (int)uVar10;
					}
					else {
						bVar4 = true;
					}
					bVar1 = *pbVar11;
					uVar10 = (ulong)bVar1;
					fVar13 = 0.0f;
					bVar5 = false;
					pbVar11 = pbVar11 + 1;
					if (uVar10 == 2) {
						pFVar9 = pFVar9 + 1;
						bVar4 = false;
					}
					else {
						if (uVar10 == 1) {
							bVar5 = true;
							bVar1 = *pbVar12;
							pbVar12 = pbVar12 + 1;
							pMVar3 = (edDList_material*)this->field_0x38[bVar1];
							local_10.pMaterialInfoA = (edDList_material*)pMVar3->pManager;
							fVar13 = (float)pMVar3->index;
							unaff_f28 = *(float*)((int)(pMVar3 + 1) + 4);
							in_f27 = *(float*)((int)(pMVar3 + 1) + 8);
							unaff_f26 = *(float*)((int)(pMVar3 + 1) + 0xc);
							in_f25 = *(float*)(pMVar3 + 2);
							in_f23 = y + (ppbVar13->field_0x14 + ppbVar13->field_0x18) / 2.0f + (float)pMVar3->mode * (*(float*)(pMVar3 + 1) - 1.0f);
							in_f21 = in_f23 + *(float*)(pMVar3 + 1);
							unaff_f22 = fVar20 + fVar13;
							unaff_f24 = fVar20;
							if (bFlag == false) {
								unaff_s7_lo = pFVar9->alpha;
							}
							else {
								unaff_s7_lo = pFVar9->altColour;
							}
						}
						else {
							if (uVar10 == 0x20) {
								fVar13 = fVar21 + pFVar9->spaceSize;
							}
							else {
								if ((uVar10 != 10) && (uVar10 != 0)) {
									pFontPacked = pFVar9->pPackedFont;
									ppcVar7 = pFontPacked->GetSymbol((uint)bVar1);
									if (ppcVar7 != (Segment_1C_Packed*)0x0) {
										local_10.pMaterialInfoA = &(RESOLVE_FONT_SUB_DATA(pFontPacked->pSubData))->materialInfo;
										if (bFlag == false) {
											unaff_s7_lo = pFVar9->alpha;
										}
										else {
											unaff_s7_lo = pFVar9->rgbaColour;
										}
										fVar14 = ppcVar7->field_0x10;
										fVar13 = pFontPacked->GetRelativeAlignment((long)local_44, uVar10);
										fVar13 = fVar14 + fVar13;
										unaff_f22 = fVar20 + fVar14;
										bVar5 = true;
										unaff_f28 = ppcVar7->field_0x0;
										in_f27 = ppcVar7->field_0x4;
										unaff_f26 = ppcVar7->field_0x8;
										in_f25 = ppcVar7->field_0xc;
										in_f23 = ppcVar7->field_0x18 + ((y + ppbVar13->field_0x1c) - ppcVar7->field_0x14);
										in_f21 = in_f23 + ppcVar7->field_0x14;
										unaff_f24 = fVar20;
									}
								}
							}
						}
					}
					if (bVar4) {
						if (bVar5) {
							fVar14 = (this->vector_0x10).x;
							fVar15 = (this->vector_0x10).z;
							fVar18 = fVar14 + fVar15;
							if ((unaff_f24 <= fVar18) && (fVar14 <= unaff_f22)) {
								fVar16 = (this->vector_0x10).y;
								fVar17 = (this->vector_0x10).w;
								fVar19 = fVar16 + fVar17;
								if ((in_f23 <= fVar19) && (fVar16 <= in_f21)) {
									if (unaff_f24 < fVar14) {
										unaff_f28 = unaff_f26 - ((unaff_f26 - unaff_f28) * (unaff_f22 - fVar14)) / (unaff_f22 - unaff_f24);
										unaff_f24 = fVar14;
									}
									if (fVar18 < unaff_f22) {
										fVar14 = fVar14 + fVar15;
										unaff_f26 = unaff_f28 + ((unaff_f26 - unaff_f28) * (fVar14 - unaff_f24)) / (unaff_f22 - unaff_f24);
										unaff_f22 = fVar14;
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
									if ((local_10.characterCount != 0) && (local_10.pMaterialInfoB != local_10.pMaterialInfoA)) {
										SendTextRenderCommands_0028b0e0(&local_10);
									}
									local_10.pMaterialInfoB = local_10.pMaterialInfoA;
									puVar7 = local_10.pCharacterData + local_10.characterCount;
									puVar7->colour = unaff_s7_lo;
									puVar7->pos_x1 = unaff_f24;
									puVar7->pos_y1 = in_f23;
									puVar7->pos_x2 = unaff_f22;
									puVar7->pos_y2 = in_f21;
									puVar7->tex_x1 = unaff_f28;
									puVar7->tex_y1 = in_f27;
									puVar7->tex_x2 = unaff_f26;
									puVar7->tex_y2 = in_f25;
									local_10.characterCount = local_10.characterCount + 1;
								}
							}
						}
						if (fVar13 != 0.0f) {
							fVar20 = fVar20 + fVar13 + local_68;
						}
					}
				} while (pbVar11 <= (byte*)ppbVar13->pTextEnd);
			}
			local_30 = local_30 + 1;
			ppbVar13 = ppbVar13 + 1;
		} while (local_30 < this->lineCount);
	}
	SendTextRenderCommands_0028b0e0(&local_10);
	return;
}

void edCTextFormat::SendTextRenderCommands_0028b0e0(DrawText16* pTextRenderCommands)
{
	int iVar1;
	uint uVar2;
	uint uVar3;
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
			uVar3 = (uVar2 >> 0x18) + 1 >> 1;
			edDListColor4u8
			((byte)uVar3, (byte)((uVar2 >> 0x10 & 0xff) + 1 >> 1), (byte)((uVar2 >> 8 & 0xff) + 1 >> 1),
				(byte)((uVar2 & 0xff) + 1 >> 1));
			edDListTexCoo2f(pCharacterData->tex_x1, pCharacterData->tex_y1);
			edDListVertex4f(pCharacterData->pos_x1, pCharacterData->pos_y1, 0.0f, uVar3);
			edDListTexCoo2f(pCharacterData->tex_x1, pCharacterData->tex_y2);
			edDListVertex4f(pCharacterData->pos_x1, pCharacterData->pos_y2, 0.0f, uVar3);
			edDListTexCoo2f(pCharacterData->tex_x2, pCharacterData->tex_y1);
			edDListVertex4f(pCharacterData->pos_x2, pCharacterData->pos_y1, 0.0f, uVar3);
			edDListTexCoo2f(pCharacterData->tex_x2, pCharacterData->tex_y2);
			edDListVertex4f(pCharacterData->pos_x2, pCharacterData->pos_y2, 0.0f, uVar3);
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
	Segment_1C_Packed* pSVar6;
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
		pTVar11->pFontFileData = pFVar12;
		pTVar11->pText = (char*)pbVar13;
		pTVar11->field_0x10 = 0.0f;
		pTVar11->field_0x20 = 0;
		pTVar11->field_0x22 = 0;
		bVar3 = false;
		bVar5 = false;
		bVar2 = false;
		pTVar11->field_0x24 = '\0';
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
					IMPLEMENTATION_GUARD();
#if 0
					fVar15 = *(float*)(field_0x38[*pbVar13] + 0x10);
					unaff_f20 = *(float*)(field_0x38[*pbVar13] + 0xc);
#endif
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
							pTVar11->field_0x24 = '\x01';
							bVar3 = true;
						}
						else {
							pFontPacked = pFVar12->pPackedFont;
							pSVar6 = pFontPacked->GetSymbol((uint)*pbVar8);
							if (pSVar6 == (Segment_1C_Packed*)0x0) {
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

bool edCTextFormat::FormatString(char* pText, char* param_3)
{
	byte bVar1;
	int iVar2;
	char* pcVar4;
	undefined8 uVar5;
	undefined8 uVar6;
	undefined8 uVar7;
	undefined8 uVar8;
	bool bVar9;
	char* pcVar10;
	int iVar11;
	char** ppcVar10;
	float fVar12;
	float fVar13;
	astruct_10* piVar13;
	int* piVar14;
	float fVar15;
	float fVar16;
	uint uVar17;
	astruct_5* paVar18;
	float fVar19;
	float fVar20;
	float fVar21;
	float fVar22;
	float fVar23;
	float fVar24;
	//double* param_3;
	int iVar25;
	byte* pbVar26;
	ulong uVar27;
	float fVar28;
	float fVar29;
	float fVar30;
	float fVar31;
	float fVar32;
	byte bVar33;
	char* pcVar34;
	char* pcVar19;
	char* unaff_s0_lo;
	edCTextStyle* pFVar35;
	byte* pbVar36;
	char** ppcVar37;
	char* pcVar38;
	astruct_5* piVar38;
	uint unaff_s5_lo;
	astruct_10* paVar37;
	astruct_5* paVar38;
	double* pdVar39;
	uint uVar40;
	char* pcVar41;
	char local_3b0;
	int local_3a0;
	long local_390;
	int local_380;
	uint local_370;
	int local_360;
	byte local_350[128];
	astruct_5 local_2d0[16];
	astruct_10 local_90[2];
	char* local_50[19];
	char* local_4;
	char* pcVar3;

	uVar40 = 0;
	bVar9 = true;
	pFVar35 = fontData_0x850;
	local_360 = 0;
	piVar38 = local_2d0;
	local_370 = 0;
	local_380 = 0;
	do {
		while (true) {
			while (true) {
				if (bVar9) {
					piVar38->pText = pText;
					piVar38->switchMode = 0;
					piVar38->characterCount = 0;
					bVar9 = false;
					piVar38->field_0x14 = 0;
					piVar38->field_0x4 = 0;
					piVar38->field_0x15 = '\x01';
					piVar38->field_0x1c = 10;
					piVar38->field_0x20 = 0;
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
						paVar18 = local_2d0;
						do {
							if (paVar18->field_0x15 != '\0') {
								(&local_90[0].field_0x0)[paVar18->field_0x18] = paVar18->switchMode;
							}
							uVar17 = uVar17 + 1;
							paVar18 = paVar18 + 1;
						} while (uVar17 < local_370);
					}
					uVar17 = 0;
					if (uVar40 != 0) {
						IMPLEMENTATION_GUARD();
						paVar37 = local_90;
						ppcVar37 = local_50;
						//do {
						//	pdVar39 = param_3;
						//	if (true) {
						//		switch (paVar37->field_0x0) {
						//		case 0:
						//		case 1:
						//		case 2:
						//			pdVar39 = param_3 + 1;
						//			*ppcVar37 = *(char**)param_3;
						//			break;
						//		case 3:
						//			pdVar39 = param_3 + 1;
						//			*ppcVar37 = *(char**)param_3;
						//			break;
						//		case 4:
						//			pdVar39 = param_3 + 1;
						//			pcVar10 = (char*)dptofp(*param_3);
						//			*ppcVar37 = pcVar10;
						//			break;
						//		case 5:
						//		case 7:
						//		case 8:
						//			pdVar39 = param_3 + 1;
						//			*ppcVar37 = *(char**)param_3;
						//		}
						//	}
						//	uVar17 = uVar17 + 1;
						//	paVar37 = (astruct_10*)&paVar37->field_0x4;
						//	ppcVar37 = ppcVar37 + 1;
						//	param_3 = pdVar39;
						//} while (uVar17 < uVar40);
					}
					pcVar10 = (char*)edSystemFastRamGetAddr();
					pScratchpadA = pcVar10;
					paVar38 = local_2d0;
					uVar40 = 0;
					pScratchpadB = pScratchpadA + local_360;
					pcVar10 = pScratchpadA;
					pcVar38 = pScratchpadB;
					field_0x30 = 0;
					field_0x34 = 0;
					paVar18 = paVar38;
					do {
						pbVar36 = (byte*)paVar38->pText;
						for (iVar11 = paVar38->characterCount; iVar11 != 0; iVar11 = iVar11 + -1) {
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
						if (paVar18->field_0x15 == '\0') {
							if (true) {
								switch (paVar18->switchMode) {
								case 5:
								case 7:
								case 8:
									local_4 = paVar38->field_0x10;
								}
							}
							ppcVar10 = &local_4;
						}
						else {
							ppcVar10 = local_50 + paVar38->field_0x18;
						}
						iVar11 = paVar38->switchMode;
						if (true) {
							switch (iVar11) {
							case 2:
							case 3:
							case 4:
								IMPLEMENTATION_GUARD();
#if 0
								iVar25 = paVar38->field_0x4;
								iVar2 = paVar38->field_0x1c;
								bVar33 = paVar38->field_0x14;
								if (paVar38->field_0x20 == 1) {
									pcVar34 = s_0123456789abcdef_00432380;
								}
								else {
									pcVar34 = s_0123456789ABCDEF_004323a0;
								}
								if (iVar11 == 4) {
									pcVar41 = *ppcVar10;
									local_3b0 = (float)pcVar41 < 0.0f;
									if ((bool)local_3b0) {
										pcVar41 = (char*)-(float)pcVar41;
									}
									if ((float)pcVar41 < 2.147484e+09f) {
										unaff_s0_lo = (char*)(int)(float)pcVar41;
									}
									else {
										unaff_s0_lo = (char*)((int)((float)pcVar41 - 2.147484e+09f) | 0x80000000);
									}
									if ((int)unaff_s0_lo < 0) {
										fVar12 = (float)((uint)unaff_s0_lo >> 1 | (uint)unaff_s0_lo & 1);
										fVar12 = fVar12 + fVar12;
									}
									else {
										fVar12 = (float)(int)unaff_s0_lo;
									}
									fVar12 = ((float)pcVar41 - fVar12) * 100000.0f;
									if (fVar12 < 2.147484e+09f) {
										unaff_s5_lo = (uint)fVar12;
									}
									else {
										unaff_s5_lo = (int)(fVar12 - 2.147484e+09f) | 0x80000000;
									}
								}
								else {
									if (iVar11 == 2) {
										unaff_s0_lo = *ppcVar10;
										local_3b0 = (int)unaff_s0_lo < 0;
										if ((bool)local_3b0) {
											unaff_s0_lo = (char*)-(int)unaff_s0_lo;
										}
										unaff_s5_lo = 0;
									}
									else {
										if (iVar11 == 3) {
											unaff_s0_lo = *ppcVar10;
											unaff_s5_lo = 0;
											local_3b0 = '\0';
										}
									}
								}
								pcVar19 = (char*)local_350;
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
										if (iVar2 == 0) {
											trap(7);
										}
										iVar25 = iVar25 + -1;
										*pcVar19 = pcVar34[unaff_s5_lo - ((int)unaff_s5_lo / iVar2) * iVar2];
										pcVar19 = pcVar19 + 1;
										field_0x30 = field_0x30 + 1;
										unaff_s5_lo = (int)unaff_s5_lo / iVar2;
									}
									for (; 1 < uVar17; uVar17 = uVar17 / 10) {
										*pcVar19 = '0';
										pcVar19 = pcVar19 + 1;
										field_0x30 = field_0x30 + 1;
									}
									*pcVar19 = '.';
									pcVar19 = pcVar19 + 1;
									field_0x30 = field_0x30 + 1;
								}
								if (unaff_s0_lo == (char*)0x0) {
									iVar25 = iVar25 + -1;
									*pcVar19 = 0x30;
									pcVar19 = (char*)((byte*)pcVar19 + 1);
									field_0x30 = field_0x30 + 1;
								}
								else {
									while (unaff_s0_lo != (char*)0x0) {
										if (iVar2 == 0) {
											trap(7);
										}
										iVar25 = iVar25 + -1;
										*pcVar19 = unaff_s0_lo[(int)(pcVar34 + -((int)(char*)((int)unaff_s0_lo / iVar2) * iVar2))];
										pcVar19 = (char*)((byte*)pcVar19 + 1);
										field_0x30 = field_0x30 + 1;
										unaff_s0_lo = (char*)((int)unaff_s0_lo / iVar2);
									}
								}
								for (; 0 < iVar25; iVar25 = iVar25 + -1) {
									if (bVar33 == 0) {
										*pcVar19 = 0x20;
									}
									else {
										*pcVar19 = 0x30;
										field_0x30 = field_0x30 + 1;
									}
									pcVar19 = (char*)((byte*)pcVar19 + 1);
								}
								while ((byte*)pcVar19 != local_350) {
									pcVar19 = (char*)((byte*)pcVar19 + -1);
									*pcVar38 = *pcVar19;
									pcVar38 = (char*)((byte*)pcVar38 + 1);
								}
#endif
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
								IMPLEMENTATION_GUARD();
#if 0
								pcVar34 = *ppcVar10;
								*pcVar38 = 1;
								pcVar38 = (char*)((byte*)pcVar38 + 1);
								*pcVar10 = (char)field_0x34;
								pcVar10 = pcVar10 + 1;
								field_0x38[field_0x34] = pcVar34;
								field_0x34 = field_0x34 + 1;
#endif
								break;
							case 8:
								IMPLEMENTATION_GUARD();
#if 0
								pcVar4 = (code*)*ppcVar10;
								*pcVar38 = 2;
								pcVar38 = (char*)((byte*)pcVar38 + 1);
								if (paVar38->field_0x20 == 1) {
									uVar5 = *(undefined8*)pFVar35->m0;
									fVar29 = pFVar35->m0[2];
									fVar31 = pFVar35->m0[3];
									uVar6 = *(undefined8*)pFVar35->m0[1];
									fVar21 = pFVar35->m0[1][2];
									fVar23 = pFVar35->m0[1][3];
									uVar7 = *(undefined8*)pFVar35->m0[2];
									fVar15 = pFVar35->m0[2][2];
									fVar16 = pFVar35->m0[2][3];
									uVar8 = *(undefined8*)pFVar35->m0[3];
									fVar12 = pFVar35->m0[3][2];
									fVar13 = pFVar35->m0[3][3];
									pFVar35[1].m0[0] = (float)uVar5;
									pFVar35[1].m0[1] = (float)((ulong)uVar5 >> 0x20);
									pFVar35[1].m0[2] = fVar29;
									pFVar35[1].m0[3] = fVar31;
									pFVar35[1].m0[1][0] = (float)uVar6;
									pFVar35[1].m0[1][1] = (float)((ulong)uVar6 >> 0x20);
									pFVar35[1].m0[1][2] = fVar21;
									pFVar35[1].m0[1][3] = fVar23;
									pFVar35[1].m0[2][0] = (float)uVar7;
									pFVar35[1].m0[2][1] = (float)((ulong)uVar7 >> 0x20);
									pFVar35[1].m0[2][2] = fVar15;
									pFVar35[1].m0[2][3] = fVar16;
									pFVar35[1].m0[3][0] = (float)uVar8;
									pFVar35[1].m0[3][1] = (float)((ulong)uVar8 >> 0x20);
									pFVar35[1].m0[3][2] = fVar12;
									pFVar35[1].m0[3][3] = fVar13;
									uVar5 = *(undefined8*)pFVar35->m1;
									fVar29 = pFVar35->m1[2];
									fVar31 = pFVar35->m1[3];
									uVar6 = *(undefined8*)pFVar35->m1[1];
									fVar21 = pFVar35->m1[1][2];
									fVar23 = pFVar35->m1[1][3];
									uVar7 = *(undefined8*)pFVar35->m1[2];
									fVar15 = pFVar35->m1[2][2];
									fVar16 = pFVar35->m1[2][3];
									uVar8 = *(undefined8*)pFVar35->m1[3];
									fVar12 = pFVar35->m1[3][2];
									fVar13 = pFVar35->m1[3][3];
									pFVar35[1].m1[0] = (float)uVar5;
									pFVar35[1].m1[1] = (float)((ulong)uVar5 >> 0x20);
									pFVar35[1].m1[2] = fVar29;
									pFVar35[1].m1[3] = fVar31;
									pFVar35[1].m1[1][0] = (float)uVar6;
									pFVar35[1].m1[1][1] = (float)((ulong)uVar6 >> 0x20);
									pFVar35[1].m1[1][2] = fVar21;
									pFVar35[1].m1[1][3] = fVar23;
									pFVar35[1].m1[2][0] = (float)uVar7;
									pFVar35[1].m1[2][1] = (float)((ulong)uVar7 >> 0x20);
									pFVar35[1].m1[2][2] = fVar15;
									pFVar35[1].m1[2][3] = fVar16;
									pFVar35[1].m1[3][0] = (float)uVar8;
									pFVar35[1].m1[3][1] = (float)((ulong)uVar8 >> 0x20);
									pFVar35[1].m1[3][2] = fVar12;
									pFVar35[1].m1[3][3] = fVar13;
									pFVar35[1].pFunction = pFVar35->pFunction;
									pFVar35[1].flags_0x84 = pFVar35->flags_0x84;
									pFVar35[1].pPackedFont = pFVar35->pPackedFont;
									pFVar35[1].field_0x8c = pFVar35->field_0x8c;
									pFVar35[1].field_0x90 = pFVar35->field_0x90;
									pFVar35[1].spaceSize = pFVar35->spaceSize;
									pFVar35[1].rotation = pFVar35->rotation;
									pFVar35[1].xScale = pFVar35->xScale;
									pFVar35[1].yScale = pFVar35->yScale;
									pFVar35[1].field_0xa4 = pFVar35->field_0xa4;
									pFVar35[1].field_0xa8 = pFVar35->field_0xa8;
									pFVar35[1].offset_0xac = pFVar35->offset_0xac;
									pFVar35[1].offset_0xb0 = pFVar35->offset_0xb0;
									pFVar35[1].rgbaColour = pFVar35->rgbaColour;
									pFVar35[1].alpha = pFVar35->alpha;
									pFVar35[1].altColour = pFVar35->altColour;
								}
								else {
									fVar28 = fontData_0x850[0].m0[1];
									fVar30 = fontData_0x850[0].m0[2];
									fVar32 = fontData_0x850[0].m0[3];
									fVar19 = fontData_0x850[0].m0[1][0];
									fVar20 = fontData_0x850[0].m0[1][1];
									fVar22 = fontData_0x850[0].m0[1][2];
									fVar24 = fontData_0x850[0].m0[1][3];
									fVar21 = fontData_0x850[0].m0[2][0];
									fVar23 = fontData_0x850[0].m0[2][1];
									fVar29 = fontData_0x850[0].m0[2][2];
									fVar31 = fontData_0x850[0].m0[2][3];
									fVar12 = fontData_0x850[0].m0[3][0];
									fVar13 = fontData_0x850[0].m0[3][1];
									fVar15 = fontData_0x850[0].m0[3][2];
									fVar16 = fontData_0x850[0].m0[3][3];
									pFVar35[1].m0[0] = fontData_0x850[0].m0[0];
									pFVar35[1].m0[1] = fVar28;
									pFVar35[1].m0[2] = fVar30;
									pFVar35[1].m0[3] = fVar32;
									pFVar35[1].m0[1][0] = fVar19;
									pFVar35[1].m0[1][1] = fVar20;
									pFVar35[1].m0[1][2] = fVar22;
									pFVar35[1].m0[1][3] = fVar24;
									pFVar35[1].m0[2][0] = fVar21;
									pFVar35[1].m0[2][1] = fVar23;
									pFVar35[1].m0[2][2] = fVar29;
									pFVar35[1].m0[2][3] = fVar31;
									pFVar35[1].m0[3][0] = fVar12;
									pFVar35[1].m0[3][1] = fVar13;
									pFVar35[1].m0[3][2] = fVar15;
									pFVar35[1].m0[3][3] = fVar16;
									fVar28 = fontData_0x850[0].m1[1];
									fVar30 = fontData_0x850[0].m1[2];
									fVar32 = fontData_0x850[0].m1[3];
									fVar19 = fontData_0x850[0].m1[1][0];
									fVar20 = fontData_0x850[0].m1[1][1];
									fVar22 = fontData_0x850[0].m1[1][2];
									fVar24 = fontData_0x850[0].m1[1][3];
									fVar21 = fontData_0x850[0].m1[2][0];
									fVar23 = fontData_0x850[0].m1[2][1];
									fVar29 = fontData_0x850[0].m1[2][2];
									fVar31 = fontData_0x850[0].m1[2][3];
									fVar12 = fontData_0x850[0].m1[3][0];
									fVar13 = fontData_0x850[0].m1[3][1];
									fVar15 = fontData_0x850[0].m1[3][2];
									fVar16 = fontData_0x850[0].m1[3][3];
									pFVar35[1].m1[0] = fontData_0x850[0].m1[0];
									pFVar35[1].m1[1] = fVar28;
									pFVar35[1].m1[2] = fVar30;
									pFVar35[1].m1[3] = fVar32;
									pFVar35[1].m1[1][0] = fVar19;
									pFVar35[1].m1[1][1] = fVar20;
									pFVar35[1].m1[1][2] = fVar22;
									pFVar35[1].m1[1][3] = fVar24;
									pFVar35[1].m1[2][0] = fVar21;
									pFVar35[1].m1[2][1] = fVar23;
									pFVar35[1].m1[2][2] = fVar29;
									pFVar35[1].m1[2][3] = fVar31;
									pFVar35[1].m1[3][0] = fVar12;
									pFVar35[1].m1[3][1] = fVar13;
									pFVar35[1].m1[3][2] = fVar15;
									pFVar35[1].m1[3][3] = fVar16;
									pFVar35[1].pFunction = fontData_0x850[0].pFunction;
									pFVar35[1].flags_0x84 = fontData_0x850[0].flags_0x84;
									pFVar35[1].pPackedFont = fontData_0x850[0].pPackedFont;
									pFVar35[1].field_0x8c = fontData_0x850[0].field_0x8c;
									pFVar35[1].field_0x90 = fontData_0x850[0].field_0x90;
									pFVar35[1].spaceSize = fontData_0x850[0].spaceSize;
									pFVar35[1].rotation = fontData_0x850[0].rotation;
									pFVar35[1].xScale = fontData_0x850[0].xScale;
									pFVar35[1].yScale = fontData_0x850[0].yScale;
									pFVar35[1].field_0xa4 = fontData_0x850[0].field_0xa4;
									pFVar35[1].field_0xa8 = fontData_0x850[0].field_0xa8;
									pFVar35[1].offset_0xac = fontData_0x850[0].offset_0xac;
									pFVar35[1].offset_0xb0 = fontData_0x850[0].offset_0xb0;
									pFVar35[1].rgbaColour = fontData_0x850[0].rgbaColour;
									pFVar35[1].alpha = fontData_0x850[0].alpha;
									pFVar35[1].altColour = fontData_0x850[0].altColour;
								}
								pFVar35 = pFVar35 + 1;
								if (pcVar4 != (code*)0x0) {
									(*pcVar4)();
								}
#endif
							}
						}
						uVar40 = uVar40 + 1;
						paVar38 = paVar38 + 1;
						paVar18 = paVar18 + 1;
					} while (uVar40 <= local_370);
					*pcVar38 = 0;
					if ((field_0x30 != 0) || (bVar9 = false, field_0x34 != 0)) {
						bVar9 = true;
					}
					return bVar9;
				}
				if (*pText == 0x25) break;
				piVar38->characterCount = piVar38->characterCount + 1;
				pText = (char*)((byte*)pText + 1);
			}
			uVar17 = (uint)((byte*)pText)[1];
			pbVar36 = (byte*)pText + 2;
			if (uVar17 != 0x25) break;
			piVar38->characterCount = piVar38->characterCount + 1;
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
				piVar38->field_0x15 = '\0';
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
			piVar38->field_0x14 = bVar33;
			piVar38->field_0x4 = iVar25;
		}
		if (uVar17 == 0x4b) {
			iVar25 = 8;
			piVar38->field_0x20 = 2;
		}
		else {
			if (uVar17 == 0x6b) {
				iVar25 = 8;
				piVar38->field_0x20 = 1;
			}
			else {
				if ((uVar17 == 0x43) || (uVar17 == 99)) {
					iVar25 = 6;
				}
				else {
					if (uVar17 == 0x73) {
						iVar25 = 5;
					}
					else {
						if (uVar17 == 0x66) {
							iVar25 = 4;
						}
						else {
							if (uVar17 == 0x58) {
								piVar38->field_0x1c = 0x10;
								iVar25 = 3;
								piVar38->field_0x20 = 2;
							}
							else {
								if (uVar17 == 0x78) {
									piVar38->field_0x1c = 0x10;
									iVar25 = 3;
									piVar38->field_0x20 = 1;
								}
								else {
									if (uVar17 == 0x6f) {
										iVar25 = 2;
										piVar38->field_0x1c = 8;
									}
									else {
										if (uVar17 == 0x75) {
											iVar25 = 3;
										}
										else {
											if ((uVar17 == 0x69) || (uVar17 == 100)) {
												iVar25 = 2;
											}
											else {
												if (uVar17 == 0x6d) {
													iVar25 = 3;
													piVar38->field_0x1c = 2;
												}
												else {
													if (uVar17 == 0x62) {
														iVar25 = 7;
													}
													else {
														iVar25 = 0;
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
		piVar38->field_0x18 = iVar11;
		if (piVar38->field_0x15 == '\0') {
			if (iVar25 == 8) {
				local_3a0 = 3;
			}
			else {
				if (iVar25 == 7) {
					local_3a0 = 2;
					local_360 = local_360 + 1;
				}
				else {
					if (iVar25 == 5) {
						local_3a0 = 1;
					}
					else {
						local_3a0 = 0;
					}
				}
			}
			IMPLEMENTATION_GUARD();
#if 0
			pcVar10 = (char*)edCTextResourcePool::GetEntry(local_390, local_3a0);
			if ((pcVar10 == (char*)0x0) && (local_3a0 != 3)) {
				iVar25 = 5;
				piVar38->field_0x10 = s_(? ? ? )_004323b8;
			}
			else {
				piVar38->field_0x10 = pcVar10;
			}
#endif
		}
		else {
			if (uVar40 < iVar11 + 1U) {
				uVar40 = iVar11 + 1U;
			}
			local_380 = local_380 + 1;
		}
		piVar38->switchMode = iVar25;
		piVar38 = piVar38 + 1;
		bVar9 = true;
		local_370 = local_370 + 1;
	} while (true);
}

bool edCTextFormat::FormatString(char* pText)
{
	undefined8 uVar1;
	undefined8 uVar2;
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
	char local_30[48];

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
	bVar4 = FormatString(pText, local_30);
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
