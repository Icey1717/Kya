#ifndef _ED_COLLISIONS_H
#define _ED_COLLISIONS_H

#include "Types.h"
#include "edMem.h"

#define COLLISION_LOG(level, format, ...) MY_LOG_CATEGORY("Collision", level, format, ##__VA_ARGS__)
#define COLLISION_LOG_VERBOSE(level, format, ...) 

#define COL_TYPE_TREE 0x1
#define COL_TYPE_TREE_DYN 0x3
#define COL_TYPE_TRIANGLE 0x4
#define COL_TYPE_QUAD 0x8
#define COL_TYPE_BOX 0xa
#define COL_TYPE_SPHERE 0xb
#define COL_TYPE_BOX_DYN 0xd
#define COL_TYPE_PRIM_OBJ 0xe
#define COL_TYPE_CAPSULE 0x13

struct edObbTREE_DYN;

PACK(
struct edF32TRIANGLE4
{
	union {
		struct {
			strd_ptr(edF32Vector*) p1;
			strd_ptr(edF32Vector*) p2;
			strd_ptr(edF32Vector*) p3;
		};

		int points[3];
	};

	uint flags;
});

static_assert(sizeof(edF32TRIANGLE4) == 0x10);


#ifdef PLATFORM_WIN
struct edF32TRIANGLE4_Stack
{
	union {
		struct {
			edF32VECTOR4* p1;
			edF32VECTOR4* p2;
			edF32VECTOR4* p3;
		};

		edF32VECTOR4* points[3];
	};

	uint flags;

	edF32TRIANGLE4_Stack() {
		p1 = nullptr;
		p2 = nullptr;
		p3 = nullptr;
		flags = 0;
	}

	// Conversion constructor from edF32TRIANGLE4*
	edF32TRIANGLE4_Stack(edF32TRIANGLE4* other) {
		p1 = LOAD_POINTER_CAST(edF32VECTOR4*, other->p1);
		p2 = LOAD_POINTER_CAST(edF32VECTOR4*, other->p2);
		p3 = LOAD_POINTER_CAST(edF32VECTOR4*, other->p3);
		flags = other->flags;
	}
};
#else
typedef edF32TRIANGLE4 edF32TRIANGLE4_Stack;
#endif

struct edColRAY_OBB_IN
{
	edF32VECTOR4* pLocation;
	edF32VECTOR4* pLeadVector;
	float lengthA;
};

PACK(
	struct edF32QUAD4 {
	strd_ptr(edF32VECTOR4*) p1;
	strd_ptr(edF32VECTOR4*) p2;
	strd_ptr(edF32VECTOR4*) p3;
	strd_ptr(edF32VECTOR4*) p4;
	uint flags;
});

static_assert(sizeof(edF32QUAD4) == 0x14);

PACK(
struct edColINFO {
	edF32MATRIX4 field_0x0;
	strd_ptr(edF32TRIANGLE4*) field_0x40;
	strd_ptr(edF32VECTOR4*) field_0x44;
	byte field_0x48;
	byte field_0x49;
	byte field_0x4a;
	byte field_0x4b;
	undefined field_0x4c;
	undefined field_0x4d;
	undefined field_0x4e;
	undefined field_0x4f;
});

PACK(
struct edColOBJECT {
	byte colType_0x0;
	byte colType_0x1;
	byte colType_0x2;
	undefined field_0x3;
	byte field_0x4;
	byte field_0x5;
	short field_0x6;
	short nbPrim;
	short nbPrimUsed;
	int field_0xc;
	uint colResult;
	strd_ptr(edDynOBJECT*) pDynObj;
	uint field_0x18[2];
	undefined field_0x20;
	undefined field_0x21;
	undefined field_0x22;
	undefined field_0x23;
	strd_ptr(edColPrimEntry*) pPrim;
	strd_ptr(CActor*) pActor;
});

struct edColINFO_OBBTREE_OBBTREE 
{
	edColOBJECT* pColObj;
	edColOBJECT* pOtherColObj;
	char* field_0x8;
	char* field_0xc;
};

struct edCol_INFO_OBB_TRIS_BOX
{
	edF32TRIANGLE4* pTris;
	edF32VECTOR4* pVector;
	int field_0x8;
	int field_0xc;
	int field_0x10;
};

struct edColARRAY_PRIM_TRI4
{
	edColOBJECT* pColObj;
	int aCount;
	int aType;
	int primSize;
	void* aData;
	edColOBJECT* pOtherColObj;
	int bCount;
	void* bData;
};

struct edColARRAY_PRIM_PRIM
{
	edColOBJECT* pColObj;
	int aCount;
	int aType;
	int primSize;
	void* aData;
	edColOBJECT* pOtherColObj;
	int bCount;
	int bType;
	int bPrimSize;
	void* bData;
};

struct edColARRAY_PRIM_QUAD4
{
	edColOBJECT* pColObj;
	int aCount;
	int aType;
	int primSize;
	void* aData;
	edColOBJECT* pOtherColObj;
	int bCount;
	void* bData;
};

struct edColARRAY_TRI4_PRIM
{
	edColOBJECT* pOtherColObj;
	uint bCount;
	int bType;
	int bSize;
	void* bData;
	edColOBJECT* pColObj;
	int bCount2;
	void* aData;
};

struct edColRAY_TRIANGLE4_IN
{
	edF32VECTOR4* pRayOrigin;
	edF32VECTOR4* pRayDirection;
	edF32TRIANGLE4_Stack* pTriangle;
};


struct edColPRIM_RAY_UNIT_BOX_UNIT_IN
{
	edF32VECTOR4* field_0x0;
	edF32VECTOR4* field_0x4;
};

PACK(
struct edColPRIM_SPHERE
{
	edF32MATRIX4 localToWorld;
	edF32MATRIX4 worldTransform;
	uint flags_0x80;	
	undefined4 field_0x84;
	undefined4 field_0x88;
	undefined4 field_0x8c;
});

static_assert(sizeof(edColPRIM_SPHERE) == 0x90);

PACK(
struct edColPRIM_BOX 
{
	edF32MATRIX4 localToWorld;
	edF32MATRIX4 worldTransform;
	uint flags_0x80;
	undefined4 field_0x84;
	undefined4 field_0x88;
	undefined4 field_0x8c;
});


static_assert(sizeof(edColPRIM_BOX) == 0x90);

PACK(
struct edColPRIM_OBJECT
{
	edF32MATRIX4 localToWorld;
	edF32MATRIX4 worldTransform;
	uint flags_0x80;
	undefined4 field_0x84;
	undefined4 field_0x88;
	undefined4 field_0x8c;
	edF32VECTOR4 scale;
	edF32VECTOR4 eulerAngles;
	edF32VECTOR4 position;
	edF32VECTOR4 field_0xc0;
	edF32VECTOR4 field_0xd0;

	edColINFO colInfo;

	undefined field_0x130;
	undefined field_0x131;
	undefined field_0x132;
	undefined field_0x133;
	undefined field_0x134;
	undefined field_0x135;
	undefined field_0x136;
	undefined field_0x137;
	undefined field_0x138;
	undefined field_0x139;
	undefined field_0x13a;
	undefined field_0x13b;
	undefined field_0x13c;
	undefined field_0x13d;
	undefined field_0x13e;
	undefined field_0x13f;
	uint field_0x140;
	undefined field_0x144;
	undefined field_0x145;
	undefined field_0x146;
	undefined field_0x147;
	undefined field_0x148;
	undefined field_0x149;
	undefined field_0x14a;
	undefined field_0x14b;
	undefined field_0x14c;
	undefined field_0x14d;
	undefined field_0x14e;
	undefined field_0x14f;
});

static_assert(sizeof(edColINFO) == 0x50);
static_assert(sizeof(edColPRIM_OBJECT) == 0x150);

PACK(
	struct edColG3D_OBB_TREE_DYN
{
	undefined field_0x0;
	byte bLoaded;
	undefined field_0x2;
	undefined field_0x3;
	int field_0x4;
	int field_0x8;
	int field_0xc;
	int vertexCount;
	int nbTriangles;
	int field_0x18;
	short field_0x1c;
	undefined field_0x1e;
	undefined field_0x1f;
	strd_ptr(edF32VECTOR*) aSourceVertices;
	strd_ptr(edF32VECTOR*) aVertices;
	strd_ptr(edF32TRIANGLE4*) aTriangles;
	strd_ptr(char*) field_0x2c;
	strd_ptr(char*) field_0x30;
	strd_ptr(char*) field_0x34;
	strd_ptr(edObbTREE*) pObbTree;
});

static_assert(sizeof(edColG3D_OBB_TREE_DYN) == 0x3c);

PACK(
struct edColG3D_OBB_TREE {
	undefined field_0x0;
	byte bLoaded;
	undefined field_0x2;
	undefined field_0x3;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	int nbTriangles;
	undefined4 field_0xc;
	int nbQuads;
	undefined field_0x14;
	undefined field_0x15;
	undefined field_0x16;
	undefined field_0x17;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
	int field_0x20;
	undefined field_0x24;
	undefined field_0x25;
	undefined field_0x26;
	undefined field_0x27;
	int field_0x28;
	strd_ptr(edF32TRIANGLE4*) aTriangles;
	strd_ptr(void*) field_0x30;
	strd_ptr(edF32QUAD4*) aQuads;
	strd_ptr(edColPRIM_BOX*) aBoxes;
	strd_ptr(edColPRIM_SPHERE*) aSpheres;
	undefined field_0x40;
	undefined field_0x41;
	undefined field_0x42;
	undefined field_0x43;
	strd_ptr(edObbTREE_DYN*) pObbTree;
});

PACK(
struct StaticCollisionEntry {
	undefined field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	int field_0x8;
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
	edColG3D_OBB_TREE obbTree;
});

struct edDynOBJECT {};

struct edColDbObj_80 {
	edColOBJECT* pColObj;
	edColOBJECT* pOtherColObj;
	undefined field_0x8;
	undefined field_0x9;
	undefined field_0xa;
	undefined field_0xb;
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
	edF32VECTOR4 field_0x10;
	undefined field_0x20;
	undefined field_0x21;
	undefined field_0x22;
	undefined field_0x23;
	undefined field_0x24;
	undefined field_0x25;
	undefined field_0x26;
	undefined field_0x27;
	undefined field_0x28;
	undefined field_0x29;
	undefined field_0x2a;
	undefined field_0x2b;
	undefined field_0x2c;
	undefined field_0x2d;
	undefined field_0x2e;
	undefined field_0x2f;
	edF32VECTOR4 location;
	edF32VECTOR4 field_0x40;
	edF32VECTOR4 field_0x50;
	short aType;
	short bType;
	void* pPrimitiveA;
	void* pPrimitiveB;
	edColINFO* field_0x6c;
	byte field_0x70;
	byte field_0x71;
	byte field_0x72;
	char field_0x73;
	uint flags;
	float depth;
	undefined field_0x7c;
	undefined field_0x7d;
	undefined field_0x7e;
	undefined field_0x7f;
};


struct IntersectResTestSub
{
	ushort type;
	ushort count;
	void* pData;
};

struct edColPRIM_RAY_UNIT_SPHERE_UNIT_IN
{
	edF32VECTOR4* field_0x0;
	edF32VECTOR4* field_0x4;
};

struct IntersectResTestA
{
	edObbTREE_DYN* pObbTree;
	IntersectResTestSub sub;
};

struct edColPrimEntry
{
	IntersectResTestSub a;
	IntersectResTestSub b;
};

struct edColDatabase {
	int curObjId;
	int curDbEntryCount;
	int curPrimId;
	undefined4 field_0xc;
	edColDbObj_80* aDbEntries;
	edColOBJECT* aColObj;
	edColPrimEntry* aPrim;
	undefined* field_0x1c;
	byte* field_0x20;
};

struct GlobalCollisionData {
	byte bInitialized;
	byte activeDatabaseId;
	undefined field_0x2;
	undefined field_0x3;
	edColDatabase* pActiveDatabase;
	edColDatabase* aDatabases;
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
	undefined4 field_0x10;
	undefined4 field_0x14;
	undefined* field_0x18;
	uint field_0x1c;
};

struct CollisionTD {
	byte field_0x0;
	byte field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	undefined* field_0x4;
	undefined* field_0x8;
	edColPrimEntry* aPrim;
	edColPrimEntry* pCurPrim;
	undefined* field_0x14;
	undefined* field_0x18;
	edColDatabase* pCurDatabase;
	edColOBJECT* pCurColObj;
	undefined field_0x24;
	undefined field_0x25;
	undefined field_0x26;
	undefined field_0x27;
	undefined field_0x28;
	undefined field_0x29;
	undefined field_0x2a;
	undefined field_0x2b;
	undefined field_0x2c;
	undefined field_0x2d;
	undefined field_0x2e;
	undefined field_0x2f;
	undefined field_0x30;
	undefined field_0x31;
	undefined field_0x32;
	undefined field_0x33;
	undefined field_0x34;
	undefined field_0x35;
	undefined field_0x36;
	undefined field_0x37;
	int field_0x38;
	int curObjId;
	int nbPrim;
	undefined4 field_0x44;
};

struct edColConfigDbTypeData
{
	short nbMax;
	short field_0x2;
};

struct edColConfigDbData
{
	edColConfigDbTypeData colObj;
	edColConfigDbTypeData dbObj;
	edColConfigDbTypeData primObj;
	edColConfigDbTypeData field_0xc;
};

struct edColConfig 
{
	byte bCreateProfileObj;
	byte field_0x1;
	byte databaseCount;
	byte field_0x3;
	byte field_0x4;
	char pad_0x5[3];
	edColConfigDbData aDbTypeData[1];
	short field_0x18;
	short nbPrimEntries;
	short field_0x1c;
	undefined field_0x1e;
	undefined field_0x1f;
	EHeap heapID_B;
	EHeap heapID_A;
	int bSetMemFlags;
};

struct edColINFO_OUT
{
	edF32VECTOR4 field_0x0;
	uint result;
	edF32VECTOR4 normal;
	edF32VECTOR4 intersectionPoint;
	edF32VECTOR4 relativeVelocity;
	float penetrationDepth;
	int field_0x48;
	int field_0x4c;
	int field_0x50;
};

void edColComputeMatrices(edColPRIM_OBJECT* pPrimObj);
edColG3D_OBB_TREE* edColLoadStatic(char* pFileBuffer, uint length, uint bConvertTriangles);

void edColIntersectRayUnitBoxUnit(edColINFO_OUT* pColInfoOut, edColPRIM_RAY_UNIT_BOX_UNIT_IN* pParams);

void edColGetNormalInWorldFromLocal(edF32VECTOR4* param_1, edF32MATRIX4* param_2, edF32VECTOR4* param_3);

uint edColArrayObjectPrimsPenatratingArrayPrims(edColARRAY_PRIM_PRIM* pParams);
uint edColArrayObjectPrimPenatratingArrayQuads4(edColARRAY_PRIM_QUAD4* pParams);
uint edColArrayObjectTriangles4PenatratingPrims(edColARRAY_TRI4_PRIM* pParams);
uint edColArrayObjectPrimPenatratingArrayTriangles4(edColARRAY_PRIM_TRI4* pParams);
void edColIntersectRayUnitSphereUnit(edColINFO_OUT* pColInfoOut, edColPRIM_RAY_UNIT_SPHERE_UNIT_IN* pParams);
bool edColIntersectRayTriangle4(float* pOutDistance, edColRAY_TRIANGLE4_IN* pRayTriangleIn);

edColConfig* edColGetConfig(void);
void edColInit(void);
void edColTerm(void);
void edColSetDataBase(int databaseId);
void edColBegin(int param_1, int databaseId);
void edColObjectSetCollisionsType(byte param_1, byte param_2, byte param_3);
void edColCollisionAddPrim(int aType, int aCount, void* param_3, void* aData, int bType, int bCount, void* bData);
edColOBJECT* edColEnd(edDynOBJECT* pDynObj);
void edColFreeTemporaryMemory();

extern GlobalCollisionData gColData;
extern edColConfig gColConfig;

extern uint prof_obb_col;
extern uint prof_prim_col;
extern uint prof_fast_col;

extern int _gColSizeOfPrims[21];

extern float FLOAT_004485a4;

#endif //_ED_COLLISIONS_H
