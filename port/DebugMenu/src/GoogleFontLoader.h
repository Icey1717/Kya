#pragma once

#include <string>

// Downloads a font from the Google Fonts API and loads it into ImGui's font atlas.
// Must be called after ImGui::CreateContext() and before the first ImGui frame.
//
// Requires the GOOGLE_FONTS_API_KEY environment variable to be set.
// The downloaded TTF is cached in the "fonts/" directory so subsequent runs
// skip the network round-trip.
namespace GoogleFontLoader {
	// fontFamily : Google Fonts family name, e.g. "Roboto", "Inter", "Noto Sans"
	// variant    : font variant key as returned by the API, e.g. "regular", "700", "italic"
	// fontSize   : size in pixels passed to ImGui
	// Returns true if the font was successfully added to the ImGui font atlas.
	bool LoadIntoImGui(const std::string& fontFamily,
	                   const std::string& variant = "regular",
	                   float fontSize = 16.0f);
}
