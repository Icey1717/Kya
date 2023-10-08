#pragma once

#include <vector>
#include <utility>

using MacroSet = const std::vector<std::pair<std::string, std::vector<int>>>;

inline const MacroSet VS_MACROS = {
	{ "TME", { 0, 1 } },
	{ "FST", { 0, 1 } },
};

inline const MacroSet PS_MACROS = {
	{ "ATST", { 0, 1, 2, 3, 4 } }, 
	{ "FOG", { 0, 1 } },
	{ "COLCLIP", { 0, 1 } },
	{ "TFX", { 0, 1, 2, 3, 4 } },
	{ "BLEND_A", { 0, 1, 2, 3 } },
	{ "BLEND_B", { 0, 1, 2, 3 } },
	{ "BLEND_C", { 0, 1, 2, 3 } },
	{ "BLEND_D", { 0, 1, 2, 3 } },
};

inline const MacroSet GS_MACROS = {
	{ "PRIM", { 0, 1, 2, 3 } },
	{ "POINT", { 0, 1 } },
	{ "LINE", { 0, 1 } },
	{ "IIP", { 0, 1 } },
};