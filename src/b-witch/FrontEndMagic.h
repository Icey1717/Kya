#ifndef FRONTEND_MAGIC_H
#define FRONTEND_MAGIC_H

#include "Types.h"

#include "FrontEndWidget.h"
#include "Rendering/edCTextStyle.h"

class CFrontendMagicGauge : public CFrontendBitmapGauge
{
public:
	CFrontendMagicGauge();

	// CWidget
	virtual void SetInterface(CInterface* pNewInterface);
	virtual void Reset();
	virtual void Init();
	virtual void Term();
	virtual void UpdatePos_StateWait(float time);
	virtual void UpdatePos_StateMove(float time);
	virtual bool UpdateDisp(float time);
	virtual void Update(float time);
	virtual void Draw();

	// CFrontendBitmapGauge
	virtual bool UpdateGauge(float time);

	// CFrontendMagicGauge
	virtual void CheckEvents(float time);

	void Magic_SetDisplay(unsigned char bNewVisible);

	void ClearLocalData();

	void ShowMagic();
	void ShowMagicAlways();
	void HideMagicAlways(bool param_2);

	void DrawMagic_BonusFlare();
	void FUN_001d7020();

	unsigned char bVisible;

	int currentMagicValueA;
	int maxMagicValueA;
	int currentMagicValueB;
	int maxMagicValueB;

	CWidgetSlot field_0x2c8;
	CWidgetSlot field_0x2d8;

	undefined4 field_0x2f8;
	int state_0x2fc;

	undefined4 field_0x300;
	int field_0x304;

	edCTextStyle textStyle;

	CSprite field_0x3d0;
	CSprite field_0x490;

	float field_0x550;
	float field_0x554;

	char field_0x558[16];
};

#endif // !FRONTEND_MAGIC_H
