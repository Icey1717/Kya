#ifndef ACTOR_CREDITS_H
#define ACTOR_CREDITS_H

#include "Types.h"
#include "Actor.h"
#include "CinematicManager.h"

#define CREDITS_BEHAVIOUR_DEFAULT 0x2

class CActorCredits;

PACK(
struct S_CREDITS_MESSAGE_STREAM
{
	int nbEntries;
	ulong aKeys[];
};
)

class CBehaviourCredits : public CBehaviour
{
public:
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);

	CActorCredits* pOwner;
};

class CBehaviourCreditsDefault : public CBehaviourCredits
{
public:
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
};

class CActorCredits : public CActor
{
public:

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Draw();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void ClearLocalData();
	char* FUN_00409750(char* param_2);

	CBehaviourCreditsDefault behaviourDefault;
	S_CREDITS_MESSAGE_STREAM* pMessageStream;
	float scrollSpeed;
	float textBoxX;
	float textBoxY;
	float textBoxWidth;
	float textBoxHeight;

	float boxFillOpacityTarget;
	float boxBorderOpacity;
	float boxBorderWidth;
	float boxBorderHeight;

	// CMusic* pMusic;
	float musicFadeInTime;
	float musicVolume;
	float musicFadeOutTime;
	int musicHandle;

	int nextLevelId;
	int nextLevelParam;
	int nextLevelSpawnParam;

	S_NTF_SWITCH completionSwitch;

	int bCreditsComplete;
	int currentMessageIndex;
	float currentScrollSpeed;
	float boxFillOpacity;
	float scrollY;
	char* pCurrentMessage;

	char lineBuffer[64];
};

#endif //ACTOR_CREDITS_H