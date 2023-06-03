#include "IniFile.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Types.h"

#if defined(PLATFORM_PS2)
#include <eekernel.h>
#endif
#include "edStr.h"

EdFileGlobal_10 g_IniFile_00450750_mem;
IniFile g_IniFile_00450750;


bool IniFile::ReadHexValueFromFile(int* hexValue)
{
	char currentCharA;
	char currentCharB;
	bool isValidChar;
	int tempValue;
	int base;
	int tempHexValue;
	long tempHexChar;
	int numOfValidChars;
	char* currentPos;

	numOfValidChars = 0;
	base = 1;
	while (currentPos = currentSeekPosition, currentPos != endSeekPosition) {
		currentCharB = *currentPos;
		currentCharA = currentCharB;
		if (('`' < currentCharB) && (currentCharB < '{')) {
			currentCharA = currentCharB + -0x20;
		}

		// Check if the current character is a line feed or carriage return
		isValidChar = currentCharA == '\r';
		if (isValidChar) {
			currentCharA = currentPos[1];
			if (('`' < currentCharA) && (currentCharA < '{')) {
				currentCharA = currentCharA + -0x20;
			}
			isValidChar = currentCharA == '\n';
		}
		if (isValidChar) break;

		// Check if the current character is a digit or a letter between 'A' and 'F'
		currentCharA = *currentPos;
		if (('`' < currentCharA) && (currentCharA < '{')) {
			currentCharA = currentCharA + -0x20;
		}
		isValidChar = '/' < currentCharA;
		if (isValidChar) {
			currentCharA = currentCharB;
			if (('`' < currentCharB) && (currentCharB < '{')) {
				currentCharA = currentCharB + -0x20;
			}
			isValidChar = currentCharA < ':';
		}
		if (!isValidChar) {
			currentCharA = currentCharB;
			if (('`' < currentCharB) && (currentCharB < '{')) {
				currentCharA = currentCharB + -0x20;
			}
			isValidChar = '@' < currentCharA;
			if (isValidChar) {
				if (('`' < currentCharB) && (currentCharB < '{')) {
					currentCharB = currentCharB + -0x20;
				}
				isValidChar = currentCharB < 'G';
			}
		}
		if ((!isValidChar) || (8 < numOfValidChars)) break;
		numOfValidChars = numOfValidChars + 1;
		currentSeekPosition = currentSeekPosition + 1;
	}
	if ((numOfValidChars < 1) || (8 < numOfValidChars)) {
		isValidChar = false;
	}
	else {
		*hexValue = 0;
		currentPos = currentSeekPosition;
		while (numOfValidChars != 0) {
			currentPos = currentPos + -1;
			numOfValidChars = numOfValidChars + -1;
			currentCharB = *currentPos;
			if (('`' < currentCharB) && (currentCharB < '{')) {
				currentCharB = currentCharB + -0x20;
			}
			tempHexChar = ((long)currentCharB << 0x38) >> 0x38;
			isValidChar = false;
			if ((0x2f < tempHexChar) && (tempHexChar < 0x3a)) {
				isValidChar = true;
			}
			tempHexValue = (int)(((long)currentCharB << 0x38) >> 0x38);
			tempValue = tempHexValue + -0x37;
			if (isValidChar) {
				tempValue = tempHexValue + -0x30;
			}
			tempValue = tempValue * base;
			base = base << 4;
			*hexValue = *hexValue + tempValue;
		}
		isValidChar = true;
	}
	return isValidChar;
}

bool IniFile::ReadInteger(int* outVal)
{
	char currentChar;
	char tempChar;
	char* streamStart;
	char* currentPosition;
	bool isHex;
	bool isSuccess;
	int parsedValue;
	uint tempIndex;
	int multiplier;

	currentChar = *currentSeekPosition;
	multiplier = 1;
	tempChar = currentChar;
	if (('`' < currentChar) && (currentChar < '{')) {
		tempChar = currentChar + -0x20;
	}
	isHex = tempChar == '0';
	if (isHex) {
		tempChar = currentSeekPosition[1];
		if (('`' < tempChar) && (tempChar < '{')) {
			tempChar = tempChar + -0x20;
		}
		isHex = tempChar == 'X';
	}
	if (isHex) {
		currentSeekPosition = currentSeekPosition + 1;
		currentSeekPosition = currentSeekPosition + 1;
		isSuccess = ReadHexValueFromFile(outVal);
	}
	else {
		tempChar = currentChar;
		if (('`' < currentChar) && (currentChar < '{')) {
			tempChar = currentChar + -0x20;
		}
		if (tempChar == '-') {
			multiplier = -1;
			currentSeekPosition = currentSeekPosition + 1;
		}
		else {
			if (('`' < currentChar) && (currentChar < '{')) {
				currentChar = currentChar + -0x20;
			}
			if (currentChar == '+') {
				multiplier = 1;
				currentSeekPosition = currentSeekPosition + 1;
			}
		}
		streamStart = currentSeekPosition;
		tempIndex = 0;
		while (currentPosition = currentSeekPosition, currentPosition != endSeekPosition) {
			currentChar = *currentPosition;
			tempChar = currentChar;
			if (('`' < currentChar) && (currentChar < '{')) {
				tempChar = currentChar + -0x20;
			}
			isHex = tempChar == '\r';
			if (isHex) {
				tempChar = currentPosition[1];
				if (('`' < tempChar) && (tempChar < '{')) {
					tempChar = tempChar + -0x20;
				}
				isHex = tempChar == '\n';
			}
			if (isHex) break;
			tempChar = currentChar;
			if (('`' < currentChar) && (currentChar < '{')) {
				tempChar = currentChar + -0x20;
			}
			isHex = '/' < tempChar;
			if (isHex) {
				if (('`' < currentChar) && (currentChar < '{')) {
					currentChar = currentChar + -0x20;
				}
				isHex = currentChar < ':';
			}
			if ((!isHex) || (10 < (int)tempIndex)) break;
			tempIndex = tempIndex + 1;
			currentSeekPosition = currentSeekPosition + 1;
		}
		if (((int)tempIndex < 1) || (10 < (int)tempIndex)) {
			isSuccess = false;
		}
		else {
			parsedValue = edStrStr2Int(streamStart, tempIndex & 0xff);
			*outVal = parsedValue;
			isSuccess = true;
			*outVal = *outVal * multiplier;
		}
	}
	return isSuccess;
}

IniFile::IniFile()
{
	name[0] = '\0';
	currentSeekPosition = NULL;
	endSeekPosition = NULL;
	fileBufferStart = NULL;

	SetupEd10_00217720(this, 0x0, &g_IniFile_00450750_mem);
}

IniFile::~IniFile()
{
	if (fileBufferStart != NULL)
	{
		free(fileBufferStart);
	}
}

void IniFile::Load_001ab200(const char* inName, char* inFileBufferStart, int fileSize)
{
	strcpy(name, inName);
	fileBufferStart = inFileBufferStart;
	fileSize = fileSize;
	currentSeekPosition = fileBufferStart;
	endSeekPosition = currentSeekPosition + fileSize;
	return;
}

bool IniFile::ReadQuotedString(char* outValue)
{
	char currentCharacter;
	char tempCharacter;
	char* currentPosition;
	bool returnValue;
	int stringIndex;

	currentCharacter = *currentSeekPosition;
	if (('`' < currentCharacter) && (currentCharacter < '{')) {
		currentCharacter = currentCharacter + -0x20;
	}
	if (currentCharacter == '\"') {
		stringIndex = 0;
		currentSeekPosition = currentSeekPosition + 1;
		while (currentPosition = currentSeekPosition, currentPosition != endSeekPosition) {
			currentCharacter = *currentPosition;
			tempCharacter = currentCharacter;
			if (('`' < currentCharacter) && (currentCharacter < '{')) {
				tempCharacter = currentCharacter + -0x20;
			}
			returnValue = tempCharacter == '\r';
			if (returnValue) {
				tempCharacter = currentPosition[1];
				if (('`' < tempCharacter) && (tempCharacter < '{')) {
					tempCharacter = tempCharacter + -0x20;
				}
				returnValue = tempCharacter == '\n';
			}
			if (returnValue) break;
			tempCharacter = currentCharacter;
			if (('`' < currentCharacter) && (currentCharacter < '{')) {
				tempCharacter = currentCharacter + -0x20;
			}
			if ((tempCharacter == '\"') || (0x7e < stringIndex)) break;
			if (('`' < currentCharacter) && (currentCharacter < '{')) {
				currentCharacter = currentCharacter + -0x20;
			}
			outValue[stringIndex] = currentCharacter;
			stringIndex = stringIndex + 1;
			currentSeekPosition = currentSeekPosition + 1;
		}
		outValue[stringIndex] = '\0';
		if (stringIndex == 0) {
			returnValue = false;
		}
		else {
			if (0x7e < stringIndex) {
				/* String will be truncated !\n */
				edDebugPrintf("String will be truncated !\n");
			}
			returnValue = true;
		}
	}
	else {
		returnValue = false;
	}
	return returnValue;
}



bool IniFile::SeekToKey_001a7ed0(const char* key, bool bHasSection)
{
	char cVar1;
	char* pcVar2;
	bool bVar3;
	bool bVar4;
	char cVar5;
	const char* pcVar6;

	do {
		if (currentSeekPosition == endSeekPosition) {
			return false;
		}
		while (true) {
			pcVar6 = currentSeekPosition;
			bVar3 = pcVar6 == endSeekPosition;
			if (bVar3) break;
			cVar5 = *pcVar6;
			cVar1 = cVar5;
			if (('`' < cVar5) && (cVar5 < '{')) {
				cVar1 = cVar5 + -0x20;
			}
			bVar4 = cVar1 == '\r';
			if (bVar4) {
				cVar1 = pcVar6[1];
				if (('`' < cVar1) && (cVar1 < '{')) {
					cVar1 = cVar1 + -0x20;
				}
				bVar4 = cVar1 == '\n';
			}
			if (bVar4) break;
			cVar1 = cVar5;
			if (('`' < cVar5) && (cVar5 < '{')) {
				cVar1 = cVar5 + -0x20;
			}
			bVar4 = cVar1 == ' ';
			if (!bVar4) {
				if (('`' < cVar5) && (cVar5 < '{')) {
					cVar5 = cVar5 + -0x20;
				}
				bVar4 = cVar5 == '\t';
			}
			if (!bVar4) break;
			currentSeekPosition = currentSeekPosition + 1;
		}
		cVar5 = *pcVar6;
		cVar1 = cVar5;
		if (('`' < cVar5) && (cVar5 < '{')) {
			cVar1 = cVar5 + -0x20;
		}
		if (cVar1 == '[') {
			if (bHasSection == false) {
				return false;
			}
		}
		else {
			bVar3 = !bVar3;
			if (bVar3) {
				cVar1 = cVar5;
				if (('`' < cVar5) && (cVar5 < '{')) {
					cVar1 = cVar5 + -0x20;
				}
				bVar3 = cVar1 == '\r';
				if (bVar3) {
					cVar1 = pcVar6[1];
					if (('`' < cVar1) && (cVar1 < '{')) {
						cVar1 = cVar1 + -0x20;
					}
					bVar3 = cVar1 == '\n';
				}
				bVar3 = !bVar3;
			}
			if (bVar3) {
				if (('`' < cVar5) && (cVar5 < '{')) {
					cVar5 = cVar5 + -0x20;
				}
				bVar3 = cVar5 == '/';
				if (bVar3) {
					cVar5 = pcVar6[1];
					if (('`' < cVar5) && (cVar5 < '{')) {
						cVar5 = cVar5 + -0x20;
					}
					bVar3 = cVar5 == '/';
				}
				bVar3 = !bVar3;
			}
			pcVar6 = key;
			if (bVar3) {
				while (pcVar2 = currentSeekPosition, pcVar2 != endSeekPosition) {
					cVar5 = *pcVar2;
					if (('`' < cVar5) && (cVar5 < '{')) {
						cVar5 = cVar5 + -0x20;
					}
					bVar3 = cVar5 == '\r';
					if (bVar3) {
						cVar5 = pcVar2[1];
						if (('`' < cVar5) && (cVar5 < '{')) {
							cVar5 = cVar5 + -0x20;
						}
						bVar3 = cVar5 == '\n';
					}
					if ((bVar3) || (cVar5 = *pcVar6, cVar5 == '\0')) break;
					cVar1 = *pcVar2;
					if (('`' < cVar1) && (cVar1 < '{')) {
						cVar1 = cVar1 + -0x20;
					}
					if (('`' < cVar5) && (cVar5 < '{')) {
						cVar5 = cVar5 + -0x20;
					}
					if (cVar1 != cVar5) break;
					currentSeekPosition = currentSeekPosition + 1;
					pcVar6 = pcVar6 + 1;
				}
				if (*pcVar6 == '\0') {
					while (true) {
						pcVar6 = currentSeekPosition;
						bVar3 = pcVar6 == endSeekPosition;
						if (bVar3) break;
						cVar5 = *pcVar6;
						cVar1 = cVar5;
						if (('`' < cVar5) && (cVar5 < '{')) {
							cVar1 = cVar5 + -0x20;
						}
						bVar4 = cVar1 == '\r';
						if (bVar4) {
							cVar1 = pcVar6[1];
							if (('`' < cVar1) && (cVar1 < '{')) {
								cVar1 = cVar1 + -0x20;
							}
							bVar4 = cVar1 == '\n';
						}
						if (bVar4) break;
						cVar1 = cVar5;
						if (('`' < cVar5) && (cVar5 < '{')) {
							cVar1 = cVar5 + -0x20;
						}
						bVar4 = cVar1 == ' ';
						if (!bVar4) {
							if (('`' < cVar5) && (cVar5 < '{')) {
								cVar5 = cVar5 + -0x20;
							}
							bVar4 = cVar5 == '\t';
						}
						if (!bVar4) break;
						currentSeekPosition = currentSeekPosition + 1;
					}
					bVar3 = !bVar3;
					if (bVar3) {
						cVar5 = *pcVar6;
						if (('`' < cVar5) && (cVar5 < '{')) {
							cVar5 = cVar5 + -0x20;
						}
						bVar3 = cVar5 == '\r';
						if (bVar3) {
							cVar5 = pcVar6[1];
							if (('`' < cVar5) && (cVar5 < '{')) {
								cVar5 = cVar5 + -0x20;
							}
							bVar3 = cVar5 == '\n';
						}
						bVar3 = !bVar3;
					}
					if (bVar3) {
						cVar5 = *pcVar6;
						if (('`' < cVar5) && (cVar5 < '{')) {
							cVar5 = cVar5 + -0x20;
						}
						bVar3 = cVar5 == '/';
						if (bVar3) {
							cVar5 = pcVar6[1];
							if (('`' < cVar5) && (cVar5 < '{')) {
								cVar5 = cVar5 + -0x20;
							}
							bVar3 = cVar5 == '/';
						}
						bVar3 = !bVar3;
					}
					if (bVar3) {
						cVar5 = *pcVar6;
						if (('`' < cVar5) && (cVar5 < '{')) {
							cVar5 = cVar5 + -0x20;
						}
						if (cVar5 == '=') {
							currentSeekPosition = currentSeekPosition + 1;
							while (true) {
								pcVar6 = currentSeekPosition;
								bVar3 = pcVar6 == endSeekPosition;
								if (bVar3) break;
								cVar5 = *pcVar6;
								cVar1 = cVar5;
								if (('`' < cVar5) && (cVar5 < '{')) {
									cVar1 = cVar5 + -0x20;
								}
								bVar4 = cVar1 == '\r';
								if (bVar4) {
									cVar1 = pcVar6[1];
									if (('`' < cVar1) && (cVar1 < '{')) {
										cVar1 = cVar1 + -0x20;
									}
									bVar4 = cVar1 == '\n';
								}
								if (bVar4) break;
								cVar1 = cVar5;
								if (('`' < cVar5) && (cVar5 < '{')) {
									cVar1 = cVar5 + -0x20;
								}
								bVar4 = cVar1 == ' ';
								if (!bVar4) {
									if (('`' < cVar5) && (cVar5 < '{')) {
										cVar5 = cVar5 + -0x20;
									}
									bVar4 = cVar5 == '\t';
								}
								if (!bVar4) break;
								currentSeekPosition = currentSeekPosition + 1;
							}
							bVar3 = !bVar3;
							if (bVar3) {
								cVar5 = *pcVar6;
								if (('`' < cVar5) && (cVar5 < '{')) {
									cVar5 = cVar5 + -0x20;
								}
								bVar3 = cVar5 == '\r';
								if (bVar3) {
									cVar5 = pcVar6[1];
									if (('`' < cVar5) && (cVar5 < '{')) {
										cVar5 = cVar5 + -0x20;
									}
									bVar3 = cVar5 == '\n';
								}
								bVar3 = !bVar3;
							}
							if (bVar3) {
								cVar5 = *pcVar6;
								if (('`' < cVar5) && (cVar5 < '{')) {
									cVar5 = cVar5 + -0x20;
								}
								bVar3 = cVar5 == '/';
								if (bVar3) {
									cVar5 = pcVar6[1];
									if (('`' < cVar5) && (cVar5 < '{')) {
										cVar5 = cVar5 + -0x20;
									}
									bVar3 = cVar5 == '/';
								}
								bVar3 = !bVar3;
							}
							if (bVar3) {
								return true;
							}
						}
					}
				}
			}
		}
		while (true) {
			pcVar6 = currentSeekPosition;
			bVar3 = pcVar6 == endSeekPosition;
			if (bVar3) break;
			cVar5 = *pcVar6;
			if (('`' < cVar5) && (cVar5 < '{')) {
				cVar5 = cVar5 + -0x20;
			}
			bVar4 = cVar5 == '\r';
			if (bVar4) {
				cVar5 = pcVar6[1];
				if (('`' < cVar5) && (cVar5 < '{')) {
					cVar5 = cVar5 + -0x20;
				}
				bVar4 = cVar5 == '\n';
			}
			if (bVar4) break;
			currentSeekPosition = currentSeekPosition + 1;
		}
		if (!bVar3) {
			currentSeekPosition = currentSeekPosition + 1;
			currentSeekPosition = currentSeekPosition + 1;
		}
	} while (true);
}


bool IniFile::ReadInt_001a9830(char* section, char* key, int* outValue)
{
	char currentChar;
	char compareChar;
	char* foundPosition;
	bool isWhiteSpace;
	bool isEndOfLine;
	char* currentPointer;

	if (fileBufferStart == (char*)0x0) {
		edDebugPrintf(".INI file empty !\n");
		return false;
	}
	currentSeekPosition = fileBufferStart;
	if (section != (char*)0x0) {
		while (isEndOfLine = false, currentSeekPosition != endSeekPosition) {
			while (true) {
				currentPointer = currentSeekPosition;
				isEndOfLine = currentPointer == endSeekPosition;
				if (isEndOfLine) break;
				compareChar = *currentPointer;
				currentChar = compareChar;
				if (('`' < compareChar) && (compareChar < '{')) {
					currentChar = compareChar + -0x20;
				}
				isWhiteSpace = currentChar == '\r';
				if (isWhiteSpace) {
					currentChar = currentPointer[1];
					if (('`' < currentChar) && (currentChar < '{')) {
						currentChar = currentChar + -0x20;
					}
					isWhiteSpace = currentChar == '\n';
				}
				if (isWhiteSpace) break;
				currentChar = compareChar;
				if (('`' < compareChar) && (compareChar < '{')) {
					currentChar = compareChar + -0x20;
				}
				isWhiteSpace = currentChar == ' ';
				if (!isWhiteSpace) {
					if (('`' < compareChar) && (compareChar < '{')) {
						compareChar = compareChar + -0x20;
					}
					isWhiteSpace = compareChar == '\t';
				}
				if (!isWhiteSpace) break;
				currentSeekPosition = currentSeekPosition + 1;
			}
			isEndOfLine = !isEndOfLine;
			if (isEndOfLine) {
				compareChar = *currentPointer;
				if (('`' < compareChar) && (compareChar < '{')) {
					compareChar = compareChar + -0x20;
				}
				isEndOfLine = compareChar == '\r';
				if (isEndOfLine) {
					compareChar = currentPointer[1];
					if (('`' < compareChar) && (compareChar < '{')) {
						compareChar = compareChar + -0x20;
					}
					isEndOfLine = compareChar == '\n';
				}
				isEndOfLine = !isEndOfLine;
			}
			if (isEndOfLine) {
				compareChar = *currentPointer;
				if (('`' < compareChar) && (compareChar < '{')) {
					compareChar = compareChar + -0x20;
				}
				isEndOfLine = compareChar == '/';
				if (isEndOfLine) {
					compareChar = currentPointer[1];
					if (('`' < compareChar) && (compareChar < '{')) {
						compareChar = compareChar + -0x20;
					}
					isEndOfLine = compareChar == '/';
				}
				isEndOfLine = !isEndOfLine;
			}
			if (isEndOfLine) {
				compareChar = *currentPointer;
				if (('`' < compareChar) && (compareChar < '{')) {
					compareChar = compareChar + -0x20;
				}
				if (compareChar == '[') {
					currentSeekPosition = currentSeekPosition + 1;
					while (true) {
						currentPointer = currentSeekPosition;
						isEndOfLine = currentPointer == endSeekPosition;
						if (isEndOfLine) break;
						compareChar = *currentPointer;
						currentChar = compareChar;
						if (('`' < compareChar) && (compareChar < '{')) {
							currentChar = compareChar + -0x20;
						}
						isWhiteSpace = currentChar == '\r';
						if (isWhiteSpace) {
							currentChar = currentPointer[1];
							if (('`' < currentChar) && (currentChar < '{')) {
								currentChar = currentChar + -0x20;
							}
							isWhiteSpace = currentChar == '\n';
						}
						if (isWhiteSpace) break;
						currentChar = compareChar;
						if (('`' < compareChar) && (compareChar < '{')) {
							currentChar = compareChar + -0x20;
						}
						isWhiteSpace = currentChar == ' ';
						if (!isWhiteSpace) {
							if (('`' < compareChar) && (compareChar < '{')) {
								compareChar = compareChar + -0x20;
							}
							isWhiteSpace = compareChar == '\t';
						}
						if (!isWhiteSpace) break;
						currentSeekPosition = currentSeekPosition + 1;
					}
					isEndOfLine = !isEndOfLine;
					if (isEndOfLine) {
						compareChar = *currentPointer;
						if (('`' < compareChar) && (compareChar < '{')) {
							compareChar = compareChar + -0x20;
						}
						isEndOfLine = compareChar == '\r';
						if (isEndOfLine) {
							compareChar = currentPointer[1];
							if (('`' < compareChar) && (compareChar < '{')) {
								compareChar = compareChar + -0x20;
							}
							isEndOfLine = compareChar == '\n';
						}
						isEndOfLine = !isEndOfLine;
					}
					if (isEndOfLine) {
						compareChar = *currentPointer;
						if (('`' < compareChar) && (compareChar < '{')) {
							compareChar = compareChar + -0x20;
						}
						isEndOfLine = compareChar == '/';
						if (isEndOfLine) {
							compareChar = currentPointer[1];
							if (('`' < compareChar) && (compareChar < '{')) {
								compareChar = compareChar + -0x20;
							}
							isEndOfLine = compareChar == '/';
						}
						isEndOfLine = !isEndOfLine;
					}
					currentPointer = section;
					if (isEndOfLine) {
						while (foundPosition = currentSeekPosition, foundPosition != endSeekPosition) {
							compareChar = *foundPosition;
							if (('`' < compareChar) && (compareChar < '{')) {
								compareChar = compareChar + -0x20;
							}
							isEndOfLine = compareChar == '\r';
							if (isEndOfLine) {
								compareChar = foundPosition[1];
								if (('`' < compareChar) && (compareChar < '{')) {
									compareChar = compareChar + -0x20;
								}
								isEndOfLine = compareChar == '\n';
							}
							if ((isEndOfLine) || (compareChar = *currentPointer, compareChar == '\0')) break;
							currentChar = *foundPosition;
							if (('`' < currentChar) && (currentChar < '{')) {
								currentChar = currentChar + -0x20;
							}
							if (('`' < compareChar) && (compareChar < '{')) {
								compareChar = compareChar + -0x20;
							}
							if (currentChar != compareChar) break;
							currentSeekPosition = currentSeekPosition + 1;
							currentPointer = currentPointer + 1;
						}
						if (*currentPointer == '\0') {
							while (true) {
								currentPointer = currentSeekPosition;
								isEndOfLine = currentPointer == endSeekPosition;
								if (isEndOfLine) break;
								compareChar = *currentPointer;
								currentChar = compareChar;
								if (('`' < compareChar) && (compareChar < '{')) {
									currentChar = compareChar + -0x20;
								}
								isWhiteSpace = currentChar == '\r';
								if (isWhiteSpace) {
									currentChar = currentPointer[1];
									if (('`' < currentChar) && (currentChar < '{')) {
										currentChar = currentChar + -0x20;
									}
									isWhiteSpace = currentChar == '\n';
								}
								if (isWhiteSpace) break;
								currentChar = compareChar;
								if (('`' < compareChar) && (compareChar < '{')) {
									currentChar = compareChar + -0x20;
								}
								isWhiteSpace = currentChar == ' ';
								if (!isWhiteSpace) {
									if (('`' < compareChar) && (compareChar < '{')) {
										compareChar = compareChar + -0x20;
									}
									isWhiteSpace = compareChar == '\t';
								}
								if (!isWhiteSpace) break;
								currentSeekPosition = currentSeekPosition + 1;
							}
							isEndOfLine = !isEndOfLine;
							if (isEndOfLine) {
								compareChar = *currentPointer;
								if (('`' < compareChar) && (compareChar < '{')) {
									compareChar = compareChar + -0x20;
								}
								isEndOfLine = compareChar == '\r';
								if (isEndOfLine) {
									compareChar = currentPointer[1];
									if (('`' < compareChar) && (compareChar < '{')) {
										compareChar = compareChar + -0x20;
									}
									isEndOfLine = compareChar == '\n';
								}
								isEndOfLine = !isEndOfLine;
							}
							if (isEndOfLine) {
								compareChar = *currentPointer;
								if (('`' < compareChar) && (compareChar < '{')) {
									compareChar = compareChar + -0x20;
								}
								isEndOfLine = compareChar == '/';
								if (isEndOfLine) {
									compareChar = currentPointer[1];
									if (('`' < compareChar) && (compareChar < '{')) {
										compareChar = compareChar + -0x20;
									}
									isEndOfLine = compareChar == '/';
								}
								isEndOfLine = !isEndOfLine;
							}
							if (isEndOfLine) {
								compareChar = *currentPointer;
								if (('`' < compareChar) && (compareChar < '{')) {
									compareChar = compareChar + -0x20;
								}
								if (compareChar == ']') {
									isEndOfLine = true;
									break;
								}
							}
						}
					}
				}
			}
			while (true) {
				currentPointer = currentSeekPosition;
				isEndOfLine = currentPointer == endSeekPosition;
				if (isEndOfLine) break;
				compareChar = *currentPointer;
				if (('`' < compareChar) && (compareChar < '{')) {
					compareChar = compareChar + -0x20;
				}
				isWhiteSpace = compareChar == '\r';
				if (isWhiteSpace) {
					compareChar = currentPointer[1];
					if (('`' < compareChar) && (compareChar < '{')) {
						compareChar = compareChar + -0x20;
					}
					isWhiteSpace = compareChar == '\n';
				}
				if (isWhiteSpace) break;
				currentSeekPosition = currentSeekPosition + 1;
			}
			if (!isEndOfLine) {
				currentSeekPosition = currentSeekPosition + 1;
				currentSeekPosition = currentSeekPosition + 1;
			}
		}
		if (!isEndOfLine) {
			edDebugPrintf("Section [%s] not found !\n", section);
			isEndOfLine = false;
			goto LAB_001aa4b8;
		}
		while (true) {
			currentPointer = currentSeekPosition;
			isEndOfLine = currentPointer == endSeekPosition;
			if (isEndOfLine) break;
			compareChar = *currentPointer;
			if (('`' < compareChar) && (compareChar < '{')) {
				compareChar = compareChar + -0x20;
			}
			isWhiteSpace = compareChar == '\r';
			if (isWhiteSpace) {
				compareChar = currentPointer[1];
				if (('`' < compareChar) && (compareChar < '{')) {
					compareChar = compareChar + -0x20;
				}
				isWhiteSpace = compareChar == '\n';
			}
			if (isWhiteSpace) break;
			currentSeekPosition = currentSeekPosition + 1;
		}
		if (!isEndOfLine) {
			currentSeekPosition = currentSeekPosition + 1;
			currentSeekPosition = currentSeekPosition + 1;
		}
	}
	isEndOfLine = SeekToKey_001a7ed0(key, section == (char*)0x0);
	if (isEndOfLine == false) {
		edDebugPrintf("Key %s = not found in section [%s]!\n", key, section);
		isEndOfLine = false;
	}
	else {
		isEndOfLine = true;
	}
LAB_001aa4b8:
	if (isEndOfLine) {
		isEndOfLine = ReadInteger(outValue);
		if (isEndOfLine == false) {
			edDebugPrintf("Invalid integer key %s in section [%s]!\n", key, section);
			isEndOfLine = false;
		}
		else {
			isEndOfLine = true;
		}
	}
	else {
		isEndOfLine = false;
	}
	return isEndOfLine;
}



bool IniFile::ReadString_001aa520(const char* section, const char* key, char* outValue)
{
	char cVar1;
	char cVar2;
	char* pcVar3;
	bool bVar4;
	bool bAtEndOfFile;
	const char* pcVar6;

	currentSeekPosition = fileBufferStart;
	if (section != (char*)0x0) {
		while (bAtEndOfFile = false, currentSeekPosition != endSeekPosition) {
			while (true) {
				pcVar6 = currentSeekPosition;
				bAtEndOfFile = pcVar6 == endSeekPosition;
				if (bAtEndOfFile) break;
				cVar2 = *pcVar6;
				cVar1 = cVar2;
				if (('`' < cVar2) && (cVar2 < '{')) {
					cVar1 = cVar2 + -0x20;
				}
				bVar4 = cVar1 == '\r';
				if (bVar4) {
					cVar1 = pcVar6[1];
					if (('`' < cVar1) && (cVar1 < '{')) {
						cVar1 = cVar1 + -0x20;
					}
					bVar4 = cVar1 == '\n';
				}
				if (bVar4) break;
				cVar1 = cVar2;
				if (('`' < cVar2) && (cVar2 < '{')) {
					cVar1 = cVar2 + -0x20;
				}
				bVar4 = cVar1 == ' ';
				if (!bVar4) {
					if (('`' < cVar2) && (cVar2 < '{')) {
						cVar2 = cVar2 + -0x20;
					}
					bVar4 = cVar2 == '\t';
				}
				if (!bVar4) break;
				currentSeekPosition = currentSeekPosition + 1;
			}
			bAtEndOfFile = !bAtEndOfFile;
			if (bAtEndOfFile) {
				cVar2 = *pcVar6;
				if (('`' < cVar2) && (cVar2 < '{')) {
					cVar2 = cVar2 + -0x20;
				}
				bAtEndOfFile = cVar2 == '\r';
				if (bAtEndOfFile) {
					cVar2 = pcVar6[1];
					if (('`' < cVar2) && (cVar2 < '{')) {
						cVar2 = cVar2 + -0x20;
					}
					bAtEndOfFile = cVar2 == '\n';
				}
				bAtEndOfFile = !bAtEndOfFile;
			}
			if (bAtEndOfFile) {
				cVar2 = *pcVar6;
				if (('`' < cVar2) && (cVar2 < '{')) {
					cVar2 = cVar2 + -0x20;
				}
				bAtEndOfFile = cVar2 == '/';
				if (bAtEndOfFile) {
					cVar2 = pcVar6[1];
					if (('`' < cVar2) && (cVar2 < '{')) {
						cVar2 = cVar2 + -0x20;
					}
					bAtEndOfFile = cVar2 == '/';
				}
				bAtEndOfFile = !bAtEndOfFile;
			}
			if (bAtEndOfFile) {
				cVar2 = *pcVar6;
				if (('`' < cVar2) && (cVar2 < '{')) {
					cVar2 = cVar2 + -0x20;
				}
				if (cVar2 == '[') {
					currentSeekPosition = currentSeekPosition + 1;
					while (true) {
						pcVar6 = currentSeekPosition;
						bAtEndOfFile = pcVar6 == endSeekPosition;
						if (bAtEndOfFile) break;
						cVar2 = *pcVar6;
						cVar1 = cVar2;
						if (('`' < cVar2) && (cVar2 < '{')) {
							cVar1 = cVar2 + -0x20;
						}
						bVar4 = cVar1 == '\r';
						if (bVar4) {
							cVar1 = pcVar6[1];
							if (('`' < cVar1) && (cVar1 < '{')) {
								cVar1 = cVar1 + -0x20;
							}
							bVar4 = cVar1 == '\n';
						}
						if (bVar4) break;
						cVar1 = cVar2;
						if (('`' < cVar2) && (cVar2 < '{')) {
							cVar1 = cVar2 + -0x20;
						}
						bVar4 = cVar1 == ' ';
						if (!bVar4) {
							if (('`' < cVar2) && (cVar2 < '{')) {
								cVar2 = cVar2 + -0x20;
							}
							bVar4 = cVar2 == '\t';
						}
						if (!bVar4) break;
						currentSeekPosition = currentSeekPosition + 1;
					}
					bAtEndOfFile = !bAtEndOfFile;
					if (bAtEndOfFile) {
						cVar2 = *pcVar6;
						if (('`' < cVar2) && (cVar2 < '{')) {
							cVar2 = cVar2 + -0x20;
						}
						bAtEndOfFile = cVar2 == '\r';
						if (bAtEndOfFile) {
							cVar2 = pcVar6[1];
							if (('`' < cVar2) && (cVar2 < '{')) {
								cVar2 = cVar2 + -0x20;
							}
							bAtEndOfFile = cVar2 == '\n';
						}
						bAtEndOfFile = !bAtEndOfFile;
					}
					if (bAtEndOfFile) {
						cVar2 = *pcVar6;
						if (('`' < cVar2) && (cVar2 < '{')) {
							cVar2 = cVar2 + -0x20;
						}
						bAtEndOfFile = cVar2 == '/';
						if (bAtEndOfFile) {
							cVar2 = pcVar6[1];
							if (('`' < cVar2) && (cVar2 < '{')) {
								cVar2 = cVar2 + -0x20;
							}
							bAtEndOfFile = cVar2 == '/';
						}
						bAtEndOfFile = !bAtEndOfFile;
					}
					pcVar6 = section;
					if (bAtEndOfFile) {
						while (pcVar3 = currentSeekPosition, pcVar3 != endSeekPosition) {
							cVar2 = *pcVar3;
							if (('`' < cVar2) && (cVar2 < '{')) {
								cVar2 = cVar2 + -0x20;
							}
							bAtEndOfFile = cVar2 == '\r';
							if (bAtEndOfFile) {
								cVar2 = pcVar3[1];
								if (('`' < cVar2) && (cVar2 < '{')) {
									cVar2 = cVar2 + -0x20;
								}
								bAtEndOfFile = cVar2 == '\n';
							}
							if ((bAtEndOfFile) || (cVar2 = *pcVar6, cVar2 == '\0')) break;
							cVar1 = *pcVar3;
							if (('`' < cVar1) && (cVar1 < '{')) {
								cVar1 = cVar1 + -0x20;
							}
							if (('`' < cVar2) && (cVar2 < '{')) {
								cVar2 = cVar2 + -0x20;
							}
							if (cVar1 != cVar2) break;
							currentSeekPosition = currentSeekPosition + 1;
							pcVar6 = pcVar6 + 1;
						}
						if (*pcVar6 == '\0') {
							while (true) {
								pcVar6 = currentSeekPosition;
								bAtEndOfFile = pcVar6 == endSeekPosition;
								if (bAtEndOfFile) break;
								cVar2 = *pcVar6;
								cVar1 = cVar2;
								if (('`' < cVar2) && (cVar2 < '{')) {
									cVar1 = cVar2 + -0x20;
								}
								bVar4 = cVar1 == '\r';
								if (bVar4) {
									cVar1 = pcVar6[1];
									if (('`' < cVar1) && (cVar1 < '{')) {
										cVar1 = cVar1 + -0x20;
									}
									bVar4 = cVar1 == '\n';
								}
								if (bVar4) break;
								cVar1 = cVar2;
								if (('`' < cVar2) && (cVar2 < '{')) {
									cVar1 = cVar2 + -0x20;
								}
								bVar4 = cVar1 == ' ';
								if (!bVar4) {
									if (('`' < cVar2) && (cVar2 < '{')) {
										cVar2 = cVar2 + -0x20;
									}
									bVar4 = cVar2 == '\t';
								}
								if (!bVar4) break;
								currentSeekPosition = currentSeekPosition + 1;
							}
							bAtEndOfFile = !bAtEndOfFile;
							if (bAtEndOfFile) {
								cVar2 = *pcVar6;
								if (('`' < cVar2) && (cVar2 < '{')) {
									cVar2 = cVar2 + -0x20;
								}
								bAtEndOfFile = cVar2 == '\r';
								if (bAtEndOfFile) {
									cVar2 = pcVar6[1];
									if (('`' < cVar2) && (cVar2 < '{')) {
										cVar2 = cVar2 + -0x20;
									}
									bAtEndOfFile = cVar2 == '\n';
								}
								bAtEndOfFile = !bAtEndOfFile;
							}
							if (bAtEndOfFile) {
								cVar2 = *pcVar6;
								if (('`' < cVar2) && (cVar2 < '{')) {
									cVar2 = cVar2 + -0x20;
								}
								bAtEndOfFile = cVar2 == '/';
								if (bAtEndOfFile) {
									cVar2 = pcVar6[1];
									if (('`' < cVar2) && (cVar2 < '{')) {
										cVar2 = cVar2 + -0x20;
									}
									bAtEndOfFile = cVar2 == '/';
								}
								bAtEndOfFile = !bAtEndOfFile;
							}
							if (bAtEndOfFile) {
								cVar2 = *pcVar6;
								if (('`' < cVar2) && (cVar2 < '{')) {
									cVar2 = cVar2 + -0x20;
								}
								if (cVar2 == ']') {
									bAtEndOfFile = true;
									break;
								}
							}
						}
					}
				}
			}
			while (true) {
				pcVar6 = currentSeekPosition;
				bAtEndOfFile = pcVar6 == endSeekPosition;
				if (bAtEndOfFile) break;
				cVar2 = *pcVar6;
				if (('`' < cVar2) && (cVar2 < '{')) {
					cVar2 = cVar2 + -0x20;
				}
				bVar4 = cVar2 == '\r';
				if (bVar4) {
					cVar2 = pcVar6[1];
					if (('`' < cVar2) && (cVar2 < '{')) {
						cVar2 = cVar2 + -0x20;
					}
					bVar4 = cVar2 == '\n';
				}
				if (bVar4) break;
				currentSeekPosition = currentSeekPosition + 1;
			}
			if (!bAtEndOfFile) {
				currentSeekPosition = currentSeekPosition + 1;
				currentSeekPosition = currentSeekPosition + 1;
			}
		}
		if (!bAtEndOfFile) {
			edDebugPrintf("Section [%s] not found !\n", section);
			bAtEndOfFile = false;
			goto LAB_001ab190;
		}
		while (true) {
			pcVar6 = currentSeekPosition;
			bAtEndOfFile = pcVar6 == endSeekPosition;
			if (bAtEndOfFile) break;
			cVar2 = *pcVar6;
			if (('`' < cVar2) && (cVar2 < '{')) {
				cVar2 = cVar2 + -0x20;
			}
			bVar4 = cVar2 == '\r';
			if (bVar4) {
				cVar2 = pcVar6[1];
				if (('`' < cVar2) && (cVar2 < '{')) {
					cVar2 = cVar2 + -0x20;
				}
				bVar4 = cVar2 == '\n';
			}
			if (bVar4) break;
			currentSeekPosition = currentSeekPosition + 1;
		}
		if (!bAtEndOfFile) {
			currentSeekPosition = currentSeekPosition + 1;
			currentSeekPosition = currentSeekPosition + 1;
		}
	}
	bAtEndOfFile = SeekToKey_001a7ed0(key, section == (char*)0x0);
	if (bAtEndOfFile == false) {
		/* Key %s = not found in section [%s]!\n */
		edDebugPrintf("Key %s = not found in section [%s]!\n", key, section);
		bAtEndOfFile = false;
	}
	else {
		bAtEndOfFile = true;
	}
LAB_001ab190:
	if (bAtEndOfFile) {
		bAtEndOfFile = ReadQuotedString(outValue);
		if (bAtEndOfFile == false) {
			/* Invalid string key %s in section [%s]!\n */
			edDebugPrintf("Invalid string key %s in section [%s]!\n", key, section);
			bAtEndOfFile = false;
		}
		else {
			bAtEndOfFile = true;
		}
	}
	else {
		bAtEndOfFile = false;
	}
	return bAtEndOfFile;
}

