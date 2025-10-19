#include "ActorHero_Inventory.h"

CInventoryInterface::CInventoryInterface()
{
	this->bActive = 0;
	this->field_0x8 = 0;
	this->field_0xc = 0;

	Clear();

	return;
}

// Should be in: D:/Projects/b-witch/Frontend.cpp
bool CInventoryInterface::Activate(int bActive)
{
	if (bActive == 0) {
		this->bActive = 0;
	}
	else {
		this->bActive = 1;
		this->aHeaderInfo[0].field_0x0 = 0;
		this->aHeaderInfo[1].field_0x0 = 0;
	}
	return true;
}

bool CInventoryInterface::CanActivate()
{
	return this->bActive == 0;
}

bool CInventoryInterface::IsActive()
{
	return this->bActive;
}

void CInventoryInterface::Clear()
{
	for (int i = 0; i < 2; i++) {
		this->aHeaderInfo[i].field_0x0 = 0;
		this->aHeaderInfo[i].field_0x4 = 1;
		this->aHeaderInfo[i].pSlot = this->aSlots[i] + 1;

		for (int j = 0; j < 16; j++) {
			this->aSlots[i][j].itemId = -1;
			this->aSlots[i][j].field_0x4 = 0;
			this->aSlots[i][j].field_0x8 = 0;
		}

		this->aSlots[i][0].field_0x8 = 0xffffffff;
	}

	return;
}