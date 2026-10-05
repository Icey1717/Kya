#pragma once

#include "FlareProjection.h"

namespace Renderer
{
	struct SimpleTexture;

	namespace Native
	{
		struct FlareDraw : FlareScreenData
		{
			SimpleTexture* pTexture = nullptr;
			bool occlusionEnabled = true;
		};
	}
}
