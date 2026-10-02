#ifndef ACTOR_WANTED_ZOO_H
#define ACTOR_WANTED_ZOO_H

#include "Types.h"
#include "Actor.h"
#include "ScenaricCondition.h"
#include "Rendering/edCTextStyle.h"

struct Zoo_10
{
	int field_0x0;
	uint* field_0x4;
	int field_0x8;
};

struct Zoo_14
{
	CScenaricCondition scenaricCondition;
	Zoo_10 field_0x4;
};

class CActorWantedZoo : public CActor
{
public:

	virtual void Create(ByteCode* pByteCode);

	virtual void Init();
	virtual void Term();

	virtual void Manage();
	virtual void Draw();

	edCTextStyle textStyle;

	Zoo_14* field_0x234;
	bool field_0x238;
	float field_0x23c;
};

#endif //ACTOR_WANTED_ZOO_H