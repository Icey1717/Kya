#ifndef _IOP_H
#define _IOP_H

#include "Types.h"

struct InputSetupParams;

class IopBase
{
public:
	virtual ~IopBase() {};

	virtual void SetupIopPaths(InputSetupParams* pSetupParams) {}
};

class IopManager : public IopBase
{
public:
	virtual ~IopManager() {};

	virtual void SetupIopPaths(InputSetupParams* pSetupParams);
};

struct edPsx2Module {
	char* name;
	int id;
	int field_0x8;
	char* cmdLine;
};

#endif //_IOP_H
