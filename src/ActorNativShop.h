#ifndef ACTOR_NATIV_SHOP_H
#define ACTOR_NATIV_SHOP_H

#include "Types.h"
#include "Actor.h"
#include "camera.h"
#include "PathFollow.h"

#define NATIV_SHOP_BEHAVIOUR_SELL 0x2

#define NATIV_SHOP_STATE_ENTERING 0x6
#define NATIV_SHOP_STATE_DISPLAY 0x7
#define NATIV_SHOP_STATE_LEAVING 0x8

class CMusic;
class edCTextStyle;

struct NativShopSubObj
{
	S_STREAM_REF<CActor> streamRefActor;
	int field_0x4;
	int field_0x8;
	int field_0xc;
	int field_0x10;
	uint field_0x14;
};

class CActorNativShop;

struct ShopObjData
{
	void Display_Object(CActor* pActor);

	CActor* pActor;
	edNODE* pNode;
	ed_3d_hierarchy* pHierarchy;
	ed_Chunck* pTextureInfo;
};

class CBehaviourNativShopSell : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Term();
	virtual void Manage();
	virtual void SectorChange(int oldSectorId, int newSectorId);
	virtual void PauseChange(int bPaused);
	virtual void Draw();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	virtual void UpdateValidity();

	void State_Entering_Init();
	void State_Entering();

	void State_Display_Init();
	void State_Display();

	void State_Leaving_Init();
	void State_Leaving();

	void Display_Cursor(float scale, _rgba color);
	void Display_BottomBackGround();
	void Display_Sprite(edDList_material* pMaterial, Rectangle* pRect, _rgba color);
	void Display_BubbleText(edCTextStyle* pStyle);
	void Display_Text(float param_1, float param_2, edCTextStyle* pStyle, char* pText);
	void Display_Mts(float param_1, float param_2, edCTextStyle* pStyle, uint param_5);

	S_STREAM_REF<CCamera> streamRefCamera;
	S_STREAM_REF<ed_zone_3d> streamRefZone;

	NativShopSubObj aSubObjs[5];

	CActorNativShop* pOwner;

	int field_0x94;
	int field_0x98;
	undefined4 field_0x9c;

	CPathFollowReader pathFollowReader;

	float field_0xa4;
	float field_0xa8;
	int field_0xac;

	undefined4 field_0xb0;
	float field_0xb4;
	CMusic* field_0xb8;

	int field_0xc0;
	float field_0xc4;
	float field_0xc8;
	float field_0xcc;

	int materialId;

	ShopObjData objData;
};

class CActorNativShop : public CActor
{
public:
	static StateConfig _gStateCfg_SHO[5];

	// CActor Interface
	virtual void Create(ByteCode* pByteCode);

	virtual void Init();

	virtual void SaveContext(uint*, int);
	virtual void LoadContext(uint*, int);

	virtual CBehaviour* BuildBehaviour(int behaviourType);

	virtual StateConfig* GetStateCfg(int state);
	// End Interface

	CInventoryInfo* GetInventoryInfoForPurchase(int param_2);

	S_ACTOR_STREAM_REF* pActorStream;

	float field_0x164;

	undefined4 field_0x168;

	char* field_0x16c;

	uint field_0x170;
	uint field_0x174;
	S_STREAM_REF<CActor> actorRef;
	undefined4 field_0x17c;

	CBehaviourNativShopSell behaviourNativShop;
};

#endif //ACTOR_NATIV_SHOP_H