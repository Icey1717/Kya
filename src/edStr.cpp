#include "edStr.h"
#include "stdarg.h"

char* edStrReturnEndPtr(char* str)
{
	for (; *str != '\0'; str = str + 1) {
	}
	return str;
}

int edStrLength(const char* str)
{
	int iVar1;

	for (iVar1 = 0; str[iVar1] != '\0'; iVar1 = iVar1 + 1) {
	}
	return iVar1;
}

const char* edStrCat(char* str1, const char* str2)
{
	char* bufferPos;
	char currentSuffixChar;

	bufferPos = edStrReturnEndPtr(str1);
	do {
		currentSuffixChar = *str2;
		str2 = str2 + 1;
		*bufferPos = currentSuffixChar;
		bufferPos = bufferPos + 1;
	} while (currentSuffixChar != '\0');
	return str1;
}

void edStrCatMulti(char* dst, char* src, ...)
{
	va_list args;
	va_start(args, src);

	char* suffix = va_arg(args, char*);

	edStrCopy(dst, src);

	while (suffix != 0x0)
	{
		edStrCat(dst, suffix);
		suffix = va_arg(args, char*);
	}

	va_end(args);
}

int edStrCopy(char* outString, const char* inString)
{
	int len;
	char currentCharacter;

	len = 0;
	if (*inString == '\0') {
		*outString = '\0';
	}
	else {
		while (true) {
			currentCharacter = *inString;
			inString = inString + 1;
			*outString = currentCharacter;
			if (currentCharacter == '\0') break;
			len = len + 1;
			outString = outString + 1;
		}
	}
	return len;
}

int edStrCopyUpper(char* outBuffer, char* inString)
{
	int stringLength;
	char outCharacter;
	int counter;
	char currentCharacter;

	counter = 0;
	do {
		stringLength = counter;
		currentCharacter = *inString;
		inString = inString + 1;
		outCharacter = currentCharacter;
		if (('`' < currentCharacter) && (currentCharacter < '{')) {
			outCharacter = currentCharacter + -0x20;
		}
		*outBuffer = outCharacter;
		outBuffer = outBuffer + 1;
		counter = stringLength + 1;
	} while (currentCharacter != '\0');
	return stringLength;
}

int edStrCmp(char* __s1, char* __s2)
{
	char cVar1;
	int iVar2;

	for (; ((cVar1 = *__s1, cVar1 != '\0' && (*__s2 != '\0')) && (cVar1 == *__s2)); __s1 = __s1 + 1) {
		__s2 = __s2 + 1;
	}
	if (((cVar1 != '\0') || (iVar2 = 0, *__s2 != '\0')) && (iVar2 = 1, cVar1 == '\0')) {
		iVar2 = -1;
	}
	return iVar2;
}

int edStrICmp(byte* __s1, byte* __s2)
{
	byte bVar1;
	int iVar2;

	for (; ((bVar1 = *__s1, bVar1 != 0 && (*__s2 != 0)) && ((bVar1 & 0xdf) == (*__s2 & 0xdf))); __s1 = __s1 + 1) {
		__s2 = __s2 + 1;
	}
	if (((bVar1 != 0) || (iVar2 = 0, *__s2 != 0)) && (iVar2 = 1, bVar1 == 0)) {
		iVar2 = -1;
	}
	return iVar2;
}

// SKIP

int edStrStr2Int(char* stream, int offset)
{
	char currentChar;
	int result;
	uint tempIndexA;
	uint tempIndexB;
	int multiplier;

	multiplier = 1;
	tempIndexA = offset - 1;
	result = 0;
	if (offset != 0) {
		do {
			tempIndexB = tempIndexA & 0xff;
			currentChar = stream[tempIndexA & 0xff];
			if ((currentChar == '-') || (currentChar == '+')) {
				if (currentChar == '-') {
					result = -result;
				}
			}
			else {
				// If the current character is a digit
				// add it to the final result
				result = result + multiplier * (currentChar + -0x30);
				multiplier = multiplier * 10;
			}
			tempIndexA = tempIndexB - 1;
		} while (tempIndexB != 0);
	}
	return result;
}

char* edStrChr(char* inString, char searchChar)
{
	char currentChar;

	/* Returns the position in the inBuffer of first instance of searchChar */
	for (; (currentChar = *inString, currentChar != '\0' && (currentChar != searchChar)); inString = inString + 1) {
	}
	if (currentChar == '\0') {
		inString = (char*)0x0;
	}
	return inString;
}


void edStrInt2Str(uint value, char* str, uint len, bool padWithSpaces)
{
	uint uVar1;
	char* pcVar2;
	uint uVar3;
	uint uVar4;
	uint uVar5;

	uVar1 = (len & 0xff) - 1;

	if ((int)value < 0) {
		uVar5 = 1;
		if (padWithSpaces != false) {
			*str = '-';
		}
		uVar4 = -value;
	}
	else {
		uVar5 = 0;
		uVar4 = value;
	}

	for (uVar3 = len & 0xff; uVar5 < uVar3; uVar3 = uVar3 - 1) {
		str[uVar3 - 1] = (char)(uVar4 % 10) + '0';

		if (uVar4 % 10 != 0) {
			uVar1 = uVar3 - 1;
		}
		uVar4 = uVar4 / 10;
	}

	str[len & 0xff] = '\0';

	if (padWithSpaces == false) {
		uVar4 = 0;

		if (uVar1 != 0) {
			if (8 < uVar1) {
				do {
					pcVar2 = str + uVar4;
					*pcVar2 = ' ';
					uVar4 = uVar4 + 8;
					pcVar2[1] = ' ';
					pcVar2[2] = ' ';
					pcVar2[3] = ' ';
					pcVar2[4] = ' ';
					pcVar2[5] = ' ';
					pcVar2[6] = ' ';
					pcVar2[7] = ' ';
				} while (uVar4 < uVar1 - 8);
			}

			for (; uVar4 < uVar1; uVar4 = uVar4 + 1) {
				str[uVar4] = ' ';
			}
		}

		if ((int)value < 0) {
			str[uVar1 - 1] = '-';
		}
	}
	return;
}


