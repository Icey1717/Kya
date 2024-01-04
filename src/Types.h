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
#ifdef PLATFORM_PS2
typedef unsigned uint128 __attribute__((mode(TI)));
#else
typedef unsigned uint128 __attribute__((mode(TI)));
#endif
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


enum ACTOR_CLASS {
	CINEMATIC = 1,
	BOOMY = 5,
	ACTOR_HERO_PRIVATE = 6,
	MICKEN = 7,
	WOLFEN = 9,
	NATIV = 0xa,
	MOVING_PLATFORM = 0xd,
	BONUS = 0xe,
	ROPE = 0xf,
	GRAVITY_AWARE = 0x10,
	SWITCH = 0x11,
	WIND = 0x12,
	COMMANDER = 0x13,
	AMBER = 0x14,
	FRUIT = 0x16,
	BRIDGE = 0x17,
	CLUSTERISER = 0x18,
	COMPANION = 0x19,
	DCA = 0x1b,
	PROJECTILE = 0x1c,
	CHECKPOINT_MANAGER = 0x1e,
	WEAPON = 0x20,
	ARAIGNOS = 0x21,
	TELEPORTER = 0x24,
	AMORTOS = 0x29,
	TRAP = 0x2c,
	FX = 0x2e,
	MONEY = 0x32,
	BOX = 0x35,
	ATON = 0x38,
	BASIC_BOX = 0x39,
	NATIV_CMD = 0x3c,
	FOG_MANAGER = 0x3d,
	SHOOT = 0x3e,
	SHOCKER = 0x4b,
	BLAZER = 0x4c,
	SHOCKER_CMD = 0x50,
	RUNE = 0x51,
	BONUS_FOUNTAIN = 0x52,
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

#define trap assert

#ifdef PLATFORM_WIN
#include "log.h"
#include <sstream>
#include <iomanip>
#endif

union edF32VECTOR2 {
	struct {
		float x;
		float y;
	};

	struct {
		float alpha;
		float beta;
	};

	float raw[2];
};

union edF32VECTOR3 {
	struct {
		float x;
		float y;
		float z;
	};

	struct {
		float alpha;
		float beta;
		float gamma;
	};

	float raw[3];

#ifdef PLATFORM_WIN
	inline std::string ToString() const {
		const int printWidth = 10;

		std::stringstream ss;
		// Set the precision for floating-point values
		ss << std::fixed << std::setprecision(4);
		ss << std::setw(printWidth) << "(" << std::setw(printWidth) << x << ", " << std::setw(printWidth) << y
			<< ", " << std::setw(printWidth) << z << ")";
		return ss.str();
	}
#endif
};

inline edF32VECTOR3 operator-(const edF32VECTOR3& lhs, const edF32VECTOR3& rhs)
{
	edF32VECTOR3 ret;
	ret.x = lhs.x - rhs.x;
	ret.y = lhs.y - rhs.y;
	ret.z = lhs.z - rhs.z;
	return ret;
}

inline edF32VECTOR3 operator+(const edF32VECTOR3& lhs, const edF32VECTOR3& rhs)
{
	edF32VECTOR3 ret;
	ret.x = lhs.x + rhs.x;
	ret.y = lhs.y + rhs.y;
	ret.z = lhs.z + rhs.z;
	return ret;
}

inline edF32VECTOR3 operator*(const edF32VECTOR3& lhs, const edF32VECTOR3& rhs)
{
	edF32VECTOR3 ret;
	ret.x = lhs.x * rhs.x;
	ret.y = lhs.y * rhs.y;
	ret.z = lhs.z * rhs.z;
	return ret;
}

inline edF32VECTOR3 operator*(const edF32VECTOR3& lhs, const float& rhs)
{
	edF32VECTOR3 ret;
	ret.x = lhs.x * rhs;
	ret.y = lhs.y * rhs;
	ret.z = lhs.z * rhs;
	return ret;
}

inline edF32VECTOR3 operator-(const edF32VECTOR3& lhs, const float& rhs)
{
	edF32VECTOR3 ret;
	ret.x = lhs.x - rhs;
	ret.y = lhs.y - rhs;
	ret.z = lhs.z - rhs;
	return ret;
}

#ifdef PLATFORM_PS2
union
#else
#pragma pack(push,1)
union
#endif 
	edF32VECTOR4 { /* Aligned */
	struct {
		float x;
		float y;
		float z;
		float w;
	};

	struct {
		int xi;
		int yi;
		int zi;
		int wi;
	};

	struct {
		edF32VECTOR3 xyz;
	};

	struct {
		edF32VECTOR2 xy;
	};

	float raw[4];

#ifdef PLATFORM_WIN
	inline std::string ToString() const {
		const int printWidth = 10;

		std::stringstream ss;
		// Set the precision for floating-point values
		ss << std::fixed << std::setprecision(4);
		ss << std::setw(printWidth) << "(" << std::setw(printWidth) << x << ", " << std::setw(printWidth) << y
			<< ", " << std::setw(printWidth) << z << ", " << std::setw(printWidth) << w << ")";
		return ss.str();
	}
#endif
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

#ifdef PLATFORM_WIN
#pragma pack(pop)
#endif

#ifdef PLATFORM_PS2
union
#else
#pragma pack(push,1)
union alignas(16)
#endif 
	edF32MATRIX3 {
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
	};

	struct  __attribute__((aligned(16))) {
		edF32VECTOR4 v0;
		edF32VECTOR4 v1;
		edF32VECTOR4 v2;
	};

	struct  __attribute__((aligned(16))) {
		edF32VECTOR4 rowX;
		edF32VECTOR4 rowY;
		edF32VECTOR4 rowZ;
	};

	float raw[12];
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

	edF32VECTOR4 vector[4];

	inline edF32MATRIX4& operator=(const edF32MATRIX3& rhs)
	{
		rowX = rhs.rowX;
		rowY = rhs.rowY;
		rowZ = rhs.rowZ;
		return *this;
	}

#ifdef PLATFORM_WIN
	inline std::string ToString() const {
		std::stringstream ss;
		ss << "\n" << rowX.ToString() << "\n" << rowY.ToString() << "\n" << rowZ.ToString() << "\n" << rowT.ToString();
		return ss.str();
	}
#endif
};
#ifdef PLATFORM_WIN
#pragma pack(pop)
#endif

#define EDITOR_BUILD PLATFORM_WIN

#ifdef PLATFORM_WIN
#ifdef _DEBUG
#define ENABLE_MY_LOG
#endif
#else
#define uintptr_t int
//#define ENABLE_MY_LOG
#endif

#ifdef PLATFORM_WIN
#define NAME_NEXT_OBJECT(format, ...) ObjectNaming::SetNextObjectName(format, __VA_ARGS__)
#else
#define NAME_NEXT_OBJECT(...)
#endif

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
	struct CSectorHierarchy* pWindSectorObj;
	int field_0xc;
};

// Overload the * operator as a non-member function
inline edF32VECTOR4 operator*(const edF32VECTOR4& lhs, const edF32MATRIX4& rhs)
{
	edF32VECTOR4 ret;
	ret = rhs.rowT + (rhs.rowZ * lhs.z) + (rhs.rowY * lhs.y) + (rhs.rowX * lhs.x);
	return ret;
}

//inline edF32MATRIX4 operator*(const edF32MATRIX4& m1, const edF32MATRIX4& m2) {
//	edF32MATRIX4 result;
//
//	for (int i = 0; i < 4; ++i) {
//		for (int j = 0; j < 4; ++j) {
//			result.raw[i * 4 + j] = m1.raw[i * 4 + 0] * m2.raw[0 * 4 + j] +
//				m1.raw[i * 4 + 1] * m2.raw[1 * 4 + j] +
//				m1.raw[i * 4 + 2] * m2.raw[2 * 4 + j] +
//				m1.raw[i * 4 + 3] * m2.raw[3 * 4 + j];
//		}
//	}
//
//	return result;
//}

/* Equivalent ^^^ */
inline edF32MATRIX4 operator*(const edF32MATRIX4& lhs, const edF32MATRIX4& rhs)
{
	edF32MATRIX4 ret;
	ret.aa = lhs.aa * rhs.aa + lhs.ba * rhs.ab + lhs.ca * rhs.ac + lhs.da * rhs.ad;
	ret.ab = lhs.ab * rhs.aa + lhs.bb * rhs.ab + lhs.cb * rhs.ac + lhs.db * rhs.ad;
	ret.ac = lhs.ac * rhs.aa + lhs.bc * rhs.ab + lhs.cc * rhs.ac + lhs.dc * rhs.ad;
	ret.ad = lhs.ad * rhs.aa + lhs.bd * rhs.ab + lhs.cd * rhs.ac + lhs.dd * rhs.ad;
	ret.ba = lhs.aa * rhs.ba + lhs.ba * rhs.bb + lhs.ca * rhs.bc + lhs.da * rhs.bd;
	ret.bb = lhs.ab * rhs.ba + lhs.bb * rhs.bb + lhs.cb * rhs.bc + lhs.db * rhs.bd;
	ret.bc = lhs.ac * rhs.ba + lhs.bc * rhs.bb + lhs.cc * rhs.bc + lhs.dc * rhs.bd;
	ret.bd = lhs.ad * rhs.ba + lhs.bd * rhs.bb + lhs.cd * rhs.bc + lhs.dd * rhs.bd;
	ret.ca = lhs.aa * rhs.ca + lhs.ba * rhs.cb + lhs.ca * rhs.cc + lhs.da * rhs.cd;
	ret.cb = lhs.ab * rhs.ca + lhs.bb * rhs.cb + lhs.cb * rhs.cc + lhs.db * rhs.cd;
	ret.cc = lhs.ac * rhs.ca + lhs.bc * rhs.cb + lhs.cc * rhs.cc + lhs.dc * rhs.cd;
	ret.cd = lhs.ad * rhs.ca + lhs.bd * rhs.cb + lhs.cd * rhs.cc + lhs.dd * rhs.cd;
	ret.da = lhs.aa * rhs.da + lhs.ba * rhs.db + lhs.ca * rhs.dc + lhs.da * rhs.dd;
	ret.db = lhs.ab * rhs.da + lhs.bb * rhs.db + lhs.cb * rhs.dc + lhs.db * rhs.dd;
	ret.dc = lhs.ac * rhs.da + lhs.bc * rhs.db + lhs.cc * rhs.dc + lhs.dc * rhs.dd;
	ret.dd = lhs.ad * rhs.da + lhs.bd * rhs.db + lhs.cd * rhs.dc + lhs.dd * rhs.dd;
	return ret;
}


#define IMPLEMENTATION_GUARD_AUDIO(x)
#define IMPLEMENTATION_GUARD_LOG(x) MY_LOG("IMPLEMENTATION_GUARD_LOG {}, {}\n", __FILE__, __LINE__);

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

#define UNPACK_S32 0x60
#define UNPACK_S16 0x61
#define UNPACK_S8 0x62
#define UNPACK_V2_32 0x64
#define UNPACK_V2_16 0x65
#define UNPACK_V2_8 0x66
#define UNPACK_V3_32 0x68
#define UNPACK_V3_16 0x69
#define UNPACK_V3_8 0x6a
#define UNPACK_V4_32 0x6c
#define UNPACK_V4_16 0x6d
#define UNPACK_V4_8 0x6e
#define UNPACK_V4_5 0x6f

#define UNPACK_S32_MASKED 0x70
#define UNPACK_S16_MASKED 0x71
#define UNPACK_S8_MASKED 0x72
#define UNPACK_V2_32_MASKED 0x74
#define UNPACK_V2_16_MASKED 0x75
#define UNPACK_V2_8_MASKED 0x76
#define UNPACK_V3_32_MASKED 0x78
#define UNPACK_V3_16_MASKED 0x79
#define UNPACK_V3_8_MASKED 0x7a
#define UNPACK_V4_32_MASKED 0x7c
#define UNPACK_V4_16_MASKED 0x7d
#define UNPACK_V4_8_MASKED 0x7e
#define UNPACK_V4_5_MASKED 0x7f

#define ACTOR_NUM_CLASSES 0x57

#endif //_TYPES_H
