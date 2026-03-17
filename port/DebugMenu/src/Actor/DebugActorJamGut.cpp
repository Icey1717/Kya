#include "DebugActorJamGut.h"
#include "ActorJamGut.h"
#include "PathFollow.h"
#include <imgui.h>
#include <sstream>
#include <iomanip>
#include "DebugPathDrawing.h"

namespace Debug::Actor::JamGut
{
	static std::string GetVector4String(const edF32VECTOR4& vec)
	{
		std::stringstream ss;
		ss << std::fixed << std::setprecision(2)
		   << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
		return ss.str();
	}

	static void ShowPathFollowReaderAbsoluteInfo(const CPathFollowReaderAbsolute& reader)
	{
		ImGui::Text("Path Follow: %p", reader.pActor3C_0x0);
		ImGui::Text("Bar Full Amount: %.2f", reader.barFullAmount_0x4);
		ImGui::Text("Field 0x8: %.2f", reader.field_0x8);
		ImGui::Text("Field 0xc: %.2f", reader.field_0xc);
		ImGui::Text("Field 0x10: %p", reader.field_0x10);
		ImGui::Text("Type: %d  Mode: %d  Field 0x1c: %d", reader.type, reader.mode, reader.field_0x1c);
	}

	static void ShowPathPlaneInfo(const CPathPlane& plane, bool bIsActive)
	{
		if (ImGui::TreeNodeEx("Path Plane", ImGuiTreeNodeFlags_None)) {
			ImGui::Text("Reader Path Follow: %p", plane.pathFollowReader.pPathFollow);
			ImGui::Text("Reader Spline Point Index: %d", plane.pathFollowReader.splinePointIndex);
			ImGui::Text("Reader Field 0x8: %d  Field 0xc: %d", plane.pathFollowReader.field_0x8, plane.pathFollowReader.field_0xc);

			ImGui::Separator();
			if (bIsActive) {
				ImGui::Text("Out Data (JamGut pos on path, live):");
			} else {
				ImGui::TextDisabled("Out Data (stale - not active path):");
			}
			ImGui::Text("  Index: %d", plane.outData.field_0x0);
			ImGui::Text("  Dist to Current Plane: %.2f", plane.outData.field_0x4);
			ImGui::Text("  Dist to Next Plane: %.2f", plane.outData.field_0x8);

			ImGui::Text("Plane Data Array: %p", plane.aPlaneData);
			ImGui::TreePop();
		}
	}

	static void DrawPathSplineLines(const CPathFollowReaderAbsolute& reader, bool bIsActive)
	{
		Debug::Path::DrawPathFollowReaderAbsolute(reader, bIsActive);
	}

	static void ShowPathInfo(S_JMG_PATH& path, int index, int activePathIndex)
	{
		const bool bIsActive = (index == activePathIndex);
		std::string label = bIsActive
			? "Path #" + std::to_string(index) + " [ACTIVE]"
			: "Path #" + std::to_string(index);

		if (ImGui::TreeNodeEx(label.c_str(), bIsActive ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None)) {
			DrawPathSplineLines(path.pathFollowReader, bIsActive);
			ImGui::Text("Zone Ref: %p", path.zoneRef.Get());
			ImGui::Text("Actor Ref: %p", path.actorRef.Get());
			ImGui::Text("Field 0x28: %.2f", path.field_0x28);
			ImGui::Text("Field 0x2c: %.2f", path.field_0x2c);
			ImGui::Text("Field 0x30: %.2f", path.field_0x30);

			ShowPathPlaneInfo(path.pathPlane, bIsActive);

			if (ImGui::TreeNodeEx("Path Follow Reader", ImGuiTreeNodeFlags_None)) {
				ShowPathFollowReaderAbsoluteInfo(path.pathFollowReader);
				ImGui::TreePop();
			}

			ImGui::TreePop();
		}
	}

	static void ShowCheckpointInfo(S_JMG_CHECKPOINT& checkpoint, int index)
	{
		std::string label = "Checkpoint #" + std::to_string(index);
		if (ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_None)) {
			ImGui::Text("Zone Ref: %p", checkpoint.zoneRef.Get());
			ImGui::Text("Waypoint Ref: %p", checkpoint.waypointRef.Get());
			ImGui::TreePop();
		}
	}

	void ShowJamGutActorDetails(CActorJamGut* pJamGutActor)
	{
		if (!pJamGutActor) {
			ImGui::Text("JamGut Actor: null");
			return;
		}

		if (ImGui::CollapsingHeader("Path State", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Active Path Index: %d", pJamGutActor->activePathIndex);
			ImGui::Text("Path Follow Direction: %s", GetVector4String(pJamGutActor->pathFollowDirection).c_str());
			ImGui::Text("Path Follow Intensity: %.2f", pJamGutActor->pathFollowIntensity);
			ImGui::Text("Path Follow Cmd Walk: %d", pJamGutActor->pathFollowCmdWalk);
			ImGui::Text("Path Follow Cmd Run: %d", pJamGutActor->pathFollowCmdRun);
			ImGui::Text("Field 0x64c: %.2f", pJamGutActor->field_0x64c);
			ImGui::Text("Time On Segment: %.2f", pJamGutActor->pathTimeOnSegment);
			ImGui::Text("Tracked Path Actor: %p", pJamGutActor->pTrackedPathActor);

			if (pJamGutActor->pTrackedPathActor != nullptr) {
				ImGui::Separator();
				ImGui::Text("Tracked Actor Path Plane Out:");
				ImGui::Text("  Index: %d", pJamGutActor->pathPlaneOutData.field_0x0);
				ImGui::Text("  Dist to Current Plane: %.2f", pJamGutActor->pathPlaneOutData.field_0x4);
				ImGui::Text("  Dist to Next Plane: %.2f", pJamGutActor->pathPlaneOutData.field_0x8);
			}

			ImGui::Separator();
			ImGui::Text("Field 0x618: %d", pJamGutActor->field_0x618);
			ImGui::Text("Field 0x61c (flags): 0x%x", pJamGutActor->field_0x61c);
			ImGui::Text("Field 0x620: 0x%x", pJamGutActor->field_0x620);
		}

		if (ImGui::CollapsingHeader("Checkpoint Positions", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Checkpoint A Pos: %s", GetVector4String(pJamGutActor->field_0x5b0).c_str());
			ImGui::Text("Checkpoint A Rot: (%.2f, %.2f, %.2f)", pJamGutActor->field_0x5c0.x, pJamGutActor->field_0x5c0.y, pJamGutActor->field_0x5c0.z);
			ImGui::Text("Checkpoint A Marker: %d", pJamGutActor->field_0x5cc);

			ImGui::Spacing();
			ImGui::Text("Checkpoint B Pos: %s", GetVector4String(pJamGutActor->field_0x5d0).c_str());
			ImGui::Text("Checkpoint B Rot: (%.2f, %.2f, %.2f)", pJamGutActor->field_0x5e0.x, pJamGutActor->field_0x5e0.y, pJamGutActor->field_0x5e0.z);
			ImGui::Text("Checkpoint B Marker: %d", pJamGutActor->field_0x5ec);

			ImGui::Spacing();
			ImGui::Text("Path Pos: %s", GetVector4String(pJamGutActor->field_0x5f0).c_str());
			ImGui::Text("Path Rot: (%.2f, %.2f, %.2f)", pJamGutActor->field_0x600.x, pJamGutActor->field_0x600.y, pJamGutActor->field_0x600.z);
			ImGui::Text("Path State Marker: %d", pJamGutActor->field_0x60c);
		}

		if (ImGui::CollapsingHeader("Paths", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Nb Paths: %d", pJamGutActor->nbPaths);
			if (pJamGutActor->nbPaths > 0 && pJamGutActor->aPaths) {
				for (int i = 0; i < pJamGutActor->nbPaths; ++i) {
					ShowPathInfo(pJamGutActor->aPaths[i], i, pJamGutActor->activePathIndex);
				}
			}
		}

		if (ImGui::CollapsingHeader("Checkpoints", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Nb Checkpoints: %d", pJamGutActor->nbCheckpoints);
			if (pJamGutActor->nbCheckpoints > 0 && pJamGutActor->aCheckpoints) {
				for (int i = 0; i < pJamGutActor->nbCheckpoints; ++i) {
					ShowCheckpointInfo(pJamGutActor->aCheckpoints[i], i);
				}
			}
		}
	}
}
