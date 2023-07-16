#include "vu1_emu.h"

#include "port/pointer_conv.h"
#include "port.h"

namespace VU1Emu {

#define FAKE_VU1_MEM_SIZE 0x100000
	char* gFakeMem = nullptr;

	char* GetFakeMem() {
		if (gFakeMem == nullptr) {
			gFakeMem = (char*)malloc(FAKE_VU1_MEM_SIZE);
			memset(gFakeMem, 0, FAKE_VU1_MEM_SIZE);
		}
		return gFakeMem;
	}

	struct alignas(32) WriteTag {
		ushort addr;
		byte count;
		byte type;
	};

	struct alignas(32) RunTag {
		ushort addr;
		byte pad;
		byte cmd;
	};

	struct alignas(32) CyclePacket {
		struct {
			byte cl;
			byte wl;
			byte pad;
			byte cmd;
		} cycle;
		struct {
			byte pad[3];
			byte cmd;
		} mask;
		struct {
			byte mask[4];
		} maskData;
		struct {
			byte pad[3];
			byte cmd;
		} row;
		edF32VECTOR4 rowData;
	};

#define VIF_NOP 0x00
#define VIF_MSCAL 0x14

#define VIF_REG_X 0
#define VIF_REG_Y 1
#define VIF_REG_Z 2
#define VIF_REG_W 3

#define VIF_LOAD_I(reg, row, off) *(int*)(GetFakeMem() + (reg << 4) + (row << 4) + (off << 2))
#define VIF_LOAD_F(reg, off) *(edF32VECTOR4*)(GetFakeMem() + (reg << 4) + (off << 4))

#define VIF_AS_I(reg, row, off) (int*)(GetFakeMem() + (reg << 4) + (row << 4) + (off << 2))
#define VIF_AS_F(reg, off) ((edF32VECTOR4*)(GetFakeMem() + (reg << 4) + (off << 4)))

#define float_to_int4(x)	(s32)((float)x * (1.0f / 0.0625f))
#define float_to_int12(x)	(s32)((float)x * (1.0f / 0.000244140625f))
#define float_to_int15(x)	(s32)((float)x * (1.0f / 0.000030517578125))

#define int4_to_float(x)	(float)((float)x * 0.0625f)
#define int12_to_float(x)	(float)((float)x * 0.000244140625f)
#define int15_to_float(x)	(float)((float)x * 0.000030517578125)

	int vi00 = 0; // Always zero?
	int vi01;
	int vi02;
	int vi03;
	int vi04;
	int vi05;
	int vi06;
	int vi07;
	int vi08;
	int vi09;
	int vi10;
	int vi11;
	int vi12;
	int vi14;
	int vi15;
	int itop;

	edF32VECTOR4 vf01;
	edF32VECTOR4 vf02;
	edF32VECTOR4 vf03;
	edF32VECTOR4 vf04;
	edF32VECTOR4 vf05;
	edF32VECTOR4 vf06;
	edF32VECTOR4 vf07;
	edF32VECTOR4 vf08;
	edF32VECTOR4 vf09;
	edF32VECTOR4 vf10;
	edF32VECTOR4 vf14;
	edF32VECTOR4 vf15;
	edF32VECTOR4 vf17;
	edF32VECTOR4 vf24;

	edF32VECTOR4 ACC;

	void _$DrawingStart_XYZ32() 
	{
		vi15 = itop;
		vi14 = VIF_LOAD_I(vi15, 0, 0);
		vi14 = (vi14 & 0xff);
		if (vi14 == 0) {
			// JUMP
			assert(false);
		}

		vi01 = VIF_LOAD_I(vi15, -1, 0);
	}

	void _$UnpackData_XYZ32_Loop(int count, edF32VECTOR4* pStartAddr) 
	{
		for (int i = 0; i < count; i++) {
			pStartAddr[i].x = int12_to_float(*(int*)&pStartAddr[i].x);
			pStartAddr[i].y = int12_to_float(*(int*)&pStartAddr[i].y);
			pStartAddr += 2;
		}
	}

	void _$UnpackData_XYZ32() 
	{
		vi03 = vi15 + 1;
		_$UnpackData_XYZ32_Loop(vi14, VIF_AS_F(vi03, 0));
	}

	void _$GouraudMapping_No_Fog_16_2() 
	{
		vi03 = vi15 + 1;
		vf05 = VIF_LOAD_F(vi14, 1);
		vf17 = VIF_LOAD_F(vi03, 2);

		vf04 = VIF_LOAD_F(vi00, 19);
		vf03 = VIF_LOAD_F(vi00, 18);
		vf02 = VIF_LOAD_F(vi00, 17);
		vf01 = VIF_LOAD_F(vi00, 16);
	}

	// Overload the * operator as a non-member function
	edF32VECTOR4 operator*(const edF32VECTOR4& lhs, const edF32VECTOR4& rhs) 
	{
		edF32VECTOR4 ret;
		ret.x = lhs.x * rhs.x;
		ret.y = lhs.y * rhs.y;
		ret.z = lhs.z * rhs.z;
		ret.w = lhs.w * rhs.w;
		return ret;
	}

	edF32VECTOR4 operator*(const edF32VECTOR4& lhs, const float& rhs) 
	{
		edF32VECTOR4 ret;
		ret.x = lhs.x * rhs;
		ret.y = lhs.y * rhs;
		ret.z = lhs.z * rhs;
		ret.w = lhs.w * rhs;
		return ret;
	}

	edF32VECTOR4 operator+(const edF32VECTOR4& lhs, const edF32VECTOR4& rhs) 
	{
		edF32VECTOR4 ret;
		ret.x = lhs.x + rhs.x;
		ret.y = lhs.y + rhs.y;
		ret.z = lhs.z + rhs.z;
		ret.w = lhs.w + rhs.w;
		return ret;
	}

	edF32VECTOR4 operator-(const edF32VECTOR4& lhs, const edF32VECTOR4& rhs)
	{
		edF32VECTOR4 ret;
		ret.x = lhs.x - rhs.x;
		ret.y = lhs.y - rhs.y;
		ret.z = lhs.z - rhs.z;
		ret.w = lhs.w - rhs.w;
		return ret;
	}

	uint gClipflag = 0;

	uint Clip(float value, const edF32VECTOR4& reg)
	{
		gClipflag <<= 6;
		if (reg.x > +value) gClipflag |= 0x01;
		if (reg.x < -value) gClipflag |= 0x02;
		if (reg.y > +value) gClipflag |= 0x04;
		if (reg.y < -value) gClipflag |= 0x08;
		if (reg.z > +value) gClipflag |= 0x10;
		if (reg.z < -value) gClipflag |= 0x20;
		gClipflag = gClipflag & 0xFFFFFF;
		return gClipflag;
	}

	void _$Culling_12_1()
	{
		vi02 = vi14;
		vi03 = vi15 + 1;
		vi04 = -24576;

		vf04 = VIF_LOAD_F(vi00, 11);
		vf03 = VIF_LOAD_F(vi00, 10);
		vf02 = VIF_LOAD_F(vi00, 9);
		vf01 = VIF_LOAD_F(vi00, 8);

		for (vi02 = vi02; vi02 > 0; vi02 -= 3) {
			vf05 = VIF_LOAD_F(vi03, 2);
			vf06 = VIF_LOAD_F(vi03, 5);
			vf07 = VIF_LOAD_F(vi03, 8);

			ACC = vf04 + vf03 * vf05.z + vf02 * vf05.y;
			vf08 = ACC + vf01 * vf05.x;

			ACC = vf04 + vf03 * vf06.z + vf02 * vf06.y;
			vf09 = ACC + vf01 * vf06.x;

			ACC = vf04 + vf03 * vf07.z + vf02 * vf07.y;
			vf10 = ACC + vf01 * vf07.x;

			if ((Clip(vf08.w, vf08) & 63) != 0) {
				auto pFlag = VIF_AS_I(vi03, 2, VIF_REG_W);
				*pFlag |= vi04;
			}

			if ((Clip(vf09.w, vf09) & 63) != 0) {
				auto pFlag = VIF_AS_I(vi03, 5, VIF_REG_W);
				*pFlag |= vi04;
			}

			if ((Clip(vf10.w, vf10) & 63) != 0) {
				auto pFlag = VIF_AS_I(vi03, 8, VIF_REG_W);
				*pFlag |= vi04;
			}

			vi03 += 9;
		}
	}

	void _$ClippingRejection_12_1()
	{
		vi02 = vi14;
		vi03 = vi15 + 1;
		vi04 = -24576;

		vf04 = VIF_LOAD_F(vi00, 11);
		vf03 = VIF_LOAD_F(vi00, 10);
		vf02 = VIF_LOAD_F(vi00, 9);
		vf01 = VIF_LOAD_F(vi00, 8);

		for (vi02 = vi02; vi02 > 0; vi02 -= 3) {
			vf05 = VIF_LOAD_F(vi03, 2);
			vf06 = VIF_LOAD_F(vi03, 5);
			vf07 = VIF_LOAD_F(vi03, 8);

			ACC = vf04 + vf03 * vf05.z + vf02 * vf05.y;
			vf08 = ACC + vf01 * vf05.x;

			ACC = vf04 + vf03 * vf06.z + vf02 * vf06.y;
			vf09 = ACC + vf01 * vf06.x;

			ACC = vf04 + vf03 * vf07.z + vf02 * vf07.y;
			vf10 = ACC + vf01 * vf07.x;

			if ((Clip(vf08.w, vf08) & 63) != 0) {
				auto pFlag = VIF_AS_I(vi03, 2, VIF_REG_W);
				*pFlag |= vi04;
			}

			if ((Clip(vf09.w, vf09) & 63) != 0) {
				auto pFlag = VIF_AS_I(vi03, 5, VIF_REG_W);
				*pFlag |= vi04;
			}

			if ((Clip(vf10.w, vf10) & 63) != 0) {
				auto pFlag = VIF_AS_I(vi03, 8, VIF_REG_W);
				*pFlag |= vi04;
			}

			vi03 += 9;
		}
	}

#define VIF_F_TO_I(a) (*reinterpret_cast<uint*>(&a)) & 0xFFFF

	void RunCode(int addr) 
	{
		if (addr == 0x3) {
			_$DrawingStart_XYZ32();
			_$UnpackData_XYZ32();
			
			if ((vi01 & 0x400) != 0) {
				// _$XYZW_16_Conv
				IMPLEMENTATION_GUARD();
			}

			vi02 = VIF_LOAD_I(vi15, -1, VIF_REG_Y);

			if ((vi02 & 0x1) != 0) {
				// ???
				IMPLEMENTATION_GUARD();
			}

			if ((vi01 & 0x8) != 0) {
				// _$BF_Culling
				IMPLEMENTATION_GUARD();
			}

			if ((vi01 & 0x20) != 0) {
				// _$Alpha_Object
				IMPLEMENTATION_GUARD();
			}

			if ((vi01 & 0x100) != 0) {
				// _$Normal_Extruder
				IMPLEMENTATION_GUARD();
			}

			if ((vi01 & 0x4) != 0) {
				// _$Animation_RGBA
				IMPLEMENTATION_GUARD();
			}

			if ((vi01 & 0x200) != 0) {
				// _$Animation_ST
				IMPLEMENTATION_GUARD();
			}

			if ((vi01 & 0x1) != 0) {
				if ((vi01 & 0x2) != 0) {
					_$Culling_12_1();
					_$ClippingRejection_12_1();

				}
				else {
					// _$NoClipCulling_12_1
					IMPLEMENTATION_GUARD();
				}
			}

			vi14 = 2;
			_$GouraudMapping_No_Fog_16_2();
		}
	}

	struct WriteEntry 
	{
		uint vuaddr;
		uint num;
		edpkt_data* data;
	};

	std::vector<WriteEntry> gWriteQueue;

	void ProcessWriteQueue()
	{
		auto p = GetFakeMem();

		for (auto& entry : gWriteQueue) {
			if (entry.data->asU32[1] == 0xe) {
				memcpy(p + (entry.vuaddr << 4), (entry.data) + 1, entry.num * sizeof(edpkt_data));
			}
			else {
				void* altSrc = LOAD_SECTION(entry.data->asU32[1]);
				memcpy(p + (entry.vuaddr << 4), altSrc, entry.num * sizeof(edpkt_data));
			}
		}

		gWriteQueue.clear();
	}

	void UnpackToAddr(uint addr, void* data, int size)
	{
		auto p = GetFakeMem();
		memcpy(p + (addr << 4), data, size);
	}
}

void VU1Emu::ProcessVifList(edpkt_data* pVifPkt) 
{
	int stcl = 0;
	int stwl = 0;
	edF32VECTOR4 strow = { 0.0f, 0.0f, 0.0f, 0.0f };
	char* pFakeMem = GetFakeMem();

	ProcessWriteQueue();

	while (pVifPkt->cmdA != 0x60000000) {
		RunTag* pRunTag = (RunTag*)(&(pVifPkt->asU32[3]));

		if (pVifPkt->asU32[3] == VIF_NOP) {
			// Execute commands at addr
			CyclePacket* pCycleMaskCmd = (CyclePacket*)LOAD_SECTION(pVifPkt->asU32[1]);
			stcl = pCycleMaskCmd->cycle.cl;
			stwl = pCycleMaskCmd->cycle.wl;

			if (pCycleMaskCmd->row.cmd == 0x30) {
				strow = pCycleMaskCmd->rowData;
			}
		}
		else if (pRunTag->cmd == VIF_MSCAL) {
			RunCode(pRunTag->addr);
		}
		else {
			WriteTag* pTag = (WriteTag*)(&(pVifPkt->asU32[3]));

			char* pWriteStart = pFakeMem + (((pTag->addr + itop) << 4) & 0x3ff0);

			const int dataSize = 0x10;

			switch (pTag->type) {
			case 0x7c:
			case 0x6c:
			{
				char* pUnpack = (char*)LOAD_SECTION(pVifPkt->asU32[1]);

				for (int i = 0; i < pTag->count; i += 1) {
					void* const pStart = pWriteStart + (i * dataSize * stcl);

					memcpy(pStart, pUnpack + (i * 0x10), 0x10);
				}
			}
			break;
			case 0x75:
			{
				short* pUnpack = (short*)LOAD_SECTION(pVifPkt->asU32[1]);

				for (int i = 0; i < pTag->count; i += 1) {
					void* const pStart = pWriteStart + (i * dataSize * stcl);

					edF32VECTOR4* pV = (edF32VECTOR4*)pStart;
					*pV = strow;

					int* pI = (int*)pStart;
					short* pU = pUnpack + (i * 2);
					pI[0] = pU[0];
					pI[1] = pU[1];
				}
			}
			break;
			case 0x7e:
			{
				byte* pUnpack = (byte*)LOAD_SECTION(pVifPkt->asU32[1]);

				for (int i = 0; i < pTag->count; i += 1) {
					void* const pStart = pWriteStart + (i * dataSize * stcl);

					byte* pB = pUnpack + (i * 4);
					int* pI = (int*)pStart;
					pI[0] = pB[0];
					pI[1] = pB[1];
					pI[2] = pB[2];
					pI[3] = pB[3];
				}
			}
			break;
			}
		}

		pVifPkt++;
	}

	//free(pFakeMem);
}

void VU1Emu::SetVifItop(uint newItop)
{
	itop = newItop;
}

#define CHECK_VIF_CMD(number, cmd) (((unsigned int)number >> (3 << 3)) & 0xff) == cmd
#define GET_UNPACK_LENGTH(number) ((unsigned int)number >> (2 << 3)) & 0xff

void VU1Emu::UpdateMemory(edpkt_data* pVifPkt, edpkt_data* pEnd)
{
	while (pVifPkt < pEnd) {
		WriteTag* pTag = (WriteTag*)(&(pVifPkt->asU32[3]));

		if (pTag->type == UNPACK_V4_32 || pTag->type == UNPACK_V4_32_MASKED) {
			if (pVifPkt->asU32[1] == 0xe) {
				UnpackToAddr(pTag->addr, pVifPkt + 1, pTag->count * 0x10);
			}
			else {
				UnpackToAddr(pTag->addr, LOAD_SECTION(pVifPkt->asU32[1]), pTag->count * 0x10);
			}
		}
		pVifPkt++;
	}
}

void VU1Emu::QueueWrite(uint vuaddr, uint num, void* data)
{
	gWriteQueue.push_back({ vuaddr, num, (edpkt_data*)data });
}

void VU1Emu::ClearWriteQueue()
{
	gWriteQueue.clear();
}

//void VU1Emu::UnpackToAddr(uint addr, void* data, int size)
//{
//	auto p = GetFakeMem();
//	memcpy(p + (addr << 4), data, size);
//}
