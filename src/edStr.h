#ifndef _ED_STR_H
#define _ED_STR_H

#include "Types.h"


char* edStrReturnEndPtr(char* str);
int edStrLength(const char* str);
const char* edStrCat(char* str1, const char* str2);
void edStrCatMulti(char* dst, char* src, ...);
int edStrCopy(char* outString, const char* inString);
int edStrnCopy(char* dst, char* src, int n);
int edStrCopyUpper(char* outBuffer, char* inString);
int edStrCmp(char* __s1, char* __s2);
int edStrICmp(byte* __s1, byte* __s2);

int edStrStr2Int(char* stream, int offset);
char* edStrChr(char* inString, char searchChar);

void edStrInt2Str(uint value, char* str, uint len, bool padWithSpaces);

#endif //_ED_STR_H
