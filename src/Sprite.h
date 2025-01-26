#ifndef SPRITE_H
#define SPRITE_H

#include "Types.h"
#include "edDlist.h"
#include "ed3D.h"

class CWidgetSlot
{
public:
	CWidgetSlot();

	static void LERP(float alpha, CWidgetSlot* pDst, CWidgetSlot* pSlotA, CWidgetSlot* pSlotB);

	edF32VECTOR2 position;
	edF32VECTOR2 scale;
};

class CSprite {
public:
	CSprite();

	virtual void ClearLocalData();
	virtual void Draw(bool bUpdateMaterial);
	virtual void DrawXYXY(uint drawFlag, float param_3, float param_4, float param_5, float param_6, float param_7);
	virtual void Draw(uint drawFlag, float param_2, float param_3, float param_4);
	virtual void Draw(uint drawFlag, float param_2, float param_3, float param_4, float param_5);
	virtual void Validate();
	virtual void PrepareDraw(bool bUpdateMaterial);
	virtual void DrawSprite();

	void Install(char* pFileBuffer);
	void Install(edDList_material* pMaterial);
	ed_g2d_layer* Install(ed_g2d_manager* pManager, char* pFileName);

	void Remove();

	// New Function from suspected inline
	void SetParent(CSprite* pNewParent);

	// New Function from suspected inline
	void UpdateSlotPosition(float x, float y);

	// New Function from suspected inline
	void UpdateSlotScale(float x, float y);

	// New Function from suspected inline
	void UpdateSlotPositionAndScale(float x, float y, float xScale, float yScale);

	byte bValid;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	CSprite* pParent;
	struct edDList_material* pMaterialInfo;
	undefined4 field_0x10;
	float field_0x14;
	float fWidth;
	float fHeight;
	undefined4 field_0x20;
	undefined4 field_0x24;
	float field_0x28;
	float field_0x2c;

	union
	{
		byte rgba[4];
		uint color;
	};

	undefined4 field_0x34;
	ushort iWidth;
	ushort iHeight;
	undefined field_0x3c;
	undefined field_0x3d;
	undefined field_0x3e;
	undefined field_0x3f;
	undefined field_0x40;
	undefined field_0x41;
	undefined field_0x42;
	undefined field_0x43;
	undefined field_0x44;
	undefined field_0x45;
	undefined field_0x46;
	undefined field_0x47;
	undefined field_0x48;
	undefined field_0x49;
	undefined field_0x4a;
	undefined field_0x4b;
	float field_0x4c;
	float field_0x50;
	float field_0x54;
	float field_0x58;
	CWidgetSlot field_0x5c;
	undefined4 field_0x6c;
	undefined* field_0x70;
	float field_0x74;
	float field_0x78;
	uint flags_0x7c;
	edDList_material materialInfo;
	ed_g2d_manager textureManager;
};

class CSpriteWindow : public CSprite
{
public:
	// CSprite
	virtual void Validate();
	virtual void DrawSprite();

	void ClampX(float clampMin, float clampMax);
	void ClampX(float clampMin, float clampMax, bool param_4);

	float xMin;
	float xMax;
};

#endif // !SPRITE_H
