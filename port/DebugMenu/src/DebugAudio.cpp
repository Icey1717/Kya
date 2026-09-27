#include "DebugAudio.h"
#include "DebugMenu.h"

#include "../../Audio/edSoundStreamService.h"
#include "../../Audio/edSoundSampleService.h"
#include "../../Audio/edMusicService.h"

#include <imgui.h>
#include <algorithm>
#include <cstdint>

namespace Debug::Audio
{
	void DrawContents()
	{
		auto streams = ::Audio::GetStreams();
		std::sort(streams.begin(), streams.end(), [](const auto& left, const auto& right) {
			return left.first < right.first;
		});

		int playingCount = 0;
		for (const auto& [id, stream] : streams) {
			if (stream.playing) { ++playingCount; }
		}
		ImGui::Text("Playing: %d    Registered: %d", playingCount, static_cast<int>(streams.size()));
		ImGui::SameLine();
		static bool showInactive = false;
		ImGui::Checkbox("Show inactive streams", &showInactive);
		ImGui::TextDisabled("Time is the decoded stream position reported by the Windows audio backend.");

		const ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
			ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;
		if (ImGui::BeginTable("##AudioStreams", 8, flags, ImVec2(0.0f, 350.0f))) {
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
		if (streams.empty() || (!showInactive && playingCount == 0)) {
			ImGui::TextDisabled("No active streams.");
		}

		if (ImGui::CollapsingHeader("Sound effects", ImGuiTreeNodeFlags_DefaultOpen)) {
			auto samples = ::Audio::GetSampleInstances();
			std::sort(samples.begin(), samples.end(), [](const auto& left, const auto& right) {
				return left.instanceId < right.instanceId;
			});
			ImGui::Text("Sample voices: %d", static_cast<int>(samples.size()));
			if (ImGui::BeginTable("##AudioSamples", 7, flags)) {
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

		if (ImGui::CollapsingHeader("Music", ImGuiTreeNodeFlags_DefaultOpen)) {
			const auto musicStreams = ::Audio::GetMusicStreams();
			if (ImGui::BeginTable("##AudioMusic", 6, flags)) {
				ImGui::TableSetupColumn("Stream");
				ImGui::TableSetupColumn("State");
				ImGui::TableSetupColumn("Song");
				ImGui::TableSetupColumn("Bank");
				ImGui::TableSetupColumn("Volume");
				ImGui::TableSetupColumn("Tempo");
				ImGui::TableHeadersRow();
				for (const auto& music : musicStreams) {
					if (!showInactive && !music.playing) { continue; }
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
