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
	edF32VECTOR4 vf11;
	edF32VECTOR4 vf12;
	edF32VECTOR4 vf13;
	edF32VECTOR4 vf14;
	edF32VECTOR4 vf15;
	edF32VECTOR4 vf16;
	edF32VECTOR4 vf17;
	edF32VECTOR4 vf18;
	edF32VECTOR4 vf19;
	edF32VECTOR4 vf20;
	edF32VECTOR4 vf21;
	edF32VECTOR4 vf22;
	edF32VECTOR4 vf24;
	edF32VECTOR4 vf26;
	edF32VECTOR4 vf27;
	edF32VECTOR4 vf28;
	edF32VECTOR4 vf29;
	edF32VECTOR4 vf30;
	edF32VECTOR4 vf31;

	edF32VECTOR4 ACC;
	float Q;

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

		// Obj to Culling matrix
		vf04 = VIF_LOAD_F(vi00, 11);
		vf03 = VIF_LOAD_F(vi00, 10);
		vf02 = VIF_LOAD_F(vi00, 9);
		vf01 = VIF_LOAD_F(vi00, 8);

		vi06 = 63;
		vi07 = 63;
		vi09 = 63;

		for (vi02 = vi02; vi02 > 0; vi02 -= 3) {
			vf05 = VIF_LOAD_F(vi03, 2);
			vf06 = VIF_LOAD_F(vi03, 5);
			vf07 = VIF_LOAD_F(vi03, 8);

			vf08 = vf04 + (vf03 * vf05.z) + (vf02 * vf05.y) + (vf01 * vf05.x);
			vf09 = vf04 + (vf03 * vf06.z) + (vf02 * vf06.y) + (vf01 * vf06.x);
			vf10 = vf04 + (vf03 * vf07.z) + (vf02 * vf07.y) + (vf01 * vf07.x);

			vi05 = Clip(vf08.w, vf08) & vi09;
			vi08 = vi05 & vi06;
			vi08 = vi08 & vi07;

			if (vi08 != 0) {
				uint* pFlag = (uint*)VIF_AS_I(vi03, 2, VIF_REG_W);
				*pFlag |= 0xa000;
			}

			vi06 = Clip(vf09.w, vf09) & vi09;
			vi08 = vi05 & vi06;
			vi08 = vi08 & vi07;

			if (vi08 != 0) {
				uint* pFlag = (uint*)VIF_AS_I(vi03, 5, VIF_REG_W);
				*pFlag |= 0xa000;
			}

			vi07 = Clip(vf10.w, vf10) & vi09;
			vi08 = vi05 & vi06;
			vi08 = vi08 & vi07;

			if (vi08 != 0) {
				uint* pFlag = (uint*)VIF_AS_I(vi03, 8, VIF_REG_W);
				*pFlag |= 0xa000;
			}

			vi03 += 9;
		}
	}

	void _$ClippingRejection_12_1()
	{
		vi02 = vi14;
		vi03 = vi15 + 1;

		// Obj to Clipping matrix
		vf04 = VIF_LOAD_F(vi00, 15);
		vf03 = VIF_LOAD_F(vi00, 14);
		vf02 = VIF_LOAD_F(vi00, 13);
		vf01 = VIF_LOAD_F(vi00, 12);

		vi06 = 0;
		vi07 = 0;
		vi09 = 63;

		for (vi02 = vi02; vi02 > 0; vi02 -= 3) {
			vf05 = VIF_LOAD_F(vi03, 2);
			vf06 = VIF_LOAD_F(vi03, 5);
			vf07 = VIF_LOAD_F(vi03, 8);

			vf08 = vf04 + (vf03 * vf05.z) + (vf02 * vf05.y) + (vf01 * vf05.x);
			vf09 = vf04 + (vf03 * vf06.z) + (vf02 * vf06.y) + (vf01 * vf06.x);
			vf10 = vf04 + (vf03 * vf07.z) + (vf02 * vf07.y) + (vf01 * vf07.x);

			vi05 = Clip(vf08.w, vf08) & vi09;
			vi08 = vi05 | vi06;
			vi08 = vi08 | vi07;

			if (vi08 != 0) {
				uint* pFlag = (uint*)VIF_AS_I(vi03, 2, VIF_REG_W);
				*pFlag |= 0x9000 | vi05;
			}

			vi06 = Clip(vf09.w, vf09) & vi09;
			vi08 = vi05 | vi06;
			vi08 = vi08 | vi07;

			if (vi08 != 0) {
				uint* pFlag = (uint*)VIF_AS_I(vi03, 5, VIF_REG_W);
				*pFlag |= 0x9000 | vi06;
			}

			vi07 = Clip(vf10.w, vf10) & vi09;
			vi08 = vi05 | vi06;
			vi08 = vi08 | vi07;

			if (vi08 != 0) {
				uint* pFlag = (uint*)VIF_AS_I(vi03, 8, VIF_REG_W);
				*pFlag |= 0x9000 | vi07;
			}

			vi03 += 9;
		}
	}

#define VIF_F_TO_I4(a) (*reinterpret_cast<const uint*>(&a)) & 0xFFFF
#define VIF_F_TO_I(a) *reinterpret_cast<const uint*>(&a)

	edF32VECTOR4 ConvertFromInt(const edF32VECTOR4& reg) {
		edF32VECTOR4 ret = {
			(float)(VIF_F_TO_I(reg.x)),
			(float)(VIF_F_TO_I(reg.y)),
			(float)(VIF_F_TO_I(reg.z)),
			(float)(VIF_F_TO_I(reg.w))
		};
		return ret;
	}

	void _$Clipping()
	{
		// Obj to Screen matrix
		vf04 = VIF_LOAD_F(vi00, 19);
		vf03 = VIF_LOAD_F(vi00, 18);
		vf02 = VIF_LOAD_F(vi00, 17);
		vf01 = VIF_LOAD_F(vi00, 16);

		// gFANbuffers
		vf28 = VIF_LOAD_F(vi00, 0);

		// Flags?
		vf29 = VIF_LOAD_F(vi00, 1);

		// gClipMulVector
		vf30 = VIF_LOAD_F(vi00, 2);

		// gClipAddVector
		vf31 = VIF_LOAD_F(vi00, 3);

		// gClipXY
		vf27 = VIF_LOAD_F(vi00, 4);

		vi02 = vi14;
		vi03 = vi15 + 1;

		for (vi02 = vi02; vi02 > 0; vi02 -= 3) {

			vf07 = VIF_LOAD_F(vi03, 2);
			vf06 = VIF_LOAD_F(vi03, 1);
			vf05 = VIF_LOAD_F(vi03, 0);

			vf10 = VIF_LOAD_F(vi03, 5);
			vf09 = VIF_LOAD_F(vi03, 4);
			vf08 = VIF_LOAD_F(vi03, 3);

			vi12 = 32;

			vi07 = vi12 & VIF_F_TO_I(vf07.w);
			vf07 = vf04 + (vf03 * vf07.z) + (vf02 * vf07.y) + (vf01 * vf07.x);

			vi08 = vi12 & VIF_F_TO_I(vf10.w);
			vf10 = vf04 + (vf03 * vf10.z) + (vf02 * vf10.y) + (vf01 * vf10.x);

			vf06 = ConvertFromInt(vf06);
			vf09 = ConvertFromInt(vf09);

			vi06 = 63;
			vi12 = 32;

			vf13 = VIF_LOAD_F(vi03, 8);
			vf12 = VIF_LOAD_F(vi03, 7);
			vf11 = VIF_LOAD_F(vi03, 6);

			vi09 = VIF_F_TO_I(vf13.w);
			vi01 = 0xf000 & vi09;
			vi09 = vi12 & vi09;

			vf13 = vf04 + (vf03 * vf13.z) + (vf02 * vf13.y) + (vf01 * vf13.x);
			vf12 = ConvertFromInt(vf12);

			vi03 += 3;

			if (vi01 != 0x9000) {
				continue;
			}

			//IMPLEMENTATION_GUARD();
		}
	}

	void _$GouraudMapping_No_Fog_16_2()
	{
		vf05 = VIF_LOAD_F(vi00, 1);

		// Obj to Screen matrix
		vf04 = VIF_LOAD_F(vi00, 19);
		vf03 = VIF_LOAD_F(vi00, 18);
		vf02 = VIF_LOAD_F(vi00, 17);
		vf01 = VIF_LOAD_F(vi00, 16);

		vi02 = vi14;
		vi03 = vi15 + 1;

		for (vi02 = vi02; vi02 > 0; vi02 -= 1) {
			vf17 = VIF_LOAD_F(vi03, 0);
			vf27 = VIF_LOAD_F(vi03, 2);

			vf18 = vf04 + (vf03 * vf17.z) + (vf02 * vf17.y) + (vf01 * vf17.x);
			vf28 = vf04 + (vf03 * vf27.z) + (vf02 * vf27.y) + (vf01 * vf27.x);
			Q = 1.0f / vf18.w;

			vf19.x = vf18.x * Q;
			vf19.y = vf18.y * Q;
			vf19.z = vf18.z * Q;

			Q = 1.0f / vf28.w;
			vf21 = vf17 * Q;

			vf29.x = vf28.x * Q;
			vf29.y = vf28.y * Q;
			vf29.z = vf28.z * Q;

			*reinterpret_cast<int*>(&vf31.x) = float_to_int4(vf29.x);
			*reinterpret_cast<int*>(&vf31.y) = float_to_int4(vf29.y);
			*reinterpret_cast<int*>(&vf31.z) = (int)vf29.z;

			auto base = VIF_AS_F(vi03, 0);
			*VIF_AS_F(vi03, 0) = vf21;

			uint skip = *VIF_AS_I(vi03, 2, VIF_REG_W);
			*VIF_AS_F(vi03, 2) = vf31;

			int x = float_to_int4(vf29.x);
			int y = float_to_int4(vf29.y);
			int z = (float)vf29.z;

			Renderer::SetRGBAQ(
				*VIF_AS_I(vi03, 1, VIF_REG_X),
				*VIF_AS_I(vi03, 1, VIF_REG_Y),
				*VIF_AS_I(vi03, 1, VIF_REG_Z),
				*VIF_AS_I(vi03, 1, VIF_REG_W), 
				vf21.z);

			Renderer::SetST(vf21.x, vf21.y);
			Renderer::SetVertexSkip(skip & 0x8000);
			Renderer::KickVertex(x, y, z);

			vi03 += 3;
		}
	}

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
				//IMPLEMENTATION_GUARD();
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
				//IMPLEMENTATION_GUARD();
			}

			if ((vi01 & 0x1) != 0) {
				if ((vi01 & 0x2) != 0) {
					_$Culling_12_1();
					_$ClippingRejection_12_1();
					_$Clipping();

				}
				else {
					// _$NoClipCulling_12_1
					//IMPLEMENTATION_GUARD();
				}
			}

			_$GouraudMapping_No_Fog_16_2();
		}
	}

	void UnpackToAddr(uint addr, void* data, int size)
	{
		const auto dst = GetFakeMem() + (addr << 4);
		memcpy(dst, data, size);
	}

	struct HW_Gif_Tag {
		u16 NLOOP : 15;
		u16 EOP : 1;
		u16 _dummy0 : 16;
		u32 _dummy1 : 14;
		u32 PRE : 1;
		u32 PRIM : 11;
		u32 FLG : 2;
		u32 NREG : 4;
		u32 REGS[2];
	};
}

void VU1Emu::ProcessVifList(edpkt_data* pVifPkt) 
{
	int stcl = 0;
	int stwl = 0;
	edF32VECTOR4 strow = { 0.0f, 0.0f, 0.0f, 0.0f };
	char* pFakeMem = GetFakeMem();

	while (pVifPkt->cmdA != 0x60000000) {
		RunTag* pRunTag = (RunTag*)(&(pVifPkt->asU32[3]));

		if (pVifPkt->asU32[3] == VIF_NOP) {
			auto pOther = (RunTag*)(&(pVifPkt->asU32[0]));

			if (pOther->cmd == 0x30) {
				// Execute commands at addr
				CyclePacket* pCycleMaskCmd = (CyclePacket*)LOAD_SECTION(pVifPkt->asU32[1]);
				stcl = pCycleMaskCmd->cycle.cl;
				stwl = pCycleMaskCmd->cycle.wl;

				if (pCycleMaskCmd->row.cmd == 0x30) {
					strow = pCycleMaskCmd->rowData;
				}
			}
			else {
				// Shouldn't end up here?
				IMPLEMENTATION_GUARD();
			}
		}
		else if (pRunTag->cmd == VIF_MSCAL) {
			// Update PRIM
			HW_Gif_Tag* pGifTag = (HW_Gif_Tag*)(pFakeMem + 0x1990);
			const uint prim = pGifTag->PRIM;
			Renderer::SetPrim(*reinterpret_cast<const GIFReg::PrimPacked*>(&prim));

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