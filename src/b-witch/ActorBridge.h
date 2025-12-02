#ifndef ACTOR_BRIDGE_H
#define ACTOR_BRIDGE_H

#include "Types.h"
#include "Actor.h"

#define BRIDGE_BEHAVIOUR_BASIC 2
#define BRIDGE_BEHAVIOUR_WIND_AWARE 3

#define BRIDGE_BASIC_STATE_STAND 5
#define BRIDGE_BASIC_STATE_SPLIT 6

class CWayPoint;
class CVerletConstraint;
class CVerletParticlesBase;

static_assert(sizeof(S_STREAM_REF<CActor>) == 4);


class CVParticleInfo
{
public:
	float inverseMass;
	char _pad[12];
	edF32VECTOR4 accumulatedForce;
};

struct S_LINK_SUB_OBJ
{
	int particleIndexA;
	int particleIndexB;
	float squaredRestDistance;
	undefined4 field_0xc;
	int iterationCount;
};

class CVerletForceGen
{
public:
	virtual void ApplyForce(CVerletParticlesBase* pParticles, CVParticleInfo* pInfo, int nbInfo) = 0;
	virtual void Render() { return; }
};

class CWindForce : public CVerletForceGen
{
public:
	virtual void ApplyForce(CVerletParticlesBase* pParticles, CVParticleInfo* pInfo, int nbInfo);
	virtual void Render() { return; }

	edF32VECTOR4 force;
};

class CVerletParticlesBase
{
public:
	CVerletConstraint** aVerletConstraints;
	int nbVerletConstraints;
	CVerletForceGen** aForces;
	int nbForces;
	int nbParticleInfo;
	int stepCount;
	edF32VECTOR4* pBridgeMem;
	edF32VECTOR4* field_0x1c;
	CVParticleInfo* aParticleInfo;
	undefined field_0x24;
	undefined field_0x25;
	undefined field_0x26;
	undefined field_0x27;
	undefined field_0x28;
	undefined field_0x29;
	undefined field_0x2a;
	undefined field_0x2b;
	undefined field_0x2c;
	undefined field_0x2d;
	undefined field_0x2e;
	undefined field_0x2f;
	edF32VECTOR4 gravityForce;
};

class CVerletParticles : public CVerletParticlesBase
{
public:
	CVerletParticles();

	virtual void* Initialize(void* param_1, int param_2, int param_3, int param_4);
};

class CVerletConstraintBase
{
public:
	int iterationPriority;
};

class CVerletConstraint : public CVerletConstraintBase
{
public:
	CVerletConstraint();

	virtual void Resolve(CVerletParticles* pParticles, edF32VECTOR4* pPositions, int nbPositions) = 0;
	virtual void Render() {}
};

class CLinkConstraint : public CVerletConstraint
{
public:
	CLinkConstraint();

	virtual void Resolve(CVerletParticles* pParticles, edF32VECTOR4* pPositions, int nbPositions);
	virtual void Render() {}

	S_LINK_SUB_OBJ* aLinkInfo;
	int nbLinkInfo;
	int field_0x10;
};

class CVerletBridge : public CVerletParticles
{
public:
	struct Config {
		int stepCount;
		int nbMaxForces;
		int nbLinkConstraints;
		bool field_0xc;
		bool field_0xd;
		bool field_0xe;

		edF32VECTOR4 field_0x10;
		edF32VECTOR4 field_0x20;

		float field_0x30;
		float field_0x34;
		float field_0x38;

		Config() {}

		Config(int stepCount, int nbMaxForces, int param_3, bool param_4, bool param_5, bool param_6, edF32VECTOR4* param_7,
			edF32VECTOR4* param_8, float param_9, float param_10, float param_11);

		void ComputeNbParticles(int* param_1, int* param_2);
		void ComputeNbLinks(int* param_1, int* param_2);
	};

	static int GetMemRequirement(CVerletBridge::Config* pConfig);

	CVerletBridge();
	void Reset(CVerletBridge::Config* pConfig);
	void Split(int splitLinkIndex);
	void ApplyForces();
	void Verlet(float param_1);
	void ResolveConstraints();

	int altStepCount;

	CLinkConstraint linkConstraint;

	bool field_0x68;
	bool field_0x69;
};

class COpenBoxConstraint : public CVerletConstraint
{
public:
	COpenBoxConstraint();

	virtual void Resolve(CVerletParticles* pParticles, edF32VECTOR4* pPositions, int nbPositions);

	edF32VECTOR4 minBounds;
	edF32VECTOR4 maxBounds;

	byte bUseTransform;
	byte bEdgeSliding;

	edF32MATRIX4 transformMatrix;
	edF32MATRIX4 inverseTransformMatrix;
};

class CActorBridge : public CActor
{
public:
	class CBhvBasic : public CBehaviour
	{
	public:
		virtual void Create(ByteCode* pByteCode);
		virtual void Manage();
		virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
		virtual void End(int newBehaviourId);

		CWindForce windForce;
		edF32VECTOR4 field_0x30;
		float field_0x40;
		float field_0x44;
		float field_0x48;
		float field_0x4c;
		float field_0x50;

		float field_0x54;
		float field_0x58;

		CActorBridge* pOwner;
	};

	class CBhvWindAware : public CBehaviour
	{
	public:
		virtual void Create(ByteCode* pByteCode);
		virtual void Manage();
		virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
		virtual void End(int newBehaviourId);
		virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

		float field_0x40;
		float field_0x44;
		float field_0x48;
		float field_0x4c;
		float field_0x50;
	};

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void SaveContext(void* pData, uint mode, uint maxSize);
	virtual void LoadContext(void* pData, uint mode, uint maxSize);
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void Initialize();
	void ApplyStepsCollisionScale(int param_2);
	void BehaviourManage(byte param_2, CWindForce* pWindForce);

	S_ACTOR_STREAM_REF* pActorStream;

	COpenBoxConstraint openBoxConstraint;

	CBhvBasic behaviourBasic;
	CBhvWindAware behaviourWindAware;

	static StateConfig gStateCfg_BRG[3];

	CWayPoint* field_0x160;
	CWayPoint* field_0x164;

	CVerletBridge::Config verletBridgeConfig;

	CVerletBridge verletBridge;

	//edF32VECTOR4 field_0x2a0;

	//int stepCount;
	//bool field_0x2d8;
	//bool field_0x2d9;
	int altStepCount;

	edF32MATRIX4 field_0x2f0;

	uint bridgeConfigFlags;

	float field_0x330;
	float field_0x334;
	float field_0x338;
	float field_0x33c;

	float field_0x340;
	uint field_0x344;
	int field_0x348;
	float stepCollisionScale;

	float* aHeights;

	void* pBridgeMem;

	int field_0x354;
};

#endif //ACTOR_BRIDGE_H