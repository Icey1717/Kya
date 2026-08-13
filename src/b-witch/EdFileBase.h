#ifndef ED_FILE_BASE_H
#define ED_FILE_BASE_H

#include "Types.h"

struct StaticEdFileBase
{
	void Add();
	void Remove();
	bool IsAvailable();
	bool Check();

	struct edCFiler* pEdFileBase;
	int nbActiveUsers;
};

extern StaticEdFileBase StaticEdFileBase_004497f0;

#endif // ED_FILE_BASE_H