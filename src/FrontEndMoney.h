#ifndef FRONTEND_MONEY_H
#define FRONTEND_MONEY_H

#include "Types.h"
#include "FrontEndWidget.h"
#include "Rendering/edCTextStyle.h"
#include "Sprite.h"

class CFrontendMoney : public CFrontendLabelText
{
public:
	CFrontendMoney();

	virtual void SetInterface(CInterface* pNewInterface);
	virtual void Reset();
	virtual void Init();
	virtual void Term();
	virtual void UpdatePos_StateWait(float time);
	virtual bool UpdateDisp(float time);
	virtual void Update(float time);
	virtual void Draw();

	void FUN_00371c00();

	void ShowMoney();

	void ShowMoneyAlways();
	void HideMoneyAlways();

	float field_0x6c;
	float field_0x70;

	int field_0x74;
	int field_0x78;
	float field_0x7c;

	CWidgetSlot slotOn;
	CWidgetSlot slotOff;

	edCTextStyle textStyle;
	CSprite sprite;
};

#endif // FRONTEND_MONEY_H
