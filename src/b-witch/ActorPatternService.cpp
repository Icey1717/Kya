#include "ActorPatternService.h"
#include "MemoryStream.h"
#include "LargeObject.h"
#include "Actor.h"
#include "MathOps.h"
#include "FileManager3D.h"
#include "DlistManager.h"
#include "TimeController.h"

CPatternPart::CPatternPart()
{
	return;
}

void CPatternPart::Create(ByteCode* pByteCode)
{
	uint uVar2;

	this->field_0x4 = pByteCode->GetF32();
	this->field_0x0 = pByteCode->GetF32();
	this->field_0xc = pByteCode->GetF32();
	this->field_0x8 = pByteCode->GetF32();
	this->field_0x10 = pByteCode->GetF32();
	this->field_0x14 = pByteCode->GetF32();
	this->materialId = pByteCode->GetS32();
	this->field_0x1c = pByteCode->GetF32();
	this->field_0x20 = pByteCode->GetF32();

	if (CScene::_pinstance->field_0x1c < 2.21f) {
		this->field_0x24 = 1.0f;
	}
	else {
		this->field_0x24 = pByteCode->GetF32();
	}

	if (this->field_0x24 <= 0.0f) {
		this->field_0x24 = 1.0f;
	}

	this->field_0x28 = pByteCode->GetU32();
	this->field_0x48 = -1.0f;

	this->nbPointPatterns = (int)this->field_0x1c;
	if (0.0f < this->field_0x1c - (float)(int)this->field_0x1c) {
		this->nbPointPatterns = this->nbPointPatterns + 1;
	}

	this->nbPointPatterns = (int)(float)((int)(float)this->nbPointPatterns * (uint)(0.0f < (float)this->nbPointPatterns)); // clamp

	uVar2 = this->nbPointPatterns;
	if (0 < (int)uVar2) {
		this->aPointPatterns = new CPointPattern[uVar2];
	}

	return;
}

void CPatternPart::Init(float param_1, CActor* pOwner)
{
	this->pOwner = pOwner;

	CActor::SV_InstallMaterialId(this->materialId);

	if (0.0f < this->field_0x14) {
		this->field_0x48 = this->field_0x14;
	}
	else {
		if (0.0f < param_1) {
			this->field_0x48 = (float)this->nbPointPatterns / param_1;
		}
		else {
			if ((this->field_0x48 <= 0.0f) && (0 < this->nbPointPatterns)) {
				this->field_0x48 = 1.0f;
			}
		}
	}

	Reset();

	return;
}

// Should be in: D:/Projects/b-witch/ActorPatternService.cpp
void CPatternPart::Reset()
{
	CPointPattern* pPointPattern;
	int iVar6;

	iVar6 = 0;
	if (0 < this->nbPointPatterns) {
		do {
			pPointPattern = this->aPointPatterns + iVar6;
			pPointPattern->field_0x20 = 0;
			pPointPattern->field_0x24 = 0;
			pPointPattern->field_0x10 = g_xVector;
			pPointPattern->field_0x25 = 0;
			iVar6 = iVar6 + 1;
		} while (iVar6 < this->nbPointPatterns);
	}

	this->field_0x40 = 0;
	this->field_0x50 = 0;
	this->field_0x4c = 0;
	this->field_0x30 = g_xVector;
	this->field_0x70 = this->field_0x48;
	this->field_0x78 = -8.0f;
	this->field_0x74 = 0;
	this->field_0x5c = -1;

	this->field_0x60.x = 0.0f;
	this->field_0x60.y = 0.0f;
	this->field_0x60.z = 0.0f;
	this->field_0x60.w = 1.0f;

	return;
}

void CPatternPart::FUN_003a7cc0(float param_2)
{
	if (this->field_0x14 < 0.0f) {
		if ((param_2 <= 0.0f) || ((float)this->nbPointPatterns <= 0.0f)) {
			if (this->field_0x48 <= 0.0f) {
				this->field_0x48 = 1.0f;
			}
		}
		else {
			this->field_0x48 = (float)this->nbPointPatterns / param_2;
		}
	}

	return;
}

void CPatternPart::Draw()
{
	int iVar1;
	bool bVar2;
	edDList_material* pMaterialInfo;
	edF32VECTOR4* pPoint;
	float fVar3;
	edF32VECTOR4 eStack16;

	if (this->field_0x50 < 3) {
		if (this->field_0x50 == 2) {
			iVar1 = (this->field_0x4c + 1) % this->nbPointPatterns;
			if (this->nbPointPatterns == 0) {
				trap(7);
			}

			pPoint = &this->field_0x60;
			if (iVar1 != this->field_0x4c) {
				pPoint = &this->aPointPatterns[iVar1].field_0x0;
			}

			edF32Vector4SubHard(&eStack16, &this->field_0x60, pPoint);
			fVar3 = edF32Vector4GetDistHard(&eStack16);
			bVar2 = true;

			if (0.001f < fVar3) goto LAB_003a59d0;
		}

		bVar2 = false;
	}
	else {
		bVar2 = true;
	}

LAB_003a59d0:
	if (bVar2) {
		pMaterialInfo = CScene::ptable.g_C3DFileManager_00451664->GetMaterialFromId(this->materialId, 0);
		bVar2 = GameDList_BeginCurrent();
		if (bVar2 != false) {
			edDListLoadIdentity();
			edDListUseMaterial(pMaterialInfo);
			DrawWithOneStripAndADCFlags();
			GameDList_EndCurrent();
		}
	}

	return;
}

void CPatternPart::DrawWithOneStripAndADCFlags()
{
	int iVar1;
	int iVar2;
	int lVar3;
	int iVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float t;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	edDListBegin(0.0f, 0.0f, 0.0f, 4, (static_cast<int>(this->field_0x1c / (float)(int)(this->field_0x20 * 16.0f) + 1.0f) + this->field_0x50) * 8);

	t = 0.0f;
	fVar6 = 0.0f;
	for (iVar4 = 0; iVar4 < 4; iVar4 = iVar4 + 1) {
		if (iVar4 == 3) {
			fVar6 = 0.2f;
		}
		else {
			if (iVar4 == 2) {
				fVar6 = 0.3f;
			}
			else {
				if (iVar4 == 1) {
					fVar6 = 0.3f;
				}
				else {
					if (iVar4 == 0) {
						fVar6 = 0.2f;
					}
				}
			}
		}

		fVar7 = this->field_0x78;
		iVar2 = this->field_0x4c;
		if (8.0f < fVar7) {
			fVar7 = fmodf(fVar7, 8.0f);
			fVar7 = fVar7 - 8.0f;
		}

		edDListColor4u8(0x80, 0x80, 0x80, 0x80);

		for (lVar3 = 0; lVar3 < this->field_0x50; lVar3 = lVar3 + 1) {
			if (lVar3 < 6) {
				FUN_003a6320(iVar2, 1);
			}

			fVar5 = FUN_003a5e10(lVar3, iVar2, iVar4, &local_10, &local_20);
			if ((8.0f < fVar7 + fVar5) && (lVar3 != 0)) {
				iVar1 = this->nbPointPatterns;
				fVar7 = (fVar7 - static_cast<float>((int)fVar7) - 8.0f);
				if (iVar1 == 0) {
					trap(7);
				}

				FUN_003a5e10(lVar3 + -1, (iVar2 + iVar1 + -1) % iVar1, iVar4, &local_30, &local_40);
				edDListTexCoo2f(fVar7, t);
				edDListVertex4f(local_30.x, local_30.y, local_30.z, 49152.0f);
				edDListTexCoo2f(fVar7, t + fVar6);
				edDListVertex4f(local_40.x, local_40.y, local_40.z, 49152.0f);
			}
			fVar7 = fVar7 + fVar5;
			if ((lVar3 == 0) || (this->aPointPatterns[iVar2].field_0x25 != 0)) {
				local_10.w = 49152.0f;
				local_20.w = 49152.0f;
			}
			else {
				local_10.w = 1.0f;
				local_20.w = 1.0f;
			}

			if (0.0f < fVar5) {
				edDListTexCoo2f(fVar7, t);
				edDListVertex4f(local_10.x, local_10.y, local_10.z, local_10.w);
				edDListTexCoo2f(fVar7, t + fVar6);
				edDListVertex4f(local_20.x, local_20.y, local_20.z, local_20.w);
			}

			iVar2 = (iVar2 + 1) % this->nbPointPatterns;

			if (this->nbPointPatterns == 0) {
				trap(7);
			}
		}

		t = t + fVar6;
	}

	edDListEnd();

	return;
}

float CPatternPart::FUN_003a6320(int param_2, int param_3)
{
	int iVar1;
	int iVar2;
	int iVar3;
	int iVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;

	iVar4 = this->field_0x50;
	if (1 < iVar4) {
		iVar4 = 2;
	}

	iVar2 = param_2;
	for (iVar3 = 0; iVar3 < iVar4; iVar3 = iVar3 + 1) {
		if (this->aPointPatterns[iVar2].field_0x25 != 0) goto LAB_003a63a8;
		iVar1 = this->nbPointPatterns;
		iVar2 = (iVar2 + iVar1 + -1) % iVar1;
		if (iVar1 == 0) {
			trap(7);
		}
	}

	iVar3 = -1;
LAB_003a63a8:
	if (static_cast<float>(iVar3) == -1.0f) {
		fVar8 = 1.0f;
	}
	else {
		fVar8 = (static_cast<float>(iVar3) + 0.2f) / static_cast<float>(iVar4);
	}

	fVar6 = this->field_0x48;
	fVar5 = (float)this->aPointPatterns[param_2].field_0x20;
	fVar7 = (fVar5 / fVar6) * 3.0f;

	if ((param_3 == 0) && ((this->field_0x28 & 1) == 0)) {
		fVar5 = ((fVar6 - fVar5) / fVar6) * 3.0f;
	}
	else {
		fVar5 = 1.0f;
	}

	if ((fVar7 <= fVar8) && (fVar8 = fVar7, fVar7 < 0.0f)) {
		fVar8 = 0.0f;
	}

	fVar6 = 1.0f;
	if ((fVar5 <= 1.0f) && (fVar6 = fVar5, fVar5 < 0.0f)) {
		fVar6 = 0.0f;
	}

	return static_cast<float>((int)fVar8 * (uint)(fVar8 < fVar6) | (int)fVar6 * (uint)(fVar8 >= fVar6));
}


float CPatternPart::FUN_003a5e10(undefined8 param_2, int param_3, int param_4, edF32VECTOR4* param_5, edF32VECTOR4* param_6)
{
	int iVar1;
	int iVar2;
	int iVar3;
	edF32VECTOR4* pCVar4;
	edF32VECTOR4* pCVar6;
	edF32VECTOR4* v2;
	edF32VECTOR4* pCVar5;
	edF32VECTOR4* pCVar13;
	edF32VECTOR4* v2_00;
	edF32VECTOR4* pCVar11;
	edF32VECTOR4* pCVar12;
	edF32VECTOR4* pCVar9;
	edF32VECTOR4* pCVar10;
	edF32VECTOR4* pCVar7;
	edF32VECTOR4* pCVar8;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float t;
	float fVar8;
	float t_00;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 local_50;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	iVar2 = this->field_0x4c;
	if (param_3 == iVar2) {
		iVar1 = (param_3 + 1) % this->nbPointPatterns;
		if (this->nbPointPatterns == 0) {
			trap(7);
		}

		if (iVar1 == iVar2) {
			pCVar6 = &this->field_0x60;
		}
		else {
			pCVar6 = &this->aPointPatterns[iVar1].field_0x0;
		}

		pCVar5 = &this->field_0x60;
		if (param_3 != iVar2) {
			pCVar5 = &this->aPointPatterns[param_3].field_0x0;
		}

		edF32Vector4SubHard(&eStack96, pCVar6, pCVar5);

		fVar4 = this->field_0x74;
	}
	else {
		iVar3 = this->nbPointPatterns;
		iVar1 = (param_3 + iVar3 + -1) % iVar3;
		if (iVar3 == 0) {
			trap(7);
		}

		pCVar4 = &this->field_0x60;
		if (param_3 != iVar2) {
			pCVar4 = &this->aPointPatterns[param_3].field_0x0;
		}

		if (iVar1 == iVar2) {
			v2 = &this->field_0x60;
		}
		else {
			v2 = &this->aPointPatterns[iVar1].field_0x0;
		}

		edF32Vector4SubHard(&eStack96, pCVar4, v2);
		fVar8 = this->field_0x20;
		fVar4 = edF32Vector4GetDistHard(&eStack96);
		fVar4 = (fVar4 * 16.0f) / static_cast<float>((int)(fVar8 * 16.0f));
	}

	fVar8 = FUN_003a6320(param_3, 0);
	if (1.0f < fVar8) {
		fVar7 = 1.0f;
	}
	else {
		fVar7 = fVar8;
		if (fVar8 < 0.0f) {
			fVar7 = 0.0f;
		}
	}

	fVar6 = this->field_0x0;
	t_00 = fVar7 * this->field_0x4;
	fVar5 = 1.0f;

	if ((fVar8 <= 1.0f) && (fVar5 = fVar8, fVar8 < 0.0f)) {
		fVar5 = 0.0f;
	}
	fVar8 = this->field_0xc;
	t = fVar5 * this->field_0x8;
	edF32Vector4NormalizeHard(&local_40, &this->aPointPatterns[param_3].field_0x10);
	local_50 = local_40;
	edF32Vector4CrossProductHard(&local_20, &eStack96, &local_40);
	edF32Vector4SafeNormalize0Hard(&local_20, &local_20);
	local_30 = local_20;
	edF32Vector4ScaleHard(fVar7 * fVar6, &local_20, &local_20);
	edF32Vector4ScaleHard(t_00, &local_30, &local_30);
	edF32Vector4ScaleHard(t, &local_40, &local_40);
	edF32Vector4ScaleHard(fVar5 * fVar8, &local_50, &local_50);

	if (param_4 == 3) {
		edF32Vector4ScaleHard(-1.0f, &local_30, &local_30);
		pCVar7 = &this->field_0x60;
		if (param_3 != this->field_0x4c) {
			pCVar7 = &this->aPointPatterns[param_3].field_0x0;
		}

		edF32Vector4AddHard(&eStack16, &local_30, pCVar7);
		edF32Vector4AddHard(param_5, &local_50, &eStack16);
		edF32Vector4ScaleHard(-1.0f, &local_20, &local_20);

		pCVar8 = &this->field_0x60;
		if (param_3 != this->field_0x4c) {
			pCVar8 = &this->aPointPatterns[param_3].field_0x0;
		}

		edF32Vector4AddHard(param_6, &local_20, pCVar8);
	}
	else {
		if (param_4 == 2) {
			pCVar9 = &this->field_0x60;
			if (param_3 != this->field_0x4c) {
				pCVar9 = &this->aPointPatterns[param_3].field_0x0;
			}

			edF32Vector4AddHard(param_5, &local_40, pCVar9);
			edF32Vector4ScaleHard(-1.0f, &local_30, &local_30);
			pCVar10 = &this->field_0x60;
			if (param_3 != this->field_0x4c) {
				pCVar10 = &this->aPointPatterns[param_3].field_0x0;
			}

			edF32Vector4AddHard(&eStack16, &local_30, pCVar10);
			edF32Vector4AddHard(param_6, &local_50, &eStack16);
		}
		else {
			if (param_4 == 1) {
				pCVar11 = &this->field_0x60;
				if (param_3 != this->field_0x4c) {
					pCVar11 = &this->aPointPatterns[param_3].field_0x0;
				}

				edF32Vector4AddHard(&eStack16, &local_30, pCVar11);
				edF32Vector4AddHard(param_5, &local_50, &eStack16);
				pCVar12 = &this->field_0x60;
				if (param_3 != this->field_0x4c) {
					pCVar12 = &this->aPointPatterns[param_3].field_0x0;
				}

				edF32Vector4AddHard(param_6, &local_40, pCVar12);
			}
			else {
				if (param_4 == 0) {
					pCVar13 = &this->field_0x60;
					if (param_3 != this->field_0x4c) {
						pCVar13 = &this->aPointPatterns[param_3].field_0x0;
					}

					edF32Vector4AddHard(param_5, &local_20, pCVar13);
					v2_00 = &this->field_0x60;
					if (param_3 != this->field_0x4c) {
						v2_00 = &this->aPointPatterns[param_3].field_0x0;
					}

					edF32Vector4AddHard(&eStack16, &local_30, v2_00);
					edF32Vector4AddHard(param_6, &local_50, &eStack16);
				}
			}
		}
	}

	return fVar4;
}

void CPatternPart::FireContinue(CActor* param_2, edF32VECTOR4* param_3, edF32VECTOR4* param_4)
{
	this->pOwner->SV_Vector4SLERP(3.141593f, &this->field_0x30, param_3);
	FireContinueWithoutAddPoint(param_2, param_4);

	return;
}

void CPatternPart::FireContinueWithoutAddPoint(CActor* param_2, edF32VECTOR4* param_3)
{
	int iVar1;
	int iVar2;
	CPointPattern* pCVar3;
	float fVar4;
	float fVar5;
	float puVar6;
	float fVar7;
	float fVar8;
	float puVar9;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	if (this->nbPointPatterns == 0) {
		return;
	}
	if (this->field_0x40 != param_2) {
		Reset();

		this->field_0x40 = param_2;
		this->pOwner->flags = this->pOwner->flags | 0x400;
	}

	if (this->field_0x5c < 0) {
		FUN_003a78f0(this->field_0x48, param_3, &this->field_0x30, 1);
		FUN_003a78f0(this->field_0x48, param_3, &this->field_0x30, 0);
	}

	fVar8 = this->field_0x48;
	if (this->nbPointPatterns == 0) {
		trap(7);
	}

	fVar7 = (this->field_0x30).y;
	fVar4 = (this->field_0x30).z;
	pCVar3 = this->aPointPatterns + (this->field_0x4c + this->field_0x50 + -1) % this->nbPointPatterns;
	fVar5 = (this->field_0x30).w;
	pCVar3->field_0x10 = this->field_0x30;
	pCVar3->field_0x0 = *param_3;
	pCVar3->field_0x20 = fVar8;
	pCVar3->field_0x24 = 1;
	pCVar3->field_0x25 = 0;

	if (this->field_0x50 == 2) {
		iVar2 = this->nbPointPatterns;
		iVar1 = (this->field_0x4c + 1) % iVar2;
		if (iVar2 == 0) {
			trap(7);
		}
		if (iVar2 == 0) {
			trap(7);
		}

		edF32Vector4SubHard(&eStack16, &this->aPointPatterns[iVar1].field_0x0, &this->aPointPatterns[(iVar1 + iVar2 + -1) % iVar2].field_0x0);
		fVar8 = edF32Vector4GetDistHard(&eStack16);
		if (fVar8 <= 0.5f) {
			if (this->nbPointPatterns == 0) {
				trap(7);
			}

			if (this->aPointPatterns[(this->field_0x4c + this->field_0x50 + -1) % this->nbPointPatterns].field_0x25 == 0) goto LAB_003a6938;
		}

		FUN_003a78f0(this->field_0x48, param_3, &this->field_0x30, 0);
	}
	else {
		iVar2 = this->nbPointPatterns;
		iVar1 = (this->field_0x4c + this->field_0x50 + -1) % iVar2;
		if (iVar2 == 0) {
			trap(7);
		}
		if (iVar2 == 0) {
			trap(7);
		}
		edF32Vector4SubHard(&eStack16, &this->aPointPatterns[iVar1].field_0x0, &this->aPointPatterns[(iVar1 + iVar2 + -1) % iVar2].field_0x0);
		iVar1 = this->nbPointPatterns;
		if (iVar1 == 0) {
			trap(7);
		}

		iVar2 = ((this->field_0x4c + this->field_0x50 + -1) % iVar1 + iVar1 + -1) % iVar1;
		if (iVar1 == 0) {
			trap(7);
		}

		if (iVar1 == 0) {
			trap(7);
		}

		edF32Vector4SubHard(&eStack32, &this->aPointPatterns[iVar2].field_0x0, &this->aPointPatterns[(iVar2 + iVar1 + -1) % iVar1].field_0x0);
		fVar8 = edF32Vector4GetDistHard(&eStack16);
		edF32Vector4SafeNormalize0Hard(&eStack16, &eStack16);
		edF32Vector4SafeNormalize0Hard(&eStack32, &eStack32);
		puVar6 = edF32Vector4DotProductHard(&eStack16, &eStack32);
		if (1.0f < puVar6) {
			puVar9 = 1.0f;
		}
		else {
			puVar9 = puVar6;
			if (puVar6 < -1.0f) {
				puVar9 = -1.0f;
			}
		}

		fVar4 = acosf(puVar9);
		if ((fVar8 <= 0.5f) || ((fVar4 * fVar8 <= 0.5f / this->field_0x24 && (0.97f <= puVar6)))) {
			if (this->nbPointPatterns == 0) {
				trap(7);
			}

			if ((this->aPointPatterns[(this->field_0x4c + this->field_0x50 + -1) % this->nbPointPatterns].field_0x25 == 0) && (fVar8 <= 3.0f / this->field_0x24)) goto LAB_003a6938;
		}

		FUN_003a78f0(this->field_0x48, param_3, &this->field_0x30, 0);
	}

LAB_003a6938:
	this->field_0x5c = this->field_0x5c + 2;
	if (this->field_0x5c < 4) {
		if (this->field_0x5c < 0) {
			this->field_0x5c = 0;
		}
	}
	else {
		this->field_0x5c = 3;
	}

	return;
}

void CPatternPart::FUN_003a78f0(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4, byte param_5)
{
	int iVar1;
	int iVar2;
	CPointPattern* pCVar3;
	int iVar4;
	int iVar5;
	int iVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	edF32VECTOR4 eStack16;

	iVar1 = this->field_0x4c;
	iVar2 = this->nbPointPatterns;
	if (iVar2 == 0) {
		trap(7);
	}
	if ((iVar1 + this->field_0x50) % iVar2 == iVar1) {
		if (this->field_0x50 < 2) {
			this->field_0x70 = param_1;
			this->field_0x74 = 0.0f;
		}
		else {
			if (iVar2 == 0) {
				trap(7);
			}

			iVar5 = 0;
			iVar6 = iVar1;
			if (0 < iVar2) {
				iVar4 = iVar1;
				do {
					iVar4 = (iVar4 + 1) % iVar2;
					iVar5 = iVar5 + 1;
					iVar6 = iVar4;
					if (this->aPointPatterns[iVar4].field_0x24 != 0) break;
					if (iVar2 == 0) {
						trap(7);
					}
					iVar6 = iVar1;
				} while (iVar5 < iVar2);
			}

			this->field_0x70 = this->aPointPatterns[iVar6].field_0x20;
			edF32Vector4SubHard(&eStack16, &this->aPointPatterns[iVar6].field_0x0, &this->aPointPatterns[iVar1].field_0x0);
			fVar7 = edF32Vector4GetDistHard(&eStack16);
			fVar7 = this->field_0x78 + (fVar7 * 16.0f) / static_cast<float>((int)(this->field_0x20 * 16.0f));
			this->field_0x78 = fVar7;
			if (8.0f < fVar7) {
				fVar7 = fmodf(fVar7, 8.0f);
				this->field_0x78 = fVar7 - 8.0f;
			}

			this->field_0x74 = 0.0f;
			if (this->aPointPatterns[iVar6].field_0x24 != 0) {
				this->aPointPatterns[iVar6].field_0x25 = 1;
			}
		}
	}

	if (this->nbPointPatterns == 0) {
		trap(7);
	}

	pCVar3 = this->aPointPatterns + (this->field_0x4c + this->field_0x50) % this->nbPointPatterns;
	pCVar3->field_0x10 = *param_4;
	pCVar3->field_0x0 = *param_3;
	pCVar3->field_0x20 = param_1;
	pCVar3->field_0x24 = 1;
	pCVar3->field_0x25 = param_5;
	iVar1 = this->nbPointPatterns;
	iVar2 = this->field_0x4c;

	if (this->field_0x50 < iVar1) {
		this->field_0x50 = this->field_0x50 + 1;
	}
	else {
		if (iVar1 == 0) {
			trap(7);
		}
		iVar4 = 0;
		iVar5 = iVar2;
		iVar6 = iVar2;
		if (0 < iVar1) {
			do {
				iVar6 = (iVar6 + 1) % iVar1;
				iVar4 = iVar4 + 1;
				iVar5 = iVar6;
				if (this->aPointPatterns[iVar6].field_0x24 != 0) break;
				if (iVar1 == 0) {
					trap(7);
				}
				iVar5 = iVar2;
			} while (iVar4 < iVar1);
		}

		this->field_0x4c = iVar5;
	}

	return;
}

bool CPatternPart::UpdatePatternPartLife()
{
	bool bVar1;
	int iVar2;
	int iVar4;
	int iVar5;
	int iVar6;
	int iVar7;
	int iVar8;
	int iVar9;
	CPointPattern* pCVar10;
	int iVar11;
	int iVar12;
	float puVar13;
	float fVar14;
	edF32VECTOR4 local_c0;
	edF32VECTOR4 local_b0;
	edF32VECTOR4 local_a0;
	_msg_hit_param local_90;

	bVar1 = false;
	iVar11 = this->field_0x50;
	iVar12 = this->field_0x4c;
	iVar4 = 0;
	if (0 < iVar11) {
		for (; iVar4 < iVar11; iVar4 = iVar4 + 1) {
			iVar6 = this->field_0x50;
			iVar5 = 2;
			if (iVar6 < 2) {
				iVar5 = iVar6;
			}
			iVar7 = 0;
			if (0 < iVar5) {
				iVar8 = iVar12;
				do {
					if (this->aPointPatterns[iVar8].field_0x25 != 0) goto LAB_003a6ce0;

					iVar2 = this->nbPointPatterns;
					iVar8 = (iVar8 + iVar2 + -1) % iVar2;

					if (iVar2 == 0) {
						trap(7);
					}

					iVar7 = iVar7 + 1;
				} while (iVar7 < iVar5);
			}

			iVar7 = -1;

		LAB_003a6ce0:
			iVar8 = this->nbPointPatterns;
			iVar2 = 0;
			if (iVar8 == 0) {
				trap(7);
			}

			iVar9 = iVar12;
			if (0 < iVar5) {
				do {
					if (iVar9 == (this->field_0x4c + iVar6 + -1) % iVar8) goto LAB_003a6d70;
					iVar9 = (iVar9 + 1) % iVar8;
					if (iVar8 == 0) {
						trap(7);
					}
					if (this->aPointPatterns[iVar9].field_0x25 != 0) goto LAB_003a6d70;
					iVar2 = iVar2 + 1;
				} while (iVar2 < iVar5);
			}

			iVar2 = -1;

		LAB_003a6d70:
			if ((static_cast<float>(iVar2) == -1.0f) || (iVar7 != -1)) {
				puVar13 = -1.0f;
			}
			else {
				puVar13 = this->field_0x48 - (((float)iVar2 / 2.0f) * this->field_0x48) / 3.0f;
			}

			pCVar10 = this->aPointPatterns + iVar12;
			if (puVar13 < pCVar10->field_0x20) {
				pCVar10->field_0x20 = pCVar10->field_0x20 - GetTimer()->cutsceneDeltaTime;
			}

			if (pCVar10->field_0x20 <= 0.0f) {
				pCVar10->field_0x20 = 0.0f;
				if (pCVar10->field_0x24 == 0) {
					iVar6 = 1;
				}
				else {
					pCVar10->field_0x24 = 0;
					iVar6 = 2;
				}
			}
			else {
				iVar6 = 4;
			}

			if (iVar6 == 4) {
				bVar1 = true;
			}
			else {
				if (((iVar6 != 1) && (iVar6 == 2)) && (this->aPointPatterns[iVar12].field_0x25 != 0)) {
					FUN_003a75b0(iVar12);
					this->aPointPatterns[iVar12].field_0x25 = 0;
				}
			}

			iVar12 = (iVar12 + 1) % this->nbPointPatterns;
			if (this->nbPointPatterns == 0) {
				trap(7);
			}
		}
	}

	if (bVar1) {
		FUN_003a6a60();

		iVar12 = this->field_0x4c;
		iVar11 = 0;
		if (0 < this->field_0x50) {
			do {
				iVar4 = FUN_003a7190(iVar12);
				if (iVar4 != 0) {
					local_90.projectileType = 1;
					local_90.damage = this->field_0x10;
					fVar14 = fmodf(static_cast<float>(iVar12 + this->field_0x50 + -1), (float)this->field_0x50);
					if (2.147484e+09f <= fVar14) {
						fVar14 = fVar14 - 2.147484e+09f;
					}
					edF32Vector4SubHard(&local_a0, &this->aPointPatterns[iVar12].field_0x0, &this->aPointPatterns[(int)fVar14].field_0x0);
					edF32Vector4SubHard(&local_b0, &this->aPointPatterns[iVar12].field_0x0, &this->field_0x40->currentLocation);

					if (0.0f < local_b0.x * local_a0.z - local_a0.x * local_b0.z) {
						local_c0.z = -local_a0.x;
						local_c0.x = local_a0.z;
					}
					else {
						local_c0.x = -local_a0.z;
						local_c0.z = local_a0.x;
					}

					local_c0.y = 0.0f;
					fVar14 = edF32Vector4GetDistHard(&local_c0);
					if (0.0f < fVar14) {
						edF32Vector4NormalizeHard(&local_c0, &local_c0);
					}
					else {
						local_c0 = gF32Vector4UnitX;
					}

					local_90.field_0x20 = local_c0;
					this->pOwner->DoMessage(this->field_0x40, MESSAGE_KICKED, &local_90);

					break;
				}

				iVar12 = (iVar12 + 1) % this->nbPointPatterns;
				if (this->nbPointPatterns == 0) {
					trap(7);
				}

				iVar11 = iVar11 + 1;
			} while (iVar11 < this->field_0x50);
		}

		this->field_0x5c = this->field_0x5c + -1;
	}
	else {
		Reset();
		this->pOwner->flags = this->pOwner->flags & 0xfffffbff;
	}

	return bVar1;
}

bool CPatternPart::FUN_003a7190(int param_2)
{
	int iVar1;
	uint boneIndex;
	edF32VECTOR4* peVar2;
	edF32VECTOR4* pCVar3;
	edF32VECTOR4* v2;
	int iVar4;
	int iVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	edF32VECTOR4 eStack144;
	edF32VECTOR4 eStack128;
	edF32VECTOR4 local_70;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	undefined4 local_4;

	fVar6 = this->field_0x0;
	fVar6 = static_cast<float>((uint)(fVar6 < 0.25f) * 0x3e800000 | (int)fVar6 * (uint)(fVar6 >= 0.25f));
	iVar1 = (param_2 + 1) % this->nbPointPatterns;
	if (this->nbPointPatterns == 0) {
		trap(7);
	}

	if ((this->aPointPatterns[iVar1].field_0x25 == 0) && (this->aPointPatterns[iVar1].field_0x24 != 0)) {
		if (this->nbPointPatterns == 0) {
			trap(7);
		}

		if (param_2 != (this->field_0x4c + this->field_0x50 + -1) % this->nbPointPatterns) {
			boneIndex = this->pOwner->DoMessage(this->field_0x40, MESSAGE_GET_BONE_ID, (MSG_PARAM)0xc);
			if (boneIndex == 0) {
				peVar2 = this->field_0x40->GetBottomPosition();
				local_70 = *peVar2;
			}
			else {
				this->field_0x40->SV_GetBoneDefaultWorldPosition(boneIndex, &local_70);
			}

			pCVar3 = &this->field_0x60;
			if (iVar1 != this->field_0x4c) {
				pCVar3 = &this->aPointPatterns[iVar1].field_0x0;
			}

			v2 = &this->field_0x60;
			if (param_2 != this->field_0x4c) {
				v2 = &this->aPointPatterns[param_2].field_0x0;
			}

			edF32Vector4SubHard(&eStack128, pCVar3, v2);
			fVar7 = edF32Vector4GetDistHard(&eStack128);
			if (0.0f < fVar7) {
				edF32Vector4NormalizeHard(&eStack128, &eStack128);
				if ((fVar7 <= 0.001f) || (fVar6 <= 0.001f)) {
					iVar5 = 0;
				}
				else {
					iVar5 = static_cast<int>(fVar7 / fVar6) + 2;
				}

				for (iVar4 = 0; iVar4 < iVar5; iVar4 = iVar4 + 1) {
					edF32Vector4ScaleHard(static_cast<float>(iVar4) * fVar6, &eStack144, &eStack128);
					pCVar3 = &this->field_0x60;
					if (param_2 != this->field_0x4c) {
						pCVar3 = &this->aPointPatterns[param_2].field_0x0;
					}

					edF32Vector4AddHard(&eStack96, pCVar3, &eStack144);
					edF32Vector4SubHard(&eStack32, &this->field_0x40->currentLocation, &eStack96);
					fVar8 = edF32Vector4DotProductHard(&eStack32, &this->aPointPatterns[param_2].field_0x10);
					edF32Vector4SubHard(&eStack48, &local_70, &eStack96);
					fVar9 = edF32Vector4DotProductHard(&eStack48, &this->aPointPatterns[param_2].field_0x10);
					fVar10 = edFIntervalLERP(static_cast<float>(iVar4), 0.0f, static_cast<float>(iVar5 + -1),
						this->aPointPatterns[param_2].field_0x20 / this->field_0x48,
						this->aPointPatterns[iVar1].field_0x20 / this->field_0x48);
					fVar7 = this->field_0x8;

					if (fVar7 <= this->field_0xc) {
						fVar7 = this->field_0xc;
					}

					if ((0.0f < fVar9) && (fVar8 < fVar10 * fVar7)) {
						edF32Vector4ScaleHard(-fabsf(fVar8), &eStack64, &this->aPointPatterns[param_2].field_0x10);
						edF32Vector4AddHard(&eStack80, &eStack64, &eStack32);
						fVar7 = edF32Vector4GetDistHard(&eStack80);

						if (fVar7 < fVar6) {
							return true;
						}
					}
				}
			}

			return false;
		}
	}

	return false;
}


void CPatternPart::FUN_003a6a60()
{
	CPointPattern* pCVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	pCVar1 = this->aPointPatterns + this->field_0x4c;

	this->field_0x60 = pCVar1->field_0x0;
	if (this->field_0x50 < 2) {
		this->field_0x74 = 0.0f;
	}
	else {
		if (this->nbPointPatterns == 0) {
			trap(7);
		}

		pCVar1 = this->aPointPatterns + (this->field_0x4c + 1) % this->nbPointPatterns;
		if (pCVar1->field_0x24 == 0) {
			local_20 = this->field_0x60;
		}
		else {
			local_20 = pCVar1->field_0x0;
		}

		edF32Vector4SubHard(&eStack16, &local_20, &this->field_0x60);
		fVar2 = edF32Vector4SafeNormalize0Hard(&eStack16, &eStack16);
		fVar2 = edFIntervalLERP(this->field_0x70 - this->aPointPatterns[this->field_0x4c].field_0x20, 0.0f , this->field_0x70, 0.0f, fVar2);
		edF32Vector4ScaleHard(fVar2, &eStack16, &eStack16);
		edF32Vector4AddHard(&this->field_0x60, &this->field_0x60, &eStack16);
		this->field_0x74 = (fVar2 * 16.0f) / static_cast<float>((int)(this->field_0x20 * 16.0f));
	}

	return;
}

void CPatternPart::FUN_003a75b0(int param_2)
{
	bool bVar1;
	int iVar2;
	CPointPattern* pCVar3;
	int iVar4;
	int iVar5;
	int iVar6;
	int iVar7;
	float fVar8;
	edF32VECTOR4 eStack16;

	iVar5 = this->nbPointPatterns;
	iVar2 = param_2 + 1;
	if (iVar5 == 0) {
		trap(7);
	}
	iVar4 = 0;
	iVar7 = param_2;
	if (0 < iVar5) {
		do {
			iVar7 = iVar2 % iVar5;
			iVar4 = iVar4 + 1;
			if (this->aPointPatterns[iVar7].field_0x24 != 0) break;
			iVar2 = iVar7 + 1;
			if (iVar5 == 0) {
				trap(7);
			}
			iVar7 = param_2;
		} while (iVar4 < iVar5);
	}

	if (param_2 == this->field_0x4c) {
		this->field_0x70 = this->aPointPatterns[iVar7].field_0x20;
		edF32Vector4SubHard(&eStack16, &this->aPointPatterns[iVar7].field_0x0, &this->aPointPatterns[this->field_0x4c].field_0x0);
		fVar8 = edF32Vector4GetDistHard(&eStack16);
		fVar8 = this->field_0x78 + (fVar8 * 16.0f) / static_cast<float>((int)(this->field_0x20 * 16.0f));
		this->field_0x78 = fVar8;
		if (8.0f < fVar8) {
			fVar8 = fmodf(fVar8, 8.0f);
			this->field_0x78 = fVar8 - 8.0f;
		}
		this->field_0x74 = 0.0f;
	}

	pCVar3 = this->aPointPatterns;
	iVar2 = this->nbPointPatterns;
	iVar6 = 0;
	iVar4 = 0;
	bVar1 = false;
	iVar5 = param_2;
	do {
		if (iVar2 == 0) {
			trap(7);
		}

		if (pCVar3[(iVar5 + 1) % iVar2].field_0x25 != 0) {
		LAB_003a7778:
			bVar1 = true;
			break;
		}

		if (iVar2 == 0) {
			trap(7);
		}

		if (iVar5 == (this->field_0x4c + this->field_0x50 + -1) % iVar2) goto LAB_003a7778;

		iVar6 = iVar6 + 1;
		if ((pCVar3[iVar5].field_0x24 != 0) || (iVar5 == param_2)) {
			iVar4 = iVar4 + 1;
		}

		iVar5 = (iVar5 + 1) % iVar2;
		if (iVar2 == 0) {
			trap(7);
		}
	} while (iVar6 < 4);

	if ((bVar1) && (1 < iVar4)) {
		if (iVar2 == 0) {
			trap(7);
		}

		pCVar3[(param_2 + 2) % iVar2].field_0x20 = this->field_0x48 - pCVar3[(param_2 + 2) % iVar2].field_0x20;
		pCVar3 = this->aPointPatterns;
	}

	if (pCVar3[iVar7].field_0x24 != 0) {
		pCVar3[iVar7].field_0x25 = 1;
	}

	iVar5 = this->field_0x4c;
	iVar2 = this->nbPointPatterns;
	if (iVar2 == 0) {
		trap(7);
	}

	iVar4 = 0;
	iVar7 = iVar5;
	if (0 < iVar2) {
		iVar6 = iVar5;
		do {
			iVar6 = (iVar6 + 1) % iVar2;
			iVar4 = iVar4 + 1;
			iVar7 = iVar6;
			if (this->aPointPatterns[iVar6].field_0x24 != 0) break;
			if (iVar2 == 0) {
				trap(7);
			}
			iVar7 = iVar5;
		} while (iVar4 < iVar2);
	}

	this->field_0x4c = iVar7;
	this->field_0x50 = this->field_0x50 - iVar4;

	return;
}
