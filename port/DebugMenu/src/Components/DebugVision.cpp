#include "DebugVision.h"
#include "Vision.h"
#include "Native/NativeDebugShapes.h"
#include <imgui.h>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace Debug::Components::Vision
{
	static std::string GetVector4String(const edF32VECTOR4& vec)
	{
		std::stringstream ss;
		ss << std::fixed << std::setprecision(2)
		   << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
		return ss.str();
	}

	static void ShowFlagsInfo(uint flags)
	{
		ImGui::Text("Flags: 0x%02x", flags);
		ImGui::Indent();
		ImGui::TextDisabled(flags & 0x01 ? "[x] Terrain collision" : "[ ] Terrain collision");
		ImGui::TextDisabled(flags & 0x02 ? "[x] Geometry collision" : "[ ] Geometry collision");
		ImGui::TextDisabled(flags & 0x08 ? "[x] Require lockable target" : "[ ] Require lockable target");
		ImGui::TextDisabled(flags & 0x10 ? "[x] Require class priority / Sort by priority" : "[ ] Require class priority / Sort by priority");
		ImGui::TextDisabled(flags & 0x20 ? "[x] Select best actor in front" : "[ ] Select best actor in front");
		ImGui::Unindent();
	}

	void ShowVisionDetails(CVision* pVision)
	{
		if (!pVision) {
			ImGui::Text("Vision: null");
			return;
		}

		if (ImGui::CollapsingHeader("Vision Config", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Owner: %p", pVision->pOwner);
			ImGui::Separator();
			ShowFlagsInfo(pVision->flags);
		}

		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Location:     %s", GetVector4String(pVision->location).c_str());
			ImGui::Text("Direction:    %s", GetVector4String(pVision->rotationQuat).c_str());
		}

		if (ImGui::CollapsingHeader("Cone Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
			const float halfAngleDeg = pVision->halfAngle * (180.0f / 3.14159265f);
			ImGui::Text("Half Angle:   %.2f deg  (%.4f rad)", halfAngleDeg, pVision->halfAngle);
			ImGui::Text("Cos Angle:    %.4f  (field_0x44)", pVision->field_0x44);
			ImGui::Text("Vision Range: %.2f", pVision->visionRange);
			ImGui::Text("﻿Apex Offset: %.2f", pVision->apexOffset);
			ImGui::Text("Half Height:  %.2f  (field_0x38)", pVision->field_0x38);
			ImGui::Text("Half Width:   %.2f  (field_0x3c)", pVision->field_0x3c);
		}

		if (ImGui::CollapsingHeader("Scan State", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Cached Target:        %p  (pActor_0x48)", pVision->pActor_0x48);
			ImGui::Text("Detection Result:     %u  (field_0x50)", pVision->field_0x50);
			ImGui::Text("Detection Accum:      %d  (field_0x54)", pVision->field_0x54);
			ImGui::Text("Amortised Scan Index: %d", pVision->amortisedScanFrameIndex);
			ImGui::Text("Last Scan Time:       %.3f  (field_0x58)", pVision->field_0x58);
		}
	}

	void DrawVisionShapes(CVision* pVision)
	{
		if (!pVision || pVision->visionRange <= 0.01f || pVision->halfAngle <= 0.001f) {
			return;
		}

		const edF32VECTOR4& origin = pVision->location;
		const edF32VECTOR4& dir    = pVision->rotationQuat;
		const float innerRadius    = pVision->apexOffset;
		const float range          = pVision->visionRange;
		const float halfAngle      = pVision->halfAngle;

		const float fx = dir.x, fy = dir.y, fz = dir.z;

		// Perpendicular right axis (Gram-Schmidt against world-up)
		float ux = 0.0f, uy = 1.0f, uz = 0.0f;
		if (fabsf(fy) > 0.99f) { ux = 1.0f; uy = 0.0f; }
		float rx = uy * fz - uz * fy;
		float ry = uz * fx - ux * fz;
		float rz = ux * fy - uy * fx;
		const float rLen = sqrtf(rx * rx + ry * ry + rz * rz);
		if (rLen > 0.0001f) { rx /= rLen; ry /= rLen; rz /= rLen; }

		// Up axis = facing × right
		const float ax = fy * rz - fz * ry;
		const float ay = fz * rx - fx * rz;
		const float az = fx * ry - fy * rx;

		// SV_PointIsInVision does the angular check from local_10 = location - field_0x40 * dir,
		// so that is the true apex. Total cone length from apex to far boundary is field_0x40 + range.
		const float totalLen   = innerRadius + range;
		const float baseRadius = totalLen * tanf(halfAngle);

		// Apex: origin - innerRadius * dir
		// Base: origin + range * dir
		// Center: midpoint = origin + dir * (range - innerRadius) * 0.5f
		const float cx = origin.x + fx * (range - innerRadius) * 0.5f;
		const float cy = origin.y + fy * (range - innerRadius) * 0.5f;
		const float cz = origin.z + fz * (range - innerRadius) * 0.5f;

		// axisY points BACKWARD (-facing) so AddCone apex (+Y/2) is at the true apex (local_10)
		// and the wide base (-Y/2) is at origin + range * dir.
		float mat[16] = {
			rx * baseRadius,  ry * baseRadius,  rz * baseRadius,  0.0f,
			-fx * totalLen,   -fy * totalLen,   -fz * totalLen,   0.0f,
			ax * baseRadius,  ay * baseRadius,  az * baseRadius,  0.0f,
			cx,               cy,               cz,               1.0f,
		};

		// Draw sphere at the origin
		Renderer::Native::DebugShapes::AddFilledSphere(origin.x, origin.y, origin.z, 0.1f, 1.0f, 0.5f, 0.2f, 1.0f);

		Renderer::Native::DebugShapes::AddFilledCone(mat, 1.0f, 0.4f, 0.0f, 0.18f);
		Renderer::Native::DebugShapes::AddCone(mat, 1.0f, 0.6f, 0.0f, 0.9f);
	}
}
