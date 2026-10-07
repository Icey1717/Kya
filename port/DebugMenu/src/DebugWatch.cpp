#include "DebugWatch.h"
#include "DebugWatchValue.h"
#include "DebugUi.h"
#include "DebugMenu.h"
#include "DebugMenuWorld.h"
#include "DebugMenuLayout.h"
#include "DebugSetting.h"
#include "ActorHero_Private.h"
#include "ActorManager.h"
#include "LevelScheduler.h"
#include "SectorManager.h"
#include "Native/NativeRenderer.h"
#include "Actor/DebugActorBehaviour.h"
#include <algorithm>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <spdlog/fmt/fmt.h>

namespace Debug::Watch {
	namespace {
		using Value = std::optional<std::string>;
		struct Source {
			const char* id;
			const char* group;
			const char* label;
			std::function<Value()> sample;
			ValueState state;
		};

		// Resolve the current scene every frame, including while its inspector is closed.
		CActorHeroPrivate* ResolveHero()
		{
			auto* hero = reinterpret_cast<CActorHeroPrivate*>(CActorHeroPrivate::_gThis);
			auto* manager = CScene::ptable.g_ActorManager_004516a4;
			if (!hero || !manager || !manager->aActors) return nullptr;
			for (int i = 0; i < manager->nbActors; ++i) {
				if (manager->aActors[i] == hero) return hero;
			}
			return nullptr;
		}

		std::vector<Source>& Sources()
		{
			static std::vector<Source> sources = {
				{ "Hero.State", "Hero", "State", []() -> Value {
					auto* hero = ResolveHero();
					return hero ? Value(Actor::State::GetActorStateName(hero)) : std::nullopt;
				} },
				{ "Hero.Position", "Hero", "Position", []() -> Value {
					auto* hero = ResolveHero();
					if (!hero) return std::nullopt;
					const auto& v = hero->currentLocation;
					return fmt::format("X {:.3f}  Y {:.3f}  Z {:.3f}", v.x, v.y, v.z);
				} },
				{ "Hero.Speed", "Hero", "Speed", []() -> Value {
					auto* hero = ResolveHero();
					return hero ? Value(fmt::format("{:.3f}", hero->dynamic.speed)) : std::nullopt;
				} },
				{ "Hero.TimeInAir", "Hero", "Time in air", []() -> Value {
					auto* hero = ResolveHero();
					return hero ? Value(fmt::format("{:.3f}", hero->timeInAir)) : std::nullopt;
				} },
				{ "Hero.Gravity", "Hero", "Gravity scale", []() -> Value {
					auto* hero = ResolveHero();
					return hero ? Value(fmt::format("{:.3f}", hero->dynamicExt.gravityScale)) : std::nullopt;
				} },
				{ "Scene.Level", "Scene", "Level", []() -> Value {
					return CLevelScheduler::gThis ? Value(fmt::format("0x{:X}", CLevelScheduler::gThis->currentLevelID)) : std::nullopt;
				} },
				{ "Scene.Sector", "Scene", "Sector", []() -> Value {
					auto* manager = CScene::ptable.g_SectorManager_00451670;
					return manager ? Value(fmt::format("0x{:X}", manager->baseSector.currentSectorID)) : std::nullopt;
				} },
				{ "Scene.Actors", "Scene", "Active actors", []() -> Value {
					auto* manager = CScene::ptable.g_ActorManager_004516a4;
					return manager ? Value(fmt::format("{} / {}", manager->nbActiveActors, manager->nbActors)) : std::nullopt;
				} },
				{ "Selection.State", "Selected actor", "State", []() -> Value {
					auto* actor = GetInspectedActor();
					return actor ? Value(Actor::State::GetActorStateName(actor)) : std::nullopt;
				} },
				{ "Selection.Position", "Selected actor", "Position", []() -> Value {
					auto* actor = GetInspectedActor();
					if (!actor) return std::nullopt;
					const auto& v = actor->currentLocation;
					return fmt::format("X {:.3f}  Y {:.3f}  Z {:.3f}", v.x, v.y, v.z);
				} },
				{ "Performance.Fps", "Performance", "FPS", []() -> Value {
					const double dt = DebugMenu::GetDeltaTime();
					return fmt::format("{:.1f}", dt > 0.0 ? 1.0 / dt : 0.0);
				} },
				{ "Performance.Frame", "Performance", "Frame time", []() -> Value {
					return fmt::format("{:.2f} ms", DebugMenu::GetDeltaTime() * 1000.0);
				} },
				{ "Performance.Render", "Performance", "Render time", []() -> Value {
					return fmt::format("{:.2f} ms", Renderer::Native::GetRenderTime());
				} },
			};
			return sources;
		}

		Setting<std::vector<std::string>>& Pins()
		{
			static Setting<std::vector<std::string>> pins("Watch Pins", {});
			return pins;
		}

		bool IsPinned(const char* id)
		{
			const auto& pins = Pins().get();
			return std::find(pins.begin(), pins.end(), id) != pins.end();
		}

		Source* Find(const char* id)
		{
			for (auto& source : Sources()) if (std::string_view(source.id) == id) return &source;
			return nullptr;
		}

		void Toggle(const char* id)
		{
			auto pins = Pins().get();
			const auto it = std::find(pins.begin(), pins.end(), id);
			if (it == pins.end()) pins.emplace_back(id);
			else pins.erase(it);
			Pins() = pins;
			if (!pins.empty()) {
				RevealDockWindow("Watch", DockRegion::Bottom);
				for (auto& menu : MenuRegisterer::GetMenus()) if (menu.name == "Watch") menu.SetOpen(true);
			}
		}

		void ValueText(const Source& source)
		{
			const float blend = source.state.Highlight(ImGui::GetTime());
			ImVec4 color = ImGui::GetStyleColorVec4(source.state.value ? ImGuiCol_Text : ImGuiCol_TextDisabled);
			if (blend > 0.0f) {
				color.x += (0.95f - color.x) * blend;
				color.y += (0.75f - color.y) * blend;
				color.z += (0.42f - color.z) * blend;
			}
			ImGui::PushStyleColor(ImGuiCol_Text, color);
			ImGui::TextWrapped("%s", source.state.value ? source.state.value->c_str() : "Unavailable");
			ImGui::PopStyleColor();
		}
	}

	void Update()
	{
		for (auto& source : Sources()) {
			source.state.Update(source.sample(), ImGui::GetTime());
		}
	}

	void PinButton(const char* id)
	{
		ImGui::PushID(id);
		const bool pinned = IsPinned(id);
		if (ImGui::SmallButton(pinned ? "Unpin" : "Pin")) Toggle(id);
		ImGui::SetItemTooltip("%s in Watch. Selected actor sources follow the current selection.", pinned ? "Remove" : "Keep updating");
		ImGui::PopID();
	}

	void DrawReadout(const char* id)
	{
		if (auto* source = Find(id)) {
			Ui::Field(source->label, [source](const char*) {
				PinButton(source->id);
				ImGui::SameLine();
				ValueText(*source);
				return false;
			});
		}
	}

	bool DrawFloatEditor(const char* id, const char* label, float* value)
	{
		return Ui::Field(label, [id, value](const char* widgetId) {
			const float pinWidth = ImGui::CalcTextSize("Unpin").x + ImGui::GetStyle().FramePadding.x * 2;
			ImGui::SetNextItemWidth(std::max(ImGui::GetFontSize() * 3, ImGui::GetContentRegionAvail().x - pinWidth - ImGui::GetStyle().ItemSpacing.x));
			const bool changed = ImGui::InputFloat(widgetId, value);
			ImGui::SameLine();
			PinButton(id);
			return changed;
		});
	}

	void ShowMenu(bool* pOpen)
	{
		ImGui::SetNextWindowSize(ImVec2(ImGui::GetFontSize() * 28, ImGui::GetFontSize() * 14), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Watch", pOpen)) {
			if (ImGui::Button("Add value")) ImGui::OpenPopup("Sources");
			if (ImGui::BeginPopup("Sources")) {
				for (auto& source : Sources()) {
					const std::string label = fmt::format("{} / {}", source.group, source.label);
					if (ImGui::MenuItem(label.c_str(), nullptr, IsPinned(source.id))) Toggle(source.id);
				}
				ImGui::EndPopup();
			}
			ImGui::TextDisabled("Changes glow briefly. Pins are saved.");
			if (Pins().get().empty()) ImGui::TextWrapped("Pin a value in an inspector, or choose Add value. Values keep updating when inspectors are closed.");
			if (ImGui::BeginTable("Watches", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_Resizable)) {
				ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthStretch, 0.4f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.6f);
				ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("Unpin").x + ImGui::GetStyle().FramePadding.x * 2);
				ImGui::TableHeadersRow();
				const auto pins = Pins().get(); // Unpin may change the saved list while drawing.
				for (const auto& id : pins) {
					auto* source = Find(id.c_str());
					ImGui::TableNextColumn();
					if (source) ImGui::TextWrapped("%s / %s", source->group, source->label);
					else ImGui::TextWrapped("%s", id.c_str());
					ImGui::TableNextColumn();
					if (source) ValueText(*source);
					else ImGui::TextDisabled("Unavailable");
					ImGui::TableNextColumn();
					PinButton(id.c_str());
				}
				ImGui::EndTable();
			}
		}
		ImGui::End();
	}

	static MenuRegisterer gWatchMenu("Watch", ShowMenu);
}
