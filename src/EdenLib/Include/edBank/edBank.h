#ifndef _ED_BANK_H
#define _ED_BANK_H

#include "Types.h"

class edCBankConfig
{
public:
	byte field_0x0;
	byte field_0x1;
	byte field_0x2;
};

void edBankTerm();
void edBankInit();

extern byte _edBankAllowNoWait;

#endif // !_ED_BANK_H
