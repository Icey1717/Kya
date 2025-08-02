#ifndef FRONTEND_INVENTORY_H
#define FRONTEND_INVENTORY_H

#include "Types.h"
#include "FrontEndWidget.h"

class CCamera;

class CInstance3D
{
public:
	edNODE* pNode;
};

class CInstance3DAnimated : public CInstance3D
{
public:

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

	float field_0x50;

	CWidgetSlot slotOn;
	CWidgetSlot slotOff;

	CInstance3DAnimated animatedInstance3d;
};

#endif // FRONTEND_INVENTORY_H