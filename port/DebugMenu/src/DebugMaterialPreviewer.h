#pragma once

#include <vector>
#include <string>
#include "imgui.h"
#include "DebugCallstackPreviewer.h"

namespace PS2 {
	struct GSTexValue;
}

struct edDList_material;

struct ImageTextureID {
	ImTextureID image;
	ImTextureID palette;
};

struct MaterialPreviewerEntry {
	MaterialPreviewerEntry(edDList_material* material, std::vector<DWORD64> inBacktrace)
		: pMaterial(material)
		, callstackEntry(inBacktrace)
	{}

	MaterialPreviewerEntry(edDList_material* material)
		: pMaterial(material)
	{}

	edDList_material* pMaterial;
	CallstackPreviewerEntry callstackEntry;

	bool operator==(const edDList_material* other) const {
		return pMaterial == other;
	}
};

namespace MaterialPreviewer {
	void Update();
	void Open(MaterialPreviewerEntry& entry, std::string name);
	void Open(const PS2::GSTexValue& texValue, const ImageTextureID& texIDs, std::string name);
	void Reset();
	void RemoveMaterial(edDList_material* pMaterial);
};