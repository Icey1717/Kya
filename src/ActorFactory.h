#ifndef _ACTOR_FACTORY_H
#define _ACTOR_FACTORY_H

#include "Types.h"

#define ACTOR_NUM_CLASSES 87

struct ActorClassProperties {
	uint flags_0x0;
	uint field_0x4;
	int field_0x8;
	uint field_0xc;
	int field_0x10;
	int field_0x14;
};

class CActorFactory {
public:
	static ActorClassProperties gClassProperties[ACTOR_NUM_CLASSES];
};


#endif // _ACTOR_FACTORY_H
