#include "edSys/ps2/edSysRPC.h"
#include "edSys/ps2/edSysDataTransfer.h"
#include "profile.h"

uint _edSysRPCProfile;
uint _edSysRPCProfileWaiting;

void _edSysInitRPC(void)
{
	edSYS_CONFIG* pIVar1;
	//edCRPCClient<1, 1163090259>* lVar2;
	//long lVar3;

	pIVar1 = edSysGetConfig();
	if (pIVar1->enableProfiling != 0) {
		_edSysRPCProfile = edProfileNew(1, 0xff, 0, 0xff, "RPC");
		_edSysRPCProfileWaiting = edProfileNew(1, 0xff, 0x80, 0xff, "RPCWait");
	}

	IMPLEMENTATION_GUARD_PS2(
	//lVar2 = (edCRPCClient<1, 1163090259> *)operator.new(0x84);
	//lVar3 = (long)(int)lVar2;
	//if (lVar3 != 0) {
	//	edCRPCClient<1, 1163090259>::edCRPCClient<1, 1163090259>(lVar2);
	//	lVar2 = (edCRPCClient<1, 1163090259> *)lVar3;
	//}
	_pedSysRPCClient = lVar2;
	_edSysMainThreadId = GetThreadId();
	ChangeThreadPriority(_edSysMainThreadId, 2);
	sceSifInitCmd();
	DIntr();
	_pedSysCmdBufferOld = (int)sceSifSetCmdBuffer(&g_SifCmdHandlerValue_00449198, 1);
	EIntr();)

	_edSysTransferInit();

	return;
}

