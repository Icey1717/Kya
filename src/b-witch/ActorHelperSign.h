#ifndef ACTOR_HELPER_SIGN_H
#define ACTOR_HELPER_SIGN_H

#include "Types.h"
#include "Actor.h"
#include "StaticMeshComponent.h"
#include "Fx.h"
#include "SpriteWidget.h"
#include "MenuElements.h"

#define HELPER_SIGN_BEHAVIOUR_STAND 2
#define HELPER_SIGN_STATE_STAND 5
#define HELPER_SIGN_STATE_SHOW 6
#define HELPER_SIGN_STATE_OPEN 7
#define HELPER_SIGN_STATE_CLOSE 8

#define HELPER_SIGN_BEHAVIOUR_HIGHSCORE 3

class CActorHelperSign;

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
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Term();
	virtual void Manage();
	virtual void ManageFrozen();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	S_STREAM_REF<CActor> field_0x8;
	int field_0xc;
	ed_3d_hierarchy_setup field_0x10;
	edF32VECTOR4 field_0x30;
	float field_0x40;
	StaticMeshComponent field_0x50;
	StaticMeshComponent field_0xb0;
	StaticMeshComponent field_0x110;
	StaticMeshComponent field_0x170;
	StaticMeshComponent field_0x1d0;
	StaticMeshComponent field_0x230;
	StaticMeshComponent field_0x290;

	CMenuWheel menuWheel;
	void* field_0x590;
	float field_0x594;
	float field_0x598;
	float field_0x59c;
	float field_0x5a0;

	void InitMeshes(bool all);
	void TermMeshes();
	void ManageHighScore();
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
	void DrawHighScoreText(int index, uint alpha);

	bool CloseEffect();
	bool OpenEffect();

	bool DetectTrigger(CActor* pSender);

	void FUN_00339ab0();

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
