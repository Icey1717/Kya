#ifndef FRONTEND_BINOCULARS_H
#define FRONTEND_BINOCULARS_H

#include "FrontEndWidget.h"
#include "Pause.h"

class CFrontendBinoculars : public CWidget
{
public:
	CFrontendBinoculars();

	virtual void Reset();
	virtual void Init();
	virtual void Term();
	virtual void UpdatePos_StateWait(float time);
	virtual bool UpdateDisp(float time);
	virtual void Update(float time);
	virtual void Draw();

	void Level_Init();
	void Level_Term();

	int field_0x4c;
	CWidgetSlot slotOn;
	CWidgetSlot slotOff;
	CSprite aHoleSprites[4];
	CSprite aBorderSprites[4];
	CInstance3D instance3d;
};

#endif // FRONTEND_BINOCULARS_H
