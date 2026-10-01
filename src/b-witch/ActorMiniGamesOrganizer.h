#ifndef ACTOR_MINI_GAME_ORGANIZER_H
#define ACTOR_MINI_GAME_ORGANIZER_H

#include "Types.h"
#include "Actor.h"
#include "ActorHelperSign.h"
#include "Audio.h"

class CActorMiniGamesOrganizer;
class CActorMiniGamesManager;
class CActorMiniGame;

struct S_MENU_WHEEL_DRAW
{
	float x;
	float y;
	byte alpha;
};

class CMenuWheel
{
public:
	CMenuWheel();
	void Init(edDList_material* pMaterial, edDList_material* pArrow, edDList_material* pArrowHighlight);
	void Reset();
	bool Manage();
	void Draw();
	void MoveWheel(bool bNext);
	void MoveWheel();
	void FUN_002efa80(float x, float y, edF32VECTOR2* pRight, edF32VECTOR2* pLeft);

	CSprite field_0x0;
	astruct_22 field_0xc0;
	float field_0x270;
	int field_0x274;
	byte field_0x278;
	int field_0x27c;
	float field_0x280;
	float field_0x284;
	bool field_0x288;
	float field_0x28c;
	bool field_0x290;
	void (*pFunc)(int, int, S_MENU_WHEEL_DRAW*, void**);
	void** field_0x298;
};

#define MINI_GAMES_ORGANIZER_BEHAVIOUR_STAND 2

class CBehaviourMiniGamesOrganizer : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	CActorMiniGamesOrganizer* pOwner;
};

class CBehaviourMiniGamesOrganizerStand : public CBehaviourMiniGamesOrganizer
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void ManageFrozen();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CActorMiniGamesOrganizer : public CActor
{
public:
	CActorMiniGamesOrganizer();

	static StateConfig _gStateCfg_ORG[12];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Draw();
	virtual void Reset();
	virtual void CheckpointReset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void BehaviourMiniGamesOrganizerStand_Manage();
	void BehaviourMiniGamesOrganizerStand_InitState(int newState);
	void BehaviourMiniGamesOrganizerStand_TermState(int oldState);
	void ClearLocalData();
	void InitAlphabet();
	void InitMenuMeshes(int state);
	void TermMenuMeshes();
	void ComputeCurPlayMode();
	void NextPlayMode();
	void PrevPlayMode();
	void ManageMenuChoose();
	void ManageMenuBet();
	void ManageMenuMulti();
	void ManageMenuResult();
	void ManageMenuEnterName();
	void InitMenuBet();
	void ManageFade();
	void ManageMusic(int state);
	void ManageZone();
	void DrawMenuChooseText();
	void DrawMenuBetText();
	void DrawMenuMultiText();
	void DrawMenuTrainText();
	void DrawMenuResultText();
	void DrawMenuEnterNameText();
	void DrawAllLetters();
	void DrawLetterCursor(float x, float y, float halfWidth, float halfHeight);
	CActorMiniGame* GetMiniGame(int index);
	char* FUN_003b2a00();
	char* GetCurNameLetter(int index);

	CBehaviourMiniGamesOrganizerStand behaviourStand;
	char* field_0x168;
	int field_0x16c;
	int textureIndex_0x170;
	int field_0x174;
	int materialId_0x178;
	S_ACTOR_STREAM_REF* pMiniGameStreamRefs;
	uint field_0x180;
	uint field_0x184;
	int field_0x188;
	int field_0x18c;
	int field_0x190;
	float field_0x194;
	ed_3d_hierarchy_setup field_0x198;
	edF32VECTOR4 field_0x1b8;
	float field_0x1c8;
	StaticMeshComponent field_0x1d0;
	StaticMeshComponent field_0x230;
	StaticMeshComponent field_0x290;
	StaticMeshComponent field_0x2f0;
	StaticMeshComponent field_0x350;
	StaticMeshComponent field_0x3b0;
	StaticMeshComponent field_0x410;
	StaticMeshComponent field_0x470;
	CMenuWheel menuWheel;
	astruct_22 field_0x76c;
	int field_0x91c;
	int field_0x920;
	int field_0x924;
	int field_0x928;
	int field_0x92c;
	int field_0x930;
	int field_0x934;
	int field_0x938;
	int field_0x93c;
	bool field_0x940;
	bool field_0x941;
	float field_0x944;
	float field_0x948;
	byte field_0x94c;
	float field_0x950;
	float field_0x954;
	char field_0x958[30][2];
	int field_0x994;
	int field_0x998;
	int field_0x99c[4];
	char field_0x9ac[3][2];
	int field_0x9b4;
	edDList_material* field_0x9b8;
	int field_0x9bc;
	ed_g2d_manager field_0x9c0;
	CActor* field_0x9f0;
	CActorMiniGamesManager* field_0x9f4;
	int field_0x9f8;
	int field_0x9fc;
	CSoundInstance field_0xa00;
};

#endif //ACTOR_MINI_GAME_ORGANIZER_H
