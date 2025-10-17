#ifndef FRONTEND_LIFE_H
#define FRONTEND_LIFE_H

#include "Types.h"
#include "FrontEndWidget.h"

class CDualSprite
{
public:
	CSprite spriteGauge;
	CSprite spriteFill;
};

class CFrontendLifeGauge : public CFrontendBitmapGauge
{
public:
	CFrontendLifeGauge();

	// CWidget
	virtual void SetInterface(CInterface* pNewInterface);
	virtual void Reset();
	virtual void Init();
	virtual void Term();
	virtual void UpdatePos_StateWait(float time);
	virtual void Update(float time);
	virtual void Draw();

	// CFrontendBitmapGauge
	virtual bool UpdateGauge(float time);

	void UpdatePercent(float value);

	void FUN_001daff0();
	void ShowLife();

	// New Function from suspected inline
	void UpdateInterface();

	// New Function from suspected inline
	void UpdateField0x380();

	void UpdateInternal();

	void ShowLifeAlways();
	void HideLifeAlways();

	void ShowLifeExt();

	CSprite field_0x2b8;
	undefined4 field_0x378;
	float valueMax;
	int nbExtraLifeGauges;
	int nbExtraFills;
	float field_0x388;

	CWidgetSlot slotShow;
	CWidgetSlot slotHide;

	int field_0x3ac;
	int state;

	int field_0x3b4;

	undefined4 field_0x3b8;

	CSprite spriteKimHead;
	CSprite spriteGauge2;
	CSprite spriteFill2;

	CDualSprite aDualSprites[4];
};

#endif // FRONTEND_LIFE_H