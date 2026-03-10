#pragma once

#include "b-witch/Types.h"

namespace Debug {
	namespace Collision {
		// Box collision primitives: 8 corners of a unit cube
		static constexpr edF32VECTOR4 g_cube_corners[8] = {
			{ 0.5f,  0.5f,  0.5f, 1.0f},
			{ 0.5f,  0.5f, -0.5f, 1.0f},
			{ 0.5f, -0.5f,  0.5f, 1.0f},
			{ 0.5f, -0.5f, -0.5f, 1.0f},
			{-0.5f,  0.5f,  0.5f, 1.0f},
			{-0.5f,  0.5f, -0.5f, 1.0f},
			{-0.5f, -0.5f,  0.5f, 1.0f},
			{-0.5f, -0.5f, -0.5f, 1.0f}
		};

		// 12 triangular faces of the cube, each defined by 3 corner indices
		static constexpr int g_cube_tri[12][3] = {
			{ 0x0, 0x2, 0x3 },
			{ 0x0, 0x3, 0x1 },
			{ 0x4, 0x6, 0x2 },
			{ 0x4, 0x2, 0x0 },
			{ 0x5, 0x7, 0x6 },
			{ 0x5, 0x6, 0x4 },
			{ 0x1, 0x3, 0x7 },
			{ 0x1, 0x7, 0x5 },
			{ 0x0, 0x1, 0x5 },
			{ 0x0, 0x5, 0x4 },
			{ 0x2, 0x6, 0x7 },
			{ 0x2, 0x7, 0x3 },
		};

		// 3 edges of a triangle, each defined by 2 vertex indices
		static constexpr int g_tri_edge[3][2] = {
			{ 0x0, 0x1 },
			{ 0x1, 0x2 },
			{ 0x2, 0x0 },
		};
	}
}
