#ifndef ACTOR_FOG_MANAGER_H
#define ACTOR_FOG_MANAGER_H

#include "Types.h"
#include "Actor.h"
#include "SectorManager.h"

class CActorFogManager;

class CStreamFogZone
{
public:
	uint flags;
	S_STREAM_REF<ed_zone_3d> zoneRef;
	S_STREAM_FOG_DEF fogDef;
	float field_0x18;
};

static_assert(sizeof(CStreamFogZone) == 0x1c, "sizeof(CStreamFogZone) != 0x1C");

class CStreamFogZoneInternal
{
public:
	int nbFogZones;
	CStreamFogZone aFogZones[];
};

class CStreamFogZoneArray
{
public:
	void Manage();

	CStreamFogZoneInternal* pInternal;
};

class CFogHierarchy : public CSectorHierarchy 
{
public:
	float field_0x20;
	float field_0x24;
};

class CFogHierarchiesArray
{
public:
	CFogHierarchiesArray();
	void Manage();

	int nbFogHierarchies;
	CFogHierarchy* aFogHierarchies;
};

class CBehaviourFogManagerZones : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Term();
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);

	CActorFogManager* pOwner;
	CStreamFogZoneArray streamFogZoneArray;
};

class CActorFogManager : public CActor 
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);

	CFogHierarchiesArray fogHierarchiesArray;

	CBehaviourFogManagerZones behaviourFogManagerZones;
};

#endif //ACTOR_FOG_MANAGER_H