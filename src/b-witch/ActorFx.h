#ifndef ACTOR_FX_H
#define ACTOR_FX_H

#include "Types.h"
#include "Actor.h"
#include "Fx.h"

#define FX_BEHAVIOUR_SINGLE 2
#define FX_BEHAVIOUR_PATH 3
#define FX_BEHAVIOUR_UNKNOWN 4

class CPathFollow;

class CActorFx : public CActor
{
public:
	class CBhvSingle : public CBehaviour
	{
	public:
		virtual void Create(ByteCode* pByteCode);
		virtual void Init(CActor* pOwner);
		virtual void Term();
		virtual void Manage();
		virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
		virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

		CActorFx* pOwner;
		byte field_0x8;
		int field_0xc;
		CFxHandle fxHandle;
	};

	class CBhvPath : public CBehaviour
	{
	public:
		virtual void Create(ByteCode* pByteCode);
		virtual void Init(CActor* pOwner);
		virtual void Term();
		virtual void Manage();
		virtual void Draw();
		virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
		virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

		CActorFx* pOwner;
		byte field_0xc;
		CPathFollow* pPathFollow;
		int field_0x14;
		CFxHandle fxHandle;
	};

	class CBhvUnknown : public CBehaviour
	{
	public:
		virtual void Create(ByteCode* pByteCode);
		virtual void Init(CActor* pOwner);
		virtual void Manage();
		virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

		int* field_0x4;
		CActor* pOwner;
	};

	virtual void Create(ByteCode* pByteCode);
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual void ChangeManageState(int state);

	CBhvSingle bhvSingle;
	CBhvPath bhvPath;
	CBhvUnknown bhvUnknown;

	float field_0x19c;
	float field_0x1a0;
	float field_0x1a4;
};

#endif //ACTOR_FX_H