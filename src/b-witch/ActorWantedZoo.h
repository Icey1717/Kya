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
	int* field_0xc;
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

	int field_0x160;
	edF32VECTOR2* field_0x164;
	int field_0x168;
	edF32VECTOR2* field_0x16c;

	edCTextStyle textStyle;

	int field_0x230;
	Zoo_14* field_0x234;
	bool field_0x238;
	float field_0x23c;
};

#endif //ACTOR_WANTED_ZOO_H
