#pragma once

struct ed_g2d_material;
namespace Renderer {
	struct SimpleTexture;
	namespace DrawTrace { struct Source; struct Draw; }
}

namespace Debug
{
	namespace Texture 
	{
		void ShowMenu(bool* bOpen);

		void Update();

		void ShowMaterialDetails(const ed_g2d_material* pMaterial);
		Renderer::SimpleTexture* FindDrawTexture(const Renderer::DrawTrace::Source& source, const Renderer::DrawTrace::Draw& draw);
		bool OpenDraw(const Renderer::DrawTrace::Source& source, const Renderer::DrawTrace::Draw& draw);
	}
}
