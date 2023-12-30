#pragma once

#include "TextureCache.h"
#include "imgui.h"
#include "DebugCallstackPreviewer.h"
#include <string>

struct edDList_material;
struct ed_g2d_material;

namespace DebugHelpers {
	struct DebugMaterialKey {
		DebugMaterialKey(void* pInMaterial)
			: pMaterial(pInMaterial)
			, texID(nullptr)
		{}

		DebugMaterialKey(const PS2::GSTexValue& inTexture, ImTextureID inTexID)
			: pMaterial(nullptr)
			, texID(inTexID)
		{}

		void* pMaterial;
		ImTextureID texID;

		bool operator==(const DebugMaterialKey* other) const {
			return pMaterial == other->pMaterial && texID == other->texID;
		}

		bool operator==(const void* other) const {
			return pMaterial == other;
		}
	};

	struct DebugMaterial {
		DebugMaterial(edDList_material* pInMaterial, uint32_t inCBP, bool bCreateTexID = true);
		DebugMaterial(const PS2::GSTexValue& inTexture, ImTextureID inTexID);
		DebugMaterial(ed_g2d_material* pInMaterial, uint32_t inCBP, bool bCreateTexID = true);

		DebugMaterialKey key;
		PS2::GSTexValue texture;
		ImTextureID texID;
		ImTextureID paletteTexID;
		CallstackPreviewerEntry callstackEntry;
		std::string name;
		uint32_t CBP;

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