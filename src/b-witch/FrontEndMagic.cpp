#include "FrontEndMagic.h"

#include "Frontend.h"
#include "FrontendBank.h"
#include "FrontendDisp.h"
#include "Pause.h"
#include "LargeObject.h"
#include "Rendering/DisplayList.h"
#include "edStr.h"
#include "MathOps.h"
#include "TimeController.h"
#include "kya.h"
#include "edText.h"

edF32VECTOR2 FLOAT_00448820 = { 0.1172f, 0.35f };
edF32VECTOR2 FLOAT_00448828 = { -0.125f, 0.35f };

CFrontendMagicGauge::CFrontendMagicGauge()
	: CFrontendBitmapGauge()
{
	//this->field_0x2f4 = 0;
	//this->field_0x2f0 = 0.0;
	//this->field_0x2ec = 0.0;
	//this->field_0x2e8 = 0;

	this->field_0x2c8.position = FLOAT_00448820;
	this->field_0x2d8.position = FLOAT_00448828;

	this->popupTime = 0.2f;
	this->field_0x554 = 2.0f;
}

void CFrontendMagicGauge::SetInterface(CInterface* pNewInterface)
{
	CMagicInterface* pMagicInterface;
	long manageResult;
	float interfaceValue;

	this->pInterface = pNewInterface;

	pMagicInterface = static_cast<CMagicInterface*>(pNewInterface);
	if (pMagicInterface != (CInterface*)0x0) {
		if (pMagicInterface->Manage() != 0) {
			const float magicValue = pMagicInterface->GetValue();
			this->currentMagicValueB = (int)magicValue;
			this->currentMagicValueA = (int)magicValue;

			const float magicValueMax = pMagicInterface->GetValueMax();
			this->maxMagicValueB = (int)magicValueMax;
			this->maxMagicValueA = (int)magicValueMax;
		}
	}

	return;
}

void CFrontendMagicGauge::Reset()
{
	CFrontendBitmapGauge::Reset();

	ClearLocalData();

	this->widgetSlotC = this->field_0x2d8;
	this->widgetSlotB = this->field_0x2d8;
	this->widgetSlotA = this->field_0x2d8;

	this->state_0x2fc = 4;

	return;
}

void CFrontendMagicGauge::Init()
{
	CFrontendBitmapGauge::Init();

	this->textStyle.SetFont(BootDataFont, false);
	this->textStyle.SetShadow(0x100);
	this->textStyle.SetShadowShift(2.0f, 2.0f);
	this->textStyle.SetHorizontalAlignment(2);

	this->field_0x3d0.Install(CScene::ptable.g_FrontEndBank_00451674->GetG2D(1), 0);
	this->field_0x490.Install(CScene::ptable.g_FrontEndBank_00451674->GetG2D(2), 0);

	ClearLocalData();

	this->widgetSlotC = this->field_0x2d8;
	this->widgetSlotB = this->field_0x2d8;
	this->widgetSlotA = this->field_0x2d8;

	this->state_0x2fc = 4;
	this->field_0x558[0] = 0;
}

void CFrontendMagicGauge::Term()
{
	this->field_0x490.Remove();
	this->field_0x3d0.Remove();
	this->SetInterface((CInterface*)0x0);

	return;
}

float FLOAT_00448830 = 2.0f;

void CFrontendMagicGauge::UpdatePos_StateWait(float time)
{
	int iVar1;
	float fVar2;
	float fVar3;
	CMagicInterface* pMagicInterface;

	fVar3 = 1.0f;
	this->slotAlpha = 1.0f;
	fVar2 = time - this->prevTime;

	switch (this->state_0x2fc) {
	case 0:
		this->state_0x2fc = 6;
		break;
	case 1:
		this->state_0x2fc = 5;
		break;
	case 2:
		this->state_0x2fc = 7;
		break;
	case 3:
		this->state_0x2fc = 4;
		if (this->bVisible != 0) {
			this->bVisible = 0;
		}
		break;
	case 5:
		if (this->field_0x554 < fVar2) {
			this->state_0x2fc = 3;
			MoveToNext(&this->field_0x2d8);
		}
		break;
	case 7:
		pMagicInterface = static_cast<CMagicInterface*>(this->pInterface);
		if (fVar2 < FLOAT_00448830) {
			fVar3 = fVar2 / FLOAT_00448830;
		}
		else {
			if (this->field_0x2f8 == 0) {
				this->state_0x2fc = 5;
			}
			else {
				this->state_0x2fc = 6;
			}

			this->prevTime = CFrontend::GetTime();
			this->maxMagicValueA = (int)pMagicInterface->GetValueMax();
		}

		iVar1 = this->maxMagicValueB;
		this->maxMagicValueA = (int)((float)iVar1 + fVar3 * (pMagicInterface->GetValueMax() - (float)iVar1));
		this->bDisplayDirty = 1;
	}

	return;
}

void CFrontendMagicGauge::UpdatePos_StateMove(float time)
{
	float deltaTime;

	deltaTime = time - this->prevTime;
	if (GetPopupTime() < deltaTime) {
		this->slotAlpha = 1.0f;
		this->prevTime = time;
		this->widgetSlotC = this->widgetSlotB;
		this->posState = 1;
	}
	else {
		this->slotAlpha = deltaTime / GetPopupTime();
	}

	return;
}

bool CFrontendMagicGauge::UpdateDisp(float time)
{
	bool bUpdated;
	char acStack16[8];
	char acStack8[8];

	bUpdated = false;
	if ((this->bDisplayDirty != 0) && (this->pInterface != (CInterface*)0x0)) {
		edStrInt2Str(this->currentMagicValueA, acStack8, 3, false);
		edStrInt2Str(this->maxMagicValueA, acStack16, 3, false);
		edStrCatMulti(this->field_0x558, acStack8, "/", acStack16, 0);
		this->bDisplayDirty = 0;
		bUpdated = true;
	}

	return bUpdated;
}

void CFrontendMagicGauge::Update(float time)
{
	CheckEvents(time);

	CWidget::Update(time);

	return;
}

void CFrontendMagicGauge::Draw()
{
	if (this->bVisible != 0) {
		if (Frontend2DDList_BeginCurrent() != false) {
			DrawMagic_BonusFlare();
			FrontendDList_EndCurrent();
		}

		if (GuiDList_BeginCurrent() != false) {
			FUN_001d7020();
			GuiDList_EndCurrent();
		}
	}

	return;
}

bool CFrontendMagicGauge::UpdateGauge(float time)
{
	return false;
}

void CFrontendMagicGauge::CheckEvents(float time)
{
	CMagicInterface* pMagicInterface;
	bool isStateActive;
	int magicInteractionResult;
	float magicValue;

	pMagicInterface = static_cast<CMagicInterface*>(this->pInterface);
	if ((pMagicInterface != (CMagicInterface*)0x0) && (pMagicInterface->Manage() != 0)) {
		magicValue = pMagicInterface->GetValue();

		if (this->currentMagicValueA != (int)magicValue) {
			ShowMagic();

			this->bDisplayDirty = 1;
			this->currentMagicValueA = (int)magicValue;
		}

		if (pMagicInterface->IsActive() && this->field_0x304 == 0) {
			this->field_0x304 = 1;
			ShowMagicAlways();
			this->bDisplayDirty = 1;
		}

		if (!pMagicInterface->IsActive() && this->field_0x304 == 1) {
			this->field_0x304 = 0;
			HideMagicAlways(false);
		}

		magicInteractionResult = pMagicInterface->HasMagicInteractionAround();
		if ((magicInteractionResult != 0) || (magicValue = pMagicInterface->GetField_0x10(), magicValue < 1.5f)) {
			if (this->field_0x300 == 0) {
				this->field_0x300 = (magicInteractionResult == 0) ? 2 : 1;
				ShowMagicAlways();
			}
		}
		else {
			magicValue = pMagicInterface->GetField_0x10();
			if ((2.0f < magicValue) && (this->field_0x300 != 0)) {
				HideMagicAlways(this->field_0x300 == 1);
				this->field_0x300 = 0;
			}
		}

		magicValue = pMagicInterface->GetValueMax();
		if ((float)this->maxMagicValueA != magicValue) {
			isStateActive = (this->state_0x2fc == 2) || (this->state_0x2fc == 7);
			if (!isStateActive) {
				UpdatePos_StateWait(time);

				this->bDisplayDirty = 1;
				this->maxMagicValueB = this->maxMagicValueA;
			}
		}
	}

	return;
}

void CFrontendMagicGauge::Magic_SetDisplay(unsigned char bNewVisible)
{
	if (bNewVisible != this->bVisible) {
		this->bVisible = bNewVisible;
	}
	return;
}

void CFrontendMagicGauge::ClearLocalData()
{
	CMagicInterface* pMagicInterface;

	this->slotAlpha = 0.0f;
	this->field_0x550 = 0.0f;
	this->bVisible = 1;

	if (this->bVisible != 0) {
		this->bVisible = 0;
	}

	if (this->bVisible != 1) {
		this->bVisible = 1;
	}

	pMagicInterface = static_cast<CMagicInterface*>(this->pInterface);
	if (pMagicInterface == (CMagicInterface*)0x0) {
		this->currentMagicValueB = 0;
		this->currentMagicValueA = 0;
		this->maxMagicValueB = 0;
		this->maxMagicValueA = 0;
	}
	else {
		const float magicValue = pMagicInterface->GetValue();
		this->currentMagicValueB = (int)magicValue;
		this->currentMagicValueA = (int)magicValue;

		const float magicValueMax = pMagicInterface->GetValueMax();
		this->maxMagicValueB = (int)magicValueMax;
		this->maxMagicValueA = (int)magicValueMax;
	}

	this->state_0x2fc = -1;
	this->field_0x2f8 = 0;
	this->field_0x304 = 0;
	this->field_0x300 = 0;
	this->bDisplayDirty = 1;

	return;
}

void CFrontendMagicGauge::ShowMagic()
{
	switch (this->state_0x2fc) {
	case 3:
		MoveToNext(&this->field_0x2c8);

		this->state_0x2fc = 1;
		break;
	case 4:
		MoveToNext(&this->field_0x2c8);

		this->state_0x2fc = 1;
		if (this->bVisible != 1) {
			this->bVisible = 1;
		}
		break;
	case 5:
		this->prevTime = CFrontend::GetTime();
	}

	return;
}

void CFrontendMagicGauge::ShowMagicAlways()
{
	switch (this->state_0x2fc) {
	case 0:
		MoveToNext(&this->field_0x2c8);
		this->state_0x2fc = 0;
		this->field_0x2f8 = this->field_0x2f8 + 1;
		break;
	case 1:
		this->state_0x2fc = 0;
		this->field_0x2f8 = this->field_0x2f8 + 1;
		break;
	case 3:
		MoveToNext(&this->field_0x2c8);
		this->state_0x2fc = 0;
		this->field_0x2f8 = this->field_0x2f8 + 1;
		break;
	case 4:
		MoveToNext(&this->field_0x2c8);
		this->state_0x2fc = 0;
		if (this->bVisible != 1) {
			this->bVisible = 1;
		}
		this->field_0x2f8 = this->field_0x2f8 + 1;
		break;
	case 5:
		this->state_0x2fc = 6;
		this->field_0x2f8 = this->field_0x2f8 + 1;
		break;
	case 6:
		this->field_0x2f8 = this->field_0x2f8 + 1;
	}

	return;
}

void CFrontendMagicGauge::HideMagicAlways(bool param_2)
{
	switch (this->state_0x2fc) {
	case 0:
		this->field_0x2f8 = this->field_0x2f8 + -1;

		if (this->field_0x2f8 == 0) {
			if (param_2 == false) {
				this->state_0x2fc = 1;
			}
			else {
				MoveToNext(&this->field_0x2d8);
				this->state_0x2fc = 3;
			}
		}
		break;
	case 6:
		this->field_0x2f8 = this->field_0x2f8 + -1;
		if (this->field_0x2f8 == 0) {
			if (param_2 == false) {
				this->prevTime = CFrontend::GetTime();
				this->state_0x2fc = 5;
			}
			else {
				MoveToNext(&this->field_0x2d8);
				this->state_0x2fc = 3;
			}
		}
	}

	return;
}

float FLOAT_00428e60 = 0.6f;
float FLOAT_00428e68 = 24.0f;
float FLOAT_00428e70 = 7.0f;
float FLOAT_00428e78 = 0.05f;

void CFrontendMagicGauge::DrawMagic_BonusFlare()
{
	bool isCoordinateComputed;
	float angle;
	float adjustedAngle;
	float cosValue1;
	float cosValue2;
	edF32VECTOR4 sceneCoordinate;

	angle = edF32Between_0_2Pi(this->field_0x550 + FLOAT_00428e70 * Timer::GetTimer()->lastFrameTime);
	this->field_0x550 = angle;

	cosValue1 = FLOAT_00428e78 * cosf(angle);
	cosValue2 = FLOAT_00428e78 * sinf(angle);

	edDListLoadIdentity();
	isCoordinateComputed = CScene::ptable.g_FrontendManager_00451680->ComputeSceneCoordinate(14.0f, &sceneCoordinate, &this->widgetSlotA.position);

	if (isCoordinateComputed) {
		edDListTranslate(sceneCoordinate.x, sceneCoordinate.y, sceneCoordinate.z);
		edDListUseMaterial(&this->field_0x3d0.materialInfo);
		edDListBegin(0.0f, 0.0f, 0.0f, 0xb, 1);
		edDListTexCoo2f(0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 1.0f);
		edDListWidthHeight2f(0.15f, 0.15f);
		edDListColor4u8(0x80, 0x80, 0x80, static_cast<uint8_t>((rand() / 2147483648.0f) * 96.0f + 64.0f));
		edDListVertex4f(cosValue1, cosValue2, 0.0f, 0.0f);
		edDListEnd();

		float currentTime = CFrontend::GetTime();
		uint frameIndex = static_cast<uint>(FLOAT_00428e68 * currentTime) & 0xf;

		if (static_cast<int>(FLOAT_00428e68 * currentTime) < 0 && frameIndex != 0) {
			frameIndex -= 0x10;
		}

		uint textureIndex = frameIndex & 7;
		if (static_cast<int>(frameIndex) < 0 && textureIndex != 0) {
			textureIndex -= 8;
		}

		float textureOffsetX = static_cast<float>(textureIndex) * 0.125f;
		int textureRow = static_cast<int>(frameIndex);
		if (textureRow < 0) {
			textureRow += 7;
		}
		float textureOffsetY = static_cast<float>(textureRow >> 3) * 0.5f;

		edDListUseMaterial(&this->field_0x490.materialInfo);
		edDListBegin(0.0f, 0.0f, 0.0f, 0xb, 1);
		edDListTexCoo2f(textureOffsetX, textureOffsetY);
		edDListTexCoo2f(textureOffsetX + 0.125f, textureOffsetY + 0.5f);
		edDListWidthHeight2f(FLOAT_00428e60, FLOAT_00428e60);
		edDListColor4u8(0x7f, 0x7f, 0x7f, 0x7f);
		edDListVertex4f(0.0f, 0.0f, 0.0f, 0.0f);
		edDListEnd();
	}

	return;
}

void CFrontendMagicGauge::FUN_001d7020()
{
	uint colorValue;
	bool isSpecialState;
	edCTextStyle* previousTextStyle;
	long colorTransition;
	float widgetYPosition;
	float currentValue;
	float maxValue;
	float screenHeight;
	float widgetXPosition;
	CMagicInterface* magicInterface;

	magicInterface = static_cast<CMagicInterface*>(this->pInterface);
	if ((magicInterface != (CMagicInterface*)0x0) && (this->field_0x558[0] != '\0')) {
		widgetYPosition = this->widgetSlotA.position.y;
		colorTransition = -1;
		isSpecialState = false;
		screenHeight = static_cast<float>(gVideoConfig.screenHeight);
		widgetXPosition = this->widgetSlotA.position.x * static_cast<float>(gVideoConfig.screenWidth);
		currentValue = magicInterface->GetValue();
		maxValue = magicInterface->GetValueMax();

		if (static_cast<int>(currentValue) == static_cast<int>(maxValue)) {
			isSpecialState = true;
			colorTransition = -0x4f5000;
		}
		else if (static_cast<int>(currentValue) == 0) {
			isSpecialState = true;
			colorTransition = -0x4fcfd0;
		}

		if (isSpecialState) {
			float timerValue = Timer::GetTimer()->totalTime * 6.0f;
			float transitionFactor = cosf(fabsf(timerValue)) * 0.5f + 0.5f;
			colorValue = static_cast<uint>(colorTransition);
			colorTransition = static_cast<long>(
				(((0xff - static_cast<int>(transitionFactor * 255.0f)) * 0xff +
					((colorValue >> 24) & 0xff) * static_cast<int>(transitionFactor * 255.0f)) << 24) |
				(((0xff - static_cast<int>(transitionFactor * 255.0f)) * 0xff +
					((colorValue >> 16) & 0xff) * static_cast<int>(transitionFactor * 255.0f)) << 16) |
				(((0xff - static_cast<int>(transitionFactor * 255.0f)) * 0xff +
					((colorValue >> 8) & 0xff) * static_cast<int>(transitionFactor * 255.0f)) << 8) |
				((0xff - static_cast<int>(transitionFactor * 255.0f)) * 0xff +
					(colorValue & 0xff) * static_cast<int>(transitionFactor * 255.0f)));
		}

		this->textStyle.rgbaColour = static_cast<uint>((static_cast<ulong>(colorTransition) << 40) >> 32) | static_cast<uint>(colorTransition >> 24);
		previousTextStyle = edTextStyleSetCurrent(&this->textStyle);
		edTextDraw(widgetXPosition, widgetYPosition * screenHeight + 16.0f, this->field_0x558);
		edTextStyleSetCurrent(previousTextStyle);
	}

	return;
}
