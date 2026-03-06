#include "DebugSceneryCollision.h"

#include "CollisionManager.h"
#include "LargeObject.h"
#include "port/pointer_conv.h"
#include "edCollision/edCollisions.h"
#include "Native/NativeDebugShapes.h"

namespace Debug {
namespace Scenery {
static void DrawObbNode(const edObbTREE* pNode, const Debug::Collision::ObbDrawConfig& config)
{
if (!pNode) {
return;
}

if (pNode->type == COL_TYPE_TREE || pNode->type == COL_TYPE_TREE_DYN) {
if (config.bShowTreeNodes) {
const auto& col = (pNode->type == COL_TYPE_TREE_DYN) ? config.colTreeNodeDyn : config.colTreeNode;
Renderer::Native::DebugShapes::AddOBB(
pNode->bbox.transform.raw,
pNode->bbox.width, pNode->bbox.height, pNode->bbox.depth,
col[0], col[1], col[2], col[3]
);
}

for (int i = 0; i < pNode->count_0x52; i++) {
DrawObbNode(LOAD_POINTER_CAST(edObbTREE*, pNode->field_0x54[i]), config);
}
}
else if (pNode->type == COL_TYPE_TRIANGLE) {
if (config.bShowLeafBbox) {
const auto& bbox = config.colLeafBbox;
Renderer::Native::DebugShapes::AddOBB(
pNode->bbox.transform.raw,
pNode->bbox.width, pNode->bbox.height, pNode->bbox.depth,
bbox[0], bbox[1], bbox[2], bbox[3]
);
}

if (config.bShowTriangles) {
const auto& edge = config.colTriEdge;
const edF32TRIANGLE4* pTriArr = LOAD_POINTER_CAST(edF32TRIANGLE4*, pNode->field_0x54[0]);
if (pTriArr) {
for (int i = 0; i < pNode->count_0x52; i++) {
const edF32TRIANGLE4& tri = pTriArr[i];
const edF32VECTOR4* v1 = LOAD_POINTER_CAST(edF32VECTOR4*, tri.p1);
const edF32VECTOR4* v2 = LOAD_POINTER_CAST(edF32VECTOR4*, tri.p2);
const edF32VECTOR4* v3 = LOAD_POINTER_CAST(edF32VECTOR4*, tri.p3);
if (v1 && v2 && v3) {
Renderer::Native::DebugShapes::AddActorLine(v1->x, v1->y, v1->z, v2->x, v2->y, v2->z, edge[0], edge[1], edge[2], edge[3]);
Renderer::Native::DebugShapes::AddActorLine(v2->x, v2->y, v2->z, v3->x, v3->y, v3->z, edge[0], edge[1], edge[2], edge[3]);
Renderer::Native::DebugShapes::AddActorLine(v3->x, v3->y, v3->z, v1->x, v1->y, v1->z, edge[0], edge[1], edge[2], edge[3]);
}
}
}
}
}
else if (pNode->type == COL_TYPE_QUAD) {
if (config.bShowLeafBbox) {
const auto& bbox = config.colLeafBbox;
Renderer::Native::DebugShapes::AddOBB(
pNode->bbox.transform.raw,
pNode->bbox.width, pNode->bbox.height, pNode->bbox.depth,
bbox[0], bbox[1], bbox[2], bbox[3]
);
}

if (config.bShowQuads) {
const auto& edge = config.colQuadEdge;
const edF32QUAD4* pQuadArr = LOAD_POINTER_CAST(edF32QUAD4*, pNode->field_0x54[0]);
if (pQuadArr) {
for (int i = 0; i < pNode->count_0x52; i++) {
const edF32QUAD4& quad = pQuadArr[i];
const edF32VECTOR4* v1 = LOAD_POINTER_CAST(edF32VECTOR4*, quad.p1);
const edF32VECTOR4* v2 = LOAD_POINTER_CAST(edF32VECTOR4*, quad.p2);
const edF32VECTOR4* v3 = LOAD_POINTER_CAST(edF32VECTOR4*, quad.p3);
const edF32VECTOR4* v4 = LOAD_POINTER_CAST(edF32VECTOR4*, quad.p4);
if (v1 && v2 && v3 && v4) {
Renderer::Native::DebugShapes::AddActorLine(v1->x, v1->y, v1->z, v2->x, v2->y, v2->z, edge[0], edge[1], edge[2], edge[3]);
Renderer::Native::DebugShapes::AddActorLine(v2->x, v2->y, v2->z, v3->x, v3->y, v3->z, edge[0], edge[1], edge[2], edge[3]);
Renderer::Native::DebugShapes::AddActorLine(v3->x, v3->y, v3->z, v4->x, v4->y, v4->z, edge[0], edge[1], edge[2], edge[3]);
Renderer::Native::DebugShapes::AddActorLine(v4->x, v4->y, v4->z, v1->x, v1->y, v1->z, edge[0], edge[1], edge[2], edge[3]);
}
}
}
}
}
else if (pNode->type == COL_TYPE_PRIM_OBJ) {
if (config.bShowLeafBbox) {
const auto& bbox = config.colLeafBbox;
Renderer::Native::DebugShapes::AddOBB(
pNode->bbox.transform.raw,
pNode->bbox.width, pNode->bbox.height, pNode->bbox.depth,
bbox[0], bbox[1], bbox[2], bbox[3]
);
}

if (config.bShowPrimObjects) {
const auto& col = config.colPrimObj;
const edColPRIM_OBJECT* pPrimArr = LOAD_POINTER_CAST(edColPRIM_OBJECT*, pNode->field_0x54[0]);
if (pPrimArr) {
for (int i = 0; i < pNode->count_0x52; i++) {
Renderer::Native::DebugShapes::AddOBB(
pPrimArr[i].localToWorld.raw,
1.0f, 1.0f, 1.0f,
col[0], col[1], col[2], col[3]
);
}
}
}
}
else if (pNode->type == COL_TYPE_BOX || pNode->type == COL_TYPE_BOX_DYN) {
if (config.bShowLeafBbox) {
const auto& bbox = config.colLeafBbox;
Renderer::Native::DebugShapes::AddOBB(
pNode->bbox.transform.raw,
pNode->bbox.width, pNode->bbox.height, pNode->bbox.depth,
bbox[0], bbox[1], bbox[2], bbox[3]
);
}

if (config.bShowBoxes) {
const auto& col = config.colBox;
const edColPRIM_BOX* pBoxArr = LOAD_POINTER_CAST(edColPRIM_BOX*, pNode->field_0x54[0]);
if (pBoxArr) {
for (int i = 0; i < pNode->count_0x52; i++) {
Renderer::Native::DebugShapes::AddOBB(
pBoxArr[i].localToWorld.raw,
1.0f, 1.0f, 1.0f,
col[0], col[1], col[2], col[3]
);
}
}
}
}
else if (pNode->type == COL_TYPE_SPHERE || pNode->type == COL_TYPE_CAPSULE) {
if (config.bShowLeafBbox) {
const auto& bbox = config.colLeafBbox;
Renderer::Native::DebugShapes::AddOBB(
pNode->bbox.transform.raw,
pNode->bbox.width, pNode->bbox.height, pNode->bbox.depth,
bbox[0], bbox[1], bbox[2], bbox[3]
);
}

if (config.bShowSpheres) {
const auto& col = config.colSphere;
const edColPRIM_SPHERE* pSphereArr = LOAD_POINTER_CAST(edColPRIM_SPHERE*, pNode->field_0x54[0]);
if (pSphereArr) {
for (int i = 0; i < pNode->count_0x52; i++) {
Renderer::Native::DebugShapes::AddOBB(
pSphereArr[i].localToWorld.raw,
1.0f, 1.0f, 1.0f,
col[0], col[1], col[2], col[3]
);
}
}
}
}
else {
if (config.bShowUnknown) {
const auto& col = config.colUnknown;
Renderer::Native::DebugShapes::AddOBB(
pNode->bbox.transform.raw,
pNode->bbox.width, pNode->bbox.height, pNode->bbox.depth,
col[0], col[1], col[2], col[3]
);
}
}
}

void DrawDebugShapes(const Debug::Collision::ObbDrawConfig& config)
{
if (!config.bEnabled) {
return;
}

CCollisionManager* pColMgr = CScene::ptable.g_CollisionManager_00451690;
if (!pColMgr) {
return;
}

for (int i = 0; i < pColMgr->staticCollisionCount; i++) {
if (pColMgr->aStaticCollisionRefs[i]) {
DrawObbNode(pColMgr->aStaticCollisionRefs[i], config);
}
}
}
}
}
