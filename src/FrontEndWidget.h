#ifndef FRONTEND_WIDGET_H
#define FRONTEND_WIDGET_H

#include "Types.h"
#include "Sprite.h"

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

#endif // !FRONTEND_WIDGET_H
