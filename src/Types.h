#ifndef _TYPES_H
#define _TYPES_H

#ifdef PLATFORM_PS2
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#define static_assert(...)
#endif

#ifdef PLATFORM_WIN
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

#ifdef  PLATFORM_PS2
#define ZONE_SCOPED
#define ZONE_SCOPED_NAME(...)
#define FRAME_MARK
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


typedef enum SWITCH_MODE {
	SWITCH_MODE_A = 0,
	SWITCH_MODE_B = 1,
	SWITCH_MODE_C = 2,
	SWITCH_MODE_D = 3,
	SWITCH_MODE_E = 4,
	SWITCH_MODE_F = 5
} SWITCH_MODE;

union Hash_4
{
	char name[4];
	uint number;

	Hash_4() { number = 0; }
	Hash_4(uint inNumber) { number = inNumber; }
};

#include <string>

union Hash_8
{
	char name[8];
	ulong number;

	Hash_8() { number = 0; }
	Hash_8(ulong inNumber) { number = inNumber; }

	// Debug
	inline std::string ToString() const {
		char buff[24];

		// Null-terminate directly in `name`
		char tempName[9];
		memcpy(tempName, name, 8);
		tempName[8] = '\0';

		// Sanitize the name to avoid issues with printf
		for (int i = 0; i < 8; ++i) {
			if (tempName[i] < ' ' || tempName[i] > '~' || tempName[i] == '\n' || tempName[i] == '\r' || tempName[i] == '\t') {
				tempName[i] = '?'; // Replace non-printable characters with '?'
			}
		}

		snprintf(buff, sizeof(buff), "%s(0x%llx)", tempName, number);
		return std::string(buff);
	}
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

union _rgba {
	struct
	{
		byte r;
		byte g;
		byte b;
		byte a;
	};

	uint rgba;

	_rgba() : rgba(0) {}

	_rgba(byte ir, byte ig, byte ib, byte ia)
		: r(ir)
		, g(ig)
		, b(ib)
		, a(ia)
	{}

	void LerpRGBA(float alpha, uint src, uint dst);
};

static_assert(sizeof(_rgba) == 4, "sizeof(_rgba) != 4");

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
	ACTOR = 0x0,
	CINEMATIC = 0x1,
	BOOMY = 0x5,
	ACTOR_HERO_PRIVATE = 0x6,
	MICKEN = 0x7,
	WOLFEN = 0x9,
	NATIV = 0xa,
	ELECTROLLA = 0xb,
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
	LIANA = 0x1a,
	DCA = 0x1b,
	PROJECTILE = 0x1c,
	EXPLOSIVE_DISTRIBUTOR = 0x1d,
	CHECKPOINT_MANAGER = 0x1e,
	WEAPON = 0x20,
	ARAIGNOS = 0x21,
	HEDGEHOG = 0x22,
	DRAGO_TERMITES = 0x23,
	TELEPORTER = 0x24,
	NOSE_MONSTER = 0x26,
	WOOD_MONSTER = 0x27,
	AMORTOS = 0x29,
	WOOF = 0x2a,
	HELPER_SIGN = 0x2b,
	TRAP = 0x2c,
	FX = 0x2e,
	EGG = 0x2f,
	PUNCHING_BALL = 0x30,
	AMORTOS_CMD = 0x31,
	MONEY = 0x32,
	JAMGUT = 0x33,
	NATIV_SHOP = 0x34,
	BOX = 0x35,
	SHIP = 0x36,
	CARE_BOY = 0x37,
	ATON = 0x38,
	BASIC_BOX = 0x39,
	ACCELERATOS = 0x3a,
	EVENT_GENERATOR = 0x3b,
	NATIV_CMD = 0x3c,
	FOG_MANAGER = 0x3d,
	SHOOT = 0x3e,
	PATTERN_CON = 0x3f,
	PATTERN_CMD = 0x40,
	MINI_GAMES_MANAGER = 0x42,
	MINI_GAMES_ORGANIZER = 0x43,
	MINI_GAME = 0x44,
	MINI_GAME_TIME_ATTACK = 0x45,
	MINI_GAME_BOX_COUNTER = 0x46,
	MINI_GAME_BOOMY = 0x47,
	HUNTER = 0x48,
	ADD_ON_PURCHASE = 0x49,
	SHOCKER = 0x4b,
	STILLER = 0x4c,
	BLAZER = 0x4d,
	BUNCH = 0x4e,
	BRAZUL = 0x4f,
	SHOCKER_CMD = 0x50,
	RUNE = 0x51,
	BONUS_FOUNTAIN = 0x52,
	BOMB_LAUNCHER = 0x53,
	WANTED_ZOO = 0x54,
	MINI_GAME_DISTANCE = 0x55,
	CREDITS = 0x56,
};

struct ED_PSX2_MODULES_INIT {
	char* pIopRoot;
	char* pImgName;
	int modulesCount;
	char* pModulesName;
};

struct edSYS_CONFIG {

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
	EFileLoadMode version;
	char* field_0x20;
	ED_PSX2_MODULES_INIT* pIopPaths;
};

EFileLoadMode GetFileLoadMode_00424d9c(void);
edSYS_CONFIG* edSysGetConfig(void);

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

	struct {
		float u;
		float v;
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
#ifdef USE_STRING_STREAMS
		const int printWidth = 10;

		std::stringstream ss;
		// Set the precision for floating-point values
		ss << std::fixed << std::setprecision(4);
		ss << std::setw(printWidth) << "(" << std::setw(printWidth) << x << ", " << std::setw(printWidth) << y
			<< ", " << std::setw(printWidth) << z << ")";
		return ss.str();
#else
		char buff[256];
		sprintf_s(buff, 256, "(%.4f, %.4f, %.4f)", x, y, z);
		return buff;
#endif
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

inline edF32VECTOR3 operator+(const edF32VECTOR3& lhs, const float& rhs)
{
	edF32VECTOR3 ret;
	ret.x = lhs.x + rhs;
	ret.y = lhs.y + rhs;
	ret.z = lhs.z + rhs;
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

inline edF32VECTOR3 operator-(const float& lhs, const edF32VECTOR3& rhs)
{
	edF32VECTOR3 ret;
	ret.x = lhs - rhs.x;
	ret.y = lhs - rhs.y;
	ret.z = lhs - rhs.z;
	return ret;
}

inline edF32VECTOR3 operator/(const edF32VECTOR3& lhs, const float& rhs)
{
	edF32VECTOR3 ret;
	ret.x = lhs.x / rhs;
	ret.y = lhs.y / rhs;
	ret.z = lhs.z / rhs;
	return ret;
}

struct Rectangle
{
	float x;
	float y;
	float width;
	float height;
};

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
		char cachedString[128];
		sprintf_s(cachedString, 128, "(%.2f, %.2f, %.2f, %.2f)", x, y, z, w);
		return cachedString;
	}
#endif
};

struct edS32VECTOR4
{
	int x;
	int y;
	int z;
	int w;
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

inline edF32VECTOR4 operator-(const float& lhs, const edF32VECTOR4& rhs)
{
	edF32VECTOR4 ret;
	ret.x = lhs - rhs.x;
	ret.y = lhs - rhs.y;
	ret.z = lhs - rhs.z;
	ret.w = lhs - rhs.w;
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


struct edS32VECTOR3
{
	int x;
	int y;
	int z;
};

struct S_BOUNDING_BOX {
	edF32VECTOR4 minPoint;
	edF32VECTOR4 maxPoint;
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
#ifdef USE_STRING_STREAMS
		std::stringstream ss;
		ss << "\n" << rowX.ToString() << "\n" << rowY.ToString() << "\n" << rowZ.ToString() << "\n" << rowT.ToString();
		return ss.str();
#else
	char buff[512];
	sprintf_s(buff, 512, "%s\n%s\n%s\n%s", rowX.ToString().c_str(), rowY.ToString().c_str(), rowZ.ToString().c_str(), rowT.ToString().c_str());
	return buff;
#endif
	}
#endif
};
#ifdef PLATFORM_WIN
#pragma pack(pop)
#endif

inline edF32MATRIX4 operator*(const edF32MATRIX4& lhs, const float& rhs)
{
	edF32MATRIX4 ret;
	ret.rowX = lhs.rowX * rhs;
	ret.rowY = lhs.rowY * rhs;
	ret.rowZ = lhs.rowZ * rhs;
	ret.rowT = lhs.rowT * rhs;
	return ret;
}

#define EDITOR_BUILD PLATFORM_WIN

#include "Logging.h"

#ifdef PLATFORM_WIN
#define NAME_NEXT_OBJECT(format, ...) ObjectNaming::SetNextObjectName(format, __VA_ARGS__)
#else
#define NAME_NEXT_OBJECT(...)
#endif

#define LOC_KEY_TO_CHAR(key) key & 0xff, (key >> 8) & 0xff, (key >> 16) & 0xff, (key >> 24) & 0xff

#define edDebugPrintf scePrintf

union edpkt_data {
	uint128 asU128;

	struct __attribute__((aligned(16))) {
		ulong cmdA;
		ulong cmdB;
	};
	uint asU32[4];
	edF32VECTOR4 asVector;
	float asF32[4];
};

enum LANGUAGE
{
	GB,
	FR,
	GE,
	SP,
	IT,
	AUTO
};

enum AUDIO_MODE
{
	MONO,
	STEREO,
	SURROUND
};

// Overload the * operator as a non-member function
inline edF32VECTOR4 operator*(const edF32VECTOR4& lhs, const edF32MATRIX4& rhs)
{
	edF32VECTOR4 ret;
	ret = rhs.rowT + (rhs.rowZ * lhs.z) + (rhs.rowY * lhs.y) + (rhs.rowX * lhs.x);
	return ret;
}

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

#define PLAY_INTRO_VIDEO 0

#define IMPLEMENTATION_GUARD_FX(x)
#define IMPLEMENTATION_GUARD_EMOTION(x)
#define IMPLEMENTATION_GUARD_PS2(x)
#define IMPLEMENTATION_GUARD_AUDIO(x)
#define IMPLEMENTATION_GUARD_ACTOR(x)
#define IMPLEMENTATION_GUARD_LIGHT(x)
#define IMPLEMENTATION_GUARD_UI(x)
#define IMPLEMENTATION_GUARD_SAVE(x)
#define IMPLEMENTATION_GUARD_SHADOW(x)
#define IMPLEMENTATION_GUARD_ADD_ON(x)
#define IMPLEMENTATION_GUARD_LOOK_AT(x)
#define IMPLEMENTATION_GUARD_LOG(x) MY_LOG_CATEGORY("ImplementationGuard", LogLevel::Verbose, "IMPLEMENTATION_GUARD_LOG {}, {}\n", __FILE__, __LINE__);

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

#include "port/pointer_conv.h"

class CSound {};

template<typename T>
struct S_STREAM_REF {
	void Init();

	T* Get() {
		return (T*)LOAD_SECTION(this->pObj);
	}

	void Reset() {}

	union {
		int pObj;
		int index;
	};
};

template<typename T, int Size>
class CFixedTable {
public:
	CFixedTable()
		: nbEntries(0)
	{
	}

	int nbEntries;
	T aEntries[Size];

	inline void Add(T value) {
		if (this->nbEntries < Size) {
			this->aEntries[this->nbEntries] = value;
			this->nbEntries++;
		}
	}

	inline bool IsInList(T value) {
		for (int i = 0; i < this->nbEntries; i++) {
			if (this->aEntries[i] == value) {
				return true;
			}
		}

		return false;
	}

	inline void Swap(T a, T b) {
		T tmp;

		// Access the element at position 'a' and store it in iVar1
		tmp = (this->aEntries + a + -1)[1];

		// Swap the values of elements at positions 'a' and 'b'
		(this->aEntries + a + -1)[1] = (this->aEntries + b + -1)[1];
		(this->aEntries + b + -1)[1] = tmp;
	}

	inline void MergeWithTable(CFixedTable<T, Size>* pTable) {
		bool bVar1;
		int iVar4;
		int iVar5;

		iVar5 = 0;
		if (0 < pTable->nbEntries) {
			do {
				iVar4 = 0;
				if (0 < this->nbEntries) {
					do {
						bVar1 = true;
						if (pTable->aEntries[iVar4] == this->aEntries[iVar4]) goto LAB_001985c0;

						iVar4 = iVar4 + 1;
					} while (iVar4 < this->nbEntries);
				}

				bVar1 = false;
			LAB_001985c0:
				if ((!bVar1) && (this->nbEntries < Size)) {
					this->aEntries[this->nbEntries] = pTable->aEntries[iVar5];
					this->nbEntries = this->nbEntries + 1;
				}
				iVar5 = iVar5 + 1;
			} while (iVar5 < pTable->nbEntries);
		}

		return;
	}

	inline T Pop(int index)
	{
		T pCVar1;
		T* ppCVar2;
		int iVar3;

		ppCVar2 = this->aEntries + index + -1;
		iVar3 = this->nbEntries + -1;
		pCVar1 = ppCVar2[1];
		if (index < iVar3) {
			do {
				index = index + 1;
				ppCVar2[1] = ppCVar2[2];
				ppCVar2 = ppCVar2 + 1;
			} while (index < iVar3);
		}

		this->nbEntries = this->nbEntries + -1;
		return pCVar1;
	}

	inline void Pop(T* pOut, int index)
	{
		T pCVar1;
		T* ppCVar2;
		int iVar3;
		ppCVar2 = this->aEntries + index + -1;
		iVar3 = this->nbEntries + -1;
		pCVar1 = ppCVar2[1];
		if (index < iVar3) {
			do {
				index = index + 1;
				ppCVar2[1] = ppCVar2[2];
				ppCVar2 = ppCVar2 + 1;
			} while (index < iVar3);
		}

		this->nbEntries = this->nbEntries + -1;
		pOut[0] = pCVar1;
		return;
	}

	inline T PopCurrent() {
		int iVar1;
		T pCVar2;

		pCVar2 = (T)0x0;
		if (this->nbEntries != 0) {
			iVar1 = this->nbEntries;
			this->nbEntries = iVar1 + -1;
			pCVar2 = this->aEntries[iVar1 + -1];
		}

		return pCVar2;
	}
};

class CActor;
class CActorsTable : public CFixedTable<CActor*, 64>
{
public:
	bool IsInList(CActor* pActor);
	bool IsInList(int value);
	CActor* Remove(CActor* pActor);
	void SortByClassPriority();
	void Swap(int a, int b);
};

PACK(
	struct ed_zone_3d {
	int field_0x0[3]; // ptr
	edF32VECTOR4 boundSphere;
	uint flags;
	int pMatrix; // edF32MATRIX4*
};)

struct S_ZONE_STREAM_REF {
	int entryCount;
	S_STREAM_REF<ed_zone_3d> aEntries[];
};

// This shows up when the VIF0 does an MR32.w to zero out the w element of the vector register.
// Define it here and check wherever it shows up in the code to make sure an appropriate MR32.w inst is ther.
const float in_vf0x = 0.0f;

#define CAMERA_HACK_ACTIVE 0x10000000

enum EBankAction
{
	BANK_ACTION_3 = 3,
	BANK_ACTION_5 = 5,
	CLOSE = 4,
	LOAD = 0,
	OPEN = 6,
	READ_STREAM = 2,
	SEEK = 1
};

#if defined(PLATFORM_PS2)
#include <libcdvd.h>
#else
struct sceCdlFILE {
	uint lsn; /* File location on disc */
	uint size; /* File size */
	char name[16]; /* File name (body) */
	u_char date[8]; /* Date */
	uint flag; /* File flag */
};
#endif

// Redifining sqrtf to handle negatives
#ifdef PLATFORM_WIN
#define sqrtf(x) (std::sqrtf(std::fabs(x)))
#endif

class CObject {
public:
	CObject() : objectId(-1) {}

	int objectId;

	virtual bool IsKindOfObject(ulong kind) { return false; }
	virtual bool InitDlistPatchable(int) { return false; }
};

struct ed_Bound_Sphere
{
	union
	{
		struct {
			float x;
			float y;
			float z;
		};

		edF32VECTOR3 xyz;
	};

	float radius;
};

#endif //_TYPES_H
