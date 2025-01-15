#ifndef ACTOR_ARAIGNOS_H
#define ACTOR_ARAIGNOS_H

#include "Types.h"
#include "ActorMovable.h"
#include "PathFollow.h"

class CActorAraignos : public CActorMovable {
public:
	class CBhvDefault : public CBehaviour
	{
		virtual void Create(ByteCode* pByteCode);
		virtual void Manage();
		virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
		virtual void End(int newBehaviourId);
		virtual void InitState(int newState);
		virtual void TermState(int oldState, int newState);

		CActorAraignos* pOwner;
	};

	static StateConfig _gStateCfg_ARA[28];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void ChangeManageState(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void BehaviourDefault_InitState(int newState);
	void BehaviourDefault_Manage();

	CAddOnGenerator addOnGenerator;

	CActorSound* pActorSound;

	CPathFollowReaderAbsolute pathFollowAbsolute;

	float field_0x1d0;
	CActor* field_0x1d4;

	float field_0x1f8;
	float field_0x1fc;
	float field_0x200;

	CSoundSample* field_0x238;
	CSoundSample* field_0x23c;
	CSoundSample* field_0x240;

	CBhvDefault behaviourDefault;
};

#endif //ACTOR_ARAIGNOS_H