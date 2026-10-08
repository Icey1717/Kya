#pragma once

struct edNODE;
struct ed_g2d_material;
struct ed_3d_sprite;
union edF32MATRIX4;

namespace Renderer
{
	struct SimpleMesh;
	namespace Kya
	{
		namespace Sprite
		{
			void SetTransform(const edF32MATRIX4& model, float normalScale);
			void ProcessVertices(ed_3d_sprite* pSprite, SimpleMesh* pMesh, const edF32MATRIX4* model = nullptr, float normalScale = 1.0f);
			void RenderNode(const edNODE* pNode);
		}
	}
}
