#ifndef FRONTEND_ENEMY_H
#define FRONTEND_ENEMY_H

#include "Types.h"
#include "FrontEndWidget.h"

class CFrontendEnemyCount : public CFrontendLabelText
{
public:
	CFrontendEnemyCount();

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

class CFrontendEnemy : public CFrontendBitmapGauge
{
public:
	CFrontendEnemy();
	virtual void SetInterface(CInterface* pNewInterface);
	virtual void Reset();
	virtual void Init();
	virtual void Term();
	virtual void UpdatePos_StateWait(float time);

	virtual bool UpdateGauge(float time);

	void UpdateSpecial(CWidgetSlot* pSlot, int param_3);
	int FUN_001d94b0();
	void DrawSpecial();

	void ResetInternal();

	CSprite sprite_0x2b8;

	float field_0x378;
	int state_0x37c;

	int field_0x380;
	int field_0x384;
	int field_0x388;
	int field_0x38c;
	int field_0x390;
	int field_0x394;

	CSprite sprite_0x398;

	undefined4 field_0x458;
};

struct EnemyInterfaceTable
{
	int nbEntries;
	CInterface* aEntries[5];

	bool Contains(CInterface* pInterface);
	void Add(CInterface* pInterface);
	CInterface* Pop();
};

class CFrontendEnemyList
{
public:
	CFrontendEnemyList();

	virtual void Init();
	virtual void Term();
	virtual void Reset();

	void Level_Term();
	void AddFighterInterface(CInterface* pInterface);
	void Update(float time);
	void Draw();

	void ResetSlots();

	CWidgetSlot aSlots[7];

	CFrontendEnemy* aWidgets[5];

	CFrontendEnemy aEnemies[5];
	EnemyInterfaceTable enemyInterfaceTable;
};

#endif // FRONTEND_ENEMY_H