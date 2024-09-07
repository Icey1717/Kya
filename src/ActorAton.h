#ifndef ACTOR_ATON_H
#define ACTOR_ATON_H

#include "Types.h"
#include "ActorAutonomous.h"

class CPathPlaneArray {
public:
	CPathPlaneArray();

	void Create(ByteCode* pByteCode);

	int field_0x0;
	int field_0x4;
	int field_0x8;
};

class CFxTail : public CObject
{
public:
	CFxTail() : pData_0x18((undefined*)0x0) {}

	void Create(float param_1, int count, int param_4, int materialId);

	undefined* pData_0x18;
};

class CStaticMeshComponent
{
public:
	CStaticMeshComponent();

	virtual void Reset();
};

class CBehaviourAddOnAton : public CBehaviourAddOnBase {
public:
	CBehaviourAddOnAton();
	virtual void Create(ByteCode* pByteCode);

	int field_0x10;
	int* field_0x14;
	int field_0x1c;
	int* field_0x20;
};

class CBehaviourAton : public CBehaviour
{

};

class CBehaviourAtonEscape : public CBehaviourAton
{

};

class CActorAton : public CActorAutonomous {
public:
	CActorAton();

	virtual void Create(ByteCode* pByteCode);
	virtual CBehaviour* BuildBehaviour(int behaviourType);

	CPathPlaneArray pathPlaneArray;
	CBehaviourAddOnAton behaviourAddOn;
	CBehaviourAtonEscape behaviourAtonEscape;

	CFxTail fxTailA;
	CFxTail fxTailB;

	float field_0x350;

	uint field_0x354;
	uint field_0x358;
	uint field_0x35c;
	uint field_0x360;

	int trailMaterialId;

	int field_0x63c;
	int field_0x644;
	int field_0x648;

	CStaticMeshComponent staticMeshComponent;
};

#endif //ACTOR_ATON_H