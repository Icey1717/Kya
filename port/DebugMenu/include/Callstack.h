#pragma once

#ifdef _WIN32
#include <Windows.h>

#include <iostream> // For console output
#include <string>	// For string manipulation
#include <vector>	// For storing callstack
#include <DbgHelp.h>

inline void CollectBacktrace(std::vector<DWORD64>& backtrace) {
	const int maxFrames = 64;
	void* stackFrames[maxFrames];

	// Get the current thread's callstack
	USHORT frameCount = CaptureStackBackTrace(0, maxFrames, stackFrames, nullptr);

	for (USHORT i = 0; i < frameCount; ++i) {
		backtrace.push_back(reinterpret_cast<DWORD64>(stackFrames[i]));
	}
}

inline void CollectCallstack(std::vector<std::string>& callstack, const std::vector<DWORD64>& backtrace, bool bShowFileName = true, bool bShowLineNumber = true) {

	HANDLE processHandle = GetCurrentProcess();
	SymInitialize(processHandle, nullptr, TRUE);

	// Create a symbol buffer for storing symbol information
	const size_t symbolBufferLen = sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR);
	char symbolBuffer[symbolBufferLen];
	SYMBOL_INFO* symbolInfo = reinterpret_cast<SYMBOL_INFO*>(symbolBuffer);
	symbolInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
	symbolInfo->MaxNameLen = MAX_SYM_NAME;

	for (auto& address : backtrace) {

		// Get symbol information for the current frame
		if (SymFromAddr(processHandle, address, nullptr, symbolInfo)) {
			DWORD displacement;
			IMAGEHLP_LINE64 lineInfo = {};
			lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

			// Get source file and line number information
			if (SymGetLineFromAddr64(processHandle, address, &displacement, &lineInfo)) {
				std::string entry = symbolInfo->Name;

				if (bShowLineNumber) {
					entry += std::string(" (") + lineInfo.FileName + ":" + std::to_string(lineInfo.LineNumber) + ")";
				}
				else if (bShowFileName) {
					entry += std::string(" (") + lineInfo.FileName + ")";
				}
				

				callstack.push_back(entry);
			}
			else {
				callstack.push_back(symbolInfo->Name);
			}
		}
		else {
			callstack.push_back("Unknown Symbol");
		}
	}

	SymCleanup(processHandle);
}

inline void CollectCallstack(std::vector<std::string>& callstack) {
	std::vector<DWORD64> backtrace;
	CollectBacktrace(backtrace);
	CollectCallstack(callstack, backtrace);
}
#endif
