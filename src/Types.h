#ifndef _TYPES_H
#define _TYPES_H

#ifdef PLATFORM_PS2
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef PLATFORM_WINDOWS
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif


typedef unsigned char   undefined;

typedef unsigned char    byte;
typedef unsigned int    dword;
typedef long double    longdouble;
typedef unsigned char    uchar;
typedef unsigned char    u_char;
typedef unsigned int    uint;
typedef unsigned int    uint3;
typedef unsigned long    ulong;
typedef unsigned char    undefined1;
typedef unsigned short    undefined2;
typedef unsigned int    undefined3;
typedef unsigned int    undefined4;
typedef unsigned long    undefined5;
typedef unsigned long    undefined8;
typedef unsigned short    ushort;
typedef unsigned short    word;
typedef unsigned long long    ulonglong;

#ifdef PLATFORM_WIN
typedef unsigned int    u_int;
#endif

struct EdFileGlobal_10 {
	EdFileGlobal_10* pPrevFileHeader;
	void* pFreeFunc;
	void* pFileData;
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
};

enum EFileLoadMode {
	FLM_CD_DVD = 2,
	FLM_Net = 4,
	New_Name = 0,
	New_Name_1 = 1,
	New_Name_3 = 3
};

struct IopPaths {
	char* pIopRoot;
	char* pImgName;
	int modulesCount;
	char* pModulesName;
};

struct InputSetupParams {

	int argc;
	char** argv;
	int headerCount;
	int handlersSize;
	undefined field_0x10;
	undefined field_0x11;
	undefined field_0x12;
	undefined field_0x13;
	int field_0x14;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	EFileLoadMode fileLoadMode;
	char* field_0x20;
	IopPaths* pIopPaths;
};

void SetupEd10_00217720(void* pObj, void* pFreeFunc, EdFileGlobal_10* pHeader);
EFileLoadMode GetFileLoadMode_00424d9c(void);
InputSetupParams* GetInputSetupParams(void);

#if defined(PLATFORM_WIN)
#define scePrintf printf
#endif

#define PrintString scePrintf

#define MY_LOG scePrintf

inline int edStringCpyL(char* outString, char* inString)
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

inline char* SearchString(char* inString, char searchChar)
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

#ifdef PLATFORM_PS2
struct __attribute__((__packed__))
#else
#pragma pack(push,1)
struct alignas(16)
#endif 
Vector { /* Aligned */
	float x;
	float y;
	float z;
	float w;
};
#ifdef PLATFORM_WINDOWS
#pragma pack(pop)
#endif

#ifdef PLATFORM_PS2
struct __attribute__((__packed__))
#else
#pragma pack(push,1)
struct alignas(16)
#endif 
Matrix {
	float aa;
	float ab;
	float ac;
	float ad;
	float ba;
	float bb;
	float bc;
	float bd;
	float ca;
	float cb;
	float cc;
	float cd;
	float da;
	float db;
	float dc;
	float dd;
};
#ifdef PLATFORM_WINDOWS
#pragma pack(pop)
#endif


#ifdef PLATFORM_WIN
typedef float		sceVu0FVECTOR[4];
typedef float		sceVu0FMATRIX[4][4];

inline void sceVu0CopyMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1)
{
	(m0)[0][0] = (m1)[0][0];
	(m0)[0][1] = (m1)[0][1];
	(m0)[0][2] = (m1)[0][2];
	(m0)[0][3] = (m1)[0][3];

	(m0)[1][0] = (m1)[1][0];
	(m0)[1][1] = (m1)[1][1];
	(m0)[1][2] = (m1)[1][2];
	(m0)[1][3] = (m1)[1][3];

	(m0)[2][0] = (m1)[2][0];
	(m0)[2][1] = (m1)[2][1];
	(m0)[2][2] = (m1)[2][2];
	(m0)[2][3] = (m1)[2][3];

	(m0)[3][0] = (m1)[3][0];
	(m0)[3][1] = (m1)[3][1];
	(m0)[3][2] = (m1)[3][2];
	(m0)[3][3] = (m1)[3][3];
};

inline void sceVu0UnitMatrix(sceVu0FMATRIX m0)
{
	(m0)[3][0] = 0.0;
	(m0)[3][1] = 0.0;
	(m0)[3][2] = 0.0;
	(m0)[3][3] = 1.0;
	(m0)[2][0] = 0.0;
	(m0)[2][1] = 0.0;
	(m0)[2][2] = 1.0;
	(m0)[2][3] = 0.0;
	(m0)[1][0] = 0.0;
	(m0)[1][1] = 1.0;
	(m0)[1][2] = 0.0;
	(m0)[1][3] = 0.0;
	(m0)[0][0] = 1.0;
	(m0)[0][1] = 0.0;
	(m0)[0][2] = 0.0;
	(m0)[0][3] = 0.0;
	return;
}

inline void sceVu0TransMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1, sceVu0FVECTOR v0)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	float fVar15;
	float fVar16;
	float fVar17;
	float fVar18;

	fVar12 = (v0)[0];
	fVar13 = (v0)[1];
	fVar14 = (v0)[2];
	fVar15 = (m1)[3][0];
	fVar16 = (m1)[3][1];
	fVar17 = (m1)[3][2];
	fVar18 = (m1)[3][3];
	fVar1 = (m1)[0][1];
	fVar2 = (m1)[0][2];
	fVar3 = (m1)[0][3];
	fVar4 = (m1)[1][0];
	fVar5 = (m1)[1][1];
	fVar6 = (m1)[1][2];
	fVar7 = (m1)[1][3];
	fVar8 = (m1)[2][0];
	fVar9 = (m1)[2][1];
	fVar10 = (m1)[2][2];
	fVar11 = (m1)[2][3];
	(m0)[0][0] = (m1)[0][0];
	(m0)[0][1] = fVar1;
	(m0)[0][2] = fVar2;
	(m0)[0][3] = fVar3;
	(m0)[1][0] = fVar4;
	(m0)[1][1] = fVar5;
	(m0)[1][2] = fVar6;
	(m0)[1][3] = fVar7;
	(m0)[2][0] = fVar8;
	(m0)[2][1] = fVar9;
	(m0)[2][2] = fVar10;
	(m0)[2][3] = fVar11;
	(m0)[3][0] = fVar15 + fVar12;
	(m0)[3][1] = fVar16 + fVar13;
	(m0)[3][2] = fVar17 + fVar14;
	(m0)[3][3] = fVar18;
	return;
}

inline void sceVu0MulMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1, sceVu0FMATRIX m2)
{
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			for (int k = 0; k < 4; ++k)
			{
				m0[i][j] += m1[i][k] * m2[k][j];
			}
		}
	}

	return;
}

#endif

#define TO_SCE_MTX float(*)[4]
#define TO_SCE_VECTOR float*

#endif //_TYPES_H
