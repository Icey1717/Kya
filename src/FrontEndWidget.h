#ifndef FRONTEND_WIDGET_H
#define FRONTEND_WIDGET_H

#include "Types.h"
#include "Sprite.h"
#include "Rendering/edCTextStyle.h"

class CInterface;

class CWidget
{
public:
	CWidget();

	virtual void SetInterface(CInterface* pNewInterface);
	virtual CInterface* GetInterface();
	virtual float GetPopupTime();
	virtual void Reset();
	virtual void CheckpointReset();
	virtual void Init();
	virtual void Term();
	virtual bool UpdatePos(float param_2);
	virtual void UpdatePos_StateWait(float param_1);
	virtual void UpdatePos_StateMove(float param_1);
	virtual bool UpdateDisp(float param_1);
	virtual void Update(float param_1);
	virtual void Draw();

	void MoveToNext(CWidgetSlot* pNext);

	CWidgetSlot widgetSlotA;
	float popupTime;

	CWidgetSlot widgetSlotB;
	CWidgetSlot widgetSlotC;

	int field_0x38;

	float field_0x3c;
	float slotAlpha;

	bool field_0x44;
	byte bVisible;

	CInterface* pInterface;
};

class CFrontendBitmapGauge : public CWidget
{
public:
	CFrontendBitmapGauge();

	// CWidget
	virtual void Reset();
	virtual void Init();
	virtual bool UpdateDisp(float param_1);

	virtual bool UpdateGauge(float param_1);

	CSprite spriteGauge;
	CSpriteWindow spriteFillLife;
	CSpriteWindow spriteFillHit;

	float field_0x4c;
	float field_0x50;

	float field_0x54;
	float field_0x58;
	float field_0x5c;

	float fillLifeMin;
	float fillLifeMax;
};

class CFrontendLabelText : public CWidget
{
public:
	CFrontendLabelText();

	char szLabel[32];
};

class CFrontendAction : public CFrontendLabelText
{
public:
	CFrontendAction();

	virtual void Reset();
	virtual void Init();
	virtual void UpdatePos_StateWait(float param_1);
	virtual bool UpdateDisp(float param_1);
	virtual void Draw();

	void SetActionA(int actionId);
	void SetActionB(int actionId);

	edCTextStyle textStyle;

	int field_0x130;
	float field_0x134;

	int curActionId;
	int otherActionId;

	CWidgetSlot slotOn;
	CWidgetSlot slotOff;

	char szOtherText[32];
};

#endif // !FRONTEND_WIDGET_H
