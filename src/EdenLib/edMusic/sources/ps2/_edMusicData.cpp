#include "edSys/ps2/edSysDataTransfer.h"

uint _edMusicLastTransferIndex;

uint _edMusicLoadDataNoWait(void* pSource, uint size, void* pCallbackParams, edSysTransferFunc* pCallback)
{
	_edMusicLastTransferIndex = _edSysTransferData(pSource, size, 4, (EdSysTransferFlags)3, 4, 5, 6, pCallbackParams, 4, 0, pCallback);
	return _edMusicLastTransferIndex;
}