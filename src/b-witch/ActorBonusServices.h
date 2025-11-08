#ifndef _ACTOR_BONUS_SERVICES_H
#define _ACTOR_BONUS_SERVICES_H

#include "Types.h"

struct ByteCode;
struct edNODE;
struct ed_3d_hierarchy;

class CActor;
class CActorBonus;
class CBehaviourBonusAddOn;
class CActorMoney;
class CBehaviourMoneyAddOn;

#define ACTOR_INSTANCE_FLAG_ALIVE		(1 << 0)
#define ACTOR_INSTANCE_FLAG_VISIBLE		(1 << 2)

class CActInstance
{
public:
	enum STT_INS_TYPE
	{
		STT_INS_WAIT = 1,
		STT_INS_GOTO_KIM = 3,
	};

	CActInstance();

	virtual void SetState(int newState);
	virtual float GetAngleRotY() = 0;
	virtual void CheckpointReset();

	void Init(CActor* pOwner, edF32VECTOR4* pPosition, edF32VECTOR4* pBoundSphere, int instanceIndex);
	void Term();
	void SetVisible(int bVisible);
	void Reset();
	void SetAlive(int bAlive);

	float DistSquared(edF32VECTOR4* pPosition);
	void ComputeDistanceToKim(edF32VECTOR4* pPosition);

	void State_GotoKim();
	void State_Wait();

	void UpdateVisibility();
	void FUN_00397ba0();

	void SetBasePosition(edF32VECTOR4* pBasePosition);
	void SetPosition(edF32VECTOR4* pPosition);

	uint flags;

	CActor* pOwner;

	edF32VECTOR4 basePosition;
	edF32VECTOR4 currentPosition;
	edF32VECTOR4 pathDelta;

	float field_0x54;
	float distanceToKim;
	float field_0x5c;
	int instanceIndex;

	edNODE* pNode;
	ed_3d_hierarchy* pHierarchy;

	int state;

	edF32VECTOR3 field_0x64;
	edF32VECTOR3 field_0x70;

	float field_0x7c;
	undefined4 field_0x84;
	undefined4 field_0x88;

	float field_0x90;
	float angleRotY;
	float field_0x98;
};

class CAddOnGenerator_SubObj
{
public:
	void Create(ByteCode* pByteCode);

	float field_0x0;
	float field_0x4;
	float field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;
};

struct SectorAddOnData
{
	int aNbMonies[3];
	int nbBonus;
};

struct MoneyTypeEntry
{
	int _gAddOn_NbTotalMoneyInLevel;
	int field_0x4;
	CActorMoney* pActor;
	CBehaviourMoneyAddOn* pBehaviour;
};

class CAddOnGenerator
{
public:
	static SectorAddOnData* _gAddOn_sectors;
	static int _gAddOn_NbTotalAddOn;
	static MoneyTypeEntry _gAddOn_aMoney[3];
	static int _gAddOn_NbTotalBonusInLevel;
	static int _gAddOn_bComputeDone;
	static int _gAddOn_NbRemaining;
	static int _gAddOn_NbMaxBonusInSector;
	static CActorBonus* _gAddOn_pBonusAct;
	static CBehaviourBonusAddOn* _gAddOn_pBonusBhv;

	void Create(CActor* pActor, ByteCode* pByteCode);
	void Init(int param_2);
	void Term();

	void Generate(edF32VECTOR4* pPosition);

	void ManageInstances();

	ushort field_0x0;
	ushort moneyType;
	int nbMoney;
	int nbBonus;
	float field_0xc;
	CActInstance** aInstances;
	int maxOrbs_0x2c;
	undefined4 field_0x30;

	CAddOnGenerator_SubObj subObj;
};

#endif // _ACTOR_BONUS_SERVICES_H