#ifndef FRONTEND_INVENTORY_H
#define FRONTEND_INVENTORY_H

#include "Types.h"
#include "FrontEndWidget.h"
#include "Pause.h"
#include "SpriteWidget.h"
#include "ActorHero_Inventory.h"

class CCamera;

class CFE_Avatar
{
public:
	void Create(CActor* pActor);

	CActor* field_0x0;
	int field_0x4;
	CInstance3DAnimated instance3d;
};

class CSprite3D : public CSprite
{
public:
	virtual void ClearLocalData();
	virtual void Validate();
	virtual void DrawSprite();

	float field_0xc0;
	float field_0xc4;
};

struct _FE_INV_ENTRY
{
	CSprite3D sprite;
};

struct FE_INV_COLUMN
{
	int field_0x0;
	undefined4 field_0x4;
	_FE_INV_ENTRY aEntries[4];
	CFE_Avatar aNodes[4];
	edCTextStyle field_0x870;
	edF32VECTOR2 field_0x930;
	edF32VECTOR2 field_0x938;
};

class CFrontendInventory : public CWidget
{
public:
	CFrontendInventory();

	// CWidget
	virtual void Reset();
	virtual void Init();
	virtual void Term();
	virtual void UpdatePos_StateWait(float time);
	virtual bool UpdateDisp(float time);
	virtual void Update(float time);
	virtual void Draw();

	void Level_Init();
	void Level_Term();

	bool ComputeGameScreenCoordinate(edF32VECTOR2* pOutCoordinate, edF32VECTOR4* pPosition, float* pOutDepth, CCamera* pCamera);

	bool PrepareTransit(CActor* pInventoryOwner, int param_3, edF32VECTOR4* param_4, CActor* param_5);
	void FUN_003ca620();
	void ManageTransit();
	void Column_DisplayText(int index);
	void Column_Display(int index);

	int field_0x4c;
	float field_0x50;

	CWidgetSlot slotOn;
	CWidgetSlot slotOff;

	CFE_Avatar avatar;

	CSprite3D aSprites3d[2];
	FE_INV_COLUMN aInvColumns[2];

	undefined4 field_0x1490;
	int field_0x1494;
	float field_0x1498;
	CActor* field_0x149c;

	edF32VECTOR4 field_0x14a0;
	FE_Position field_0x14b0;
	CActor* field_0x14b8;

	edNODE* pNode;
};

#endif // FRONTEND_INVENTORY_H