#ifndef ACTOR_ROPE_H
#define ACTOR_ROPE_H

#include "Types.h"
#include "Actor.h"
#include "CinematicManager.h"

#define ROPE_BEHAVIOUR_STAND 0x2

#define ROPE_STAND_STATE_SNAPPED 0x6

class CActorRope;
class CPathFollow;

class CBehaviourRopeStand : public CBehaviour
{
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	CActorRope* pOwner;
};

class CActorRope : public CActor
{
public:
	static StateConfig _gStateCfg_ROP[2];

	~CActorRope();

	virtual void Create(ByteCode* pByteCode);

	virtual void Init();
	virtual void Term();

	virtual void Draw();

	virtual void Reset();

	virtual void SaveContext(void* pData, uint mode, uint maxSize);
	virtual void LoadContext(uint*, int);

	virtual CBehaviour* BuildBehaviour(int behaviourType);

	virtual StateConfig* GetStateCfg(int state);

	virtual bool IsLockable();

	virtual int GetNumVisualDetectionPoints();
	virtual void GetVisualDetectionPoint(edF32VECTOR4* pOutPoint, int index);

	void BuildCollisionPrims();
	void PrecomputeDrawParams();
	void DrawTightened();
	float DrawRopeSegment(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4, edF32VECTOR4* param_5);
	int GetNearestSegment(edF32VECTOR4* pPosition);
	void TieActors(int bDoSwitches);

	CPathFollow* pPathFollow;
	S_STREAM_REF<CActor> actorStreamRefA;
	S_STREAM_REF<CActor> actorStreamRefB;
	S_NTF_SWITCH targetSwitch;
	int ropeMaterialId;
	float ropeScale;
	float field_0x17c;
	_rgba ropeColor;
	uint ropeFlags;
	undefined4 field_0x188;
	S_STREAM_REF<CSound> soundStreamRef;
	CActorSound* pActorSound;
	int field_0x194;
	int nbSegments;
	DisplayList* pDisplayList;
	edF32MATRIX4 field_0x1a0;

	CBehaviourRopeStand behaviourRopeStand;
};

#endif //ACTOR_ROPE_H