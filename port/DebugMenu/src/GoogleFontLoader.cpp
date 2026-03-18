#include "GoogleFontLoader.h"

#include <windows.h>
#include <winhttp.h>
#include <imgui.h>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

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

// Performs an HTTP/HTTPS GET and returns the response body as a byte vector.
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

} // anonymous namespace

bool GoogleFontLoader::LoadIntoImGui(const std::string& fontFamily,
                                     const std::string& variant,
                                     float fontSize)
{
	std::filesystem::create_directories("fonts");
	const std::string cacheFile = "fonts/" + fontFamily + "-" + variant + ".ttf";

	if (!std::filesystem::exists(cacheFile)) {
		// Read API key from environment
		char apiKeyBuf[256] = {};
		if (GetEnvironmentVariableA("GOOGLE_FONTS_API_KEY", apiKeyBuf, sizeof(apiKeyBuf)) == 0) {
			std::cerr << "GoogleFontLoader: GOOGLE_FONTS_API_KEY env var not set; "
			             "font '" << fontFamily << "' will not be downloaded\n";
			return false;
		}

		// Query Google Fonts Developer API v1
		const std::wstring apiUrl = L"https://www.googleapis.com/webfonts/v1/webfonts?key="
			+ ToWide(apiKeyBuf) + L"&family=" + ToWide(fontFamily);

		std::cout << "GoogleFontLoader: querying API for '" << fontFamily << "'...\n";
		const auto apiResponse = HttpGet(apiUrl);
		if (apiResponse.empty()) {
			std::cerr << "GoogleFontLoader: empty response from Google Fonts API\n";
			return false;
		}

		// Parse response JSON
		const auto json = nlohmann::json::parse(
			apiResponse.begin(), apiResponse.end(), nullptr, /*allow_exceptions=*/false);
		if (json.is_discarded()) {
			std::cerr << "GoogleFontLoader: failed to parse API JSON response\n";
			return false;
		}

		if (!json.contains("items") || json["items"].empty()) {
			std::cerr << "GoogleFontLoader: font family '" << fontFamily << "' not found\n";
			return false;
		}

		const auto& files = json["items"][0]["files"];
		if (!files.contains(variant)) {
			std::cerr << "GoogleFontLoader: variant '" << variant
			          << "' not found for family '" << fontFamily << "'\n";
			std::cerr << "GoogleFontLoader: available variants:";
			for (auto& [k, v] : files.items())
				std::cerr << " " << k;
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

		// Cache to disk
		std::ofstream outFile(cacheFile, std::ios::binary);
		if (!outFile) {
			std::cerr << "GoogleFontLoader: failed to write cache file: " << cacheFile << "\n";
			return false;
		}
		outFile.write(reinterpret_cast<const char*>(fontData.data()),
		              static_cast<std::streamsize>(fontData.size()));
		std::cout << "GoogleFontLoader: cached to " << cacheFile << "\n";
	}

	// Load into ImGui
	ImGuiIO& io = ImGui::GetIO();
	ImFont* pFont = io.Fonts->AddFontFromFileTTF(cacheFile.c_str(), fontSize);
	if (!pFont) {
		std::cerr << "GoogleFontLoader: ImGui failed to load font from " << cacheFile << "\n";
		return false;
	}

	std::cout << "GoogleFontLoader: loaded '" << fontFamily << "' (" << variant
	          << ") at " << fontSize << "px\n";
	return true;
}
