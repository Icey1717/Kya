#ifndef _ACTOR_MANAGER_H
#define _ACTOR_MANAGER_H

#include "Types.h"
#include "LargeObject.h"

struct CActor;

struct CActorManager : public Manager {
	virtual void Level_AddAll(struct ByteCode* pMemoryStream);

	struct ActorManagerAnimLinkData* field_0x4;
	struct Actor* pActorArray_0x8;
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
	int actorCount_0x58;
	void** componentsToUpdate;
	int componentEnd;
	void** elementArrayStart;
	int numElements;
	struct AnimationController* pAnimationControllerArray_0x6c;
	int count_0x70;
	undefined4 field_0x74;
	undefined* field_0x78;
	undefined4 field_0x7c;
	undefined4 field_0x80;
	undefined* pActor_0x84;
	int field_0x88;
	int field_0x8c;
	//struct ActorTypeData field_0x90[87];
};

#endif // _ACTOR_MANAGER_H
