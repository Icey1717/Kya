#ifndef _ACTOR_FACTORY_H
#define _ACTOR_FACTORY_H

#include "Types.h"

struct ActorClassProperties {
	uint flags;
	uint field_0x4;
	int classPriority;
	uint field_0xc;
	int field_0x10;
	int field_0x14;
};

class CActor;

class CActorFactory {
public:
	static CActor* Factory(ACTOR_CLASS classId, int count, int* outSize, void* pAlloc);
	static ActorClassProperties gClassProperties[ACTOR_NUM_CLASSES];
};


#endif // _ACTOR_FACTORY_H
