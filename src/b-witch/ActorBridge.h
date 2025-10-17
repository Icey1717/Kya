#ifndef ACTOR_BRIDGE_H
#define ACTOR_BRIDGE_H

#include "Types.h"
#include "Actor.h"

class CWayPoint;
class CLinkConstraint;

static_assert(sizeof(S_STREAM_REF<CActor>) == 4);


struct S_PARTICLE_SUB_OBJ {
	float field_0x0;
	char a[28];
};

struct S_LINK_SUB_OBJ {
	int field_0x0;
	int field_0x4;
	float field_0x8;
	undefined4 field_0xc;
	int field_0x10;
};

class CVerletParticlesBase
{
public:
	CLinkConstraint** aLinkConstraints;
	int field_0x4;
	S_PARTICLE_SUB_OBJ* field_0x8;
	int field_0xc;
	int field_0x10;
	int stepCount;
	edF32VECTOR4* pBridgeMem;
	edF32VECTOR4* field_0x1c;
	S_PARTICLE_SUB_OBJ* aParticleSubObjs;
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
	edF32VECTOR4 field_0x30;
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
	int field_0x0;
};

class CVerletConstraint : public CVerletConstraintBase
{
public:
	CVerletConstraint();
	virtual void Resolve(CVerletParticles* pParticles, edF32VECTOR4* param_3, int param_4) = 0;
	virtual void Render() {}
};

class CLinkConstraint : public CVerletConstraint
{
public:
	CLinkConstraint();

	virtual void Resolve(CVerletParticles* pParticles, edF32VECTOR4* param_3, int param_4);
	virtual void Render() {}

	S_LINK_SUB_OBJ* field_0x8;
	int field_0xc;
	int field_0x10;
};

class CVerletBridge : public CVerletParticles
{
public:
	struct Config {
		int stepCount;
		int nbUnknown;
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

		Config(int stepCount, int param_2, int param_3, bool param_4, bool param_5, bool param_6, edF32VECTOR4* param_7,
			edF32VECTOR4* param_8, float param_9, float param_10, float param_11);

		void ComputeNbParticles(int* param_1, int* param_2);
		void ComputeNbLinks(int* param_1, int* param_2);
	};

	static int GetMemRequirement(CVerletBridge::Config* pConfig);

	CVerletBridge();
	void Reset(CVerletBridge::Config* pConfig);
	void Split(int param_1);

	int altStepCount;

	CLinkConstraint linkConstraint;

	bool field_0x68;
	bool field_0x69;
};

class CWindForce {};

class COpenBoxConstraint : public CVerletConstraint
{
public:
	COpenBoxConstraint();

	virtual void Resolve(CVerletParticles* pParticles, edF32VECTOR4* param_3, int param_4);

	edF32VECTOR4 field_0x10;
	edF32VECTOR4 field_0x20;

	byte field_0x30;
	byte field_0x31;

	edF32MATRIX4 field_0x40;

	edF32MATRIX4 field_0x80;
};

class CActorBridge : public CActor {
public:
	class CBhvBasic : public CBehaviour {
	public:
		virtual void Create(ByteCode* pByteCode);
		virtual void Manage();
		virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
		virtual void End(int newBehaviourId);

		edF32VECTOR4 field_0x20;
		edF32VECTOR4 field_0x30;
		float field_0x40;
		float field_0x44;
		float field_0x48;
		float field_0x4c;
		float field_0x50;

		float field_0x54;
		float field_0x58;

		CActorBridge* pOwner;

		CWindForce windForce;
	};

	class CBhvWindAware : public CBehaviour {
	public:
		virtual void Create(ByteCode* pByteCode);

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

	uint field_0x358;

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