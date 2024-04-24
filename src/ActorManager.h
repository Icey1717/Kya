#ifndef _ACTOR_MANAGER_H
#define _ACTOR_MANAGER_H

#include "Types.h"
#include "LargeObject.h"

class CActor;
class CCluster;
class CAnimation;
class CShadow {};
struct edAnmLayer;

struct CClassInfo {
	CActor* aActors;
	int totalCount;
	int allocatedCount;
	int size;
};

struct ed_Bound_Box
{
	edF32VECTOR4 br;
	edF32VECTOR4 tl;
};

class CClusterNode
{
public:
	CClusterNode();

	void Insert(CCluster* pCluster, CActor* pActor);
	void Update(CCluster* pCluster);

	CActor* pActor;
	CClusterNode* pNext;
	CClusterNode* pPrev;
	int nodeIndex;
};

typedef void (ColCallbackFuncPtr)(CActor*, void*);
typedef bool (CritenionFunc)(CActor*, void*);


class CCluster 
{
public:
	CCluster();
	void Init(int actorCount, ed_Bound_Box* pBoundBox, int param_4);
	void Term();
	void SetWorldBox(ed_Bound_Box* pBoundBox, int param_3);
	uint GetMapCoords(edS32VECTOR3* pOutCoords, edF32VECTOR4* pLocation);
	void DeleteNode(CClusterNode* pNode);
	CClusterNode* NewNode(CActor* pActor);

	bool GetActorsIntersectingSphereWithCriterion(CActorsTable* pTable, edF32VECTOR4* pLocation, CritenionFunc* pFunc, void* pData);

	void ApplyCallbackToActorsIntersectingSphere(edF32VECTOR4* pSphere, ColCallbackFuncPtr* pFunc, void* pParams);
	void ApplyCallbackToActorsIntersectingBox(S_BOUNDING_BOX* pBoundingBox, ColCallbackFuncPtr* pFunc, void* pParams);

	CClusterNode** ppNodes;

	undefined2 field_0x30;
	undefined2 field_0x32;

	CClusterNode* field_0x34;

	int nbClusterNodes;

	CClusterNode* pClusterNode;
	CClusterNode* aClusterNodes;

	edF32VECTOR4 vectorA;
	edF32VECTOR4 worldBox;
};

struct CActorManager : public CObjectManager {

	virtual void Level_Init();
	virtual void Level_AddAll(struct ByteCode* pMemoryStream);
	virtual void Level_Manage();
	virtual void Level_Draw();
	virtual void Level_SectorChange(int oldSectorId, int newSectorId);

	void PrecomputeSectorsBoundindBoxes();
	void UpdateLinkedActors();
	void Level_LoadClassesInfo(struct ByteCode* pMemoryStream);

	CActor* GetActorByHashcode(int hashCode);
	void GetActorsByClassID(int classId, CActorsTable* pOutList);

	struct ActorManagerAnimLinkData* field_0x4;
	CActor* pActorArray_0x8;
	int field_0xc;
	float field_0x14;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	int field_0x1c;
	undefined field_0x20;
	undefined field_0x21;
	undefined field_0x22;
	undefined field_0x23;
	int field_0x24;
	float field_0x28;
	float field_0x2c;
	float field_0x30;
	float field_0x34;
	float field_0x38;
	float field_0x3c;
	undefined field_0x40;
	undefined field_0x41;
	undefined field_0x42;
	undefined field_0x43;
	undefined field_0x44;
	undefined field_0x45;
	undefined field_0x46;
	undefined field_0x47;
	undefined field_0x48;
	undefined field_0x49;
	undefined field_0x4a;
	undefined field_0x4b;
	undefined field_0x4c;
	undefined field_0x4d;
	undefined field_0x4e;
	undefined field_0x4f;
	ed_Bound_Box* aSectorBoundingBoxes;

	CCluster cluster;

	CActor** aActors;
	int nbActors;

	CActor** aActiveActors;
	int nbActiveActors;

	CActor** aSectorActors;
	int nbSectorActors;

	CAnimation* aAnimation;
	int initializedAnimationCount;
	int animationCount;

	edAnmLayer* aAnimLayers;
	int initalizedAnimLayerCount;
	int animLayerCount;

	CShadow* aShadows;
	int initializedShadowCount;
	int shadowCount;

	CClassInfo aClassInfo[ACTOR_NUM_CLASSES];
};

#endif // _ACTOR_MANAGER_H
