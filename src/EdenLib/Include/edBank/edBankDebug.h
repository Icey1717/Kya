#ifndef _ED_BANK_DEBUG
#define _ED_BANK_DEBUG

#include "Types.h"

struct edCBankDebug {
	byte field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	int bankCount;
	struct edCBankBuffer* pNextBank;
	undefined4 field_0xc;
	int selectedFileIndex;
	void buffer_link(edCBankBuffer* pBankBuffer);
	void buffer_unlink(edCBankBuffer* pBankBuffer);
};

extern edCBankDebug edBankDebug;

#endif //_ED_BANK_DEBUG