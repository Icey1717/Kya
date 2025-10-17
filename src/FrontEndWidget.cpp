#include "FrontEndWidget.h"
#include "Frontend.h"
#include "MathOps.h"
#include "Pause.h"
#include "LocalizationManager.h"
#include "edStr.h"
#include "Rendering/DisplayList.h"
#include "Rendering/edCTextFormat.h"
#include "kya.h"

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
	this->posState = 1;
	this->bVisible = 0;
	this->slotAlpha = 0.0f;
	this->prevTime = CFrontend::GetTime();
	this->bDisplayDirty = 0;

	return;
}

void CWidget::CheckpointReset()
{
	this->posState = 1;
	this->bVisible = 0;
	this->slotAlpha = 0.0f;
	this->prevTime = CFrontend::GetTime();
	this->bDisplayDirty = 0;

	return;
}

void CWidget::Init()
{
	this->posState = 1;
	this->bVisible = 0;
	this->slotAlpha = 0.0f;
	this->prevTime = CFrontend::GetTime();
	this->bDisplayDirty = 0;

	return;
}

void CWidget::Term()
{
	SetInterface((CInterface*)0x0);

	return;
}

bool CWidget::UpdatePos(float time)
{
	int iVar1;
	bool bVar2;
	float fVar3;

	iVar1 = this->posState;
	fVar3 = this->slotAlpha;

	if (iVar1 == 0) {
		UpdatePos_StateMove(time);
	}
	else {
		if (iVar1 == 1) {
			UpdatePos_StateWait(time);
		}
	}

	bVar2 = fVar3 != this->slotAlpha;
	if (!bVar2) {
		bVar2 = this->posState != iVar1;
	}

	return bVar2;
}

void CWidget::UpdatePos_StateWait(float time)
{
	return;
}

void CWidget::UpdatePos_StateMove(float time)
{
	float deltaTime;

	deltaTime = time - this->prevTime;
	if (GetPopupTime() < deltaTime) {
		this->slotAlpha = 1.0f;
		this->posState = 1;
		this->prevTime = time;

		this->widgetSlotC = this->widgetSlotB;
	}
	else {
		this->slotAlpha = deltaTime / GetPopupTime();
	}

	return;
}

bool CWidget::UpdateDisp(float time)
{
	bool bVar1;

	bVar1 = this->bDisplayDirty;
	this->bDisplayDirty = 0;

	return bVar1;
}

void CWidget::Update(float time)
{
	byte cVar1;
	byte cVar2;

	cVar1 = UpdatePos(time);
	cVar2 = UpdateDisp(time);
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
	CWidgetSlot::LERP(this->slotAlpha, &this->widgetSlotA, &this->widgetSlotC, &this->widgetSlotB);

	this->widgetSlotC.position = this->widgetSlotA.position;
	this->widgetSlotC.scale = this->widgetSlotA.scale;

	this->widgetSlotB.position = pNext->position;
	this->widgetSlotB.scale = pNext->scale;

	this->prevTime = CFrontend::GetTime();
	this->posState = 0;
	this->bDisplayDirty = 1;
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

	this->fillAlpha = 1.0f;
	this->hitAlpha = 1.0f;
	this->field_0x58 = 0.0f;

	return;
}

void CFrontendBitmapGauge::Init()
{
	CWidget::Init();

	this->fillAlpha = 1.0f;
	this->hitAlpha = 1.0f;
	this->field_0x58 = 0.0f;

	return;
}

bool CFrontendBitmapGauge::UpdateDisp(float time)
{
	if (this->bDisplayDirty != 0) {
		this->bDisplayDirty = UpdateGauge(time);
	}

	return false;
}

bool CFrontendBitmapGauge::UpdateGauge(float time)
{
	bool bVar1;
	float fVar2;
	float fVar3;

	fVar2 = this->field_0x58;
	if (time < fVar2) {
		bVar1 = true;
	}
	else {
		fVar3 = this->fillAlpha;
		fVar2 = this->hitAlpha - this->field_0x50 * (time - fVar2);

		if (fVar2 < fVar3) {
			this->hitAlpha = fVar3;
		}
		else {
			this->hitAlpha = fVar2;
		}

		bVar1 = fVar2 >= fVar3;
		this->field_0x58 = time;
	}

	return bVar1;
}

CFrontendLabelText::CFrontendLabelText()
	: CWidget()
{
	this->szLabel[0] = '\0';

	return;
}

edF32VECTOR2 ACTION_POS_DRAW_ON = { 0.5f, 0.1f };
edF32VECTOR2 ACTION_POS_DRAW_OFF = { 0.5f, -0.1f };

CFrontendAction::CFrontendAction()
	: CFrontendLabelText()
{
	this->textStyle.Reset();

	this->popupTime = 0.1f;
	this->field_0x134 = 0.5f;

	this->slotOn.position = ACTION_POS_DRAW_ON;
	this->slotOff.position = ACTION_POS_DRAW_OFF;

	this->szOtherText[0] = '\0';
}

void CFrontendAction::Reset()
{
	CWidget::Reset();

	this->curActionId = 0;
	this->otherActionId = 0;

	this->slotAlpha = 0.0f;

	this->widgetSlotC.position = this->slotOff.position;
	this->widgetSlotC.scale = this->slotOff.scale;

	this->widgetSlotB.position = this->slotOff.position;
	this->widgetSlotB.scale = this->slotOff.scale;

	this->widgetSlotA.position = this->slotOff.position;
	this->widgetSlotA.scale = this->slotOff.scale;

	this->field_0x130 = 2;

	return;
}

void CFrontendAction::Init()
{
	this->textStyle.SetFont(BootDataFont, false);
	(this->textStyle).rgbaColour = -1;
	this->textStyle.SetShadow(0x100);
	this->textStyle.SetShadowShift(0x40000000, 0x40000000);
	this->textStyle.SetHorizontalAlignment(2);
	this->curActionId = 0;
	this->otherActionId = 0;

	this->slotAlpha = 0.0f;

	this->widgetSlotC.position = this->slotOff.position;
	this->widgetSlotC.scale = this->slotOff.scale;

	this->widgetSlotB.position = this->slotOff.position;
	this->widgetSlotB.scale = this->slotOff.scale;

	this->widgetSlotA.position = this->slotOff.position;
	this->widgetSlotA.scale = this->slotOff.scale;

	this->field_0x130 = 2;

	return;
}

void CFrontendAction::UpdatePos_StateWait(float param_1)
{
	this->slotAlpha = 1.0f;

	if (this->field_0x130 == 1) {
		this->field_0x130 = 2;
		this->bVisible = 0;
	}
	else {
		if (this->field_0x130 == 0) {
			this->field_0x130 = 3;
		}
	}

	return;
}

bool CFrontendAction::UpdateDisp(float time)
{
	bool bSuccess;
	CLanguageManager* pLanguageManager;
	char* pcVar2;

	bSuccess = false;
	if (this->bDisplayDirty != 0) {
		pLanguageManager = CLanguageManager::GetLanguageManager();

		pcVar2 = pLanguageManager->GetActionString(this->curActionId);
		if (pcVar2 == (char*)0x0) {
			this->szLabel[0] = '\0';
		}
		else {
			edStrCopy(this->szLabel, pcVar2);
		}

		pcVar2 = pLanguageManager->GetOtherActionString(this->otherActionId);
		if (pcVar2 == (char*)0x0) {
			this->szOtherText[0] = '\0';
		}
		else {
			edStrCopy(this->szOtherText, pcVar2);
		}

		this->bDisplayDirty = 0;
		bSuccess = true;
	}

	return bSuccess;
}

float FLOAT_004488c8 = 3.0f;
uint UINT_00448f28 = 0x0;
uint UINT_004488c8 = 0x40101030;

void CFrontendAction::Draw()
{
	bool bVar1;
	edCTextStyle* pNewFont;
	float x;
	float y;

	if ((((this->bVisible != 0) && ((GameFlags & 0x3c) == 0)) && ((this->szLabel[0] != '\0' || (this->szOtherText[0] != '\0')))) && (bVar1 = GuiDList_BeginCurrent(), bVar1 != false)) {
		x = (float)gVideoConfig.screenWidth * this->widgetSlotA.position.x;
		y = (float)gVideoConfig.screenHeight * this->widgetSlotA.position.y;

		pNewFont = edTextStyleSetCurrent(&this->textStyle);
		if (this->szLabel[0] != '\0') {
			edCTextFormat textFormat;
			textFormat.FormatString(this->szLabel);
			CPauseManager::DrawRectangleBorder(((float)gVideoConfig.screenWidth + 6.0f) * 0.5f, y + textFormat.field_0xc * 0.5f,
				FLOAT_004488c8 * 2.0f + textFormat.field_0x8,
				FLOAT_004488c8 * 2.0f + textFormat.field_0xc, FLOAT_004488c8, FLOAT_004488c8,
				UINT_004488c8, UINT_00448f28, 0);

			textFormat.Display(x, y);
			y = y + FLOAT_004488c8 * 2.0f + textFormat.field_0xc;
		}

		if (this->szOtherText[0] != '\0') {
			edCTextFormat auStack10784;
			auStack10784.FormatString(this->szOtherText);
			CPauseManager::DrawRectangleBorder(((float)gVideoConfig.screenWidth + 6.0f) * 0.5f, y + auStack10784.field_0xc * 0.5f,
				FLOAT_004488c8 * 2.0f + auStack10784.field_0x8,
				FLOAT_004488c8 * 2.0f + auStack10784.field_0xc, FLOAT_004488c8, FLOAT_004488c8,
				UINT_004488c8, UINT_00448f28, 0);

			auStack10784.Display(x, y);
		}

		edTextStyleSetCurrent(pNewFont);
		GuiDList_EndCurrent();
	}

	return;
}

void CFrontendAction::SetActionA(int actionId)
{
	int iVar1;
	float fVar2;

	if ((this->otherActionId != actionId) && (iVar1 = this->field_0x130, iVar1 != 1)) {
		if (iVar1 == 2) {
			this->otherActionId = actionId;
			CWidgetSlot::LERP(this->slotAlpha, &this->widgetSlotA, &this->widgetSlotC, &this->widgetSlotB);

			this->widgetSlotC.position = this->widgetSlotA.position;
			this->widgetSlotC.scale = this->widgetSlotA.scale;

			this->widgetSlotB.position = this->slotOn.position;
			this->widgetSlotB.scale = this->slotOn.scale;

			this->prevTime = CFrontend::GetTime();
			this->posState = 0;
			this->bDisplayDirty = 1;
			this->slotAlpha = 0.0f;
			this->field_0x130 = 0;
			this->bVisible = 1;
		}
		else {
			if (iVar1 == 3) {
				CWidgetSlot::LERP(this->slotAlpha, &this->widgetSlotA, &this->widgetSlotC, &this->widgetSlotB);
				this->widgetSlotC.position = this->widgetSlotA.position;
				this->widgetSlotC.scale = this->widgetSlotA.scale;

				this->widgetSlotB.position = this->slotOff.position;
				this->widgetSlotB.scale = this->slotOff.scale;

				this->prevTime = CFrontend::GetTime();
				this->posState = 0;
				this->bDisplayDirty = 1;
				this->slotAlpha = 0.0f;
				this->field_0x130 = 1;
			}
		}
	}
	return;
}

void CFrontendAction::SetActionB(int actionId)
{
	int iVar1;
	float fVar2;

	if (this->curActionId != actionId) {
		if ((CLanguageManager::_commands[this->curActionId] == 0) && (CLanguageManager::_commands[actionId] == 0)) {
			this->curActionId = actionId;
		}
		else {
			iVar1 = this->field_0x130;
			if (iVar1 != 1) {
				if (iVar1 == 2) {
					this->curActionId = actionId;
					CWidgetSlot::LERP(this->slotAlpha, &this->widgetSlotA, &this->widgetSlotC, &this->widgetSlotB);

					this->widgetSlotC.position = this->widgetSlotA.position;
					this->widgetSlotC.scale = this->widgetSlotA.scale;

					this->widgetSlotB.position = this->slotOn.position;
					this->widgetSlotB.scale = this->slotOn.scale;

					this->prevTime = CFrontend::GetTime();
					this->posState = 0;
					this->bDisplayDirty = 1;
					this->slotAlpha = 0.0f;
					this->field_0x130 = 0;
					this->bVisible = 1;
				}
				else {
					if (iVar1 == 3) {
						CWidgetSlot::LERP(this->slotAlpha, &this->widgetSlotA, &this->widgetSlotC, &this->widgetSlotB);

						this->widgetSlotC.position = this->widgetSlotA.position;
						this->widgetSlotC.scale = this->widgetSlotA.scale;

						this->widgetSlotB.position = this->slotOff.position;
						this->widgetSlotB.scale = this->slotOff.scale;

						this->prevTime = CFrontend::GetTime();
						this->posState = 0;
						this->bDisplayDirty = 1;
						this->slotAlpha = 0.0f;
						this->field_0x130 = 1;
					}
				}
			}
		}
	}
	return;
}
