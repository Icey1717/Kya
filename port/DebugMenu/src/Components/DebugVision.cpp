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

	static void DrawVisionShapesImpl(CVision* pVision, bool bFilled, bool bWireframe)
	{
		if (!pVision || pVision->visionRange <= 0.01f || pVision->halfAngle <= 0.001f) {
			return;
		}

		const edF32VECTOR4& origin = pVision->location;
		const edF32VECTOR4& dir    = pVision->rotationQuat;
		const float innerRadius    = pVision->apexOffset;
		const float range          = pVision->visionRange;
		const float halfAngle      = pVision->halfAngle;
		const float halfHeight     = pVision->field_0x38;

		const float fx = dir.x, fy = dir.y, fz = dir.z;
		const float totalLen = innerRadius + range;

		// True apex: the point used for the angular/range check in SV_PointIsInVision.
		const float apexX = origin.x - innerRadius * fx;
		const float apexY = origin.y - innerRadius * fy;
		const float apexZ = origin.z - innerRadius * fz;

		// Draw a small sphere at the vision origin (location).
		Renderer::Native::DebugShapes::AddFilledSphere(origin.x, origin.y, origin.z, 0.1f, 1.0f, 0.5f, 0.2f, 1.0f);

		if (fabsf(fy) < 0.00125f)
		{
			// Horizontal facing: SV_PointIsInVision does all checks in XZ with a separate
			// ±halfHeight slab in Y. The true detection volume is a wedge (XZ sector) capped
			// by flat top/bottom planes at apexY ± halfHeight — NOT a 3D cone.
			// Draw that wedge exactly so the visualisation matches the game's geometry.

			// Normalised XZ facing (fy ≈ 0, so length ≈ 1 already, but normalise for safety).
			const float fxzLen = sqrtf(fx * fx + fz * fz);
			const float dfx = (fxzLen > 0.0001f) ? fx / fxzLen : 1.0f;
			const float dfz = (fxzLen > 0.0001f) ? fz / fxzLen : 0.0f;

			const float cosA = cosf(halfAngle);
			const float sinA = sinf(halfAngle);

			// Left and right edge directions in XZ (rotate ±halfAngle around world-up).
			const float leftX = dfx * cosA - dfz * sinA,  leftZ = dfx * sinA + dfz * cosA;
			const float rightX = dfx * cosA + dfz * sinA, rightZ = -dfx * sinA + dfz * cosA;

			const float topY = apexY + halfHeight;
			const float botY = apexY - halfHeight;

			// Far-face corners (at XZ distance totalLen from apex).
			const float flx = apexX + totalLen * leftX,  flz = apexZ + totalLen * leftZ;
			const float frx = apexX + totalLen * rightX, frz = apexZ + totalLen * rightZ;

			// Inner-face corners (at XZ distance innerRadius from apex — the minimum range).
			const float ilx = apexX + innerRadius * leftX,  ilz = apexZ + innerRadius * leftZ;
			const float irx = apexX + innerRadius * rightX, irz = apexZ + innerRadius * rightZ;

			// Filled approximation: fan of OBB slices covering the arc, each a thin rectangular
			// prism at the slice's mid-angle direction.  Drawn first so wireframe sits on top.
			if (bFilled) {
				constexpr int kFillSlices = 8;
				const float dTheta    = 2.0f * halfAngle / (float)kFillSlices;
				const float avgR      = (innerRadius + totalLen) * 0.5f;
				const float halfDepth = range * 0.5f;
				const float sliceHW   = avgR * sinf(dTheta * 0.5f);

				for (int i = 0; i < kFillSlices; ++i) {
					const float theta = -halfAngle + ((float)i + 0.5f) * dTheta;
					const float cosT  = cosf(theta), sinT = sinf(theta);
					// Mid-direction of this slice in XZ.
					const float mdx = dfx * cosT - dfz * sinT;
					const float mdz = dfx * sinT + dfz * cosT;
					// Perpendicular direction (rotate 90° CW in XZ).
					const float pdx = -mdz, pdz = mdx;
					const float scx = apexX + mdx * (innerRadius + halfDepth);
					const float scz = apexZ + mdz * (innerRadius + halfDepth);
					// mat4x4: axisX = perp, axisY = world-up, axisZ = mid-dir, rowT = centre.
					float sliceMat[16] = {
						pdx,  0.0f, pdz,  0.0f,
						0.0f, 1.0f, 0.0f, 0.0f,
						mdx,  0.0f, mdz,  0.0f,
						scx,  apexY, scz, 1.0f,
					};
					Renderer::Native::DebugShapes::AddFilledOBB(sliceMat, sliceHW, halfHeight, halfDepth,
						1.0f, 0.4f, 0.0f, 0.18f);
				}
			}

			if (bWireframe) {
				auto line = [](float x1, float y1, float z1, float x2, float y2, float z2) {
					Renderer::Native::DebugShapes::AddLine(x1, y1, z1, x2, y2, z2, 1.0f, 0.6f, 0.0f, 0.9f);
				};

				// Vertical line at apex centre.
				line(apexX, topY, apexZ, apexX, botY, apexZ);

				// Lateral edges from apex to far corners (top and bottom faces).
				line(apexX, topY, apexZ, flx, topY, flz);
				line(apexX, topY, apexZ, frx, topY, frz);
				line(apexX, botY, apexZ, flx, botY, flz);
				line(apexX, botY, apexZ, frx, botY, frz);

				// Vertical edges at the far corners.
				line(flx, topY, flz, flx, botY, flz);
				line(frx, topY, frz, frx, botY, frz);

				// Inner-face boundary (minimum range), if significant.
				if (innerRadius > 0.01f) {
					line(ilx, topY, ilz, irx, topY, irz);
					line(ilx, botY, ilz, irx, botY, irz);
					line(ilx, topY, ilz, ilx, botY, ilz);
					line(irx, topY, irz, irx, botY, irz);
				}

				// Arc along the far face in both the top and bottom planes.
				constexpr int kArcSegs = 8;
				float prevX = flx, prevZ = flz;
				for (int i = 1; i <= kArcSegs; ++i) {
					const float t     = static_cast<float>(i) / kArcSegs;
					const float angle = halfAngle * (1.0f - 2.0f * t); // +halfAngle → -halfAngle
					const float nx    = apexX + totalLen * (dfx * cosf(angle) - dfz * sinf(angle));
					const float nz    = apexZ + totalLen * (dfx * sinf(angle) + dfz * cosf(angle));
					line(prevX, topY, prevZ, nx, topY, nz);
					line(prevX, botY, prevZ, nx, botY, nz);
					prevX = nx; prevZ = nz;
				}
			}
		}
		else
		{
			// Non-horizontal facing: SV_PointIsInVision uses a proper 3D cone check.
			// A 3D cone correctly represents this detection volume.

			// Perpendicular right axis (Gram-Schmidt against world-up).
			float ux = 0.0f, uy = 1.0f, uz = 0.0f;
			if (fabsf(fy) > 0.99f) { ux = 1.0f; uy = 0.0f; }
			float rx = uy * fz - uz * fy;
			float ry = uz * fx - ux * fz;
			float rz = ux * fy - uy * fx;
			const float rLen = sqrtf(rx * rx + ry * ry + rz * rz);
			if (rLen > 0.0001f) { rx /= rLen; ry /= rLen; rz /= rLen; }

			// Up axis = facing × right.
			const float ax = fy * rz - fz * ry;
			const float ay = fz * rx - fx * rz;
			const float az = fx * ry - fy * rx;

			const float baseRadius = totalLen * tanf(halfAngle);

			// AddCone: apex at +Y/2, base at -Y/2.  Point axisY backward so apex = local_10.
			const float cx = origin.x + fx * (range - innerRadius) * 0.5f;
			const float cy = origin.y + fy * (range - innerRadius) * 0.5f;
			const float cz = origin.z + fz * (range - innerRadius) * 0.5f;

			float mat[16] = {
				rx * baseRadius,  ry * baseRadius,  rz * baseRadius,  0.0f,
				-fx * totalLen,   -fy * totalLen,   -fz * totalLen,   0.0f,
				ax * baseRadius,  ay * baseRadius,  az * baseRadius,  0.0f,
				cx,               cy,               cz,               1.0f,
			};

			if (bFilled)    { Renderer::Native::DebugShapes::AddFilledCone(mat, 1.0f, 0.4f, 0.0f, 0.18f); }
			if (bWireframe) { Renderer::Native::DebugShapes::AddCone(mat, 1.0f, 0.6f, 0.0f, 0.9f); }
		}
	}

	void DrawVisionShapesFilled(CVision* pVision)    { DrawVisionShapesImpl(pVision, true,  false); }
	void DrawVisionShapesWireframe(CVision* pVision) { DrawVisionShapesImpl(pVision, false, true);  }
	void DrawVisionShapes(CVision* pVision)          { DrawVisionShapesImpl(pVision, true,  true);  }
}
