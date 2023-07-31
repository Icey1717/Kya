#pragma once

#include <vector>
#include <string>
#include "imgui.h"
#include "DebugCallstackPreviewer.h"

namespace PS2 {
	struct GSTexValue;
}

struct MaterialPreviewerEntry {
	MaterialPreviewerEntry(struct edDList_material* material, std::vector<DWORD64> inBacktrace)
		: pMaterial(material)
		, callstackEntry(inBacktrace)
	{}

	MaterialPreviewerEntry(struct edDList_material* material)
		: pMaterial(material)
	{}

	struct edDList_material* pMaterial;
	CallstackPreviewerEntry callstackEntry;

	bool operator==(const edDList_material* other) const {
		return pMaterial == other;
	}
};

namespace MaterialPreviewer {
	void Show(MaterialPreviewerEntry& entry, std::string name, bool& bOpen);
	void Show(const PS2::GSTexValue& texValue, const ImTextureID& texID, std::string name, bool& bOpen);
	void Reset();
	void RemoveMaterial(struct edDList_material* pMaterial);
};