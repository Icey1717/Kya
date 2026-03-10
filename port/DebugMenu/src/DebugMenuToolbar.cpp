#include "DebugMenuToolbar.h"
#include "DebugMenuWorld.h"
#include "DebugMenuDebugPanel.h"
#include "DebugMenuLayout.h"
#include "DebugMenu.h"
#include "DebugCamera.h"

#include <imgui.h>
#include <cstdio>

#include "SectorManager.h"
#include "LevelScheduler.h"
#include "TimeController.h"
#include "Actor.h"

namespace Debug {

static void ShowSectorMenu(bool* bOpen) {
ImGui::Begin("Sector", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

ImGui::Text("Current Sector: %d", CScene::ptable.g_SectorManager_00451670->baseSector.currentSectorID);

for (int i = 0; i < 5; i++) {
char buttonText[256];
std::sprintf(buttonText, "Switch Sector %d", i);
if (ImGui::Button(buttonText)) {
CScene::ptable.g_SectorManager_00451670->SwitchToSector(i, true);
}
}

ImGui::End();
}

static void ShowLevelSchedulerMenu(bool* bOpen) {
ImGui::Begin("Level Scheduler", bOpen, ImGuiWindowFlags_AlwaysAutoResize);
ImGui::Text("Boomy Level: %d", CLevelScheduler::GetBoomyLevel());
ImGui::Text("Medallion Level: %d", CLevelScheduler::GetMedallionLevel());
ImGui::Text("Fight Level: %d", CLevelScheduler::GetFightLevel());
ImGui::InputInt("Current Level", &CLevelScheduler::gThis->currentLevelID);
ImGui::End();
}

static bool gShowCameraWindow = false;
static int gToolbarLevelId = 0;
static int gToolbarSectorId = 0;
static int gStepFramesPending = 0;
static float gResumeTimeScale = 1.0f;

bool GetShowCameraWindow() { return gShowCameraWindow; }

void UpdateSingleStepState() {
if (gStepFramesPending <= 0) {
return;
}

--gStepFramesPending;
if (gStepFramesPending == 0) {
if (auto* pTimer = GetTimer(); pTimer != nullptr) {
pTimer->timeScale = 0.0f;
}
}
}

void DrawToolbar() {
auto* pTimer = GetTimer();
auto* pSectorManager = CScene::ptable.g_SectorManager_00451670;
const double deltaTime = DebugMenu::GetDeltaTime();
const double fps = deltaTime > 0.0 ? (1.0 / deltaTime) : 0.0;

if (CLevelScheduler::gThis != nullptr && gToolbarLevelId == 0) {
gToolbarLevelId = CLevelScheduler::gThis->currentLevelID;
}
if (pSectorManager != nullptr && gToolbarSectorId == 0) {
gToolbarSectorId = pSectorManager->baseSector.currentSectorID;
}

if (ImGui::BeginMainMenuBar()) {
const bool bPaused = pTimer != nullptr && pTimer->timeScale == 0.0f;

bool bWorld = Debug::GetShowWorldPanel();
bool bInspector = Debug::GetShowInspectorPanel();
bool bDebug = Debug::GetShowDebugPanel();

ImGui::Checkbox("World", &bWorld);
ImGui::SameLine();
ImGui::Checkbox("Inspector", &bInspector);
ImGui::SameLine();
ImGui::Checkbox("Debug", &bDebug);
ImGui::SameLine();
if (ImGui::SmallButton("Reset Layout")) {
Debug::RequestResetDockLayout();
}

Debug::SetShowWorldPanel(bWorld);
Debug::SetShowInspectorPanel(bInspector);
Debug::SetShowDebugPanel(bDebug);

ImGui::SameLine();
ImGui::Separator();
ImGui::SameLine();
if (ImGui::BeginMenu("Tools")) {
ImGui::MenuItem("Camera", nullptr, &gShowCameraWindow);
ImGui::Separator();

for (auto& menu : Debug::MenuRegisterer::GetMenus()) {
bool bOpen = menu.GetOpen();
if (ImGui::MenuItem(menu.name.c_str(), nullptr, &bOpen)) {
menu.SetOpen(bOpen);
}
}

ImGui::EndMenu();
}

ImGui::SameLine();
ImGui::Separator();
ImGui::SameLine();
ImGui::Text("FPS %.1f | %s", fps, bPaused ? "Paused" : "Running");

ImGui::EndMainMenuBar();
}
}

void DrawCameraWindow() {
if (!gShowCameraWindow) {
return;
}

if (GetRightDockId() != 0 && !HasSavedWindowSettings("Camera")) {
ImGui::SetNextWindowDockID(GetRightDockId(), ImGuiCond_FirstUseEver);
}
Debug::Camera::ShowCamera();
}

} // namespace Debug

namespace {
Debug::MenuRegisterer sDemoMenuReg("Demo", ImGui::ShowDemoWindow);
Debug::MenuRegisterer sSectorMenuReg("Sector", Debug::ShowSectorMenu, true);
Debug::MenuRegisterer sLevelSchedulerMenuReg("Level Scheduler", Debug::ShowLevelSchedulerMenu, true);
}
