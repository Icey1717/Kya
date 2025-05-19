#ifndef ED_FILE_BASE_H
#define ED_FILE_BASE_H

#include "Types.h"

struct StaticEdFileBase
{
	void Add();
	void Remove();
	bool FUN_00401f30();
	bool Check();

	struct edCFiler* pEdFileBase;
	undefined4 field_0x4;
};

extern StaticEdFileBase StaticEdFileBase_004497f0;

#endif // ED_FILE_BASE_H