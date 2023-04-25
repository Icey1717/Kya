#ifndef _IOP_H
#define _IOP_H

#include "Types.h"

struct InputSetupParams;

class IopBase
{
public:
	virtual ~IopBase() {};

	virtual void SetupIopPaths(InputSetupParams* pSetupParams) {}
	virtual void IOPFunc_0x10(int param) { }
	virtual void IOPFunc_0x14(int param) { }
	virtual bool GetAnyControllerConnected(int param) { return false; }
};

class IopManager : public IopBase
{
public:
	virtual ~IopManager() {};

	virtual void SetupIopPaths(InputSetupParams* pSetupParams);
	virtual bool GetAnyControllerConnected();

	int disconnectedController;
};

struct edPsx2Module {
	char* name;
	int id;
	int field_0x8;
	char* cmdLine;
};

extern IopManager* PTR_IopManager_00448cf8;

#endif //_IOP_H
