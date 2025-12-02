#ifndef ED_PARTICLES_H
#define ED_PARTICLES_H

#include "Types.h"

#define PARTICLE_STATE_ALIVE 0x2
#define PARTICLE_STATE_DEAD 0x3
#define PARTICLE_STATE_DESTROYED 0x4

#ifdef PLATFORM_WIN
#define PACKED_FIELD(type) PackedType<type>
#pragma clang optimize off
template<typename T>
struct PackedType
{
	int32_t index;

	T Get() const
	{
		if (index == 0) {
			return (T)0x0;
		}

		return LOAD_POINTER_CAST(T, index);
	}

	T operator=(T value)
	{
		if (value == (T)0x0) {
			index = 0;
			return value;
		}

		index = STORE_POINTER(value);
		return value;
	}

	// Define == operator for comparison
	bool operator==(T* other) const
	{
		return Get() == other;
	}

	// Define != operator for comparison
	bool operator!=(T* other) const
	{
		return Get() != other;
	}


	// Define -> operator for direct assignment
	T operator->() const
	{
		return Get();
	}

	// Implicit conversion operator
	operator T() const
	{
		return Get();
	}
};

static_assert(sizeof(PackedType<void*>) == 4, "PackedType<void*> size is not 4 bytes.");

#else
#define PACKED_FIELD(type) type
#define LOAD_PACKED_FIELD(name) name
#endif

template<typename T>
union OffsetPointer
{
	PACKED_FIELD(T) pData;
	int offset;
};
#pragma clang optimize on

#define PARTICLE_SHAPE_BOX 1
#define PARTICLE_SHAPE_SPHERE 2
#define PARTICLE_SHAPE_CYLINDER 3
#define PARTICLE_SHAPE_CYLINDER_BOX 4
#define PARTICLE_SHAPE_CONE 5
#define PARTICLE_SHAPE_MESH 6
#define PARTICLE_SHAPE_DOT 8

struct _ed_particle_generator_param;
struct DisplayList;
struct edNODE;
struct ed_g2d_manager;
struct edDList_material;
struct ed_3D_Scene;

struct _ed_particle;

struct _ed_particle_cube
{
	int field_0x0;
	float field_0x4;
	int field_0x8;
	undefined4 field_0xc;
	float field_0x10;
	float field_0x14;
	float field_0x18;
	undefined4 field_0x1c;
	PackedType<_ed_particle**> field_0x20;
};

struct _ed_particle_meshe_param
{
	int field_0x0;
	int field_0x4;
	int field_0x8;
	int field_0xc;
	PackedType<_ed_particle**> field_0x10;
	int field_0x14;
	undefined4 field_0x18;
	PackedType<_ed_particle_cube*> field_0x1c;
};

struct _ed_particle
{
	byte field_0x0;
	byte field_0x1;
	byte field_0x2;
	byte state;
	byte field_0x4;
	byte field_0x5;
	short field_0x6;

	union 	{
		struct
		{
			byte field_0x8;
			byte field_0x9;
			byte field_0xa;
			byte field_0xb;
		};

		uint field_0x8_uint; // Used for bitwise operations
	};
	
	byte field_0xc;
	byte field_0xd;
	byte field_0xe;
	byte field_0xf;
	byte field_0x10;
	byte field_0x11;
	byte field_0x12;
	byte field_0x13;

	float field_0x14;
	float field_0x18;
	float age;
	float lifetime;
	float yScale;
	float field_0x28;
	float field_0x2c;
	float field_0x30;
	PackedType<edNODE*> pNode;
	PackedType<_ed_particle_cube*> field_0x38;
	uint seed;
};

static_assert(sizeof(_ed_particle) == 0x40, "Size of _ed_particle is not 0x40 bytes.");

struct _ed_particle_selector_param
{
	byte field_0x0;
	byte field_0x1;
	byte field_0x2;
	byte field_0x3;

	byte _pad_0_[0x4];

	byte field_0x8;

	byte _pad_1_[0x4];
	
	int field_0x10;
	OffsetPointer<OffsetPointer<void*>*> field_0x14;

	byte _pad_2_[0x8];
};

static_assert(sizeof(_ed_particle_selector_param) == 0x20, "Size of _ed_particle_selector_param is not 0x20 bytes.");

struct _ed_particle_shaper_param
{
	byte field_0x0;
	byte field_0x1;
	byte field_0x2;
	byte field_0x3;
	byte field_0x4;
	byte field_0x5;
	byte field_0x6;
	byte field_0x7;
	byte field_0x8;
	byte field_0x9;
	byte field_0xa;
	byte field_0xb;
	int field_0xc;

	byte _pad_10_[0x10];

	edF32VECTOR3 field_0x20;

	byte _pad_0_[0x14];

	OffsetPointer<ed_g2d_manager*> field_0x40;
	OffsetPointer<edDList_material*> field_0x44;
	OffsetPointer<ulong*> field_0x48;

	byte _pad_1_[0x24];

	float aspectRatio;

	byte _pad_3_[0xc];
	
	ushort field_0x80;
	short field_0x82;
	short field_0x84;
	short field_0x86;

	float field_0x88;
	float field_0x8c;
	float field_0x90;
	float field_0x94;

	int field_0x98;

	uint field_0x9c[8];

	byte _pad_2_[0x4];

	float field_0xc0;

	union 
	{
		struct
		{
			byte field_0xc4;
			byte field_0xc5;
			byte field_0xc6;
			byte field_0xc7;
		};

		int field_0xc4_uint; // Used for bitwise operations
	};

	byte _pad_4_[0x8];

	edF32MATRIX4 field_0xd0;
	undefined4 field_0x110;
	float field_0x114;
	int field_0x118;
	int field_0x11c;
	uint field_0x120;
	PackedType<DisplayList*> field_0x124[26];
	byte field_0x18c;
	byte field_0x18d;
	byte field_0x18e;
	byte field_0x18f;
	undefined4 field_0x190;
	OffsetPointer<void*> field_0x194;
	PackedType<ed_3D_Scene*> field_0x198;
	undefined4 field_0x19c;
};

static_assert(sizeof(_ed_particle_shaper_param) == 0x1a0, "Size of _ed_particle_shaper_param is not 0x1a0 bytes.");

struct GeneratorFunc
{
	int pPosFunc;

	byte _pad_4_[0x4];

	byte field_0x8;
	byte field_0x9;
	byte field_0xa;
	byte field_0xb;
	byte field_0xc;
	byte field_0xd;
	byte field_0xe;
	byte field_0xf;
	byte field_0x10;
	byte field_0x11;
	byte field_0x12;
	byte field_0x13;
	byte field_0x14;
	byte field_0x15;
	byte field_0x16;
	byte field_0x17;

	float field_0x18;

	byte _pad_1_[0x4];

	uint field_0x20;
	float field_0x24;

	float field_0x28;
	float field_0x2c;
	float field_0x30;

	byte _pad_6_[0xc];
	int field_0x40;
};

struct _ed_particle_effector_param_10
{
	uint flags;
	float field_0x4;
	int field_0x8;
	OffsetPointer<float*> field_0xc;
};

struct _ed_particle_effector_param
{
	byte field_0x0;
	byte field_0x1;
	byte field_0x2;
	byte field_0x3;

	byte _pad_0_[0x4];

	ulong hash;

	edF32VECTOR4 field_0x10;
	edF32VECTOR4 field_0x20;
	edF32VECTOR4 position;

	edF32MATRIX4 field_0x40;
	edF32MATRIX4 field_0x80;
	edF32MATRIX4 field_0xc0;

	byte _pad_1_[0x40];

	float field_0x140;

	byte _pad_5_[0xc];

	edF32VECTOR4 field_0x150;

	edF32VECTOR4 field_0x160;

	edF32VECTOR4 field_0x170;

	byte _pad_7_[0x10];

	float field_0x190;
	float field_0x194;
	float field_0x198;
	float field_0x19c;

	byte _pad_6_[0x10];

	_ed_particle_effector_param_10 aSubParams[6];

	undefined4 field_0x210;
	int nbSubParams;
	int field_0x218;

	byte _pad_3_[0x8];

	byte field_0x224;
	byte field_0x225;
	ushort field_0x226;

	byte _pad_2_[0x8];
};

static_assert(sizeof(_ed_particle_effector_param) == 0x230, "Size of _ed_particle_effector_param is not 0x230 bytes.");

struct _ed_particle_generator_param
{
	byte field_0x0;
	byte field_0x1;
	byte field_0x2;
	byte field_0x3;
	byte field_0x4;
	byte field_0x5;
	byte field_0x6;
	byte field_0x7;
	uint field_0x8;
	byte field_0xc;
	byte field_0xd;
	byte shape;
	byte field_0xf;

	edF32VECTOR4 scale;
	edF32VECTOR4 rotationEuler;
	edF32VECTOR4 position;
	edF32MATRIX4 field_0x40;
	edF32MATRIX4 field_0x80;
	edF32MATRIX4 field_0xc0;

	float field_0x100;

	byte _pad_3_[0x3c];

	float field_0x140;
	float field_0x144;
	float field_0x148;
	float field_0x14c;
	float field_0x150;
	float field_0x154;
	float field_0x158;
	float field_0x15c;
	float field_0x160;
	int field_0x164;
	int field_0x168;
	int field_0x16c;
	float field_0x170;
	float field_0x174;
	float field_0x178;
	undefined4 field_0x17c;

	float field_0x180;
	float field_0x184;

	int field_0x188;

	byte _pad_0_[0x4];

	GeneratorFunc speedFunc;
	GeneratorFunc posFunc;

	int field_0x218;
	uint field_0x21c;

	undefined4 field_0x220;

	byte field_0x224;
	byte field_0x225;
	short field_0x226;
	float field_0x228;

	undefined4 field_0x22c;
};

static_assert(sizeof(_ed_particle_generator_param) == 0x230, "Size of _ed_particle_generator_param is not 0x230 bytes.");

struct _ed_particle_group
{
	byte field_0x0;
	byte field_0x1;
	byte field_0x2;
	byte field_0x3;
	byte field_0x4;
	byte field_0x5;

	short field_0x6;

	byte field_0x8;
	byte field_0x9;

	byte _pad_0_[0x6];

	int field_0x10;
	int field_0x14;

	OffsetPointer<_ed_particle*> pParticle;

	int field_0x1c;
	int field_0x20;

	OffsetPointer<OffsetPointer<_ed_particle_generator_param*>*> field_0x24;
	int field_0x28;
	int field_0x2c;
	OffsetPointer<OffsetPointer<_ed_particle_effector_param*>*> field_0x30;
	int field_0x34;
	int field_0x38;
	OffsetPointer<OffsetPointer<_ed_particle_selector_param*>*> field_0x3c;
	int field_0x40;
	int field_0x44;
	OffsetPointer<OffsetPointer<_ed_particle_shaper_param*>*> field_0x48;
	float field_0x4c;

	float deltaTime;
	undefined4 field_0x54;
	float field_0x58;
	int field_0x5c;

	edF32VECTOR4 instanceIndex;

	PackedType<void*> field_0x70;
	PackedType<_ed_particle_meshe_param*> field_0x74;

	undefined4 field_0x78;
	undefined4 field_0x7c;

	int field_0x80;

	edF32VECTOR3 field_0x84;
};

static_assert(sizeof(_ed_particle_group) == 0x90, "Size of _ed_particle_group is not 0x90 bytes.");

struct _ed_particle_vectors
{
	edF32VECTOR4 field_0x0;
	edF32VECTOR4 field_0x10;
};

struct _ed_particle_manager
{
	byte bSetup;
	undefined field_0x1;
	byte field_0x2;
	undefined field_0x3;

	float field_0x4;

	undefined field_0x8;
	undefined field_0x9;
	undefined field_0xa;
	undefined field_0xb;

	OffsetPointer<undefined*> field_0xc;
	int field_0x10;

	OffsetPointer<_ed_particle*> aParticles;
	int nbParticles;

	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;

	OffsetPointer<_ed_particle_vectors*> aParticleVectors;
	int nbParticleVectors;

	undefined field_0x28;
	undefined field_0x29;
	undefined field_0x2a;
	undefined field_0x2b;

	OffsetPointer<edF32VECTOR4*> aVectors;
	int nbVectors;

	undefined field_0x34;
	undefined field_0x35;
	undefined field_0x36;
	undefined field_0x37;

	OffsetPointer<_ed_particle_group*> aGroups;
	int nbGroups;
	int nbTotalGroups;

	OffsetPointer<_ed_particle_generator_param*> aGeneratorParams;
	int nbConfigGeneratorParams;
	int nbGeneratorParams;

	OffsetPointer<_ed_particle_effector_param*> aEffectorParams;
	int nbConfigEffectorParams;
	int nbEffectorParams;

	OffsetPointer<_ed_particle_selector_param*> aSelectorParams;
	int nbConfigSelectorParams;
	int nbSelectorParams;

	OffsetPointer<_ed_particle_shaper_param*> aShaperParams;
	int nbConfigShaperParams;
	int nbShaperParams;
};

static_assert(sizeof(_ed_particle_manager) == 0x74, "Size of _ed_particle_manager is not 0x74 bytes.");

struct ParticleFileData
{
	byte pad[0x20];
	_ed_particle_manager manager;
};

static_assert(sizeof(ParticleFileData) == 0x94, "Size of ParticleFileData is not 0x94 bytes.");

struct ed_3D_Scene;
struct ed_g2d_manager;
struct ed_g3d_manager;

struct ed_part_config
{
	int nbParticles;
	int nbGroups;
	int field_0x8;
	int nbGeneratorParams;
	int field_0x10;
	int nbEffectorParams;
	int field_0x18;
	int nbSelectorParams;
	int field_0x20;
	int nbShaperParams;
	int field_0x28;
	int heapId;
};

struct ed_part_system_config
{
	byte bObjectsAllocated;
	byte nbEffectObj;
	short bProfile;
	int heapId;
};

struct ed_particle_system
{
	ed_part_config config;
	_ed_particle_manager manager;
};

void edPartInit();
ed_part_config* edParticlesGetConfig();

void edPartSystemInit();
ed_part_system_config* edParticlesGetSystemConfig();

_ed_particle_manager* edParticlesInstall(ParticleFileData* pFileData, ed_3D_Scene* pScene, ed_g2d_manager* param_3, edDList_material** ppMaterials, ulong* pHashes, int materialIndex, ed_g3d_manager* p3dManager, bool param_8);
void edParticlesSetSystem(_ed_particle_manager* pManager);
void edParticlesUpdate(float time);
void edPartSetDisplayMatrix(_ed_particle_manager* pManager, edF32MATRIX4* pMatrix);
void edParticlesDraw(_ed_particle_manager* pManager, float param_2);
void edParticlesUnInstall(_ed_particle_manager* pManager, ed_3D_Scene* pScene);
void edPartGeneratorComputeMatrices(_ed_particle_generator_param* pParam);
void edPartEffectorComputeMatrices(_ed_particle_effector_param* pParam);
_ed_particle_effector_param* edPartGetEffector(_ed_particle_manager* pManager, char* szName);
void edPartSetRespawning(_ed_particle_manager* pManager, int bRespawning);

void edPart_0027cd80(int param_1);

template<typename T>
T* _edPartGetEntityByHashcode(T* pEntities, int nbEntities, ulong* pHash)
{
	int i = 0;

	for (; (i < nbEntities && (pEntities->hash != *pHash)); pEntities = pEntities + 1) {
		i = i + 1;
	}

	if (i == nbEntities) {
		pEntities = (_ed_particle_effector_param*)0x0;
	}

	return pEntities;
}

#endif // ED_PARTICLES_H