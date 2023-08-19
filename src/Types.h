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
typedef unsigned uint128 __attribute__((mode(TI)));
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

struct _rgba {
	byte r;
	byte g;
	byte b;
	byte a;

	_rgba() {}

	_rgba(byte ir, byte ig, byte ib, byte ia)
		: r(ir)
		, g(ig)
		, b(ib)
		, a(ia)
	{}

	void LerpRGBA(float alpha, _rgba param_3, _rgba param_4);
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

EFileLoadMode GetFileLoadMode_00424d9c(void);
InputSetupParams* edSysGetConfig(void);

#ifdef PLATFORM_WIN
#include "log.h"
#endif

#define EDITOR_BUILD PLATFORM_WIN

//#define ENABLE_MY_LOG

#define LOC_KEY_TO_CHAR(key) key & 0xff, (key >> 8) & 0xff, (key >> 16) & 0xff, (key >> 24) & 0xff

#ifdef ENABLE_MY_LOG
#if defined(PLATFORM_WIN)
#define scePrintf(format, ...) Log::GetInstance().AddLog(LogLevel::Info, "PS2", format, ##__VA_ARGS__)
#define MY_LOG(format, ...) Log::GetInstance().AddLog(LogLevel::Info, "General", format, ##__VA_ARGS__)
#define RENDER_LOG(format, ...) Log::GetInstance().AddLog(LogLevel::VeryVerbose, "Rendering", format, ##__VA_ARGS__)
#define RENDER_LOGF(format, ...) Log::GetInstance().AddLog(LogLevel::VeryVerbose, "Rendering", format, ##__VA_ARGS__)

#define MY_LOG_CATEGORY(category, level, format, ...) Log::GetInstance().AddLog(level, category, format, ##__VA_ARGS__)

#else
#include <eekernel.h>
#define MY_LOG(...) scePrintf(##__VA_ARGS__); scePrintf("\n")
#define RENDER_LOG(...) scePrintf(##__VA_ARGS__); scePrintf("\n")
#define MY_LOG_CATEGORY(category, level, format, ...) scePrintf(format, ##__VA_ARGS__); scePrintf("\n")
#endif

#include <stdio.h>

#include <stdlib.h>

inline void PrintVector(edF32VECTOR4* vector)
{
	//MY_LOG("%5.2f, %5.2f, %5.2f, %5.2f\n", vector->x, vector->y, vector->z, vector->w);
	char buff[256] = { 0 };
	sprintf(buff, "%5.2f, %5.2f, %5.2f, %5.2f\n", vector->x, vector->y, vector->z, vector->w);
	MY_LOG("%s", buff);
}

inline void PrintMatrix(edF32MATRIX4* matrix)
{
	PrintVector((edF32VECTOR4*)&matrix->aa);
	PrintVector((edF32VECTOR4*)&matrix->ba);
	PrintVector((edF32VECTOR4*)&matrix->ca);
	PrintVector((edF32VECTOR4*)&matrix->da);
}

#define PRINT_VECTOR(a) PrintVector(a)
#define PRINT_MATRIX(a) PrintMatrix(a)

#else
#define MY_LOG(...)
#define RENDER_LOG(...)
#define MY_LOG_CATEGORY(...)
#define PRINT_VECTOR(...)
#define PRINT_MATRIX(...)

#ifdef PLATFORM_WIN
#define scePrintf(...)
#endif
#endif

#define edDebugPrintf scePrintf

#ifdef PLATFORM_PS2
struct __attribute__((aligned(16)))
#else
#pragma pack(push,1)
struct //alignas(16)
#endif 
edF32VECTOR4 { /* Aligned */
	float x;
	float y;
	float z;
	float w;
};

// Overload the * operator as a non-member function
inline edF32VECTOR4 operator*(const edF32VECTOR4& lhs, const edF32VECTOR4& rhs)
{
	edF32VECTOR4 ret;
	ret.x = lhs.x * rhs.x;
	ret.y = lhs.y * rhs.y;
	ret.z = lhs.z * rhs.z;
	ret.w = lhs.w * rhs.w;
	return ret;
}

inline edF32VECTOR4 operator*(const edF32VECTOR4& lhs, const float& rhs)
{
	edF32VECTOR4 ret;
	ret.x = lhs.x * rhs;
	ret.y = lhs.y * rhs;
	ret.z = lhs.z * rhs;
	ret.w = lhs.w * rhs;
	return ret;
}

inline edF32VECTOR4 operator+(const edF32VECTOR4& lhs, const edF32VECTOR4& rhs)
{
	edF32VECTOR4 ret;
	ret.x = lhs.x + rhs.x;
	ret.y = lhs.y + rhs.y;
	ret.z = lhs.z + rhs.z;
	ret.w = lhs.w + rhs.w;
	return ret;
}

inline edF32VECTOR4 operator+(const edF32VECTOR4& lhs, const float& rhs)
{
	edF32VECTOR4 ret;
	ret.x = lhs.x + rhs;
	ret.y = lhs.y + rhs;
	ret.z = lhs.z + rhs;
	ret.w = lhs.w + rhs;
	return ret;
}

inline edF32VECTOR4 operator-(const edF32VECTOR4& lhs, const edF32VECTOR4& rhs)
{
	edF32VECTOR4 ret;
	ret.x = lhs.x - rhs.x;
	ret.y = lhs.y - rhs.y;
	ret.z = lhs.z - rhs.z;
	ret.w = lhs.w - rhs.w;
	return ret;
}

struct edF32VECTOR3 {
	float x;
	float y;
	float z;
};

#ifdef PLATFORM_WIN
#pragma pack(pop)
#endif

#ifdef PLATFORM_PS2
union
#else
#pragma pack(push,1)
union alignas(16)
#endif 
edF32MATRIX4 {
	struct  __attribute__((aligned(16))) {
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

	struct  __attribute__((aligned(16))) {
		edF32VECTOR4 v0;
		edF32VECTOR4 v1;
		edF32VECTOR4 v2;
		edF32VECTOR4 v3;
	};

	struct  __attribute__((aligned(16))) {
		edF32VECTOR4 rowX;
		edF32VECTOR4 rowY;
		edF32VECTOR4 rowZ;
		edF32VECTOR4 rowT;
	};

	float raw[16];
};
#ifdef PLATFORM_WIN
#pragma pack(pop)
#endif


#ifdef PLATFORM_WIN
//typedef float		sceVu0FVECTOR[4];
//typedef float		sceVu0FMATRIX[4][4];

#endif

union edpkt_data {
	uint128 asU128;

	struct __attribute__((aligned(16))) {
		ulong cmdA;
		ulong cmdB;
	};
	uint asU32[4];
	edF32VECTOR4 asVector;
};

#define TO_SCE_MTX float(*)[4]
#define TO_SCE_VECTOR float*

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

#define UNPACK_V4_32 0x6c
#define UNPACK_V4_32_MASKED 0x7c

#endif //_TYPES_H
