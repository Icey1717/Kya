#ifndef INVENTORY_INFO_H
#define INVENTORY_INFO_H

#include "Types.h"

// Boomy
#define INVENTORY_ITEM_BASE_BOOMY 0x2
#define INVENTORY_ITEM_SILVER_BOOMY 0x3
#define INVENTORY_ITEM_GOLD_BOOMY 0x4

// Bracelet
#define INVENTORY_ITEM_WHITE_BRACELET 0x5
#define INVENTORY_ITEM_YELLOW_BRACELET 0x6
#define INVENTORY_ITEM_GREEN_BRACELET 0x7
#define INVENTORY_ITEM_BLUE_BRACELET 0x8
#define INVENTORY_ITEM_BROWN_BRACELET 0x9
#define INVENTORY_ITEM_BLACK_BRACELET 0xa
#define INVENTORY_ITEM_SILVER_BRACELET 0xb
#define INVENTORY_ITEM_GOLD_BRACELET 0xc

struct ByteCode;

class CInventoryInfo
{
public:
	int ProcessMessage(CActor* pSender, int param_3, void* param_4);
	void Reset();
	void Init(CActor* pOwner);
	void Create(ByteCode* pByteCode);

	void ObjectPurchased();
	static uint IsObjectPurchased(int objId);

	int field_0x0;
	CActor* pOwner;
	ulong field_0x8;
	ulong field_0x10;
	ulong field_0x18;
	int purchaseId;
	int objectId;
	int field_0x28;
	int field_0x2c;
	int moneyCost;
};

#endif // INVENTORY_INFO_H