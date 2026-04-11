#ifndef OBB_TREE_H
#define OBB_TREE_H

#include "Types.h"

struct edColINFO_OBBTREE_OBBTREE;
struct edCol_INFO_OBB_TRIS_BOX;
struct edColRAY_OBB_IN;

PACK(
struct edObbBOX
{
	edF32MATRIX4 transform;

	float width;
	float height;
	float depth;

	undefined field_0x4c;
	undefined field_0x4d;
	undefined field_0x4e;
	undefined field_0x4f;
});

PACK(
struct edObbTREE
{
	edObbBOX bbox;
	undefined field_0x50;
	byte type;
	byte count_0x52;
	undefined field_0x53;
	strd_ptr(edObbTREE*) field_0x54[7];
});

static_assert(sizeof(edObbTREE) == 0x70);

PACK(
struct edObbTREE_DYN : public edObbTREE
{
	edF32MATRIX4 localMatrix;
	edF32VECTOR4 field_0xb0;
});

static_assert(sizeof(edObbTREE_DYN) == 0xc0);

uint edObbTreeIntersectObbTree(edColINFO_OBBTREE_OBBTREE* pColInfoObbTree, edObbTREE_DYN* pObbTreeA, edObbTREE_DYN* pObbTreeB);
float edObbIntersectObbTreeRayPrim(void** pOutHit, uint* pOutType, edObbTREE_DYN* pObbTree, edColRAY_OBB_IN* pRay);
bool edObbIntersect(edObbBOX* pBoxA, edObbBOX* pBoxB);
float edObbIntersectObbRay(edObbBOX* pBox, edF32VECTOR4* pRayStart, edF32VECTOR4* pRayEnd);
bool edObbRayPlaneClip(float param_1, float param_2, float* param_3, float* param_4);
int edObbIntersectObbtreeTrisBox(edCol_INFO_OBB_TRIS_BOX* pColInfoObbTrisBox, edObbTREE_DYN* pObbTree, edObbBOX* pObbBox, int* param_4, uint param_5);

#endif