#pragma once

#include <array>

namespace Debug {
	namespace Collision {
		struct ObbDrawConfig {
			bool bEnabled;
			bool bShowTreeNodes;
			bool bShowLeafBbox;
			bool bShowTriangles;
			bool bShowQuads;
			bool bShowPrimObjects;
			bool bShowBoxes;
			bool bShowSpheres;
			bool bShowUnknown;
			std::array<float, 4> colTreeNode;
			std::array<float, 4> colTreeNodeDyn;
			std::array<float, 4> colLeafBbox;
			std::array<float, 4> colTriEdge;
			std::array<float, 4> colQuadEdge;
			std::array<float, 4> colPrimObj;
			std::array<float, 4> colBox;
			std::array<float, 4> colSphere;
			std::array<float, 4> colUnknown;
		};

		ObbDrawConfig BuildActorConfig();

		void DrawDebugShapes();
		void DrawContents();
		void ShowMenu(bool* bOpen);
		void AddRay(float sx, float sy, float sz, float ex, float ey, float ez, float r, float g, float b, float a);
	}
}
