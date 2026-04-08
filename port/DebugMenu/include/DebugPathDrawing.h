#pragma once

#include "PathFollow.h"
#include "Native/NativeDebugShapes.h"
#include <imgui.h>

namespace Debug {
	namespace Path {
		inline void DrawPathFollow(const CPathFollow* pPath, bool bIsActive)
		{
			if (!pPath || !pPath->aSplinePoints || pPath->splinePointCount < 2) {
				return;
			}

			const float r = bIsActive ? 1.0f : 0.3f;
			const float g = bIsActive ? 1.0f : 0.5f;
			const float b = bIsActive ? 0.0f : 1.0f;
			constexpr float a = 1.0f;
			constexpr float kNodeRadius = 1.0f;

			for (int i = 0; i < pPath->splinePointCount - 1; ++i) {
				const edF32VECTOR4& cur = pPath->aSplinePoints[i];
				const edF32VECTOR4& next = pPath->aSplinePoints[i + 1];
				Renderer::Native::DebugShapes::AddLine(cur.x, cur.y, cur.z, next.x, next.y, next.z, r, g, b, a);
				Renderer::Native::DebugShapes::AddSphere(cur.x, cur.y, cur.z, kNodeRadius, r, g, b, a);
			}

			// Draw the last node
			const edF32VECTOR4& last = pPath->aSplinePoints[pPath->splinePointCount - 1];
			Renderer::Native::DebugShapes::AddSphere(last.x, last.y, last.z, kNodeRadius, r, g, b, a);
		}

		inline void DrawPathFollowReaderAbsolute(const CPathFollowReaderAbsolute& reader, bool bIsActive)
		{
			DrawPathFollow(reader.pPathFollow, bIsActive);
		}

		inline void ShowPathFollowReaderAbsoluteInfo(const CPathFollowReaderAbsolute& reader)
		{
			ImGui::Text("Path Follow: %p", reader.pPathFollow);
			ImGui::Text("field_0x4: %.2f", reader.field_0x4);
			ImGui::Text("Total Traversal Time: %.2f", reader.totalTraversalTime);
			ImGui::Text("Mid Point: %.2f", reader.midPoint);

			float sum = 0.0f;

			if (ImGui::CollapsingHeader("Segment Durations")) {
				for (int i = 0; i < reader.pPathFollow->nbLeadInPoints; i++) {
					ImGui::Text("Duration: %d: %f", i, reader.aSegmentDurations[i]);
					sum += reader.aSegmentDurations[i];
				}
			}

			ImGui::Text("Sum of Segment Durations: %.2f", sum);
			
			ImGui::Text("Type: %d  Mode: %d  Field 0x1c: %d", reader.type, reader.mode, reader.field_0x1c);
		}

		inline void ShowPathPlaneInfo(const CPathPlane& plane, bool bIsActive)
		{
			if (ImGui::TreeNodeEx("Path Plane", ImGuiTreeNodeFlags_None)) {
				ImGui::Text("Reader Path Follow: %p", plane.pathFollowReader.pPathFollow);
				ImGui::Text("Reader Spline Point Index: %d", plane.pathFollowReader.splinePointIndex);
				ImGui::Text("Reader Field 0x8: %d  Field 0xc: %d", plane.pathFollowReader.field_0x8, plane.pathFollowReader.field_0xc);

				ImGui::Separator();
				if (bIsActive) {
					ImGui::Text("Out Data (JamGut pos on path, live):");
				}
				else {
					ImGui::TextDisabled("Out Data (stale - not active path):");
				}
				ImGui::Text("  Index: %d", plane.outData.field_0x0);
				ImGui::Text("  Dist to Current Plane: %.2f", plane.outData.field_0x4);
				ImGui::Text("  Dist to Next Plane: %.2f", plane.outData.field_0x8);

				ImGui::Text("Plane Data Array: %p", plane.aPlaneData);
				ImGui::TreePop();
			}
		}
	}
}
