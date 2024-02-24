#ifndef _INIFILE_H
#define _INIFILE_H

#include "Types.h"

class CIniFile
{
public:
	bool ReadHexValueFromFile(int* outVal);
	bool ReadInteger(int* outVal);

	CIniFile();

	~CIniFile();

	void Load_001ab200(const char* inName, char* inFileBufferStart, int fileSize);
	bool ReadQuotedString(char* outValue);

	bool SeekToKey_001a7ed0(const char* key, bool bHasSection);

	bool ReadInt_001a9830(char* section, char* key, int* outValue);
	bool ReadString_001aa520(const char* section, const char* key, char* outValue);

	char* fileBufferStart;
private:
	char name[128];
	char* currentSeekPosition;
	char* endSeekPosition;
};

extern CIniFile g_IniFile_00450750;

#endif //_INIFILE_H
