#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Debug::Socket
{
	// All operations run on the main thread. Port zero selects an available port (tests).
	bool Start(uint16_t port);
	void Stop();
	void Poll();
	uint16_t GetPort();

	struct RequestInfo
	{
		uint64_t sequence = 0;
		uint64_t frame = 0;
		std::string command, requestId, peer, state, error;
		double processingMs = 0, elapsedMs = 0;
		size_t responseBytes = 0, sentBytes = 0;
		bool finished = false;
	};
	// Newest first, including live connections. History contains metadata only.
	std::vector<RequestInfo> GetRequests();
	void ClearHistory();
}
