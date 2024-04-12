#ifndef ACTOR_CLUSTERISER_H
#define ACTOR_CLUSTERISER_H

#include "Types.h"
#include "Actor.h"

class CActorClusteriser;
class CCluster;
class CClusterNode;

struct S_LIGHT_STREAM_REF;
struct S_CAMERA_STREAM_REF;

class CBehaviourClusteriserZones : public CBehaviour
{
public:
	struct _S_ZONE_CLUSTER {
		_S_ZONE_CLUSTER() {}
		uint flags;
		S_STREAM_REF<ed_zone_3d> field_0x4;
		S_STREAM_REF<ed_zone_3d> field_0x8;
		S_ACTOR_STREAM_REF* field_0xc;
		S_ACTOR_STREAM_REF* field_0x10;
		int nbActors;
		CActor** field_0x18;
		S_CAMERA_STREAM_REF* field_0x1c;
		int nbEvents;
		undefined4 field_0x24;
		S_LIGHT_STREAM_REF* field_0x28;
		int nbLights;
		undefined4 field_0x30;
		int* field_0x34;
	};

	// CBehaviour
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	
	void TriggerManageClusterOff(_S_ZONE_CLUSTER* pZoneCluster);
	void TriggerManageClusterOn(_S_ZONE_CLUSTER* pZoneCluster);
	void TriggerSceneries(_S_ZONE_CLUSTER* pZoneCluster, int param_3);

	CActorClusteriser* pOwner;

	_S_ZONE_CLUSTER* aZoneClusters;
	int nbZoneClusters;

	static CCluster* _pall_cluster;
	static CClusterNode* _pact_nodes;
};

class CActorClusteriser : public CActor 
{
public:
	CActorClusteriser();

	CBehaviourClusteriserZones behaviourClusteriserZones;

	// CActor
	virtual void Init();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);

	void Sector_Enter();
	int FillActorsTableFromZone(CActor** param_2, ed_zone_3d* pZone, CCluster* pCluster, S_ACTOR_STREAM_REF** param_5);
};

#endif //ACTOR_CLUSTERISER_H