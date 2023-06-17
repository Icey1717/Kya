#pragma once

#include "TextureCache.h"
#include "imgui.h"

struct edDList_material;

namespace DebugHelpers {
	struct DebugMaterial {
		DebugMaterial(edDList_material* pInMaterial, bool bCreateTexID = true);

		edDList_material* pMaterial;
		PS2::GSTexValue texture;
		ImTextureID texID;

		Renderer::TextureData LoadTextureData();

		inline ImTextureID GetTextureID() {
			return texID;
		}

		bool operator==(const edDList_material* other) const {
			return pMaterial == other;
		}
	};
}