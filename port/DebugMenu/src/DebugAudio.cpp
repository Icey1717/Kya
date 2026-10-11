#include "DebugAudio.h"
#include "DebugMenu.h"
#include "Audio.h"

#include "../../Audio/edSoundStreamService.h"
#include "../../Audio/edSoundSampleService.h"
#include "../../Audio/edMusicService.h"

#include <imgui.h>
#include <algorithm>
#include <cstdint>

namespace Debug::Audio
{
	static ImVec2 GetTableSize(size_t rowCount, int maxRows)
	{
		const auto& style = ImGui::GetStyle();
		const float rowHeight = ImGui::GetTextLineHeight() + style.CellPadding.y * 2.0f;
		return ImVec2(0.0f, rowHeight * static_cast<float>(1 + std::min(rowCount, static_cast<size_t>(maxRows))) +
			style.ScrollbarSize + style.WindowPadding.y * 2.0f);
	}

	static void DrawAmbiance(ImGuiTableFlags flags)
	{
		if (!ImGui::CollapsingHeader("Ambiance", ImGuiTreeNodeFlags_DefaultOpen)) { return; }
		const auto* manager = CScene::ptable.g_AudioManager_00451698;
		if (!manager || !manager->aMusicAmbiance || manager->nbMusicAmbiance <= 0) {
			ImGui::TextDisabled("No registered ambiance.");
			return;
		}
		// The 3D sound pool is populated after ambiance references are resolved in
		// Level_Init, and emptied during teardown. Avoid reading unresolved entries.
		if (!manager->field_0xac || !manager->field_0xb0 ||
			(!manager->field_0xac->pHead && !manager->field_0xb0->pHead)) {
			ImGui::TextDisabled("Ambiance is not initialized for this level.");
			return;
		}

		if (manager->activeAmbianceIndex >= 0 && manager->activeAmbianceIndex < manager->nbMusicAmbiance) {
			ImGui::Text("Selected ambiance: %d", manager->activeAmbianceIndex);
		}
		else {
			ImGui::TextDisabled("No selected ambiance.");
		}
		static bool showInactiveAmbiance = false;
		ImGui::Checkbox("Show inactive ambiance", &showInactiveAmbiance);
		if (NoAudio) { ImGui::TextDisabled("Audio is disabled."); }

		int activeCount = 0;
		for (int index = 0; index < manager->nbMusicAmbiance; ++index) {
			const auto& ambiance = manager->aMusicAmbiance[index];
			if (ambiance.field_0x14 != 0 || ambiance.field_0x1c != 0.0f) { ++activeCount; }
		}
		ImGui::Text("Active: %d    Registered: %d", activeCount, manager->nbMusicAmbiance);
		const auto samples = ::Audio::GetSampleInstances();
		int visibleCount = 0;
		for (int index = 0; index < manager->nbMusicAmbiance; ++index) {
			const auto& ambiance = manager->aMusicAmbiance[index];
			const bool selected = index == manager->activeAmbianceIndex;
			const bool enabled = ambiance.field_0x14 != 0;
			if (!showInactiveAmbiance && !selected && !enabled && ambiance.field_0x1c == 0.0f) { continue; }
			++visibleCount;
			const char* state = ambiance.field_0x18 > 0.0f ? (enabled ? "Fading in" : "Fading out") :
				(enabled ? "Enabled" : "Inactive");
			ImGui::PushID(index);
			if (ImGui::TreeNodeEx("##AmbianceGroup", ImGuiTreeNodeFlags_DefaultOpen,
				"Ambiance %d%s - %s", index, selected ? " (selected)" : "", state)) {
				ImGui::Text("Volume: %.2f    Sounds: %u", ambiance.field_0x1c, ambiance.nbSoundAmbiance);
				ImGui::Text("Fade: %.2f s remaining    In: %.2f s    Out: %.2f s",
					ambiance.field_0x18, ambiance.field_0x8, ambiance.field_0xc);
				if (!ambiance.aSoundAmbiance || ambiance.nbSoundAmbiance == 0) {
					ImGui::TextDisabled("No ambiance sounds.");
				}
				else if (ImGui::BeginTable("##AmbianceSounds", 8, flags, GetTableSize(ambiance.nbSoundAmbiance, 6))) {
					ImGui::TableSetupScrollFreeze(1, 1);
					for (const char* column : { "Sound", "Type", "State", "Instance ID", "Sample handle", "Volume", "Replay delay", "Position" }) {
						ImGui::TableSetupColumn(column);
					}
					ImGui::TableHeadersRow();
					for (uint soundIndex = 0; soundIndex < ambiance.nbSoundAmbiance; ++soundIndex) {
						const auto& sound = ambiance.aSoundAmbiance[soundIndex];
						const uint id = sound.soundInstance.soundId;
						const bool alive = pedSoundInstances && (id & 0xffff) < edSoundMaxInstances && edSoundInstanceIsAlive(id);
						const auto* instance = alive ? &pedSoundInstances[id & 0xffff] : nullptr;
						const auto sample = std::find_if(samples.begin(), samples.end(), [id](const auto& voice) { return voice.instanceId == id; });
						const bool stream = sound.soundInstance.pSound &&
							sound.soundInstance.pSound == LOAD_POINTER_CAST(CSoundStream*, sound.field_0x8.pStream);
						const char* soundState = !sound.soundInstance.pSound ? "Unavailable" :
							(!enabled && ambiance.field_0x1c == 0.0f ? "Inactive" : "Waiting");
						if (instance) {
							soundState = (instance->flags & 0x200) != 0 ? "Paused" : "Active";
							if (sample != samples.end()) {
								soundState = sample->playing ? "Playing" : (sample->started ? "Paused" : "Ready");
							}
							else if ((instance->flags & 0x10) != 0 && instance->pSoundStream) {
								::Audio::StreamInfo info;
								if (::Audio::GetStreamInfo(static_cast<uint>(instance->pSoundStream->streamBufferId[0]), info)) {
									soundState = info.playing ? "Playing" : (info.finished ? "Finished" : (info.ready ? "Ready" : "Loading"));
								}
							}
						}
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("%u", soundIndex);
						ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(sound.soundInstance.pSound ? (stream ? "Stream" : "Sample") : "-");
						ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(soundState);
						ImGui::TableSetColumnIndex(3); if (alive) ImGui::Text("0x%08X", id); else ImGui::TextUnformatted("-");
						ImGui::TableSetColumnIndex(4); if (sample != samples.end()) ImGui::Text("%u", sample->sampleHandle); else ImGui::TextUnformatted("-");
						ImGui::TableSetColumnIndex(5); if (instance) ImGui::Text("%.2f", instance->volume); else ImGui::TextUnformatted("-");
						ImGui::TableSetColumnIndex(6);
						if (sound.field_0x68 >= 0.0f && (enabled || ambiance.field_0x1c != 0.0f)) {
							ImGui::Text("%.2f s", std::max(0.0f, sound.field_0x6c - sound.field_0x68));
						}
						else { ImGui::TextUnformatted("-"); }
						ImGui::TableSetColumnIndex(7);
						if (LOAD_POINTER(sound.wayPointRef.pObj)) {
							const auto& position = sound.soundPosData.position;
							ImGui::Text("%.2f, %.2f, %.2f", position.x, position.y, position.z);
						}
						else { ImGui::TextUnformatted("Non-spatial"); }
					}
					ImGui::EndTable();
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		if (visibleCount == 0) { ImGui::TextDisabled("No active ambiance."); }
	}

	void DrawContents()
	{
		if (!ImGui::BeginChild("##AudioContents")) {
			ImGui::EndChild();
			return;
		}

		auto streams = ::Audio::GetStreams();
		std::sort(streams.begin(), streams.end(), [](const auto& left, const auto& right) {
			return left.first < right.first;
		});

		int playingCount = 0;
		for (const auto& [id, stream] : streams) {
			if (stream.playing) { ++playingCount; }
		}
		static bool showInactive = false;

		const ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
			ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
			ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;
		if (ImGui::CollapsingHeader("Streams", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Playing: %d    Registered: %d", playingCount, static_cast<int>(streams.size()));
			ImGui::Checkbox("Show inactive streams", &showInactive);
			ImGui::TextWrapped("Time is the decoded stream position reported by the Windows audio backend.");
			const size_t visibleStreams = showInactive ? streams.size() : static_cast<size_t>(playingCount);
			if (visibleStreams > 0 && ImGui::BeginTable("##AudioStreams", 8, flags, GetTableSize(visibleStreams, 6))) {
				ImGui::TableSetupScrollFreeze(1, 1);
				ImGui::TableSetupColumn("Stream ID");
				ImGui::TableSetupColumn("State");
				ImGui::TableSetupColumn("Playtime");
				ImGui::TableSetupColumn("Duration");
				ImGui::TableSetupColumn("Progress");
				ImGui::TableSetupColumn("Volume");
				ImGui::TableSetupColumn("Format");
				ImGui::TableSetupColumn("File");
				ImGui::TableHeadersRow();

				for (const auto& [id, stream] : streams) {
					if (!showInactive && !stream.playing) { continue; }
					// The host cursor is in compressed VAG bytes (16 bytes per 28 samples).
					const double playtime = stream.sampleRate != 0
						? static_cast<double>(stream.position) * 28.0 / (16.0 * stream.sampleRate) : 0.0;
					const float progress = stream.duration > 0.0f
						? std::clamp(static_cast<float>(playtime / stream.duration), 0.0f, 1.0f) : 0.0f;
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("%u", id);
					ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(stream.playing ? "Playing" : (stream.ready ? "Idle" : "Loading"));
					ImGui::TableSetColumnIndex(2); ImGui::Text("%.2f s", playtime);
					ImGui::TableSetColumnIndex(3); ImGui::Text("%.2f s", stream.duration);
					ImGui::TableSetColumnIndex(4); ImGui::ProgressBar(progress, ImVec2(110.0f, 0.0f), "");
					ImGui::TableSetColumnIndex(5); ImGui::Text("%.2f", stream.volume);
					ImGui::TableSetColumnIndex(6); ImGui::Text("%u Hz / %u ch", stream.sampleRate, stream.channels);
					ImGui::TableSetColumnIndex(7); ImGui::TextUnformatted(stream.path.empty() ? "<unloaded>" : stream.path.c_str());
				}
				ImGui::EndTable();
			}
			if (visibleStreams == 0) {
				ImGui::TextDisabled(showInactive ? "No registered streams." : "No active streams.");
			}
		}

		if (ImGui::CollapsingHeader("Music", ImGuiTreeNodeFlags_DefaultOpen)) {
			const auto musicStreams = ::Audio::GetMusicStreams();
			static bool showInactiveMusic = true;
			const auto playingMusic = std::count_if(musicStreams.begin(), musicStreams.end(), [](const auto& music) {
				return music.playing;
			});
			ImGui::Text("Playing: %d    Registered: %d", static_cast<int>(playingMusic), static_cast<int>(musicStreams.size()));
			ImGui::Checkbox("Show inactive music", &showInactiveMusic);
			const size_t visibleMusic = showInactiveMusic ? musicStreams.size() : static_cast<size_t>(playingMusic);
			if (visibleMusic == 0) {
				ImGui::TextDisabled(showInactiveMusic ? "No registered music." : "No active music.");
			}
			if (visibleMusic > 0 && ImGui::BeginTable("##AudioMusic", 6, flags, GetTableSize(visibleMusic, 6))) {
				ImGui::TableSetupScrollFreeze(1, 1);
				ImGui::TableSetupColumn("Stream");
				ImGui::TableSetupColumn("State");
				ImGui::TableSetupColumn("Song");
				ImGui::TableSetupColumn("Bank");
				ImGui::TableSetupColumn("Volume");
				ImGui::TableSetupColumn("Tempo");
				ImGui::TableHeadersRow();
				for (const auto& music : musicStreams) {
					if (!showInactiveMusic && !music.playing) { continue; }
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("%u", music.index);
					ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(music.playing ? "Playing" : (music.prepared ? "Paused" : "Idle"));
					ImGui::TableSetColumnIndex(2); if (music.song == UINT32_MAX) ImGui::TextUnformatted("-"); else ImGui::Text("%u", music.song);
					ImGui::TableSetColumnIndex(3); if (music.bank == UINT32_MAX) ImGui::TextUnformatted("-"); else ImGui::Text("%u", music.bank);
					ImGui::TableSetColumnIndex(4); ImGui::Text("%.2f", music.volume);
					ImGui::TableSetColumnIndex(5); ImGui::Text("%.2f%s", music.tempo, music.looping ? " (loop)" : "");
				}
				ImGui::EndTable();
			}
		}

		DrawAmbiance(flags);

		if (ImGui::CollapsingHeader("Sound effects", ImGuiTreeNodeFlags_DefaultOpen)) {
			auto samples = ::Audio::GetSampleInstances();
			std::sort(samples.begin(), samples.end(), [](const auto& left, const auto& right) {
				return left.instanceId < right.instanceId;
			});
			ImGui::Text("Sample voices: %d", static_cast<int>(samples.size()));
			if (samples.empty()) {
				ImGui::TextDisabled("No sample voices.");
			}
			if (!samples.empty() && ImGui::BeginTable("##AudioSamples", 7, flags, GetTableSize(samples.size(), 8))) {
				ImGui::TableSetupScrollFreeze(1, 1);
				ImGui::TableSetupColumn("Instance ID");
				ImGui::TableSetupColumn("Sample handle");
				ImGui::TableSetupColumn("State");
				ImGui::TableSetupColumn("Elapsed");
				ImGui::TableSetupColumn("Length");
				ImGui::TableSetupColumn("Volume L/R");
				ImGui::TableSetupColumn("Pitch");
				ImGui::TableHeadersRow();
				for (const auto& sample : samples) {
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("0x%08X", sample.instanceId);
					ImGui::TableSetColumnIndex(1); ImGui::Text("%u", sample.sampleHandle);
					ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(sample.playing ? "Playing" : (sample.started ? "Paused" : "Ready"));
					ImGui::TableSetColumnIndex(3); ImGui::Text("%.2f s", sample.playtime);
					ImGui::TableSetColumnIndex(4); ImGui::Text("%.2f s%s", sample.duration, sample.looping ? " (loop)" : "");
					ImGui::TableSetColumnIndex(5); ImGui::Text("%.2f / %.2f", sample.controls.left, sample.controls.right);
					ImGui::TableSetColumnIndex(6); ImGui::Text("%.2f", sample.controls.pitch);
				}
				ImGui::EndTable();
			}
		}
		ImGui::EndChild();
	}

	void ShowMenu(bool* bOpen)
	{
		ImGui::SetNextWindowSize(ImVec2(1050.0f, 460.0f), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Audio", bOpen)) { DrawContents(); }
		ImGui::End();
	}
}

namespace Debug
{
	MenuRegisterer sDebugAudioMenuReg("Audio", Debug::Audio::ShowMenu, false);
}
