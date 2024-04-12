#ifndef _COLLISION_MANAGER_H
#define _COLLISION_MANAGER_H

#include "Types.h"
#include "LargeObject.h"
#include "EdenLib/edCollision/sources/edCollisions.h"

#define MATERIAL_TABLE_SIZE 0xc

struct s_collision_contact
{
	edF32VECTOR4 location;
	edF32VECTOR4 field_0x10;
	uint materialFlags;
	short nbCollisionsA;
	short nbCollisionsB;
};

class CActor;

struct S_TIED_ACTOR_ENTRY {
	CActor* pActor;
	S_TIED_ACTOR_ENTRY* pNext;
	S_TIED_ACTOR_ENTRY* pPrev;
	int carryMethod;
};

struct S_CARRY_ACTOR_ENTRY {
	edF32MATRIX4 m0;
	edF32MATRIX4 m1;
	int field_0x80;
};

class CCollision
{
public:

	CCollision();

	uint flags_0x0;
	uint flags_0x4;
	byte primType;

	S_TIED_ACTOR_ENTRY* pTiedActorEntry;
	S_CARRY_ACTOR_ENTRY* pCarryActorEntry;

	edColOBJECT* pColObj;

	s_collision_contact aCollisionContact[3];

	edF32VECTOR4 field_0x90;

	edColG3D_OBB_TREE_DYN* pDynCol;

	edObbTREE_DYN* pObbTree;
	edColPRIM_OBJECT* pObbPrim;

	edF32MATRIX4 transformMatrix;

	edF32VECTOR4 lowestVertex;
	edF32VECTOR4 highestVertex;

	CActor* actorField;
	CActor* actorFieldA;

	int field_0x18;

	CActor* field_0x1c;

	void Init() {}

	void Create(CActor* pActor, int index);
	void ClearInternalData(void);
	void SetupInternalData(CActor* pActor);
	void InvalidatePrims();

	static edObbTREE_DYN* GetFirstObbPrimRecurse(edObbTREE_DYN* pObbTree);
	static void PatchObbTreeFlagsRecurse(edObbTREE_DYN* pObbTree, int param_2, int param_3, int param_4);
	static void SetObbTreePositionRecurse(edObbTREE_DYN* pObbTree, edF32MATRIX4* pMatrix);
	static void SetObbTreeMatrixNoRotationRecurse(edObbTREE_DYN* pObbTree, edF32MATRIX4* param_2, edF32MATRIX4* param_3);
	static void SetObbTreePositionNoRotationRecurse(edObbTREE_DYN* pObbTree, edF32MATRIX4* param_2);
	static void ComputeG3DObbTreeLowestAndHighestVertices(edF32VECTOR4* pHighestVertex, edF32VECTOR4* pLowestVertex, int param_3, edColG3D_OBB_TREE_DYN* pDynCol);
	static void ComputeObbTreeLowestAndHighestVerticesRecurse(edF32VECTOR4* pHighestVertex, edF32VECTOR4* pLowestVertex, int param_3, edObbTREE_DYN* pObbTree);
	static void ComputePrimLowestAndHighestVertices(edF32VECTOR4* pHighestVertex, edF32VECTOR4* pLowestVertex, int param_3, edColPRIM_OBJECT* pPrimObj, byte type);
	static void SetObbTreeMatrixRecurse(edObbTREE_DYN* pObbTree, edF32MATRIX4* param_2, edF32MATRIX4* param_3);
	static void TransformG3DObbTreeVertices(edColG3D_OBB_TREE_DYN* pDynCol, int matrixType, edF32MATRIX4* pTransformMatrix);
	static float GetWallNormalYLimit(s_collision_contact* pContact);
	static int GetPrimNumDeltaSubdivisions(edColPRIM_OBJECT* pPrimObj, byte colType, edF32VECTOR4* param_3);

	uint CheckCollisionsWithActors(CActor* pActor, edF32MATRIX4* m0);
	uint CheckCollisionsWithScenery(int param_2);
	uint ResolveContacts(CActor* pActor, edF32VECTOR4* pTranslation, int param_4);
	void PreprocessActorContacts(float param_1, CActor* pActor, CActorsTable* pTable, CActor** pOutActor);
	uint CheckCollisions_OBBTree(CActor* pActor, edF32MATRIX4* m0, edF32MATRIX4* param_4, edF32VECTOR4* pTranslation, CActorsTable* param_6, CActor** param_7, int param_8);
	void CheckCollisions_UpdateCollisionMatrix(CActor* pActor, edF32MATRIX4* pMatrix, CActorsTable* pActorTable, CActor* param_5, int param_6);
	void CheckCollisions_TranslateActor(CActor* pActor, edF32VECTOR4* param_3, CActorsTable* param_4, CActor* param_5, int param_6);
	void UpdateMatrix(edF32MATRIX4* param_2);

	void RegisterTiedActor(CActor* pActorTo, CActor* pActor, int carryMethod);
	CActor* FindTiedActor(CActor* pActor);
	float GetSubjectiveCarriedWeight();
	float GetCarriedWeight();

	void CheckCollisions_MoveActor(CActor* pActor, edF32MATRIX4* m0, CActorsTable* pActorsTable, CActor* pOtherActor, int param_6);
	void CheckCollisions_MoveActor(CActor* pActor, edF32VECTOR4* pVector, CActorsTable* pActorsTable, CActor* pOtherActor, int param_6);
};

struct MaterialTableEntry
{
	float field_0x0;
	float field_0x4;
	float field_0x8;
	float slidingCoefficient;
	float field_0x10;
};

PACK(
struct DynColEntry
{
	int field_0x0;
	int field_0x4;
	int field_0x8;
	char szTag[8];
	int field_0x14;
	int field_0x18;
	int field_0x1c;
	edColG3D_OBB_TREE_DYN dynCollision;
});

struct BankCollision_14
{
	DynColEntry* pBase;
	int dynColSize;
	int field_0x8;
	int useCount_0xc;
	DynColEntry* pNextFree;
};

class CCollisionManager : public CObjectManager {
public:

	static S_TIED_ACTOR_ENTRY _tied_actors_table[0x100];
	static S_CARRY_ACTOR_ENTRY _carry_info_table[0x80];

	CCollisionManager();
	void Level_Create(ByteCode* pMemoryStream);
	void Level_PostCreate();
	void Level_AddAll(ByteCode* pByteCode);

	static bool IsASlidingGround(s_collision_contact* pContact);
	static float GetMaterialSlidingCoef(s_collision_contact* pContact);


	CCollision* NewCCollision();

	void ClearStaticData();

	edColG3D_OBB_TREE_DYN* InstanciateDynCol(int index);
	edObbTREE_DYN* InstallColFile(char* pFile, uint size);

	int bankCollisionCount;
	int loadedBankCount_0x8;
	int totalCollisionCount;
	DynColEntry* pDynColHeap;

	edObbTREE_DYN* aStaticCollisionRefs[0x10];
	int staticCollisionCount;

	int inUseCollisionDataCount;
	CCollision* pCollisionDataArray;

	BankCollision_14* pBankCollisionData;

	static MaterialTableEntry _material_table[MATERIAL_TABLE_SIZE];
};

#endif // _COLLISION_MANAGER_H
