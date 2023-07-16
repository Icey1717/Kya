#pragma once
#include "Types.h"

namespace VU1Emu {
	void ProcessVifList(edpkt_data* pVifPkt);
	void SetVifItop(uint newItop);
	void UpdateMemory(edpkt_data* pVifPkt, edpkt_data* pEnd);
	void QueueWrite(uint vuaddr, uint num, void* data);
	void ClearWriteQueue();
	//void UnpackToAddr(uint addr, void* data, int size);
}