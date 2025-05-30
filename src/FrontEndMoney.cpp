#include "FrontEndMoney.h"
#include "Actor.h"
#include "FrontEndBank.h"
#include "Pause.h"
#include "TimeController.h"
#include "MathOps.h"
#include "edStr.h"
#include "Frontend.h"
#include "Rendering/DisplayList.h"
#include "kya.h"
#include "edText.h"

edF32VECTOR2 MONEY_POS_DRAW_ON = { 0.1172f, 0.874f };
edF32VECTOR2 MONEY_POS_DRAW_OFF = { -0.125f, 0.874f };
edF32VECTOR2 MONEY_SCALE = { 0.125f, 0.125f };

CFrontendMoney::CFrontendMoney()
{
	this->popupTime = 0.2f;
	this->field_0x7c = 2.0f;

	this->slotOn.position = MONEY_POS_DRAW_ON;
	this->slotOn.scale = MONEY_SCALE;

	this->slotOff.position = MONEY_POS_DRAW_OFF;
	this->slotOff.scale = MONEY_SCALE;
}

void CFrontendMoney::SetInterface(CInterface* pNewInterface)
{
	this->pInterface = pNewInterface;

	if (GetInterface() != (CInterface*)0x0) {
		if (GetInterface()->Manage() != 0) {
			this->field_0x70 = GetInterface()->GetValue();
		}
	}

	return;
}

void CFrontendMoney::Reset()
{
	CInterface* pCVar1;
	long lVar2;
	float fVar3;

	CWidget::Reset();

	this->slotAlpha = 0.0f;
	this->bVisible = 1;
	this->field_0x74 = 1;
	this->bDisplayDirty = 1;

	pCVar1 = this->pInterface;
	if ((pCVar1 == (CInterface*)0x0) || (lVar2 = pCVar1->Manage(), lVar2 == 0)) {
		this->field_0x70 = 0.0f;
		this->field_0x6c = 0.0f;
	}
	else {
		pCVar1 = this->pInterface;
		fVar3 = pCVar1->GetValue();
		this->field_0x70 = fVar3;
		this->field_0x6c = fVar3;
	}

	this->widgetSlotC = this->slotOff;
	this->widgetSlotB = this->slotOff;
	this->widgetSlotA = this->slotOff;

	this->field_0x78 = 4;

	return;
}

void CFrontendMoney::Init()
{
	CWidget::Init();
	this->sprite.Install(CScene::ptable.g_FrontEndBank_00451674->GetG2D(0), "money");

	this->textStyle.SetFont(BootDataFont, false);
	this->textStyle.rgbaColour = -1;
	this->textStyle.SetShadow(0x100);
	this->textStyle.SetShadowShift(2.0f, 2.0f);
	this->textStyle.SetHorizontalAlignment(2);
	
	Reset();

	return;
}

void CFrontendMoney::Term()
{
	SetInterface((CInterface*)0x0);
	this->sprite.Remove();

	return;
}

void CFrontendMoney::UpdatePos_StateWait(float time)
{
	CInterface* pCVar1;
	bool bVar2;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;

	this->slotAlpha = 1.0f;
	fVar7 = time - this->prevTime;
	switch (this->field_0x78) {
	case 0:
		this->field_0x78 = 5;
		break;
	case 1:
		this->field_0x78 = 6;
		break;
	case 2:
		this->field_0x78 = 7;
		break;
	case 3:
		this->field_0x78 = 4;
		this->bVisible = 0;
		break;
	case 5:
		if (this->field_0x7c < fVar7) {
			this->field_0x78 = 3;
			MoveToNext(&this->slotOff);
		}
		break;
	case 7:
		fVar4 = this->pInterface->GetValue();
		fVar5 = this->field_0x70;
		if (this->field_0x7c < fVar7) {
			this->field_0x78 = 5;
			this->prevTime = time;
			this->field_0x70 = fVar4;
		}
		else {
			fVar6 = edFIntervalLERP(fVar7, 0.0f, this->field_0x7c, this->field_0x6c, fVar4);
			this->field_0x70 = fVar6;
		}
		if (fVar4 - this->field_0x6c < 15.0f) {
			bVar2 = (int)this->field_0x70 != (int)fVar5;
		}
		else {
			fVar4 = (fVar7 * 15.0f) / this->field_0x7c;
			bVar2 = (int)fVar4 != (int)(((fVar7 - Timer::GetTimer()->lastFrameTime) * 15.0f) / this->field_0x7c);
		}
		if (bVar2) {
			IMPLEMENTATION_GUARD_AUDIO(
			CFrontendSamplePlayer::PlaySample(1.0f, (float*)(CScene::ptable.g_FrontendManager_00451680)->field_0x78, 4, 0);)
		}

		this->bDisplayDirty = 1;
	}

	return;
}

bool CFrontendMoney::UpdateDisp(float time)
{
	bool bSuccess;
	char local_10[16];

	bSuccess = false;
	if ((this->bDisplayDirty != 0) && (this->pInterface != (CInterface*)0x0)) {
		local_10[0] = '\0';
		this->bDisplayDirty = 0;
		edStrInt2Str((int)this->field_0x70, local_10, 5, false);
		edStrCatMulti(this->szLabel, "x", local_10, NULL);
		bSuccess = true;
	}

	return bSuccess;
}

void CFrontendMoney::Update(float time)
{
	uint uVar2;
	long lVar3;
	float currentValue;
	float previousValue;

	if ((this->pInterface != (CInterface*)0x0) && (this->pInterface->Manage() != false)) {
		currentValue = this->pInterface->GetValue();
		previousValue = this->field_0x70;

		if ((previousValue != currentValue) && ((uVar2 = this->field_0x78, uVar2 != 2 && (uVar2 != 7)))) {
			if ((currentValue - previousValue <= 5.0f) || (currentValue < previousValue)) {
				switch (uVar2) {
				case 3:
					MoveToNext(&this->slotOn);
					this->field_0x78 = 0;
					break;
				case 4:
					MoveToNext(&this->slotOn);
					this->field_0x78 = 0;
					this->bVisible = 1;
					break;
				case 5:
					this->prevTime = CFrontend::GetTime();
				}

				this->field_0x70 = currentValue;
				this->bDisplayDirty = 1;
			}
			else {
				FUN_00371c00();
				this->field_0x6c = (float)(int)this->field_0x70;
			}
		}
	}

	CWidget::Update(time);

	return;
}

float FLOAT_00448ba0 = 18.0f;

void CFrontendMoney::Draw()
{
	bool bVar1;
	edCTextStyle* pNewFont;
	float fVar2;
	float y;

	if (((this->field_0x74 != 0) && (this->bVisible != 0)) && (this->field_0x78 != 4)) {
		bVar1 = this->widgetSlotA.position.x == (this->sprite).field_0x5c.position.x;

		if ((bVar1) && (bVar1 = true, this->widgetSlotA.position.y != (this->sprite).field_0x5c.position.y)) {
			bVar1 = false;
		}

		if ((bVar1) && (bVar1 = true, this->widgetSlotA.scale.x != (this->sprite).field_0x5c.scale.x)) {
			bVar1 = false;
		}

		if ((bVar1) && (bVar1 = true, this->widgetSlotA.scale.y != (this->sprite).field_0x5c.scale.y)) {
			bVar1 = false;
		}

		if (!bVar1) {
			(this->sprite).bValid = false;
			(this->sprite).field_0x5c = this->widgetSlotA;
		}

		bVar1 = GuiDList_BeginCurrent();
		if (bVar1 != false) {
			this->sprite.Draw(1);
			fVar2 = this->widgetSlotA.position.x * (float)gVideoConfig.screenWidth;
			y = FLOAT_00448ba0 + this->widgetSlotA.position.y * (float)gVideoConfig.screenHeight;
			pNewFont = edTextStyleSetCurrent(&this->textStyle);
			edTextDraw(fVar2, y, this->szLabel);
			edTextStyleSetCurrent(pNewFont);
			GuiDList_EndCurrent();
		}
	}

	return;
}

void CFrontendMoney::FUN_00371c00()
{
	switch (this->field_0x78) {
	case 0:
		this->field_0x78 = 2;
		break;
	case 3:
		CWidget::MoveToNext(&this->slotOn);
		this->field_0x78 = 2;
		break;
	case 4:
		CWidget::MoveToNext(&this->slotOn);
		this->field_0x78 = 2;
		this->bVisible = 1;
		break;
	case 5:
		this->prevTime = CFrontend::GetTime();
		this->field_0x78 = 7;
	}
	return;
}

void CFrontendMoney::ShowMoney()
{
	switch (this->field_0x78) {
	case 3:
		CWidget::MoveToNext(&this->slotOn);
		this->field_0x78 = 0;
		break;
	case 4:
		CWidget::MoveToNext(&this->slotOn);
		this->field_0x78 = 0;
		this->bVisible = 1;
		break;
	case 5:
		this->prevTime = CFrontend::GetTime();
	}

	return;
}

void CFrontendMoney::ShowMoneyAlways()
{
	switch (this->field_0x78) {
	case 0:
		this->field_0x78 = 1;
		break;
	case 2:
		this->field_0x78 = 1;
		break;
	case 3:
		CWidget::MoveToNext(&this->slotOn);
		this->field_0x78 = 1;
		break;
	case 4:
		CWidget::MoveToNext(&this->slotOn);
		this->field_0x78 = 1;
		this->bVisible = 1;
		break;
	case 5:
		this->field_0x78 = 6;
		break;
	case 7:
		this->field_0x78 = 6;
	}
	return;
}

void CFrontendMoney::HideMoneyAlways()
{
	switch (this->field_0x78) {
	case 1:
		CWidget::MoveToNext(&this->slotOff);
		this->field_0x78 = 3;
		break;
	case 6:
		this->prevTime = CFrontend::GetTime();
		this->field_0x78 = 5;
	}
	return;
}
