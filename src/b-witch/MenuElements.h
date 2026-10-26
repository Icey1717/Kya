#ifndef MENU_ELEMENTS_H
#define MENU_ELEMENTS_H

#include "Types.h"
#include "SpriteWidget.h"

class CMenuArrows
{
public:
	bool FUN_002ef890();
	void FUN_002ef4e0(float param_1, float param_2, float param_3, float param_4, byte param_6);
	void FUN_002ef500();
	void Init(edDList_material* param_2, edDList_material* param_3);
	void FUN_002ef9b0(edDList_material* param_2, edDList_material* param_3);
	void MoveMenuArrow(bool bNext);
	void Reset();
	void Term();

	CSprite field_0x0;
	CSprite field_0xc0;

	float field_0x180;
	float field_0x184;
	float field_0x188;
	float field_0x18c;
	float field_0x190;
	byte field_0x194;
	float field_0x198;
	bool field_0x19c;
	bool field_0x19d;

	uint field_0x1a0;
	byte field_0x1a4;
	int field_0x1a8;
	union {
		int field_0x1ac;
		float centerX;
		void* pContext;
	};
};

struct S_MENU_WHEEL_DRAW
{
	float x;
	float y;
	byte alpha;
};

class CMenuWheel
{
public:
	CMenuWheel();
	void Init(edDList_material* pMaterial, edDList_material* pArrow, edDList_material* pArrowHighlight);
	void Reset();
	bool Manage();
	void Draw();
	void MoveWheel(bool bNext);
	void MoveWheel();
	void FUN_002efa80(float x, float y, edF32VECTOR2* pRight, edF32VECTOR2* pLeft);

	CSprite field_0x0;
	CMenuArrows field_0xc0;
	float field_0x270;
	int field_0x274;
	byte field_0x278;
	int field_0x27c;
	float field_0x280;
	float field_0x284;
	bool field_0x288;
	float field_0x28c;
	bool field_0x290;
	void (*pFunc)(int, int, S_MENU_WHEEL_DRAW*, void**);
	void** field_0x298;
};

#endif // MENU_ELEMENTS_H
