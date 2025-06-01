#ifndef FRONTEND_ENEMY_H
#define FRONTEND_ENEMY_H

#include "Types.h"
#include "FrontEndWidget.h"

class CFrontendEnemy : public CFrontendLabelText
{
public:
	CFrontendEnemy();

	virtual void SetInterface(CInterface* pNewInterface);
	virtual void Reset();
	virtual void Init();
	virtual void Term();
	virtual void UpdatePos_StateWait(float time);
	virtual bool UpdateDisp(float time);
	virtual void Update(float time);
	virtual void Draw();

	void ShowEnemy();

	float field_0x6c;
	int field_0x70;
	int field_0x74;
	float field_0x78;

	CWidgetSlot slotOn;
	CWidgetSlot slotOff;

	edCTextStyle textStyle;
	CSprite sprite;
};

#endif // FRONTEND_ENEMY_H