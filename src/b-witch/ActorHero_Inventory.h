#ifndef ACTOR_HERO_INVENTORY_H
#define ACTOR_HERO_INVENTORY_H

#include "Actor.h"
#include "Frontend.h"

struct InventorySlot
{
	int itemId;
	int nbItems;
	int bInUse;
};

struct InventoryHeaderInfo
{
	int activeItemIndex;
	int nbUsedSlots;
	InventorySlot* pSlot;
};

struct FE_Position
{
	int headerSlot;
	int itemSlot;
};

class CInventoryInterface : public CInterface
{
public:
	CInventoryInterface();

	int bActive;
	undefined4 field_0x8;
	int bDirty;
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

	InventorySlot* GetExistingInventorySlot(int headerSlot, int ItemId);
	InventorySlot* GetFreeSlot(int headerSlot);

	CActor* FindActorInState(FE_Position* pPosition, int mode);
	CActor* FindActorForCell(FE_Position* pPosition);

	bool Cmd_Apply();
	bool Cmd_AddItem(int ItemId, int count, uint flags);
	void Cmd_Abort();
	void Cmd_NextItem(int headerSlot);
	bool Cmd_RemoveItem(int itemId, uint flags);

	void Clear();

	bool FindByType(FE_Position* pPosition, int itemId);
	bool HasChanged();
};

#endif // ACTOR_HERO_INVENTORY_H