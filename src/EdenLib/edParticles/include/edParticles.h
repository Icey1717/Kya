#ifndef ED_PARTICLES_H
#define ED_PARTICLES_H

#include "Types.h"

#ifdef PLATFORM_WIN
#define PACKED_FIELD(type) PackedType<type>

template<typename T>
struct PackedType
{
	int32_t index;

	T Get() const
	{
		if (index == 0) {
			return (T)0x0;
		}

		return LOAD_SECTION_CAST(T, index);
	}

	T operator=(T value)
	{
		if (value == (T)0x0) {
			index = 0;
			return value;
		}

		index = STORE_SECTION(value);
		return Get();
	}

	// Define -> operator for direct assignment
	T operator->()
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

struct _ed_particle_generator_param;
struct DisplayList;
struct edNODE;
struct ed_g2d_manager;
struct edDList_material;
struct ed_3D_Scene;

struct _ed_particle
{
	byte field_0x0;
	byte field_0x1;
	byte field_0x2;
	byte field_0x3;
	byte field_0x4;
	byte field_0x5;
	short field_0x6;
	byte _pad_2_[0x5];
	byte field_0xd;
	byte field_0xe;
	byte field_0xf;

	byte _pad_0_[0xc];

	float field_0x1c;
	float field_0x20;
	float field_0x24;
	byte _pad_3_[0x8];
	float field_0x30;
	PackedType<edNODE*> pNode;
	byte _pad_4_[0x8];
};

static_assert(sizeof(_ed_particle) == 0x40, "Size of _ed_particle is not 0x40 bytes.");

struct Particle_0x20
{
	byte field_0x0;
	byte field_0x1;
	byte field_0x2;
	byte field_0x3;

	byte _pad_0_[0x4];

	byte field_0x8;

	byte _pad_1_[0x4];
	
	int field_0x10;
	OffsetPointer<OffsetPointer<_ed_particle_generator_param*>*> field_0x14;

	byte _pad_2_[0x8];
};

static_assert(sizeof(Particle_0x20) == 0x20, "Size of Particle_0x20 is not 0x20 bytes.");

struct Particle_0x1a0
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

	byte _pad_0_[0x30];

	OffsetPointer<ed_g2d_manager*> field_0x40;
	OffsetPointer<OffsetPointer<edDList_material*>*> field_0x44;
	OffsetPointer<ulong*> field_0x48;

	byte _pad_1_[0x24];

	float field_0x70;

	byte _pad_2_[0x5c];

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

static_assert(sizeof(Particle_0x1a0) == 0x1a0, "Size of Particle_0x1a0 is not 0x1a0 bytes.");

struct _ed_particle_generator_param
{
	byte field_0x0;
	byte field_0x1;
	byte field_0x2;
	byte field_0x3;
	byte _pad_0_[0x3c];

	edF32MATRIX4 field_0x40;
	edF32MATRIX4 field_0x80;
	edF32MATRIX4 field_0xc0;

	float field_0x100;

	byte _pad_1_[0x110];

	int field_0x214;
	undefined4 field_0x218;
	int field_0x21c;

	byte _pad_2_[0x10];
};

static_assert(sizeof(_ed_particle_generator_param) == 0x230, "Size of _ed_particle_generator_param is not 0x230 bytes.");

struct _ed_particle_group
{
	byte field_0x0;
	byte field_0x1;
	byte field_0x2;
	byte field_0x3;

	short field_0x4;
	short field_0x6;

	byte _pad_0_[0x8];

	int field_0x10;

	undefined4 field_0x14;

	OffsetPointer<_ed_particle*> pParticle;

	undefined4 field_0x1c;

	int field_0x20;

	OffsetPointer<OffsetPointer<_ed_particle_generator_param*>*> field_0x24;
	int field_0x28;
	int field_0x2c;
	OffsetPointer<OffsetPointer<_ed_particle_generator_param*>*> field_0x30;
	int field_0x34;
	int field_0x38;
	OffsetPointer<OffsetPointer<Particle_0x20*>*> field_0x3c;
	int field_0x40;
	int field_0x44;
	OffsetPointer<OffsetPointer<Particle_0x1a0*>*> field_0x48;
	int field_0x4c;

	byte _pad_2_[0x8];
	float field_0x58;
	int field_0x5c;

	byte _pad_3_[0x10];

	PackedType<Particle_0x1a0*> field_0x70;

	undefined4 field_0x74;
	undefined4 field_0x78;
	undefined4 field_0x7c;

	int field_0x80;

	float field_0x84;
	float field_0x88;
	float field_0x8c;
};

static_assert(sizeof(_ed_particle_group) == 0x90, "Size of _ed_particle_group is not 0x90 bytes.");

struct _ed_particle_vectors
{

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
	int field_0x24;

	undefined field_0x28;
	undefined field_0x29;
	undefined field_0x2a;
	undefined field_0x2b;

	OffsetPointer<edF32VECTOR4*> field_0x2c;
	int field_0x30;

	undefined field_0x34;
	undefined field_0x35;
	undefined field_0x36;
	undefined field_0x37;

	OffsetPointer<_ed_particle_group*> aGroups;
	int nbGroups;

	int nbParams;

	OffsetPointer<_ed_particle_generator_param*> aParams;
	int field_0x48;

	int field_0x4c;

	OffsetPointer<_ed_particle_generator_param*> field_0x50;
	int field_0x54;

	int field_0x58;

	OffsetPointer<Particle_0x20*> field_0x5c;
	int field_0x60;

	int field_0x64;

	OffsetPointer<Particle_0x1a0*> field_0x68;
	int field_0x6c;
	int field_0x70;
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

_ed_particle_manager* edParticlesInstall(ParticleFileData* pFileData, ed_3D_Scene* pScene, ed_g2d_manager* param_3, edDList_material** ppMaterials, ulong* pHashes, int materialIndex, ed_g3d_manager* p3dManager, bool param_8);
void edParticlesSetSystem(_ed_particle_manager* pManager);
void edParticlesUpdate(float time);
void edPartSetDisplayMatrix(_ed_particle_manager* pManager, edF32MATRIX4* pMatrix);
void edParticlesDraw(_ed_particle_manager* pManager, float param_2);

#endif // ED_PARTICLES_H