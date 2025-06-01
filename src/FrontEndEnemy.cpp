#include "FrontEndEnemy.h"
#include "Actor.h"
#include "Pause.h"
#include "BootData.h"
#include "edStr.h"
#include "Frontend.h"
#include "kya.h"
#include "Rendering/DisplayList.h"
#include "edText.h"

edF32VECTOR2 ENEMY_POS_DRAW_ON = { 0.8828f, 0.874f };
edF32VECTOR2 ENEMY_POS_DRAW_OFF = { 1.125f, 0.874f };
edF32VECTOR2 ENEMY_SCALE = { 0.125f, 0.125f };

CFrontendEnemy::CFrontendEnemy()
{
	this->popupTime = 0.2f;
	this->field_0x78 = 2.0f;

	this->slotOn.position = ENEMY_POS_DRAW_ON;
	this->slotOn.scale = ENEMY_SCALE;

	this->slotOff.position = ENEMY_POS_DRAW_OFF;
	this->slotOff.scale = ENEMY_SCALE;
}

void CFrontendEnemy::SetInterface(CInterface* pNewInterface)
{
	this->pInterface = pNewInterface;

	if (GetInterface() != (CInterface*)0x0) {
		if (GetInterface()->Manage() != 0) {
			this->field_0x6c = GetInterface()->GetValue();
		}
	}

	return;
}

void CFrontendEnemy::Reset()
{
	CInterface* pCVar1;
	long lVar2;

	CWidget::Reset();

	this->slotAlpha = 0.0f;
	this->bVisible = 1;
	this->field_0x70 = 1;
	this->bDisplayDirty = 1;

	pCVar1 = this->pInterface;
	if ((pCVar1 == (CInterface*)0x0) || (lVar2 = pCVar1->Manage(), lVar2 == 0)) {
		this->field_0x6c = 0.0f;
	}
	else {
		this->field_0x6c = this->pInterface->GetValue();
	}

	this->widgetSlotC = this->slotOff;
	this->widgetSlotB = this->slotOff;
	this->widgetSlotA = this->slotOff;

	this->field_0x74 = 3;

	return;
}

void CFrontendEnemy::Init()
{
	CWidget::Init();

	this->sprite.Install(&MenuBitmaps[1].materialInfo);

	this->textStyle.SetFont(BootDataFont, false);
	this->textStyle.rgbaColour = -1;
	this->textStyle.SetShadow(0x100);
	this->textStyle.SetShadowShift(2.0f, 2.0f);
	this->textStyle.SetHorizontalAlignment(2);

	Reset();

	return;
}

void CFrontendEnemy::Term()
{
	SetInterface((CInterface*)0x0);
	this->sprite.Remove();

	return;
}

void CFrontendEnemy::UpdatePos_StateWait(float time)
{
	int iVar1;

	this->slotAlpha = 1.0;
	iVar1 = this->field_0x74;
	if (iVar1 == 2) {
		this->field_0x74 = 3;
		this->bVisible = 0;
	}
	else {
		if (iVar1 != 5) {
			if (iVar1 == 4) {
				if (this->field_0x78 < time - this->prevTime) {
					this->field_0x74 = 2;
					MoveToNext(&this->slotOff);
				}
			}
			else {
				if (iVar1 == 1) {
					this->field_0x74 = 5;
				}
				else {
					if (iVar1 == 0) {
						this->field_0x74 = 4;
					}
				}
			}
		}
	}

	return;
}

bool CFrontendEnemy::UpdateDisp(float time)
{
	bool bSuccess;
	char local_10[16];

	bSuccess = false;
	if ((this->bDisplayDirty != 0) && (this->pInterface != (CInterface*)0x0)) {
		local_10[0] = '\0';
		this->bDisplayDirty = 0;
		edStrInt2Str((int)this->field_0x6c, local_10, 5, false);
		edStrCatMulti(this->szLabel, "x", local_10, NULL);
		bSuccess = true;
	}

	return bSuccess;
}

void CFrontendEnemy::Update(float time)
{
	float currentValue;

	if ((this->pInterface != (CInterface*)0x0) && (this->pInterface->Manage() != false) && (currentValue = pInterface->GetValue(), currentValue != this->field_0x6c)) {
		int iVar2 = this->field_0x74;
		if (iVar2 == 2) {
			MoveToNext(&this->slotOn);
			this->field_0x74 = 0;
		}
		else {
			if (iVar2 == 3) {
				MoveToNext(&this->slotOn);
				this->field_0x74 = 0;
				this->bVisible = 1;
			}
			else {
				if (iVar2 == 4) {
					this->prevTime = CFrontend::GetTime();
				}
			}
		}

		this->field_0x6c = currentValue;
		this->bDisplayDirty = 1;
	}

	CWidget::Update(time);

	return;
}

float FLOAT_00448da8 = 18.0f;

void CFrontendEnemy::Draw()
{
	bool bVar1;
	edCTextStyle* pNewFont;
	float fVar2;
	float y;

	if (((this->field_0x74 != 0) && (this->bVisible != 0)) && (this->field_0x78 != 3)) {
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
			y = FLOAT_00448da8 + this->widgetSlotA.position.y * (float)gVideoConfig.screenHeight;
			pNewFont = edTextStyleSetCurrent(&this->textStyle);
			edTextDraw(fVar2, y, this->szLabel);
			edTextStyleSetCurrent(pNewFont);
			GuiDList_EndCurrent();
		}
	}

	return;
}

void CFrontendEnemy::ShowEnemy()
{
	switch (this->field_0x74) {
	case 2:
		CWidget::MoveToNext(&this->slotOn);
		this->field_0x74 = 0;
		break;
	case 3:
		CWidget::MoveToNext(&this->slotOn);
		this->field_0x74 = 0;
		this->bVisible = 1;
		break;
	case 4:
		this->prevTime = CFrontend::GetTime();
	}

	return;
}
