#include "FrontEndLife.h"
#include "LevelScheduleManager.h"
#include "Actor.h"
#include "ActorAutonomous.h"
#include "FrontEndBank.h"
#include "Frontend.h"
#include "Rendering/DisplayList.h"
#include "TimeController.h"

float gLifePopupTime = 0.2f;

edF32VECTOR2 CURRENT_SCALE = { 1.0f, 1.0f };
edF32VECTOR2 POSITION_SHOW = { 0.08f, 0.135f };
edF32VECTOR2 POSITION_HIDE = { 0.08f, -0.125f };
edF32VECTOR2 HEAD_OFFSET = { 0.0f, 0.0f };
edF32VECTOR2 HEAD_SCALE = { 0.125f, 0.125f };
edF32VECTOR2 FILL_LIFE_OFFSET = { 0.0f, 0.0f };
edF32VECTOR2 FILL_LIFE_SCALE = { 1.0f, 1.0f };
edF32VECTOR2 FILL_HIT_OFFSET = { 0.0f, 0.0f };
edF32VECTOR2 FILL_SCALE = { 1.0f, 1.0f };
edF32VECTOR2 GAUGE_OFFSET = { 0.131f, 0.02f };
edF32VECTOR2 GAUGE_SCALE = { 0.25f, 0.0625f };

CFrontendLifeGauge::CFrontendLifeGauge()
{
	int iVar1;

	this->fillLifeMin = 0.2222f;
	this->fillLifeMax = 0.8f;

	iVar1 = 0;
	this->popupTime = gLifePopupTime;
	(this->slotShow).scale = CURRENT_SCALE;
	(this->slotShow).position = POSITION_SHOW;

	(this->slotHide).scale = CURRENT_SCALE;
	(this->slotHide).position = POSITION_HIDE;

	this->spriteKimHead.UpdateSlotPosition(HEAD_OFFSET.x, HEAD_OFFSET.y);
	this->spriteKimHead.UpdateSlotScale(HEAD_SCALE.x, HEAD_SCALE.y);

	this->spriteFillLife.UpdateSlotPosition(FILL_LIFE_OFFSET.x, FILL_LIFE_OFFSET.y);
	this->spriteFillLife.UpdateSlotScale(FILL_LIFE_SCALE.x, FILL_LIFE_SCALE.y);

	this->spriteFillHit.UpdateSlotPosition(FILL_HIT_OFFSET.x, FILL_HIT_OFFSET.y);
	this->spriteFillHit.UpdateSlotScale(FILL_SCALE.x, FILL_SCALE.y);

	this->spriteGauge.UpdateSlotPosition(GAUGE_OFFSET.x, GAUGE_OFFSET.y);
	this->spriteGauge.UpdateSlotScale(GAUGE_SCALE.x, GAUGE_SCALE.y);

	do {
		this->aDualSprites[iVar1].spriteGauge.UpdateSlotPosition(GAUGE_OFFSET.x, GAUGE_OFFSET.y);
		this->aDualSprites[iVar1].spriteGauge.UpdateSlotScale(GAUGE_SCALE.x, GAUGE_SCALE.y);

		this->aDualSprites[iVar1].spriteFill.UpdateSlotPosition(FILL_LIFE_OFFSET.x, FILL_LIFE_OFFSET.y);
		this->aDualSprites[iVar1].spriteFill.UpdateSlotScale(FILL_LIFE_SCALE.x, FILL_LIFE_SCALE.y);

		iVar1 = iVar1 + 1;
	} while (iVar1 < 4);

	return;
}

void CFrontendLifeGauge::SetInterface(CInterface* pNewInterface)
{
	CInterface* pCVar1;
	int iVar2;
	int iVar3;
	long lVar4;
	float fVar5;

	this->pInterface = pNewInterface;

	UpdateInterface();

	return;
}

void CFrontendLifeGauge::Reset()
{
	CFrontendBitmapGauge::Reset();

	FUN_001daff0();

	this->widgetSlotC.position = this->slotHide.position;
	this->widgetSlotC.scale = this->slotHide.scale;

	this->widgetSlotB.position = this->slotHide.position;
	this->widgetSlotB.scale = this->slotHide.scale;

	this->widgetSlotA.position = this->slotHide.position;
	this->widgetSlotA.scale = this->slotHide.scale;

	this->state = 4;

	return;
}

char* G2D_RES_LIFE_BACK = "kim_gauge";
char* G2D_RES_LIFE_FILL_ORANGE = "kim_fill_life";
char* G2D_RES_LIFE_FILL_GREEN = "kim_fill_hit";
char* G2D_RES_KIM_HEAD = "head_kim";
char* G2D_RES_LIFE_FILL_ORANGE_2 = "kim_gauge_2";
char* G2D_RES_LIFE_FILL_GREEN_2 = "kim_fill_2";

void CFrontendLifeGauge::Init()
{
	CDualSprite* pDualSprite;
	int curIndex;
	CFrontendBank* pBank;

	CFrontendBitmapGauge::Init();

	pBank = CScene::ptable.g_FrontEndBank_00451674;
	this->spriteGauge.Install(pBank->GetG2D(0), G2D_RES_LIFE_BACK);
	this->spriteFillLife.Install(pBank->GetG2D(0), G2D_RES_LIFE_FILL_ORANGE);
	this->spriteFillHit.Install(pBank->GetG2D(0), G2D_RES_LIFE_FILL_GREEN);
	this->spriteKimHead.Install(pBank->GetG2D(0), G2D_RES_KIM_HEAD);
	this->spriteGauge2.Install(pBank->GetG2D(0), G2D_RES_LIFE_FILL_ORANGE_2);
	this->spriteFill2.Install(pBank->GetG2D(0), G2D_RES_LIFE_FILL_GREEN_2);

	curIndex = 0;
	pDualSprite = this->aDualSprites;
	do {
		pDualSprite->spriteGauge.Install(&(this->spriteGauge2).materialInfo);
		pDualSprite->spriteFill.Install(&(this->spriteFill2).materialInfo);

		curIndex = curIndex + 1;
		pDualSprite = pDualSprite + 1;
	} while (curIndex < 4);

	this->field_0x3ac = 0;

	FUN_001daff0();

	this->widgetSlotC.position = this->slotHide.position;
	this->widgetSlotC.scale = this->slotHide.scale;

	this->widgetSlotB.position = this->slotHide.position;
	this->widgetSlotB.scale = this->slotHide.scale;

	this->widgetSlotA.position = this->slotHide.position;
	this->widgetSlotA.scale = this->slotHide.scale;

	this->state = 4;

	return;
}

void CFrontendLifeGauge::Term()
{
	CDualSprite* pCVar1;
	int iVar2;

	iVar2 = 0;
	pCVar1 = this->aDualSprites;
	do {
		pCVar1->spriteGauge.Remove();
		pCVar1->spriteFill.Remove();
		iVar2 = iVar2 + 1;
		pCVar1 = pCVar1 + 1;
	} while (iVar2 < 4);

	this->spriteFill2.Remove();
	this->spriteGauge2.Remove();
	this->spriteKimHead.Remove();
	this->spriteFillHit.Remove();
	this->spriteFillLife.Remove();
	this->spriteGauge.Remove();

	SetInterface((CInterface*)0x0);

	return;
}

void CFrontendLifeGauge::UpdatePos_StateWait(float param_1)
{
	CLifeInterface* pLife;
	CFrontendLifeGauge* pCVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	edF32VECTOR2 local_10;
	edF32VECTOR2 local_8;
	bool bVar1;

	fVar5 = 1.0f;

	this->slotAlpha = 1.0f;
	fVar4 = param_1 - this->field_0x3c;

	switch (this->state) {
	case 0:
		this->state = 5;
		break;
	case 1:
		this->state = 6;
		break;
	case 2:
		this->state = 7;
		break;
	case 3:
		this->state = 4;
		this->bVisible = 0;
		break;
	case 5:
		IMPLEMENTATION_GUARD(
		pLife = (CLifeInterface*)this->pInterface;
		bVar1 = false;
		if (pLife != (CLifeInterface*)0x0) {
			fVar5 = (*pLife->pVtable->GetValue)((CInterface*)pLife);
			fVar3 = CLifeInterface::GetValueMax(pLife);
			bVar1 = fVar5 <= fVar3 * 0.1;
		}

		if ((!bVar1) && (FLOAT_0044885c < fVar4)) {
			this->state = 3;
			CWidget::MoveToNext((CWidget*)this, &this->slotHide);
		})
		break;
	case 7:
		IMPLEMENTATION_GUARD(
		if (fVar4 < DISPLAY_TIME_EXT) {
			fVar5 = fVar4 / DISPLAY_TIME_EXT;
		}
		else {
			this->field_0x378 = 0;
			if (this->field_0x3ac == 0) {
				this->state = 5;
			}
			else {
				this->state = 6;
			}
			fVar4 = CFrontend::GetTime();
			this->field_0x3c = fVar4;
		}

		edF32Vector2LERP(&local_8, &GAUGE_OFFSET, edF32VECTOR2_ARRAY_0042c670 + this->field_0x384 + -1, fVar5);
		edF32Vector2LERP(&local_10, &GAUGE_SCALE, &edF32VECTOR2_0042c6b0, fVar5);
		pCVar2 = (CFrontendLifeGauge*)((int)this + (this->field_0x380 + -1) * 0x180);
		pCVar2->aDualSprites[0].spriteGauge.bValid = 0;
		pCVar2->aDualSprites[0].spriteGauge.field_0x5c.position.x = local_8.x;
		pCVar2->aDualSprites[0].spriteGauge.field_0x5c.position.y = local_8.y;
		pCVar2 = (CFrontendLifeGauge*)((int)this + (this->field_0x380 + -1) * 0x180);
		pCVar2->aDualSprites[0].spriteGauge.bValid = 0;
		pCVar2->aDualSprites[0].spriteGauge.field_0x5c.scale.x = local_10.x;
		pCVar2->aDualSprites[0].spriteGauge.field_0x5c.scale.y = local_10.y;
		(&this->spriteFill2)[this->field_0x380 * 2].field_0x4 = 0;
		)
	}

	return;
}

void CFrontendLifeGauge::Update(float param_1)
{
	UpdateInternal();

	CWidget::Update(param_1);

	return;
}

void CFrontendLifeGauge::Draw()
{
	CLifeInterface* pLife;
	bool bVar2;
	bool bVar3;
	bool bVar4;
	int iVar5;
	float* pfVar6;
	int iVar8;
	float fVar9;
	float fVar10;
	uint local_4;

	if (this->bVisible != 0) {
		bVar3 = false;
		bVar2 = false;
		bVar4 = false;

		if ((this->widgetSlotA.position.x == (this->field_0x2b8).field_0x5c.position.x) &&
			(this->widgetSlotA.position.y == (this->field_0x2b8).field_0x5c.position.y)) {
			bVar4 = true;
		}

		if ((bVar4) && (this->widgetSlotA.scale.x == (this->field_0x2b8).field_0x5c.scale.x)) {
			bVar2 = true;
		}

		if ((bVar2) && (this->widgetSlotA.scale.y == (this->field_0x2b8).field_0x5c.scale.y)) {
			bVar3 = true;
		}

		if (!bVar3) {
			this->field_0x2b8.UpdateSlotPositionAndScale(this->widgetSlotA.position.x, this->widgetSlotA.position.y, this->widgetSlotA.scale.x, this->widgetSlotA.scale.y);
		}

		pLife = (CLifeInterface*)this->pInterface;
		if ((pLife != (CLifeInterface*)0x0) && (this->valueMax == -1.0f)) {
			this->valueMax = pLife->GetValueMax();
			UpdateField0x380();
		}

		bVar4 = GuiDList_BeginCurrent();
		if (bVar4 != false) {
			if ((this->field_0x2b8).bValid == 0) {
				this->spriteGauge.bValid = 0;
				(this->spriteKimHead).bValid = 0;
				this->spriteFillHit.bValid = 0;
				this->spriteFillLife.bValid = 0;

				for (iVar8 = 0; iVar8 < 4; iVar8 = iVar8 + 1) {
					this->aDualSprites->spriteGauge.bValid = 0;
					this->aDualSprites->spriteFill.bValid = 0;
				}
			}

			pLife = static_cast<CLifeInterface*>(this->pInterface);

			bVar4 = false;
			if (pLife != (CLifeInterface*)0x0) {
				fVar9 = pLife->GetValue();
				fVar10 = pLife->GetValueMax();
				bVar4 = fVar9 <= fVar10 * 0.1f;
			}

			if (bVar4) {
				fVar9 = cosf(Timer::GetTimer()->totalTime * 8.0f) * 64.0f + 64.0f;
				if (2.147484e+09 <= fVar9) {
					fVar9 = fVar9 - 2.147484e+09;
				}

				local_4 = ((int)fVar9 & 0xffU) << 0x10 | 0xa0000000 | ((int)fVar9 & 0xffU) << 8 | 0x7f;
			}
			else {
				local_4 = 0xa07f7f7f;
			}

			this->spriteGauge.color = local_4;

			for (iVar8 = 0; iVar8 < this->field_0x380; iVar8 = iVar8 + 1) {
				this->aDualSprites->spriteGauge.color = local_4;
				this->aDualSprites->spriteGauge.Draw(true);
			}

			for (iVar8 = 0; iVar8 < this->field_0x384; iVar8 = iVar8 + 1) {
				this->aDualSprites->spriteFill.color = local_4;
				this->aDualSprites->spriteFill.Draw(true);
			}

			this->spriteGauge.Draw(true);
			this->spriteFillHit.Draw(true);
			this->spriteFillLife.Draw(true);
			this->spriteKimHead.Draw(true);

			GuiDList_EndCurrent();
		}
	}

	return;
}

bool CFrontendLifeGauge::UpdateGauge(float param_1)
{
	bool bVar1;
	float fVar2;
	float fVar3;

	bVar1 = CFrontendBitmapGauge::UpdateGauge(param_1);

	fVar2 = this->fillLifeMin;
	fVar3 = this->fillLifeMax - fVar2;
	this->spriteFillHit.ClampX(fVar2 + fVar3 * this->field_0x5c, fVar2 + fVar3 * this->field_0x54, true);

	return bVar1;
}

void CFrontendLifeGauge::UpdatePercent(float value)
{
	IMPLEMENTATION_GUARD();
}

void CFrontendLifeGauge::FUN_001daff0()
{
	int iVar2;
	int iVar3;
	long lVar4;
	CSprite* pCVar5;
	CSprite* pCVar6;
	float fVar7;

	UpdateInterface();

	this->slotAlpha = 0.0f;
	this->bVisible = 1;
	this->field_0x3b8 = 0;

	this->spriteFillLife.ClampX(this->fillLifeMin, this->fillLifeMax);
	this->spriteFillHit.ClampX(this->fillLifeMin, this->fillLifeMax);

	this->spriteKimHead.SetParent(&this->field_0x2b8);
	this->spriteGauge.SetParent(&this->field_0x2b8);
	this->spriteFillLife.SetParent(&this->spriteGauge);
	this->spriteFillHit.SetParent(&this->spriteGauge);

	this->aDualSprites[0].spriteGauge.SetParent(&this->field_0x2b8);
	this->aDualSprites[0].spriteFill.SetParent(&this->aDualSprites[0].spriteGauge);

	this->aDualSprites[1].spriteGauge.SetParent(&this->field_0x2b8);
	this->aDualSprites[1].spriteFill.SetParent(&this->aDualSprites[1].spriteGauge);

	this->aDualSprites[2].spriteGauge.SetParent(&this->field_0x2b8);
	this->aDualSprites[2].spriteFill.SetParent(&this->aDualSprites[2].spriteGauge);

	this->aDualSprites[3].spriteGauge.SetParent(&this->field_0x2b8);
	this->aDualSprites[3].spriteFill.SetParent(&this->aDualSprites[3].spriteGauge);

	this->field_0x384 = 0;
	this->field_0x378 = 0;

	UpdateField0x380();

	return;
}

void CFrontendLifeGauge::ShowLife()
{
	switch (this->state) {
	case 3:
		MoveToNext(&this->slotShow);
		this->state = 0;
		break;
	case 4:
		MoveToNext(&this->slotShow);
		this->state = 0;
		this->bVisible = 1;
		break;
	case 5:
		this->field_0x3c = CFrontend::GetTime();
	}

	return;
}

void CFrontendLifeGauge::UpdateInterface()
{
	if (GetInterface() != (CInterface*)0x0) {
		if (GetInterface()->Manage() != 0) {
			UpdateField0x380();
			UpdatePercent(GetInterface()->GetValue());
			CLifeInterface* pLifeInterface = reinterpret_cast<CLifeInterface*>(GetInterface());
			this->valueMax = pLifeInterface->GetValueMax();

			return;
		}
	}

	this->field_0x388 = 0.0f;
	this->valueMax = -1.0f;
}

void CFrontendLifeGauge::UpdateField0x380()
{
	int iVar3 = CLevelScheduler::ScenVar_Get(0x15);

	if (iVar3 == 0) {
		trap(7);
	}

	this->field_0x380 = CLevelScheduler::ScenVar_Get(0x14) / iVar3 + -1;
}

void CFrontendLifeGauge::UpdateInternal()
{
	CInterface* pCVar1;
	CLifeInterface* pLife;
	int iVar2;
	int iVar3;
	long lVar4;
	float fVar5;
	float fVar6;

	pCVar1 = this->pInterface;
	if ((pCVar1 != (CInterface*)0x0) && (lVar4 = pCVar1->Manage(), lVar4 != 0)) {
		pLife = static_cast<CLifeInterface*>(this->pInterface);
		fVar6 = this->field_0x388;
		fVar5 = pLife->GetValue();
		if (fVar6 != fVar5) {
			ShowLife();
			UpdateField0x380();
			UpdatePercent(fVar5);
		}

		iVar2 = pLife->GetPriority();
		if ((iVar2 == 4) || (pLife->field_0x10 != 0)) {
			if (this->field_0x3b4 != 4) {
				ShowLifeAlways();
				this->field_0x3b4 = 4;
			}
		}
		else {
			if (this->field_0x3b4 == 4) {
				HideLifeAlways();
				this->field_0x3b4 = 0;
			}
		}

		fVar6 = this->valueMax;
		fVar5 = pLife->GetValue();
		if (((fVar6 != fVar5) && (fVar6 != -1.0f)) && (this->field_0x378 == 0)) {
			this->field_0x378 = 1;
			this->valueMax = pLife->GetValueMax();
			UpdateField0x380();
			ShowLifeExt();
		}
	}

	return;
}

void CFrontendLifeGauge::ShowLifeAlways()
{
	IMPLEMENTATION_GUARD();
}

void CFrontendLifeGauge::HideLifeAlways()
{
	IMPLEMENTATION_GUARD();
}

void CFrontendLifeGauge::ShowLifeExt()
{
	IMPLEMENTATION_GUARD();
}

