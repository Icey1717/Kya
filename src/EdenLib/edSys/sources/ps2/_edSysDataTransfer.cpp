#include "edSys/ps2/edSysDataTransfer.h"

int _edSoundLastTransferIndex;
int _edSysTransferIndex;
int _edSysCompletedTransferIndex;

void _edSysTransferInit(void)
{
	_edSysTransferIndex = 0;
	_edSysCompletedTransferIndex = 0;

	IMPLEMENTATION_GUARD_PS2(
	DIntr();
	sceSifAddCmdHandler(0, _edSysTransferReturnTransferData, (void*)0x0);
	EIntr();)

	return;
}

void _edSysWaitUntilTransferFinished(uint param_1)
{
	if ((_edSysTransferIndex != 0) && (param_1 != 0)) {
		do {
		} while (_edSysCompletedTransferIndex < param_1);
	}

	return;
}

void _edSysWaitUntilAllTransfersFinished(void)
{
	_edSysWaitUntilTransferFinished(_edSysTransferIndex);

	_edSysTransferIndex = 0;
	_edSysCompletedTransferIndex = 0;

	return;
}

/*
  The complete path is:

  EE _edSysTransferData
	  │ sends 0x40-byte request through RPC service 0x45535953
	  ▼
  IOP _edSysRPCTransfer
	  │ creates an IOP transfer node
	  ▼
  IOP _edSysTransferThread
	  │ transfers pSource into its IOP buffer
	  │ runs setup/transfer/end callbacks
	  │ callbacks write result into node->preturn_buffer
	  ▼
  IOP sceSifSendCmd(0, _pedSysCmdPacket, ...)
	  │ inline result is at packet + 0x1c
	  ▼
  EE _sceSifCmdIntrHdlr
	  ▼
  EE _edSysTransferReturnTransferData
  */

/*
  struct EdSysTransferNode
  {
	  void* pdata;                       // 0x00 = pSource
	  uint nbBytes;                      // 0x04
	  uint grain;                        // 0x08 = alignment
	  int setupCallbackIdent;            // 0x0c
	  int transferCallbackIdent;         // 0x10
	  int endCallbackIdent;              // 0x14
	  uint transferIndex;                // 0x18
	  edSysTransferFunc* pfuncCallback;  // 0x1c
	  void* pReturnAddress;              // 0x20 = pCallbackParams
	  uint returnNbBytes;                // 0x24
	  EdSysTransferNode* pNext;          // 0x28
	  void* pReturnBuffer;               // 0x2c, assigned to packet + 0x1c
	  void* pBuffer;                     // 0x30
	  uint blockNbBytes;                 // 0x34
	  uint setupFlags;                   // 0x38
	  uint userData;                     // 0x3c
  };*/

uint _edSysTransferData(void* pSource, uint size, uint alignment, EdSysTransferFlags setupFlags, _EDSYS_RPC_CB_IDENT setupCallbackIdent, _EDSYS_RPC_CB_IDENT transferCallbackIdent, _EDSYS_RPC_CB_IDENT endCallbackIdent,
	void* pCallbackParams, uint returnNbBytes, uint userData, edSysTransferFunc* pCallback)
{
	uint newTransferIndex;

	if (size == 0) {
		newTransferIndex = 0;
	}
	else {
		_edSysTransferIndex = _edSysTransferIndex + 1;
		IMPLEMENTATION_GUARD_PS2(
		//edCRPCClient<1, 1163090259>::WaitRPCCompletion(_pedSysRPCClient);
		_edSysTransferNode.pCallback = pCallback;
		_edSysTransferNode.returnNbBytes = returnNbBytes;
		_edSysTransferNode.userData = userData;
		_edSysTransferNode.transferIndex = _edSysTransferIndex;
		_edSysTransferNode.pSource = pSource;
		_edSysTransferNode.size = size;
		_edSysTransferNode.alignment = alignment;
		_edSysTransferNode.setupCallbackIdent = setupCallbackIdent;
		_edSysTransferNode.transferCallbackIdent = transferCallbackIdent;
		_edSysTransferNode.endCallbackIdent = endCallbackIdent;
		_edSysTransferNode.pCallbackParams = pCallbackParams;
		_edSysTransferNode.setupFlags = setupFlags;
		//edCRPCClient<1, 1163090259>::CallRPCNoWait(_pedSysRPCClient, 0, 0, &_edSysTransferNode, 0x40, 0, 0, 0, (uint*)0x0);
		)
		newTransferIndex = _edSysTransferIndex;
	}

	return newTransferIndex;
}