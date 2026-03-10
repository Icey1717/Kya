#include "DebugMenu.h"
#include "DebugHeroReplay.h"

#include <imgui.h>
#include <filesystem>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#include "DebugHelpers.h"
#include "DebugSetting.h"
#include "input_functions.h"
#include "EdenLib/edDev/Sources/edDev.h"
#include "LevelScheduler.h"

namespace Debug {
namespace HeroReplay {

// -----------------------------------------------------------------------
// Data structures
// -----------------------------------------------------------------------

// The number of Pad_d slots allocated per controller port.
constexpr uint32_t NUM_ROUTES = 0x18; // KeyboardController::numButtons

// Per-route state captured each frame.
// analogValue stores whichever of pPadD.analogValue / pPadD.clickValue is
// non-zero for this route (they are mutually exclusive — see input.cpp).
struct RecordedRoute {
	bool  bPressed;
	bool  bReleased;
	float analogValue;
};

constexpr int MAX_RECORDING_FRAMES = 3600; // ~1 minute at 60 fps

struct InputRecording {
	int version    = 1;
	int frameCount = 0;
	RecordedRoute frames[MAX_RECORDING_FRAMES][NUM_ROUTES]{};
};

// -----------------------------------------------------------------------
// State
// -----------------------------------------------------------------------

enum class State { Idle, Recording, Replaying };

static State                  gState         = State::Idle;
static InputRecording         gRecording;
static int                    gCurrentFrame  = 0;
static Input::InputFunctions  gSavedInputFunctions;

static Debug::Setting<bool>   gAutoRecord("HeroReplay_AutoRecord", false);

// Auto-save the in-progress recording every N frames so a crash doesn't lose it.
constexpr int  AUTO_SAVE_INTERVAL_FRAMES = 300; // ~5 s at 60 fps
constexpr auto LAST_RECORDING_NAME       = "last_recording";

// -----------------------------------------------------------------------
// Forward declarations
// -----------------------------------------------------------------------

static void SaveRecording(const std::string& name);

// -----------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------

static EDDEV_PORT* GetControllerPort()
{
	if (edDevRoot.nbPorts == 0 || !edDevRoot.aPorts[0].pPadD) {
		return nullptr;
	}
	return &edDevRoot.aPorts[0];
}

static void CaptureFrame(int frameIndex)
{
	EDDEV_PORT* pPort = GetControllerPort();
	if (!pPort) {
		return;
	}

	for (uint32_t r = 0; r < NUM_ROUTES; ++r) {
		const Pad_d& pad = pPort->pPadD[r];
		auto& dst        = gRecording.frames[frameIndex][r];
		dst.bPressed     = pad.bPressed;
		dst.bReleased    = pad.bReleased;
		// One of analogValue / clickValue is non-zero per route; store the max.
		dst.analogValue  = std::max(pad.analogValue, pad.clickValue);
	}
}

static void InstallReplayInputFunctions()
{
	gSavedInputFunctions = Input::gInputFunctions;

	// During replay every input query returns the recorded frame's value.
	// controllerAnyPressed must return true so PollGamepad doesn't skip its block.

	Input::gInputFunctions.keyPressed = [](uint32_t route) -> bool {
		if (gCurrentFrame >= gRecording.frameCount || route >= NUM_ROUTES) return false;
		return gRecording.frames[gCurrentFrame][route].bPressed;
	};

	Input::gInputFunctions.keyReleased = [](uint32_t route) -> bool {
		if (gCurrentFrame >= gRecording.frameCount || route >= NUM_ROUTES) return false;
		return gRecording.frames[gCurrentFrame][route].bReleased;
	};

	Input::gInputFunctions.keyAnalog = [](uint32_t route) -> float {
		if (gCurrentFrame >= gRecording.frameCount || route >= NUM_ROUTES) return 0.0f;
		return gRecording.frames[gCurrentFrame][route].analogValue;
	};

	Input::gInputFunctions.controllerPressed = [](uint32_t route) -> bool {
		if (gCurrentFrame >= gRecording.frameCount || route >= NUM_ROUTES) return false;
		return gRecording.frames[gCurrentFrame][route].bPressed;
	};

	Input::gInputFunctions.controllerReleased = [](uint32_t route) -> bool {
		if (gCurrentFrame >= gRecording.frameCount || route >= NUM_ROUTES) return false;
		return gRecording.frames[gCurrentFrame][route].bReleased;
	};

	Input::gInputFunctions.controllerAnalog = [](uint32_t route) -> float {
		if (gCurrentFrame >= gRecording.frameCount || route >= NUM_ROUTES) return 0.0f;
		return gRecording.frames[gCurrentFrame][route].analogValue;
	};

	Input::gInputFunctions.controllerAnyPressed = []() -> bool { return true; };
}

static void RestoreInputFunctions()
{
	Input::gInputFunctions = gSavedInputFunctions;
}

// -----------------------------------------------------------------------
// Recording / replay control
// -----------------------------------------------------------------------

static void StartRecording()
{
	gRecording      = InputRecording{};
	gCurrentFrame   = 0;
	gState          = State::Recording;
}

static void StopRecording()
{
	gRecording.frameCount = gCurrentFrame;
	SaveRecording(LAST_RECORDING_NAME);
	gState = State::Idle;
}

static void StartReplay()
{
	if (gRecording.frameCount == 0) {
		return;
	}

	gCurrentFrame = 0;
	gState        = State::Replaying;
	InstallReplayInputFunctions();
}

static void StopReplay()
{
	RestoreInputFunctions();
	gState = State::Idle;
}

// -----------------------------------------------------------------------
// File I/O
// -----------------------------------------------------------------------

static std::filesystem::path GetRecordingsDir()
{
	return std::filesystem::path("recordings");
}

static void BackupLastRecording()
{
	auto dir    = GetRecordingsDir();
	auto source = dir / (std::string(LAST_RECORDING_NAME) + ".rec");
	if (std::filesystem::exists(source)) {
		auto dest = dir / (std::string(LAST_RECORDING_NAME) + "_backup.rec");
		std::filesystem::copy_file(source, dest, std::filesystem::copy_options::overwrite_existing);
	}
}

static void SaveRecording(const std::string& name)
{
	auto dir = GetRecordingsDir();
	if (!std::filesystem::exists(dir)) {
		std::filesystem::create_directory(dir);
	}
	DebugHelpers::SaveTypeToFile(dir / (name + ".rec"), gRecording);
}

static bool LoadRecording(const std::filesystem::path& path)
{
	InputRecording loaded{};
	DebugHelpers::LoadTypeFromFile(path, loaded);

	if (loaded.version != 1 || loaded.frameCount <= 0 || loaded.frameCount > MAX_RECORDING_FRAMES) {
		std::cerr << "HeroReplay: invalid recording file: " << path << std::endl;
		return false;
	}

	gRecording = loaded;
	return true;
}

static std::vector<std::string> EnumerateRecordings()
{
	std::vector<std::string> names;
	auto dir = GetRecordingsDir();
	if (!std::filesystem::exists(dir)) {
		return names;
	}
	for (const auto& entry : std::filesystem::directory_iterator(dir)) {
		if (entry.path().extension() == ".rec") {
			names.emplace_back(entry.path().filename().string());
		}
	}
	return names;
}

// -----------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------

void Update()
{
	// Auto-record: start recording on the very first update if enabled.
	static bool bStartupChecked = false;
	if (CLevelScheduler::gThis->currentLevelID != 0x10 && CLevelScheduler::gThis->currentLevelID != 0xe && !bStartupChecked) {
		bStartupChecked = true;
		if (gAutoRecord && gState == State::Idle) {
			BackupLastRecording();
			StartRecording();
		}
	}

	switch (gState) {
	case State::Recording:
		if (gCurrentFrame < MAX_RECORDING_FRAMES) {
			CaptureFrame(gCurrentFrame);
			++gCurrentFrame;

			// Periodically flush to disk so a crash doesn't lose the whole run.
			if (gCurrentFrame % AUTO_SAVE_INTERVAL_FRAMES == 0) {
				gRecording.frameCount = gCurrentFrame;
				SaveRecording(LAST_RECORDING_NAME);
			}
		} else {
			// Auto-stop at the limit.
			StopRecording();
		}
		break;

	case State::Replaying:
		// Advance to the next recorded frame.
		// The overridden gInputFunctions lambdas read gCurrentFrame, so next
		// frame's poll will return the newly incremented index's data.
		++gCurrentFrame;
		if (gCurrentFrame >= gRecording.frameCount) {
			StopReplay();
		}
		break;

	default:
		break;
	}
}

void DrawContents()
{
	// ---- Status --------------------------------------------------------
	const char* stateStr = "Idle";
	if (gState == State::Recording) stateStr = "Recording";
	if (gState == State::Replaying) stateStr = "Replaying";

	ImGui::Text("State: %s", stateStr);
	ImGui::Text("Recorded frames: %d  (%.1f s at 60 fps)",
		gRecording.frameCount,
		gRecording.frameCount / 60.0f);

	if (gState == State::Recording || gState == State::Replaying) {
		ImGui::Text("Frame: %d / %d", gCurrentFrame, gRecording.frameCount);
	}

	if (gState == State::Recording) {
		const int framesUntilSave = AUTO_SAVE_INTERVAL_FRAMES - (gCurrentFrame % AUTO_SAVE_INTERVAL_FRAMES);
		ImGui::TextDisabled("Auto-save in %d frames  (file: recordings/%s.rec)", framesUntilSave, LAST_RECORDING_NAME);
	}

	ImGui::Separator();

	// ---- Auto Record ---------------------------------------------------
	if (ImGui::Checkbox("Auto Record on Startup", &gAutoRecord.operator bool&())) {
		gAutoRecord.UpdateValue();
	}

	{
		auto backupPath = GetRecordingsDir() / (std::string(LAST_RECORDING_NAME) + "_backup.rec");
		if (std::filesystem::exists(backupPath)) {
			static char backupDupName[256] = "recording";
			ImGui::InputText("Name##backup_dup", backupDupName, IM_ARRAYSIZE(backupDupName));
			ImGui::SameLine();
			if (ImGui::Button("Save Backup As")) {
				auto dest = GetRecordingsDir() / (std::string(backupDupName) + ".rec");
				std::filesystem::copy_file(backupPath, dest, std::filesystem::copy_options::overwrite_existing);
			}
		}
	}

	ImGui::Separator();

	// ---- Record --------------------------------------------------------
	if (gState == State::Idle) {
		if (ImGui::Button("Start Recording")) {
			StartRecording();
		}
	} else if (gState == State::Recording) {
		if (ImGui::Button("Stop Recording")) {
			StopRecording();
		}
	}

	// ---- Replay --------------------------------------------------------
	ImGui::Spacing();

	if (gState == State::Idle) {
		const bool hasFrames = gRecording.frameCount > 0;
		if (!hasFrames) {
			ImGui::BeginDisabled();
		}
		if (ImGui::Button("Replay")) {
			StartReplay();
		}
		if (!hasFrames) {
			ImGui::EndDisabled();
		}
	} else if (gState == State::Replaying) {
		if (ImGui::Button("Stop Replay")) {
			StopReplay();
		}
	}

	// ---- Save / Load ---------------------------------------------------
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Save / Load", ImGuiTreeNodeFlags_DefaultOpen)) {
		// Save
		if (gRecording.frameCount > 0 && gState == State::Idle) {
			static char saveName[256] = "recording";
			ImGui::InputText("Name##save", saveName, IM_ARRAYSIZE(saveName));
			if (ImGui::Button("Save Recording")) {
				SaveRecording(saveName);
			}
		}

		ImGui::Spacing();

		// Load
		auto recordings = EnumerateRecordings();
		if (!recordings.empty()) {
			static int selectedIdx = 0;
			selectedIdx = std::clamp(selectedIdx, 0, static_cast<int>(recordings.size()) - 1);

			auto itemGetter = [](void* data, int idx, const char** out) {
				auto& v = *static_cast<std::vector<std::string>*>(data);
				*out = v[idx].c_str();
				return true;
			};

			ImGui::Combo("File##load", &selectedIdx, itemGetter, &recordings, static_cast<int>(recordings.size()));

			if (ImGui::Button("Load Recording")) {
				auto path = GetRecordingsDir() / recordings[selectedIdx];
				LoadRecording(path);
			}
		} else {
			ImGui::TextDisabled("No recordings found in recordings/");
		}
	}
}

void ShowMenu(bool* bOpen)
{
	ImGui::Begin("Hero Replay", bOpen, ImGuiWindowFlags_AlwaysAutoResize);
	DrawContents();
	ImGui::End();
}

} // namespace HeroReplay
} // namespace Debug

namespace Debug {
    MenuRegisterer sDebugHeroReplayMenuReg("Hero Replay", Debug::HeroReplay::ShowMenu, true);
    UpdateRegisterer sDebugHeroReplayUpdateReg(Debug::HeroReplay::Update);
}

