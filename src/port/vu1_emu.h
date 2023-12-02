#pragma once
#include "Types.h"

namespace VU1Emu {
	void SendVu1Code(unsigned char* pCode, size_t size);
	void ProcessVifList(edpkt_data* pVifPkt, bool bRunCode = true);
	void SetVifItop(uint newItop);
	void UpdateMemory(const edpkt_data* pVifPkt, const edpkt_data* pEnd);
	bool& GetInterpreterEnabled();
	bool& GetRunSingleThreaded();
	bool& GetRunSimplifiedCode();
	bool& GetTraceVtx();

	char* GetVertexDataStart();

	void QueueDraw();

	// Multithread
	void Wait();
}