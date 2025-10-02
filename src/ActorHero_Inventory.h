#ifndef ACTOR_HERO_INVENTORY_H
#define ACTOR_HERO_INVENTORY_H

#include "Actor.h"

struct InventorySlot
{
	int itemId;
	undefined4 field_0x4;
	undefined4 field_0x8;
};

struct InventoryHeaderInfo
{
	undefined4 field_0x0;
	undefined4 field_0x4;
	InventorySlot* pSlot;
};

class CInventoryInterface : public CInterface
{
public:
	CInventoryInterface();

	int bActive;
	undefined4 field_0x8;
	undefined4 field_0xc;
	InventoryHeaderInfo aHeaderInfo[2];
	InventorySlot aSlots[2][16];

	// CInterface
	virtual bool Activate(int bActive);
	virtual bool CanActivate();
	virtual bool IsActive();
	virtual void Draw() {}
	virtual void Reset() {}
	virtual void SetValue(float value) {}
	virtual float GetValue() { return 0.0f; }

	void Clear();
};

#endif // ACTOR_HERO_INVENTORY_H