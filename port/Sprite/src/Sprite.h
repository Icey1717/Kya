#pragma once

struct edNODE;
struct ed_g2d_material;
struct ed_3d_sprite;

namespace Renderer
{
	struct SimpleMesh;
	namespace Kya
	{
		namespace Sprite
		{
			void ProcessVertices(ed_3d_sprite* pSprite, SimpleMesh* pMesh);
			void RenderNode(const edNODE* pNode);
		}
	}
}
