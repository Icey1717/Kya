#pragma once

#include "edCollision/edCollisions.h"
#include "Native/NativeDebugShapes.h"
#include "port/pointer_conv.h"
#include "DebugCollisionPrimitives.h"
#include "MathOps.h"

namespace Debug {
	namespace Collision {
		// Shared drawing helper for box collision primitives
		inline void DrawObbNodeTriangles(edColPRIM_BOX* pBox, const std::array<float, 4>& col)
		{
			// Draw the 12 triangular faces of the box collision primitive
			for (int triIdx = 0; triIdx < 12; triIdx++) {
				const edF32VECTOR4& v1 = g_cube_corners[g_cube_tri[triIdx][0]];
				const edF32VECTOR4& v2 = g_cube_corners[g_cube_tri[triIdx][1]];
				const edF32VECTOR4& v3 = g_cube_corners[g_cube_tri[triIdx][2]];

				// Transform each vertex by the box's local-to-world matrix
				edF32VECTOR4 v1_transformed, v2_transformed, v3_transformed;
				edF32Matrix4MulF32Vector4Hard(&v1_transformed, &pBox->localToWorld, (edF32VECTOR4*)&v1);
				edF32Matrix4MulF32Vector4Hard(&v2_transformed, &pBox->localToWorld, (edF32VECTOR4*)&v2);
				edF32Matrix4MulF32Vector4Hard(&v3_transformed, &pBox->localToWorld, (edF32VECTOR4*)&v3);

				// Draw the three edges of the triangle
				Renderer::Native::DebugShapes::AddActorLine(
					v1_transformed.x, v1_transformed.y, v1_transformed.z,
					v2_transformed.x, v2_transformed.y, v2_transformed.z,
					col[0], col[1], col[2], col[3]
				);
				Renderer::Native::DebugShapes::AddActorLine(
					v2_transformed.x, v2_transformed.y, v2_transformed.z,
					v3_transformed.x, v3_transformed.y, v3_transformed.z,
					col[0], col[1], col[2], col[3]
				);
				Renderer::Native::DebugShapes::AddActorLine(
					v3_transformed.x, v3_transformed.y, v3_transformed.z,
					v1_transformed.x, v1_transformed.y, v1_transformed.z,
					col[0], col[1], col[2], col[3]
				);
			}
		}

		// Shared DrawObbNode implementation for both actor and scenery collision trees
		inline void DrawObbNode(const edObbTREE* pNode, const ObbDrawConfig& config)
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
					edColPRIM_BOX* pBoxArr = LOAD_POINTER_CAST(edColPRIM_BOX*, pNode->field_0x54[0]);
					if (pBoxArr) {
						for (int i = 0; i < pNode->count_0x52; i++) {
							DrawObbNodeTriangles(&pBoxArr[i], col);
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
	}
}
