#include "FrontEndWidget.h"
#include "Frontend.h"
#include "MathOps.h"

edF32VECTOR2 POS_DRAW_OFF = { 0.0f, 0.0f };
edF32VECTOR2 SCALE_DRAW = { 0.25f, 0.125f };

CWidget::CWidget()
{
	this->popupTime = 1.0f;

	this->widgetSlotB.position = POS_DRAW_OFF;
	this->widgetSlotB.scale = SCALE_DRAW;

	this->widgetSlotC.position = POS_DRAW_OFF;
	this->widgetSlotC.scale = SCALE_DRAW;

	this->pInterface = (CInterface*)0x0;

	Reset();

	return;
}

void CWidget::SetInterface(CInterface* pNewInterface)
{
	this->pInterface = pNewInterface;

	return;
}

CInterface* CWidget::GetInterface()
{
	return this->pInterface;
}

float CWidget::GetPopupTime()
{
	return this->popupTime;
}

void CWidget::Reset()
{
	this->field_0x38 = 1;
	this->bVisible = 0;
	this->slotAlpha = 0.0f;
	this->field_0x3c = CFrontend::GetTime();
	this->field_0x44 = 0;

	return;
}

void CWidget::CheckpointReset()
{
	this->field_0x38 = 1;
	this->bVisible = 0;
	this->slotAlpha = 0.0f;
	this->field_0x3c = CFrontend::GetTime();
	this->field_0x44 = 0;

	return;
}

void CWidget::Init()
{
	this->field_0x38 = 1;
	this->bVisible = 0;
	this->slotAlpha = 0.0f;
	this->field_0x3c = CFrontend::GetTime();
	this->field_0x44 = 0;

	return;
}

void CWidget::Term()
{
	SetInterface((CInterface*)0x0);

	return;
}

bool CWidget::UpdatePos(float param_2)
{
	int iVar1;
	bool bVar2;
	float fVar3;

	iVar1 = this->field_0x38;
	fVar3 = this->slotAlpha;

	if (iVar1 == 0) {
		UpdatePos_StateMove(param_2);
	}
	else {
		if (iVar1 == 1) {
			UpdatePos_StateWait(param_2);
		}
	}

	bVar2 = fVar3 != this->slotAlpha;
	if (!bVar2) {
		bVar2 = this->field_0x38 != iVar1;
	}

	return bVar2;
}

void CWidget::UpdatePos_StateWait(float param_1)
{
	return;
}

void CWidget::UpdatePos_StateMove(float param_1)
{
	float fVar1;
	float fVar2;

	fVar2 = param_1 - this->field_0x3c;
	if (GetPopupTime() < fVar2) {
		this->slotAlpha = 1.0f;
		this->field_0x38 = 1;
		this->field_0x3c = param_1;

		(this->widgetSlotC).position = (this->widgetSlotB).position;
		(this->widgetSlotC).scale = (this->widgetSlotB).scale;
	}
	else {
		this->slotAlpha = fVar2 / GetPopupTime();
	}

	return;
}

bool CWidget::UpdateDisp(float param_1)
{
	bool bVar1;

	bVar1 = this->field_0x44;
	this->field_0x44 = 0;

	return bVar1;
}

void CWidget::Update(float param_1)
{
	byte cVar1;
	byte cVar2;

	cVar1 = UpdatePos(param_1);
	cVar2 = UpdateDisp(param_1);
	if (cVar1 != false || cVar2 != false) {
		CWidgetSlot::LERP(this->slotAlpha, &this->widgetSlotA, &this->widgetSlotC, &this->widgetSlotB);
	}
	return;
}

void CWidget::Draw()
{
	return;
}

void CWidget::MoveToNext(CWidgetSlot* pNext)
{
	float fVar1;

	CWidgetSlot::LERP(this->slotAlpha, &this->widgetSlotA, &this->widgetSlotC, &this->widgetSlotB);

	this->widgetSlotC.position = this->widgetSlotA.position;
	this->widgetSlotC.scale = this->widgetSlotA.scale;

	this->widgetSlotB.position = pNext->position;
	this->widgetSlotB.scale = pNext->scale;

	this->field_0x3c = CFrontend::GetTime();
	this->field_0x38 = 0;
	this->field_0x44 = 1;
	this->slotAlpha = 0.0f;

	return;
}

CWidgetSlot::CWidgetSlot()
{
	this->scale.x = 0.0f;
	this->scale.y = 0.0f;

	this->position.x = 0.0f;
	this->position.y = 0.0f;

	return;
}

void CWidgetSlot::LERP(float alpha, CWidgetSlot* pDst, CWidgetSlot* pSlotA, CWidgetSlot* pSlotB)
{
	edF32Vector2LERP(&pDst->position, &pSlotA->position, &pSlotB->position, alpha);
	edF32Vector2LERP(&pDst->scale, &pSlotA->scale, &pSlotB->scale, alpha);

	return;
}

CFrontendBitmapGauge::CFrontendBitmapGauge()
{
	this->field_0x4c = 0.5f;
	this->field_0x50 = 0.5f;

	return;
}

void CFrontendBitmapGauge::Reset()
{
	CWidget::Reset();

	this->field_0x5c = 1.0f;
	this->field_0x54 = 1.0f;
	this->field_0x58 = 0.0f;

	return;
}

void CFrontendBitmapGauge::Init()
{
	CWidget::Init();

	this->field_0x5c = 1.0f;
	this->field_0x54 = 1.0f;
	this->field_0x58 = 0.0f;

	return;
}

bool CFrontendBitmapGauge::UpdateDisp(float param_1)
{
	if (this->field_0x44 != 0) {
		this->field_0x44 = UpdateGauge(param_1);
	}

	return false;
}

bool CFrontendBitmapGauge::UpdateGauge(float param_1)
{
	bool bVar1;
	float fVar2;
	float fVar3;

	fVar2 = this->field_0x54;
	if (param_1 < fVar2) {
		bVar1 = true;
	}
	else {
		fVar3 = this->field_0x58;
		fVar2 = this->field_0x50 - this->field_0x4c * (param_1 - fVar2);

		if (fVar2 < fVar3) {
			this->field_0x50 = fVar3;
		}
		else {
			this->field_0x50 = fVar2;
		}

		bVar1 = fVar2 >= fVar3;
		this->field_0x54 = param_1;
	}

	return bVar1;
}
