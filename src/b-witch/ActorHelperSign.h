#ifndef ACTOR_HELPER_SIGN_H
#define ACTOR_HELPER_SIGN_H

#include "Types.h"
#include "Actor.h"
#include "ActorAmbre.h"
#include "SpriteWidget.h"

#define HELPER_SIGN_BEHAVIOUR_STAND 2
#define HELPER_SIGN_STATE_STAND 5
#define HELPER_SIGN_STATE_SHOW 6
#define HELPER_SIGN_STATE_OPEN 7
#define HELPER_SIGN_STATE_CLOSE 8

#define HELPER_SIGN_BEHAVIOUR_HIGHSCORE 3

class CActorHelperSign;

struct astruct_22
{
	bool FUN_002ef890();
	void FUN_002ef4e0(float param_1, float param_2, float param_3, float param_4, byte param_6);
	void FUN_002ef500();
	void Init(edDList_material* param_2, edDList_material* param_3);
	void FUN_002ef9b0(edDList_material* param_2, edDList_material* param_3);
	void Term();

	CSprite field_0x0;
	CSprite field_0xc0;

	float field_0x180;
	float field_0x184;
	float field_0x188;
	float field_0x18c;
	float field_0x190;
	byte field_0x194;
	float field_0x198;
	bool field_0x19c;
	bool field_0x19d;

	uint field_0x1a0;
	byte field_0x1a4;
	int field_0x1a8;
	int field_0x1ac;
};

class CBehaviourHelperSign : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	CActorHelperSign* pOwner;
};

class CBehaviourHelperSignStand : public CBehaviourHelperSign
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Term();
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CBehaviourHelperSignHighScore : public CBehaviourHelperSign
{
public:
	S_STREAM_REF<CActor> field_0x8;
	StaticMeshComponent field_0x50;
};

class CActorHelperSign : public CActor
{
public:
	static StateConfig _gStateCfg_HLP[5];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();	
	virtual void Draw();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void ChangeManageState(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void DrawHelpText();

	bool CloseEffect();
	bool OpenEffect();

	bool DetectTrigger(CActor* pSender);

	ulong field_0x160;
	int field_0x168;
	char* field_0x16c;
	CActor* field_0x170;
	float field_0x174;
	float field_0x178;
	float field_0x17c;
	float field_0x180;
	float field_0x184;
	float field_0x188;
	float field_0x18c;
	byte field_0x190;
	CFxHandle field_0x194;
	CBehaviourHelperSignStand behaviourStand;
};

#endif //ACTOR_HELPER_SIGN_H