#ifndef FRONTEND_WIDGET_H
#define FRONTEND_WIDGET_H

#include "Types.h"
#include "SpriteWidget.h"
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
	virtual bool UpdatePos(float time);
	virtual void UpdatePos_StateWait(float time);
	virtual void UpdatePos_StateMove(float time);
	virtual bool UpdateDisp(float time);
	virtual void Update(float time);
	virtual void Draw();

	void MoveToNext(CWidgetSlot* pNext);

	CWidgetSlot widgetSlotA;
	float popupTime;

	CWidgetSlot widgetSlotB;
	CWidgetSlot widgetSlotC;

	int posState;

	float prevTime;
	float slotAlpha;

	bool bDisplayDirty;
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
	virtual bool UpdateDisp(float time);

	virtual bool UpdateGauge(float time);

	CSprite spriteGauge;
	CSpriteWindow spriteFillLife;
	CSpriteWindow spriteFillHit;

	float field_0x4c;
	float field_0x50;

	float hitAlpha;
	float field_0x58;
	float fillAlpha;

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
