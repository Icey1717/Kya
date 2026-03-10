#include "DebugMenu.h"
#include "DebugRenderer.h"

#include <imgui.h>
#include "log.h"
#include "DebugHelpers.h"

#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <chrono>

#include "ed3D.h"
#include "edDlist.h"
#include "port/pointer_conv.h"
#include "DebugMaterialPreviewer.h"
#include "Callstack.h"
#include "TextureCache.h"
#include "ed3D/ed3DG2D.h"
#include "DebugSetting.h"
#include "ActorHero_Private.h"

#include "DebugMenuLog.h"
#include "DebugMenuWorld.h"
#include "DebugMenuDebugPanel.h"
#include "DebugMenuLayout.h"
#include "DebugMenuToolbar.h"

#define DEBUG_LOG(level, format, ...) MY_LOG_CATEGORY("Debug", level, format, ##__VA_ARGS__)

std::unique_ptr<std::vector<Debug::Menu>> Debug::MenuRegisterer::gMenus;
std::unique_ptr<std::vector<std::function<void()>>> Debug::UpdateRegisterer::gCallbacks;
std::unique_ptr<std::vector<std::function<void()>>> Debug::StartupRegisterer::gCallbacks;

void Debug::Menu::Show()
{
	bool bOpen = GetOpen();
	if (bOpen) {
		ShowFunction(&bOpen);
		SetOpen(bOpen);
	}
}

bool Debug::Menu::GetOpen() const
{
	return pOpenSetting ? pOpenSetting->get() : false;
}

void Debug::Menu::SetOpen(bool bNewOpen)
{
	if (pOpenSetting && pOpenSetting->get() != bNewOpen) {
		*pOpenSetting = bNewOpen;
	}
}

Debug::MenuRegisterer::MenuRegisterer(const std::string& name, std::function<void(bool*)> showFunction, bool bDefaultOpen)
{
	if (!gMenus) {
		gMenus = std::make_unique<std::vector<Menu>>();
	}

	std::string settingName = "MenuOpen." + name;
	gMenus->push_back({
	name,
	showFunction,
	std::make_shared<Debug::Setting<bool>>(settingName, bDefaultOpen)
		});
}

std::vector<Debug::Menu>& Debug::MenuRegisterer::GetMenus()
{
	return *gMenus;
}

Debug::UpdateRegisterer::UpdateRegisterer(std::function<void()> updateFunction)
{
	if (!gCallbacks) {
		gCallbacks = std::make_unique<std::vector<std::function<void()>>>();
	}
	gCallbacks->push_back(std::move(updateFunction));
}

std::vector<std::function<void()>>& Debug::UpdateRegisterer::GetCallbacks()
{
	if (!gCallbacks) {
		gCallbacks = std::make_unique<std::vector<std::function<void()>>>();
	}
	return *gCallbacks;
}

Debug::StartupRegisterer::StartupRegisterer(std::function<void()> startupFunction)
{
	if (!gCallbacks) {
		gCallbacks = std::make_unique<std::vector<std::function<void()>>>();
	}
	gCallbacks->push_back(std::move(startupFunction));
}

std::vector<std::function<void()>>& Debug::StartupRegisterer::GetCallbacks()
{
	if (!gCallbacks) {
		gCallbacks = std::make_unique<std::vector<std::function<void()>>>();
	}
	return *gCallbacks;
}

extern bool bOther;

namespace Debug {
	static bool bShowMenus = true;

	static std::unordered_map<const PS2::GSTexEntry*, ImageTextureID> debugTextures;
	static std::vector<MaterialPreviewerEntry> materialList;

	static void OnMaterialLoaded(edDList_material* pNewMaterial) {
		std::vector<DWORD64> backtrace;
		CollectBacktrace(backtrace);
		materialList.emplace_back(pNewMaterial, backtrace);
	}

	static void OnMaterialUnloaded(edDList_material* pMaterial) {
		auto it = std::find(materialList.begin(), materialList.end(), pMaterial);
		if (it != materialList.end()) {
			materialList.erase(it);
		}
	}

	struct TextureListEntry {
		TextureListEntry(struct ed_g2d_manager* texture, std::vector<DWORD64> inBacktrace, std::string inName)
			: pTexture(texture)
			, callstackEntry(inBacktrace)
			, name(inName)
		{}

		struct ed_g2d_manager* pTexture;
		CallstackPreviewerEntry callstackEntry;
		std::string name;
	};

	static std::vector<TextureListEntry> textureList;

	static void OnTextureLoaded(ed_g2d_manager* pNewTexture, std::string name) {
		std::vector<DWORD64> backtrace;
		CollectBacktrace(backtrace);
		textureList.emplace_back(pNewTexture, backtrace, ObjectNaming::GetNextObjectName());
	}

	struct MeshListEntry {
		MeshListEntry(struct ed_g3d_manager* mesh, std::vector<DWORD64> inBacktrace, std::string inName)
			: pMesh(mesh)
			, callstackEntry(inBacktrace)
			, name(inName)
		{}

		ed_g3d_manager* pMesh;
		CallstackPreviewerEntry callstackEntry;
		std::string name;
	};

	static std::vector<MeshListEntry> meshList;

	static void OnMeshLoaded(ed_g3d_manager* pNewMesh, std::string name) {
		std::vector<DWORD64> backtrace;
		CollectBacktrace(backtrace);
		meshList.emplace_back(pNewMesh, backtrace, ObjectNaming::GetNextObjectName());
	}

	static double deltaTime;

	static void Update() {
		static std::chrono::steady_clock::time_point prevTime = std::chrono::steady_clock::now();
		const auto currentTime = std::chrono::steady_clock::now();
		deltaTime = std::chrono::duration<double>(currentTime - prevTime).count();
		prevTime = currentTime;
	}

	static int selectedTextureIndex = -1;
	static int selectedMaterialIndex = -1;
	static std::vector<edDList_material> textureMaterials;

	static int GetMaterialCountFromTexture(ed_g2d_manager* pTexture) {
		int i = 0;
		while (ed3DG2DGetMaterialFromIndex(pTexture, i)) {
			i++;
		}
		return i;
	}

	static void DrawInternal() {
		if (ImGui::IsKeyPressed(ImGuiKey_F10)) {
			bShowMenus = !bShowMenus;
		}

		static bool bRunningTerm = false;
		UpdateSingleStepState();

		for (auto& cb : UpdateRegisterer::GetCallbacks()) {
			cb();
		}

		if (bRunningTerm) {
			bRunningTerm = false;
		}

		if ((GameFlags & 0x2) != 0 && !bRunningTerm) {
			bRunningTerm = true;
		}

		if (!bShowMenus || bRunningTerm) {
			DrawFullscreenGameViewportWindow();
			return;
		}

		DrawToolbar();

		const ImGuiID dockspaceId = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
		BuildDefaultDockLayout(dockspaceId);

		DrawWorldPanel();
		DrawInspectorPanel();
		DrawGameViewportWindow();
		DrawDebugPanel();
		DrawLegacyMenus();
		DrawCameraWindow();
	}
}

void DebugMenu::BuildImguiCommands()
{
	Debug::Update();
	Debug::DrawInternal();
}

double DebugMenu::GetDeltaTime()
{
	return Debug::deltaTime;
}

void DebugMenu::Init()
{
	edDListGetMaterialLoadedDelegate() += Debug::OnMaterialLoaded;
	edDListGetMaterialUnloadedDelegate() += Debug::OnMaterialUnloaded;
	ed3DGetTextureLoadedDelegate() += Debug::OnTextureLoaded;
	ed3DGetMeshLoadedDelegate() += Debug::OnMeshLoaded;

	Debug::EnsureRegistrations();

	for (auto& cb : Debug::StartupRegisterer::GetCallbacks()) {
		cb();
	}
}