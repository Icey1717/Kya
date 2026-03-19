#include "GoogleFontLoader.h"

#include <windows.h>
#include <winhttp.h>
#include <imgui.h>
#include <nlohmann/json.hpp>

#include "DebugMenu.h"
#include "DebugSetting.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

// ---- Font settings (persisted via DebugSetting) --------------------------------

struct FontOption {
	const char* label;
	const char* family;
};

static constexpr FontOption kFontOptions[] = {
	{ "Roboto (clean, neutral)",       "Roboto"        },
	{ "Inter (modern, readable)",      "Inter"         },
	{ "Noto Sans (broad char support)","Noto Sans"     },
	{ "JetBrains Mono (monospace)",    "JetBrains Mono"},
	{ "Nunito (rounded, friendly)",    "Nunito"        },
};
static constexpr int kNumFontOptions = static_cast<int>(std::size(kFontOptions));

static const char* kFontLabels[kNumFontOptions] = {
	kFontOptions[0].label, kFontOptions[1].label, kFontOptions[2].label,
	kFontOptions[3].label, kFontOptions[4].label,
};

static Debug::ComboSetting gFontFamily("Google Font Family", 0,
	{kFontLabels[0], kFontLabels[1], kFontLabels[2], kFontLabels[3], kFontLabels[4]});
static Debug::Setting<float> gFontSize("Google Font Size", 16.0f);

// ---- HTTP helpers ---------------------------------------------------------------

std::wstring ToWide(const std::string& s)
{
	if (s.empty()) return {};
	int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
	std::wstring w(len - 1, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), len);
	return w;
}

std::string ToNarrow(const std::wstring& w)
{
	if (w.empty()) return {};
	int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
	std::string s(len - 1, '\0');
	WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, s.data(), len, nullptr, nullptr);
	return s;
}

std::vector<uint8_t> HttpGet(const std::wstring& url)
{
	URL_COMPONENTS urlComp = {};
	urlComp.dwStructSize   = sizeof(urlComp);
	wchar_t scheme[16]     = {};
	wchar_t host[256]      = {};
	wchar_t path[2048]     = {};
	urlComp.lpszScheme     = scheme; urlComp.dwSchemeLength   = ARRAYSIZE(scheme);
	urlComp.lpszHostName   = host;   urlComp.dwHostNameLength = ARRAYSIZE(host);
	urlComp.lpszUrlPath    = path;   urlComp.dwUrlPathLength  = ARRAYSIZE(path);

	if (!WinHttpCrackUrl(url.c_str(), 0, 0, &urlComp)) {
		std::cerr << "GoogleFontLoader: failed to parse URL: " << ToNarrow(url) << "\n";
		return {};
	}

	HINTERNET hSession = WinHttpOpen(L"KyaDebug/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession) { std::cerr << "GoogleFontLoader: WinHttpOpen failed\n"; return {}; }

	HINTERNET hConnect = WinHttpConnect(hSession, host, urlComp.nPort, 0);
	if (!hConnect) {
		std::cerr << "GoogleFontLoader: WinHttpConnect failed\n";
		WinHttpCloseHandle(hSession);
		return {};
	}

	const DWORD reqFlags = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path,
		nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, reqFlags);
	if (!hRequest) {
		std::cerr << "GoogleFontLoader: WinHttpOpenRequest failed\n";
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return {};
	}

	if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
	                        WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
	    !WinHttpReceiveResponse(hRequest, nullptr))
	{
		std::cerr << "GoogleFontLoader: request failed (error " << GetLastError() << ")\n";
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return {};
	}

	std::vector<uint8_t> data;
	DWORD bytesRead = 0;
	uint8_t buf[8192];
	while (WinHttpReadData(hRequest, buf, sizeof(buf), &bytesRead) && bytesRead > 0)
		data.insert(data.end(), buf, buf + bytesRead);

	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);
	return data;
}

// ---- Debug menu -----------------------------------------------------------------

void ShowFontMenu(bool* pOpen)
{
	if (!ImGui::Begin("Fonts", pOpen)) { ImGui::End(); return; }

	ImGui::TextDisabled("Changes take effect on next restart.");
	ImGui::Spacing();

	gFontFamily.DrawImguiControl();
	ImGui::SetItemTooltip("Select a Google Font to download and use for the debug UI.");

	ImGui::SetNextItemWidth(120.0f);
	gFontSize.DrawImguiControl();
	ImGui::SetItemTooltip("Font size in pixels.");

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// Show cache status for each font
	ImGui::TextDisabled("Cache status:");
	for (int i = 0; i < kNumFontOptions; i++) {
		const std::string cacheFile = std::string("fonts/") + kFontOptions[i].family + "-regular.ttf";
		const bool bCached = std::filesystem::exists(cacheFile);
		if (bCached)
			ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "  [cached] %s", kFontOptions[i].label);
		else
			ImGui::TextDisabled("  [not downloaded] %s", kFontOptions[i].label);
	}

	ImGui::End();
}

Debug::MenuRegisterer sFontMenuReg("Fonts", ShowFontMenu);

} // anonymous namespace

// ---- Public API -----------------------------------------------------------------

bool GoogleFontLoader::LoadIntoImGui(const std::string& fontFamily,
                                     const std::string& variant,
                                     float fontSize)
{
	// Use persisted settings if the caller passed the defaults
	const int familyIdx = gFontFamily.get();
	const std::string resolvedFamily  = (familyIdx >= 0 && familyIdx < kNumFontOptions)
	                                    ? kFontOptions[familyIdx].family : fontFamily;
	const float resolvedSize = gFontSize.get();

	std::filesystem::create_directories("fonts");
	const std::string cacheFile = "fonts/" + resolvedFamily + "-" + variant + ".ttf";

	if (!std::filesystem::exists(cacheFile)) {
		char apiKeyBuf[256] = {};
		if (GetEnvironmentVariableA("GOOGLE_FONTS_API_KEY", apiKeyBuf, sizeof(apiKeyBuf)) == 0) {
			std::cerr << "GoogleFontLoader: GOOGLE_FONTS_API_KEY env var not set; "
			             "font '" << resolvedFamily << "' will not be downloaded\n";
			return false;
		}

		const std::wstring apiUrl = L"https://www.googleapis.com/webfonts/v1/webfonts?key="
			+ ToWide(apiKeyBuf) + L"&family=" + ToWide(resolvedFamily);

		std::cout << "GoogleFontLoader: querying API for '" << resolvedFamily << "'...\n";
		const auto apiResponse = HttpGet(apiUrl);
		if (apiResponse.empty()) {
			std::cerr << "GoogleFontLoader: empty response from Google Fonts API\n";
			return false;
		}

		const auto json = nlohmann::json::parse(
			apiResponse.begin(), apiResponse.end(), nullptr, false);
		if (json.is_discarded() || !json.contains("items") || json["items"].empty()) {
			std::cerr << "GoogleFontLoader: font family '" << resolvedFamily << "' not found\n";
			return false;
		}

		const auto& files = json["items"][0]["files"];
		if (!files.contains(variant)) {
			std::cerr << "GoogleFontLoader: variant '" << variant
			          << "' not found for family '" << resolvedFamily << "'\n";
			std::cerr << "GoogleFontLoader: available variants:";
			for (auto& [k, v] : files.items()) std::cerr << " " << k;
			std::cerr << "\n";
			return false;
		}

		const std::string fontUrl = files[variant].get<std::string>();
		std::cout << "GoogleFontLoader: downloading from " << fontUrl << "\n";

		const auto fontData = HttpGet(ToWide(fontUrl));
		if (fontData.empty()) {
			std::cerr << "GoogleFontLoader: failed to download font from " << fontUrl << "\n";
			return false;
		}

		std::ofstream outFile(cacheFile, std::ios::binary);
		if (!outFile) {
			std::cerr << "GoogleFontLoader: failed to write cache file: " << cacheFile << "\n";
			return false;
		}
		outFile.write(reinterpret_cast<const char*>(fontData.data()),
		              static_cast<std::streamsize>(fontData.size()));
		std::cout << "GoogleFontLoader: cached to " << cacheFile << "\n";
	}

	ImGuiIO& io = ImGui::GetIO();
	ImFont* pFont = io.Fonts->AddFontFromFileTTF(cacheFile.c_str(), resolvedSize);
	if (!pFont) {
		std::cerr << "GoogleFontLoader: ImGui failed to load font from " << cacheFile << "\n";
		return false;
	}

	std::cout << "GoogleFontLoader: loaded '" << resolvedFamily << "' (" << variant
	          << ") at " << resolvedSize << "px\n";
	return true;
}

