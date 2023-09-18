#pragma once

#include "TextureCache.h"
#include "imgui.h"
#include "DebugCallstackPreviewer.h"
#include <string>

struct edDList_material;

namespace DebugHelpers {
	struct DebugMaterialKey {
		DebugMaterialKey(edDList_material* pInMaterial)
			: pMaterial(pInMaterial)
			, texID(nullptr)
		{}

		DebugMaterialKey(const PS2::GSTexValue& inTexture, ImTextureID inTexID)
			: pMaterial(nullptr)
			, texID(inTexID)
		{}

		edDList_material* pMaterial;
		ImTextureID texID;

		bool operator==(const DebugMaterialKey* other) const {
			return pMaterial == other->pMaterial && texID == other->texID;
		}

		bool operator==(const edDList_material* other) const {
			return pMaterial == other;
		}
	};

	struct DebugMaterial {
		DebugMaterial(edDList_material* pInMaterial, uint32_t CBP, bool bCreateTexID = true);
		DebugMaterial(const PS2::GSTexValue& inTexture, ImTextureID inTexID);

		DebugMaterialKey key;
		PS2::GSTexValue texture;
		ImTextureID texID;
		ImTextureID paletteTexID;
		CallstackPreviewerEntry callstackEntry;
		std::string name;

		inline ImTextureID GetTextureID() {
			return texID;
		}

		bool operator==(const DebugMaterialKey* other) const {
			return key == other;
		}

		bool operator==(const edDList_material* other) const {
			return key == other;
		}
	};

	inline bool operator==(const DebugMaterialKey& lhs, const DebugMaterialKey& rhs) {
		// Implement the comparison logic for DebugMaterialKey.
		// Return true if the key is equal, otherwise return false.
		// Example implementation:
		return lhs.pMaterial == rhs.pMaterial && lhs.texID == rhs.texID;
	}
}