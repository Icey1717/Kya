#ifndef ACTOR_WEAPON_H
#define ACTOR_WEAPON_H

#include "Types.h"
#include "ActorMovable.h"
#include "Pause.h"

#define WEAPON_BEHAVIOUR_PISTOL 0x3
#define WEAPON_BEHAVIOUR_SNIPER 0x4
#define WEAPON_BEHAVIOUR_RPG 0x6

#define WEAPON_STATE_IDLE 0x5
#define WEAPON_STATE_FIRE 0x9
#define WEAPON_STATE_RELOAD 0xa

class CActorWeapon;
class CActorWolfen;

class CBehaviourWeapon : public CBehaviour
{
public:
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);

	virtual bool Func_0x50() { return false; }
	virtual void Func_0x54() { IMPLEMENTATION_GUARD(); }
	virtual void Func_0x58() { IMPLEMENTATION_GUARD(); }
	virtual void Func_0x5c() { return; }

	CActorWeapon* pOwner;
};

struct AdvancedEntry
{
	float field_0x0;
	int field_0x4;
	float field_0x8;
	float field_0xc;
	int field_0x10;
	float field_0x14;
};

class CBehaviourWeaponBurst : public CBehaviourWeapon
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);

	virtual int GetBurstState();
	virtual float GetLaunchSpeed(int index);
	virtual bool Action(edF32VECTOR4* pPosition, CActor* pActor);

	virtual bool Func_0x68();
	virtual bool Func_0x74();
	virtual bool Func_0x78();

	void UpdateBurst();

	AdvancedEntry field_0x8[2];

	int field_0x38;
};

class CSniperBullet
{
public:
	void FUN_002d3be0(float param_1);
	void Draw(float param_1, edF32VECTOR4* pPosition, edF32VECTOR4* param_4, edF32VECTOR4* param_5, int param_6, uint param_7);
	edF32VECTOR4 field_0x0[0xc];
	int bInUse;
	float field_0xc4;
};

struct Bullet
{
	edF32VECTOR4 position;
	edF32VECTOR4 field_0x10;
	edF32VECTOR4 direction;

	int bulletState;
	float currentLifetime;
	float speed;
	CInstance3D* pInstance3d;
	CSniperBullet* field_0x40;
};

class CBehaviourWeaponRpg : public CBehaviourWeaponBurst
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);

	virtual bool Func_0x50() { return true; }

	S_ACTOR_STREAM_REF* field_0x3c;
};

class CBehaviourWeaponBullets : public CBehaviourWeaponBurst
{
public:
	// CBehaviour
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Term();
	virtual void Manage();
	virtual void Draw();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);

	// CBehaviourWeaponBurst
	virtual bool Action(edF32VECTOR4* pPosition, CActor* pActor);

	virtual void ResetBullet(Bullet* pBullet);
	virtual void FireBullet(Bullet* pBullet);
	virtual void HitBullet(float param_1, Bullet* pBullet);
	virtual void DrawBullets() = 0;

	Bullet* GetFreeBullet();
	void UpdateBulletsLife();
	void StartFx(Bullet* pBullet, uint param_3);

	S_STREAM_REF<CSound> field_0x3c;
	uint field_0x40;
	uint field_0x44;
	int field_0x48;
	float angle;
	CActorSound* field_0x50;
	int nbBullets;

	Bullet aBullets[8];
};

struct WeaponGlobal
{
	WeaponGlobal();
	void Init(float param_1, float param_2, uint param_4, ed_3D_Scene* pScene);
	CInstance3D* Add();
	void Remove(CInstance3D* pInstance);
	void Term();
	int g3dTextureIndex;
	int g3dMeshIndex;
	ed_3D_Scene* field_0x8;
	int nbEntries;
	int field_0x10;
	CInstance3D** pEntries;
	CInstance3D* field_0x18;
	float field_0x1c;
	edF32VECTOR4 boundingSphere;
	ed_3d_hierarchy_setup hierarchySetup;
};

class CBehaviourWeaponSniper : public CBehaviourWeaponBullets
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Term();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);

	virtual void ResetBullet(Bullet* pBullet);
	virtual void FireBullet(Bullet* pBullet);
	virtual void HitBullet(float param_1, Bullet* pBullet);
	virtual void DrawBullets();

	int field_0x344;
};

class CBehaviourWeaponPistol : public CBehaviourWeaponBullets
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);

	virtual void DrawBullets();

	int field_0x2e0;
	int field_0x2e4;
};

class CActorWeapon : public CActorMovable
{
public:
	static StateConfig _gStateCfg_WEA[9];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Reset();
	virtual void CheckpointReset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);

	void UnlinkWeapon();
	void LinkWeapon(CActorWolfen* pWolfen, uint boneId);

	int FUN_002d57c0();
	void FUN_002d57e0(int index);

	bool FUN_002d58a0();
	bool FUN_002d5830();

	float FUN_002d5710();

	float GetLaunchSpeed(int index);
	int GetBurstState();

	bool Action(edF32VECTOR4* pPosition, CActor* pActor);

	int field_0x1d0;
	int field_0x1d4;
	float field_0x1d8;
	float field_0x1dc;
	CActorWolfen* pOwningWolfen;
};

#endif //ACTOR_WEAPON_H