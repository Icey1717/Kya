#ifndef FRONTEND_INVENTORY_H
#define FRONTEND_INVENTORY_H

#include "Types.h"
#include "FrontEndWidget.h"
#include "Pause.h"

class CCamera;

class CFE_Avatar
{
public:
	CInstance3DAnimated animatedInstance3d;
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

	bool FUN_003c9b00(CActor* pInventoryOwner, int param_3, edF32VECTOR4* param_4, CActor* param_5);

	float field_0x50;

	CWidgetSlot slotOn;
	CWidgetSlot slotOff;

	CFE_Avatar avatar;
};

#endif // FRONTEND_INVENTORY_H