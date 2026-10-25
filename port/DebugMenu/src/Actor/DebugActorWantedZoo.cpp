#include "DebugActorWantedZoo.h"

#include "ActorWantedZoo.h"
#include "CinematicManager.h"
#include "../DebugTexture.h"
#include "LevelScheduler.h"
#include "MemoryStream.h"
#include "Texture.h"
#include "TranslatedTextData.h"
#include "edBank/edBankFile.h"
#include "renderer.h"

#include <imgui.h>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Debug::Actor::WantedZoo
{
	static std::string Lowercase(std::string text)
	{
		for (char& c : text) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		return text;
	}

	static const char* GetText(const Zoo_10& data, int index)
	{
		if (!data.field_0x4 || index >= data.field_0x0) return "-";
		const uint* key = data.field_0x4 + index * 2;
		const char* text = gMessageManager.get_message(ByteCode::BuildU64(key[0], key[1]));
		return text ? text : "(missing translation)";
	}

	static CCinematic* FindPosterCinematic(CScenaricCondition& condition)
	{
		if (!g_CinematicManager_0048efc || !condition.pData) return nullptr;
		const auto* conditions = reinterpret_cast<const S_STREAM_SIMPLE_COND*>(condition.pData + 1);
		for (int i = 0; i < g_CinematicManager_0048efc->GetNumCutscenes_001c50b0(); ++i) {
			auto* cinematic = g_CinematicManager_0048efc->GetCinematic(i);
			if (!cinematic || !cinematic->fileName || !cinematic->cond_0x248.pData) continue;
			const auto name = Lowercase(cinematic->fileName);
			if (name.find("quest_") != 0 || name.find("_stand") == std::string::npos) continue;
			const auto* posterConditions = reinterpret_cast<const S_STREAM_SIMPLE_COND*>(cinematic->cond_0x248.pData + 1);
			for (int j = 0; j < condition.GetNumSimpleConds(); ++j) {
				for (int k = 0; k < cinematic->cond_0x248.GetNumSimpleConds(); ++k) {
					if (conditions[j].variable == posterConditions[k].variable) return cinematic;
				}
			}
		}
		return nullptr;
	}

	// Keep preview buffers/managers at stable addresses for the texture library's lifetime.
	// Install only G2D data; previewing a poster never starts its cinematic.
	struct PosterBank
	{
		std::vector<char> data;
		ed_g2d_manager manager{};
	};
	static std::map<std::string, std::unique_ptr<PosterBank>> gPosterBanks;

	static bool LoadPosterBank(CCinematic* cinematic, const std::string& textureName)
	{
		auto* scheduler = CLevelScheduler::gThis;
		if (!scheduler || !cinematic->pBankName_0x48 || scheduler->currentLevelID < 0 || scheduler->currentLevelID >= 16) return false;
		const auto path = std::filesystem::path(scheduler->levelPath).relative_path()
			/ scheduler->aLevelInfo[scheduler->currentLevelID].levelName / "CINE" / cinematic->pBankName_0x48;
		const std::string key = path.string();
		const auto existing = gPosterBanks.find(key);
		if (existing != gPosterBanks.end()) return !existing->second->data.empty();
		auto& preview = gPosterBanks[key];
		preview = std::make_unique<PosterBank>();
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file) file = std::ifstream(std::filesystem::path("assets") / path, std::ios::binary | std::ios::ate);
		if (!file) return false;
		const auto length = file.tellg();
		if (length < sizeof(edCFiler_Bnk_static_header)) return false;
		std::vector<char> bytes(static_cast<size_t>(length));
		file.seekg(0);
		if (!file.read(bytes.data(), bytes.size())) return false;
		auto* header = reinterpret_cast<edCFiler_Bnk_static_header*>(bytes.data());
		if (memcmp(header->fileHeader.header, "KNAB", 4) != 0) return false;
		// Quest banks have uncompressed G2D entries. Check the tables and entry
		// bounds before using the bank's offset-based accessors on our copy.
		const auto& bank = header->fileHeader;
		if (bank.fileHeaderDataOffset <= 0 || (bank.flags_0x4 & 1) != 0 ||
			uint64_t(bank.fileHeaderDataOffset) + 8 + uint64_t(bank.fileCount) * 16 > bytes.size()) return false;
		for (uint i = 0; i < bank.fileCount; ++i) {
			const auto* entry = header->fileHeader.get_entry(i);
			if (entry->offset < 0 || entry->size < sizeof(GXD_FileHeader) ||
				uint64_t(entry->offset) + uint64_t(entry->size) > bytes.size()) continue;
			const char* data = header->fileHeader.get_entry_data(i);
			const auto* g2d = reinterpret_cast<const GXD_FileHeader*>(data);
			if (g2d->hash != HASH_CODE_T2D || (g2d->flags & 1) == 0) continue;
			preview->data.assign(data, data + entry->size);
			int unused;
			NAME_NEXT_OBJECT("Wanted Zoo preview: %s", textureName.c_str());
			ed3DInstallG2D(preview->data.data(), entry->size, &unused, &preview->manager, 1);
			return true;
		}
		return false;
	}

	struct PosterTexture
	{
		const Renderer::SimpleTexture* texture = nullptr;
		const ed_g2d_material* material = nullptr;
	};

	static PosterTexture FindPosterTexture(CCinematic* cinematic)
	{
		if (!cinematic || !cinematic->fileName) return {};
		const auto name = Lowercase(cinematic->fileName);
		const auto textureName = name.substr(0, name.find("_stand")) + ".g2d";
		PosterTexture result;
		auto find = [&](const Renderer::Kya::G2D& file) {
			if (Lowercase(file.GetName()).find(textureName) == std::string::npos) return;
			for (const auto& material : file.GetMaterials()) {
				for (const auto& layer : material.layers) {
					for (const auto& texture : layer.textures) {
						const auto* bitmap = texture.bitmap.GetBitmap();
						// Verified in all nine quest banks: one 128x128 portrait,
						// three 128x64 poster parts, and an 8x8 swatch. Material
						// ordering varies; Silver Micken's portrait is material 3.
						if (bitmap && bitmap->width == 128 && bitmap->height == 128 && texture.pSimpleTexture) {
							result = { texture.pSimpleTexture.get(), material.pMaterial };
							return;
						}
					}
				}
			}
		};
		Renderer::Kya::GetTextureLibrary().ForEach(find);
		if (!result.texture && LoadPosterBank(cinematic, textureName)) Renderer::Kya::GetTextureLibrary().ForEach(find);
		return result;
	}

	static std::string GetCaptureName(CCinematic* cinematic, const Zoo_10& data)
	{
		if (!cinematic || !cinematic->fileName) return GetText(data, 1);
		const std::string name = cinematic->fileName;
		const auto start = Lowercase(name).find("_stand");
		if (start == std::string::npos) return GetText(data, 1);
		const auto end = name.find('_', start + 6);
		const auto variant = name.substr(start + 6, end - (start + 6));
		std::string label;
		for (char c : variant) {
			if (!label.empty() && std::isupper(static_cast<unsigned char>(c))) label += ' ';
			label += c;
		}
		return label.empty() ? GetText(data, 1) : label;
	}

	static void ShowDetails(CActorWantedZoo* zoo, Zoo_14& entry, CCinematic* cinematic)
	{
		auto& condition = entry.scenaricCondition;
		if (condition.pData) {
			static const char* comparisons[] = { "always", "<", "<=", "==", ">=", ">", "!=" };
			static const char* operations[] = { "AND", "OR", "XOR" };
			const auto* conditions = reinterpret_cast<const S_STREAM_SIMPLE_COND*>(condition.pData + 1);
			if (condition.GetNumSimpleConds() == 0) ImGui::TextUnformatted("Always eligible.");
			for (int i = 0; i < condition.GetNumSimpleConds(); ++i) {
				const auto& cond = conditions[i];
				ImGui::Text("%s variable 0x%X %s %d (current: %d)",
					cond.field_0x0 >= 0 && cond.field_0x0 < 3 ? operations[cond.field_0x0] : "?", cond.variable,
					cond.field_0x8 >= 0 && cond.field_0x8 < 7 ? comparisons[cond.field_0x8] : "?", cond.field_0xc,
					CLevelScheduler::ScenVar_Get(cond.variable));
			}
		}
		if (cinematic) ImGui::TextWrapped("Poster cinematic: %s", cinematic->fileName);
		const auto& data = entry.field_0x4;
		for (int i = 0; data.field_0x4 && i < data.field_0x0; ++i) {
			const uint* key = data.field_0x4 + i * 2;
			ImGui::Text("Text %d: 0x%016llX", i, static_cast<unsigned long long>(ByteCode::BuildU64(key[0], key[1])));
			ImGui::TextWrapped("%s", GetText(data, i));
			if (i < zoo->field_0x160 && zoo->field_0x164) {
				const auto& pos = zoo->field_0x164[i];
				ImGui::TextDisabled("Position: (%.3f, %.3f)", pos.x, pos.y);
			}
		}
		// Create (PS2 0x00404c20) leaves the pointer uninitialized at zero count.
		for (int i = 0; i < data.field_0x8; ++i) {
			if (!data.field_0xc) break;
			ImGui::Text("Number %d: %d", i, data.field_0xc[i]);
			if (i < zoo->field_0x168 && zoo->field_0x16c) {
				const auto& pos = zoo->field_0x16c[i];
				ImGui::TextDisabled("Position: (%.3f, %.3f)", pos.x, pos.y);
			}
		}
	}

	static void ShowEntries(CActorWantedZoo* zoo)
	{
		if (!zoo->field_0x234 || zoo->field_0x230 <= 0) {
			ImGui::TextDisabled("No captures loaded.");
			return;
		}
		ImGui::Text("%d captures", zoo->field_0x230);
		if (!ImGui::BeginTable("ZooCaptures", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) return;
		ImGui::TableSetupColumn("Poster", ImGuiTableColumnFlags_WidthFixed, 112.0f);
		ImGui::TableSetupColumn("Capture", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Reward", ImGuiTableColumnFlags_WidthFixed, 65.0f);
		ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 85.0f);
		ImGui::TableHeadersRow();
		bool foundMatch = false;
		for (int i = 0; i < zoo->field_0x230; ++i) {
			auto& entry = zoo->field_0x234[i];
			const auto& data = entry.field_0x4;
			const bool eligible = entry.scenaricCondition.pData && entry.scenaricCondition.IsVerified() != 0;
			// Draw (PS2 0x00404810) uses the first verified entry.
			const bool selected = eligible && !foundMatch;
			foundMatch |= eligible;
			auto* cinematic = FindPosterCinematic(entry.scenaricCondition);
			const auto poster = FindPosterTexture(cinematic);
			ImGui::PushID(i);
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			if (poster.texture) {
				Debug::Texture::ShowPreview(poster.texture, std::clamp(ImGui::GetContentRegionAvail().x, 1.0f, 112.0f));
				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					Debug::Texture::ShowPreview(poster.texture, 256.0f);
					ImGui::TextUnformatted(poster.texture->GetName().c_str());
					ImGui::EndTooltip();
				}
				if (ImGui::SmallButton("Inspect")) Debug::Texture::ShowMaterialDetails(poster.material);
			}
			else ImGui::TextWrapped("Poster unavailable");
			ImGui::TableSetColumnIndex(1);
			ImGui::TextWrapped("%s", GetCaptureName(cinematic, data).c_str());
			ImGui::TextWrapped("%s", GetText(data, 0));
			if (ImGui::TreeNode("Details")) {
				ShowDetails(zoo, entry, cinematic);
				ImGui::TreePop();
			}
			ImGui::TableSetColumnIndex(2);
			if (data.field_0x8 > 0 && data.field_0xc) ImGui::Text("%d", data.field_0xc[0]);
			else ImGui::TextUnformatted("-");
			ImGui::TableSetColumnIndex(3);
			ImGui::TextWrapped("%s", selected ? "Selected" : eligible ? "Eligible" : "Inactive");
			ImGui::PopID();
		}
		ImGui::EndTable();
	}

	void ShowActorDetails(CActorWantedZoo* zoo)
	{
		if (!zoo) {
			ImGui::TextDisabled("Wanted Zoo Actor: null");
			return;
		}
		if (ImGui::CollapsingHeader("Zoo Captures", ImGuiTreeNodeFlags_DefaultOpen)) ShowEntries(zoo);
		if (ImGui::CollapsingHeader("Wanted Zoo State")) {
			ImGui::Text("Display requested: %s | Fade: %.3f", zoo->field_0x238 ? "yes" : "no", zoo->field_0x23c);
			ImGui::Text("Text slots: %d | Number slots: %d", zoo->field_0x160, zoo->field_0x168);
		}
	}
}
