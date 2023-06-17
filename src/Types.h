#ifndef _TYPES_H
#define _TYPES_H

#ifdef PLATFORM_PS2
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef PLATFORM_WIN
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
typedef unsigned long long    ulong;
typedef unsigned char    undefined1;
typedef unsigned short    undefined2;
typedef unsigned int    undefined3;
typedef unsigned int    undefined4;
typedef unsigned long    undefined5;
typedef unsigned long long    undefined8;
typedef unsigned short    ushort;
typedef unsigned short    word;
typedef unsigned long long    ulonglong;
typedef long    int7;
typedef unsigned long    uint7;


#ifdef PLATFORM_WIN
typedef unsigned int    u_int;
#endif

union Hash_4
{
	char name[4];
	uint number;

	Hash_4() { number = 0; }
	Hash_4(uint inNumber) { number = inNumber; }
};

union Hash_8
{
	char name[8];
	ulong number;

	Hash_8() { number = 0; }
	Hash_8(ulong inNumber) { number = inNumber; }
};

struct EdFileGlobal_10 {
	EdFileGlobal_10* pPrevFileHeader;
	void* pFreeFunc;
	void* pFileData;
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
};

struct ByteColor {
	byte r;
	byte g;
	byte b;
	byte a;
};

struct ByteColor3 {
	byte r;
	byte g;
	byte b;
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
InputSetupParams* edSysGetConfig(void);

#ifdef PLATFORM_WIN
#include "log.h"
#endif

#define ENABLE_MY_LOG

#define LOC_KEY_TO_CHAR(key) key & 0xff, (key >> 8) & 0xff, (key >> 16) & 0xff, (key >> 24) & 0xff

#ifdef ENABLE_MY_LOG
#if defined(PLATFORM_WIN)
#define scePrintf(format, ...) Log::GetInstance().AddLog(LogLevel::Info, "PS2", format, ##__VA_ARGS__)
#define MY_LOG(format, ...) Log::GetInstance().AddLog(LogLevel::Info, "General", format, ##__VA_ARGS__)
#define RENDER_LOG(format, ...) Log::GetInstance().AddLog(LogLevel::VeryVerbose, "Rendering", format, ##__VA_ARGS__)
#else
#include <eekernel.h>
#define MY_LOG scePrintf
#define RENDER_LOG scePrintf
#endif

#define edDebugPrintf scePrintf

#include <stdio.h>

#include <stdlib.h>

#else
#define MY_LOG(...)
#define RENDER_LOG(...)
#endif

#ifdef PLATFORM_PS2
struct __attribute__((aligned(16)))
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

struct edF32VECTOR3 {
	float x;
	float y;
	float z;
};

#ifdef PLATFORM_WIN
#pragma pack(pop)
#endif

#ifdef PLATFORM_PS2
struct __attribute__((aligned(16)))
#else
#pragma pack(push,1)
struct alignas(16)
#endif 
edF32MATRIX4 {
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
#ifdef PLATFORM_WIN
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
	int i, j, k;
	sceVu0FMATRIX temp;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			temp[i][j] = 0;
			for (k = 0; k < 4; k++) {
				temp[i][j] += m1[i][k] * m2[k][j];
			}
		}
	}

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			m0[i][j] = temp[i][j];
		}
	}
}

#endif

#ifdef PLATFORM_PS2
struct __attribute__((aligned(16)))
#else
struct alignas(16)
#endif
edpkt_data {
	ulong cmdA;
	ulong cmdB;
};

//#ifdef PLATFORM_WIN
inline uint* edpktAsU32(edpkt_data* pkt) {
	return reinterpret_cast<uint*>(pkt);
}
//#endif

#define TO_SCE_MTX float(*)[4]
#define TO_SCE_VECTOR float*

inline void PrintVector(Vector* vector)
{
	//MY_LOG("%5.2f, %5.2f, %5.2f, %5.2f\n", vector->x, vector->y, vector->z, vector->w);
	char buff[256] = { 0 };
	sprintf(buff, "%5.2f, %5.2f, %5.2f, %5.2f\n", vector->x, vector->y, vector->z, vector->w);
	MY_LOG("%s", buff);
}

inline void PrintMatrix(edF32MATRIX4* matrix)
{
	PrintVector((Vector*)&matrix->aa);
	PrintVector((Vector*)&matrix->ba);
	PrintVector((Vector*)&matrix->ca);
	PrintVector((Vector*)&matrix->da);
}

#define PRINT_VECTOR(a) PrintVector(a)
#define PRINT_MATRIX(a) PrintMatrix(a)

//#define PRINT_VECTOR(a) MY_LOG("%.02f, %.02f, %.02f, %.02f\n", ((float*)&a)[0], ((float*)&a)[1], ((float*)&a)[2], ((float*)&a)[3])
//#define PRINT_MATRIX(a) PRINT_VECTOR((float*)&a); PRINT_VECTOR(((float*)&a) + 4); PRINT_VECTOR(((float*)&a) + 8); PRINT_VECTOR(((float*)&a) + 12)

enum LANGUAGE
{
	GB,
	FR,
	GE,
	SP,
	IT,
	AUTO
};

extern int g_SetOffsetX;
extern int g_SetOffsetY;

struct SectorManagerSubObj {
	uint flags;
	undefined* pFileData;
	struct WindSectorObj* pWindSectorObj;
	int field_0xc;
};

#ifdef PLATFORM_WIN
#include <assert.h>
#define IMPLEMENTATION_GUARD(x) assert(false);
#else
#define IMPLEMENTATION_GUARD(x) MY_LOG("Hit an assert! %s, %d\n", __FILE__, __LINE__);
#endif

#ifdef PLATFORM_WIN
#define RESOLVE_FONT_SUB_DATA(a) (FontPacked_2C*)LOAD_SECTION(a)
#else
#define RESOLVE_FONT_SUB_DATA(a) a
#endif

#endif //_TYPES_H
