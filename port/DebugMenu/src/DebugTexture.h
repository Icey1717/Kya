#pragma once

struct ed_g2d_material;

namespace Debug
{
	namespace Texture 
	{
		void ShowMenu(bool* bOpen);

		void Update();

		void ShowMaterialDetails(const ed_g2d_material* pMaterial);
	}
}