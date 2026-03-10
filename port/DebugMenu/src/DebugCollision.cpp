#include "DebugCollision.h"

#include <imgui.h>

#include "DebugSetting.h"
#include "Actor/DebugActorCollision.h"
#include "DebugSceneryCollision.h"
#include "Native/NativeDebugShapes.h"
#include "DebugHero.h"

namespace Debug {
	namespace Collision {

		// --- Actor OBB settings ---
		static Setting<bool> gShowActorObbs        ("Show Actor OBBs",             false);
		static Setting<bool> gActorShowTreeNodes   ("Actor: Show Tree Nodes",       true);
		static Setting<bool> gActorShowLeafBbox    ("Actor: Show Leaf BBox",        true);
		static Setting<bool> gActorShowTriangles   ("Actor: Show Triangles",        true);
		static Setting<bool> gActorShowQuads       ("Actor: Show Quads",            true);
		static Setting<bool> gActorShowPrimObjects ("Actor: Show Prim Objects",     true);
		static Setting<bool> gActorShowBoxes       ("Actor: Show Boxes",            true);
		static Setting<bool> gActorShowSpheres     ("Actor: Show Spheres",          true);
		static Setting<bool> gActorShowUnknown     ("Actor: Show Unknown",          true);

		static Setting<std::array<float, 4>> gActorColTreeNode    ("Actor OBB Color: Tree Node",           { 0.0f, 1.0f, 0.0f, 1.0f });
		static Setting<std::array<float, 4>> gActorColTreeNodeDyn ("Actor OBB Color: Tree Node (Dynamic)", { 0.5f, 1.0f, 0.0f, 1.0f });
		static Setting<std::array<float, 4>> gActorColLeafBbox    ("Actor OBB Color: Leaf BBox",           { 0.0f, 1.0f, 1.0f, 1.0f });
		static Setting<std::array<float, 4>> gActorColTriEdge     ("Actor OBB Color: Triangle Edges",      { 1.0f, 0.5f, 0.0f, 1.0f });
		static Setting<std::array<float, 4>> gActorColQuadEdge    ("Actor OBB Color: Quad Edges",          { 1.0f, 0.5f, 0.0f, 1.0f });
		static Setting<std::array<float, 4>> gActorColPrimObj     ("Actor OBB Color: Prim Object",         { 1.0f, 1.0f, 0.0f, 1.0f });
		static Setting<std::array<float, 4>> gActorColBox         ("Actor OBB Color: Box",                 { 1.0f, 1.0f, 0.0f, 1.0f });
		static Setting<std::array<float, 4>> gActorColSphere      ("Actor OBB Color: Sphere / Capsule",    { 1.0f, 0.0f, 1.0f, 1.0f });
		static Setting<std::array<float, 4>> gActorColUnknown     ("Actor OBB Color: Unknown",             { 0.0f, 1.0f, 1.0f, 1.0f });

		// --- Scenery OBB settings ---
		static Setting<bool> gShowSceneryObbs        ("Show Scenery OBBs",            false);
		static Setting<bool> gSceneryShowTreeNodes   ("Scenery: Show Tree Nodes",      true);
		static Setting<bool> gSceneryShowLeafBbox    ("Scenery: Show Leaf BBox",       true);
		static Setting<bool> gSceneryShowTriangles   ("Scenery: Show Triangles",       true);
		static Setting<bool> gSceneryShowQuads       ("Scenery: Show Quads",           true);
		static Setting<bool> gSceneryShowPrimObjects ("Scenery: Show Prim Objects",    true);
		static Setting<bool> gSceneryShowBoxes       ("Scenery: Show Boxes",           true);
		static Setting<bool> gSceneryShowSpheres     ("Scenery: Show Spheres",         true);
		static Setting<bool> gSceneryShowUnknown     ("Scenery: Show Unknown",         true);

		static Setting<std::array<float, 4>> gSceneryColTreeNode    ("Scenery OBB Color: Tree Node",           { 0.0f, 0.8f, 0.3f, 1.0f });
		static Setting<std::array<float, 4>> gSceneryColTreeNodeDyn ("Scenery OBB Color: Tree Node (Dynamic)", { 0.4f, 0.8f, 0.0f, 1.0f });
		static Setting<std::array<float, 4>> gSceneryColLeafBbox    ("Scenery OBB Color: Leaf BBox",           { 0.2f, 0.8f, 1.0f, 1.0f });
		static Setting<std::array<float, 4>> gSceneryColTriEdge     ("Scenery OBB Color: Triangle Edges",      { 1.0f, 0.8f, 0.2f, 1.0f });
		static Setting<std::array<float, 4>> gSceneryColQuadEdge    ("Scenery OBB Color: Quad Edges",          { 1.0f, 0.6f, 0.2f, 1.0f });
		static Setting<std::array<float, 4>> gSceneryColPrimObj     ("Scenery OBB Color: Prim Object",         { 0.8f, 1.0f, 0.2f, 1.0f });
		static Setting<std::array<float, 4>> gSceneryColBox         ("Scenery OBB Color: Box",                 { 0.8f, 1.0f, 0.2f, 1.0f });
		static Setting<std::array<float, 4>> gSceneryColSphere      ("Scenery OBB Color: Sphere / Capsule",    { 0.8f, 0.2f, 1.0f, 1.0f });
		static Setting<std::array<float, 4>> gSceneryColUnknown     ("Scenery OBB Color: Unknown",             { 0.5f, 0.5f, 0.5f, 1.0f });

		static Setting<bool> gShowCollisionRays("Show Collision Rays", false);

		static ObbDrawConfig BuildActorConfig()
		{
			ObbDrawConfig cfg;
			cfg.bEnabled        = gShowActorObbs;
			cfg.bShowTreeNodes  = gActorShowTreeNodes;
			cfg.bShowLeafBbox   = gActorShowLeafBbox;
			cfg.bShowTriangles  = gActorShowTriangles;
			cfg.bShowQuads      = gActorShowQuads;
			cfg.bShowPrimObjects = gActorShowPrimObjects;
			cfg.bShowBoxes      = gActorShowBoxes;
			cfg.bShowSpheres    = gActorShowSpheres;
			cfg.bShowUnknown    = gActorShowUnknown;
			cfg.colTreeNode     = gActorColTreeNode.get();
			cfg.colTreeNodeDyn  = gActorColTreeNodeDyn.get();
			cfg.colLeafBbox     = gActorColLeafBbox.get();
			cfg.colTriEdge      = gActorColTriEdge.get();
			cfg.colQuadEdge     = gActorColQuadEdge.get();
			cfg.colPrimObj      = gActorColPrimObj.get();
			cfg.colBox          = gActorColBox.get();
			cfg.colSphere       = gActorColSphere.get();
			cfg.colUnknown      = gActorColUnknown.get();
			return cfg;
		}

		static ObbDrawConfig BuildSceneryConfig()
		{
			ObbDrawConfig cfg;
			cfg.bEnabled         = gShowSceneryObbs;
			cfg.bShowTreeNodes   = gSceneryShowTreeNodes;
			cfg.bShowLeafBbox    = gSceneryShowLeafBbox;
			cfg.bShowTriangles   = gSceneryShowTriangles;
			cfg.bShowQuads       = gSceneryShowQuads;
			cfg.bShowPrimObjects = gSceneryShowPrimObjects;
			cfg.bShowBoxes       = gSceneryShowBoxes;
			cfg.bShowSpheres     = gSceneryShowSpheres;
			cfg.bShowUnknown     = gSceneryShowUnknown;
			cfg.colTreeNode      = gSceneryColTreeNode.get();
			cfg.colTreeNodeDyn   = gSceneryColTreeNodeDyn.get();
			cfg.colLeafBbox      = gSceneryColLeafBbox.get();
			cfg.colTriEdge       = gSceneryColTriEdge.get();
			cfg.colQuadEdge      = gSceneryColQuadEdge.get();
			cfg.colPrimObj       = gSceneryColPrimObj.get();
			cfg.colBox           = gSceneryColBox.get();
			cfg.colSphere        = gSceneryColSphere.get();
			cfg.colUnknown       = gSceneryColUnknown.get();
			return cfg;
		}

		void DrawDebugShapes()
		{
			const ObbDrawConfig actorCfg   = BuildActorConfig();
			const ObbDrawConfig sceneryCfg = BuildSceneryConfig();

			Debug::Actor::DrawDebugShapes(actorCfg);
			Debug::Scenery::DrawDebugShapes(sceneryCfg);
			Debug::Hero::DrawCollisionContacts();
		}

		// Draws a checkbox + color picker on one row.
		static void DrawTypeRow(Setting<bool>& toggle, Setting<std::array<float, 4>>& color)
		{
			toggle.DrawImguiControl();
			ImGui::SameLine();
			color.DrawImguiControl();
		}

		// Draws a checkbox + two color pickers on one row (for tree node static/dynamic).
		static void DrawTypeRowDual(Setting<bool>& toggle,
			Setting<std::array<float, 4>>& colorA,
			Setting<std::array<float, 4>>& colorB)
		{
			toggle.DrawImguiControl();
			ImGui::SameLine();
			colorA.DrawImguiControl();
			ImGui::SameLine();
			colorB.DrawImguiControl();
		}

		static void DrawLayerSection(
			Setting<bool>& showToggle,
			Setting<bool>& showTreeNodes,  Setting<bool>& showLeafBbox,
			Setting<bool>& showTriangles,  Setting<bool>& showQuads,
			Setting<bool>& showPrimObjs,   Setting<bool>& showBoxes,
			Setting<bool>& showSpheres,    Setting<bool>& showUnknown,
			Setting<std::array<float, 4>>& colTreeNode,    Setting<std::array<float, 4>>& colTreeNodeDyn,
			Setting<std::array<float, 4>>& colLeafBbox,
			Setting<std::array<float, 4>>& colTriEdge,    Setting<std::array<float, 4>>& colQuadEdge,
			Setting<std::array<float, 4>>& colPrimObj,    Setting<std::array<float, 4>>& colBox,
			Setting<std::array<float, 4>>& colSphere,     Setting<std::array<float, 4>>& colUnknown)
		{
			showToggle.DrawImguiControl();

			if (showToggle) {
				DrawTypeRowDual(showTreeNodes, colTreeNode, colTreeNodeDyn);
				DrawTypeRow(showLeafBbox,    colLeafBbox);
				DrawTypeRow(showTriangles,   colTriEdge);
				DrawTypeRow(showQuads,       colQuadEdge);
				DrawTypeRow(showPrimObjs,    colPrimObj);
				DrawTypeRow(showBoxes,       colBox);
				DrawTypeRow(showSpheres,     colSphere);
				DrawTypeRow(showUnknown,     colUnknown);
			}
		}

		void DrawContents()
		{
			ImGui::PushID("CollisionDebug");

			ImGui::Text("Debug lines this frame: %u", Renderer::Native::DebugShapes::GetLineCount());

			ImGui::Separator();

			gShowCollisionRays.DrawImguiControl();

			if (ImGui::CollapsingHeader("Actor Collision")) {
				ImGui::PushID("Actor");
				DrawLayerSection(
					gShowActorObbs,
					gActorShowTreeNodes,   gActorShowLeafBbox,
					gActorShowTriangles,   gActorShowQuads,
					gActorShowPrimObjects, gActorShowBoxes,
					gActorShowSpheres,     gActorShowUnknown,
					gActorColTreeNode,     gActorColTreeNodeDyn,
					gActorColLeafBbox,
					gActorColTriEdge,      gActorColQuadEdge,
					gActorColPrimObj,      gActorColBox,
					gActorColSphere,       gActorColUnknown);
				ImGui::PopID();
			}

			if (ImGui::CollapsingHeader("Scenery Collision")) {
				ImGui::PushID("Scenery");
				DrawLayerSection(
					gShowSceneryObbs,
					gSceneryShowTreeNodes,   gSceneryShowLeafBbox,
					gSceneryShowTriangles,   gSceneryShowQuads,
					gSceneryShowPrimObjects, gSceneryShowBoxes,
					gSceneryShowSpheres,     gSceneryShowUnknown,
					gSceneryColTreeNode,     gSceneryColTreeNodeDyn,
					gSceneryColLeafBbox,
					gSceneryColTriEdge,      gSceneryColQuadEdge,
					gSceneryColPrimObj,      gSceneryColBox,
					gSceneryColSphere,       gSceneryColUnknown);
				ImGui::PopID();
			}

			ImGui::PopID();
		}

		void ShowMenu(bool* bOpen)
		{
			ImGui::Begin("Collision Debug", bOpen, ImGuiWindowFlags_AlwaysAutoResize);
			DrawContents();
			ImGui::End();
		}


		void AddRay(float sx, float sy, float sz, float ex, float ey, float ez, float r, float g, float b, float a)
		{
			if (gShowCollisionRays) {
				Renderer::Native::DebugShapes::AddLine(sx, sy, sz, ex, ey, ez, r, g, b, a);
			}
		}
	}
}
