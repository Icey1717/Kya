#ifndef _ACTOR_MANAGER_H
#define _ACTOR_MANAGER_H

#include "Types.h"
#include "LargeObject.h"

class CActor;
class CAnimation;
class CShadow {};
struct edAnmLayer;

struct CClassInfo {
	CActor* aActors;
	int totalCount;
	int allocatedCount;
	int size;
};

struct CActorManager : public CObjectManager {

	virtual void Level_Init();
	virtual void Level_AddAll(struct ByteCode* pMemoryStream);
	virtual void Level_Manage();
	virtual void Level_Draw();
	virtual void Level_SectorChange(int oldSectorId, int newSectorId);

	void UpdateLinkedActors();
	void Level_LoadClassesInfo(struct ByteCode* pMemoryStream);

	CActor* GetActorByHashcode(int hashCode);

	struct ActorManagerAnimLinkData* field_0x4;
	CActor* pActorArray_0x8;
	int field_0xc;
	struct Kya* kyaChild;
	float field_0x14;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	int field_0x1c;
	undefined field_0x20;
	undefined field_0x21;
	undefined field_0x22;
	undefined field_0x23;
	int field_0x24;
	float field_0x28;
	float field_0x2c;
	float field_0x30;
	float field_0x34;
	float field_0x38;
	float field_0x3c;
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
	undefined field_0x4c;
	undefined field_0x4d;
	undefined field_0x4e;
	undefined field_0x4f;
	struct ActorManagerSector* pActorManagerSectorArray;

	CActor** aActors;
	int nbActors;

	CActor** aActiveActors;
	int nbActiveActors;

	CActor** aSectorActors;
	int nbSectorActors;

	CAnimation* aAnimation;
	int initializedAnimationCount;
	int animationCount;

	edAnmLayer* aAnimLayers;
	int initalizedAnimLayerCount;
	int animLayerCount;

	CShadow* aShadows;
	int initializedShadowCount;
	int shadowCount;

	CClassInfo aClassInfo[ACTOR_NUM_CLASSES];
};

#endif // _ACTOR_MANAGER_H
