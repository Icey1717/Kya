#include <port.h>
#include <windows.h>

void sceScfGetLocalTimefromRTC(sceCdCLOCK* localTime)
{
	SYSTEMTIME systemTime;
	GetLocalTime(&systemTime);
	localTime->second = systemTime.wSecond;
}

