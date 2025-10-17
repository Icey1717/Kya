#ifndef _IOP_H
#define _IOP_H

#include "Types.h"

struct edSYS_CONFIG;
struct ed_video_config;

class CCompatibilityHandling
{
public:

	virtual void edSysInit(edSYS_CONFIG* pSetupParams) { }
	virtual void edVideoInit(ed_video_config* param) { }
	virtual void BeginFrame(int area) { }
	virtual bool HandleDisconnectedDevices(int area) { return false; }
	virtual bool TestCheatCode_CheatMenu() { return false; }
	virtual bool TestCheatCode_IdleCamera() { return false; }
	virtual void ActivatePadAutoRepeat() { }
	virtual void DisactivatePadAutoRepeat() { }
};

class CCompatibilityHandlingPS2 : public CCompatibilityHandling
{
public:
	virtual void edSysInit(edSYS_CONFIG* pSetupParams);
	virtual bool HandleDisconnectedDevices(int area);
	virtual bool TestCheatCode_CheatMenu();
	virtual bool TestCheatCode_IdleCamera();
	virtual void ActivatePadAutoRepeat();
	virtual void DisactivatePadAutoRepeat();

	int disconnectedController;
};

struct edPsx2Module {
	char* name;
	int id;
	int field_0x8;
	char* cmdLine;
};

extern CCompatibilityHandling* gCompatibilityHandlingPtr;

#endif //_IOP_H
