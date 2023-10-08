#pragma once
#include "Types.h"

namespace VU1Emu {
	void SendVu1Code(unsigned char* pCode, size_t size);
	void ProcessVifList(edpkt_data* pVifPkt);
	void SetVifItop(uint newItop);
	void UpdateMemory(edpkt_data* pVifPkt, edpkt_data* pEnd);
	bool& GetInterpreterEnabled();
}