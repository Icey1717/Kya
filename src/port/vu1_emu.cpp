#include "vu1_emu.h"

#include "port/pointer_conv.h"
#include "port.h"

#include "PCSX2_VU.h"
#include "log.h"

namespace VU1Emu {

#define FAKE_VU1_MEM_SIZE 0x100000
	char gFakeMem[FAKE_VU1_MEM_SIZE] = {};
	unsigned char gFakeMemInterpreter[FAKE_VU1_MEM_SIZE] = {};

	unsigned char gVu1Code[FAKE_VU1_MEM_SIZE] = {};

	char* GetFakeMem() {
		return gFakeMem;
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
#define VIF_LOAD_F(reg, off) (*(edF32VECTOR4*)(GetFakeMem() + (reg << 4) + (off << 4)))

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
	int vi13;
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
	edF32VECTOR4 vf23;
	edF32VECTOR4 vf24;
	edF32VECTOR4 vf25;
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
		vi14 = (vi14 & 0xFF);
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
		value = fabs(value);
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

	void _$NoClipCulling_12_1()
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

			vi10 = vf05.wi;
			vi11 = vf06.wi;
			vi12 = vf07.wi;

			vf08 = vf04 + (vf03 * vf05.z) + (vf02 * vf05.y) + (vf01 * vf05.x);
			vf09 = vf04 + (vf03 * vf06.z) + (vf02 * vf06.y) + (vf01 * vf06.x);
			vf10 = vf04 + (vf03 * vf07.z) + (vf02 * vf07.y) + (vf01 * vf07.x);

			vi05 = Clip(vf08.w, vf08);
			vi05 = vi09 & vi05;
			vi08 = vi05 & vi06;
			vi08 = vi08 & vi07;

			if (vi08 != 0) {
				vi10 = vi10 | 0xa000;
				*VIF_AS_I(vi03, 2, VIF_REG_W) = vi10;

				//uint* pFlag = (uint*)VIF_AS_I(vi03, 2, VIF_REG_W);
				//*pFlag |= 0xa000;
			}

			vi06 = Clip(vf09.w, vf09);
			vi06 = vi09 & vi06;
			vi08 = vi05 & vi06;
			vi08 = vi08 & vi07;

			if (vi08 != 0) {
				vi11 = vi11 | 0xa000;
				*VIF_AS_I(vi03, 5, VIF_REG_W) = vi11;

				//uint* pFlag = (uint*)VIF_AS_I(vi03, 5, VIF_REG_W);
				//*pFlag |= 0xa000;
			}

			vi07 = Clip(vf10.w, vf10);
			vi07 = vi09 & vi07;
			vi08 = vi05 & vi06;
			vi08 = vi08 & vi07;

			if (vi08 != 0) {
				vi12 = vi12 | 0xa000;
				*VIF_AS_I(vi03, 8, VIF_REG_W) = vi12;

				//uint* pFlag = (uint*)VIF_AS_I(vi03, 8, VIF_REG_W);
				//*pFlag |= 0xa000;
			}

			vi03 += 9;
		}
	}

	void _$Culling_12_1()
	{
		_$NoClipCulling_12_1();
	}

	void _$ClippingRejection_12_1_OLD()
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

			vi05 = Clip(vf08.w, vf08);
			vi05 = vi09 & vi05;
			vi08 = vi05 | vi06;
			vi08 = vi08 | vi07;

			if (vi08 != 0) {
				vi10 = *VIF_AS_I(vi03, 2, VIF_REG_W);
				vi10 = vi10 | 0x9000;
				vi10 = vi10 | vi05;
				*VIF_AS_I(vi03, 2, VIF_REG_W) = vi10;
			}

			vi06 = Clip(vf09.w, vf09);
			vi06 = vi09 & vi06;
			vi08 = vi05 | vi06;
			vi08 = vi08 | vi07;

			if (vi08 != 0) {
				uint* pFlag = (uint*)VIF_AS_I(vi03, 5, VIF_REG_W);
				*pFlag |= 0x9000 | vi06;
			}

			vi07 = Clip(vf10.w, vf10);
			vi07 = vi09 & vi07;
			vi08 = vi05 | vi06;
			vi08 = vi08 | vi07;

			if (vi08 != 0) {
				uint* pFlag = (uint*)VIF_AS_I(vi03, 8, VIF_REG_W);
				*pFlag |= 0x9000 | vi07;
			}

			vi03 += 9;
		}
	}

	void _$ClippingRejection_12_1()
	{
		vi02 = vi14;
		vi03 = vi15 + 1;

		vf05 = VIF_LOAD_F(vi03, 2);

		// Obj to Clipping matrix
		vf04 = VIF_LOAD_F(vi00, 15);
		vf03 = VIF_LOAD_F(vi00, 14);
		vf02 = VIF_LOAD_F(vi00, 13);
		vf01 = VIF_LOAD_F(vi00, 12);

		vi04 = 0x9000;

		vf08 = vf04 + (vf03 * vf05.z) + (vf02 * vf05.y) + (vf01 * vf05.x);

		vf06 = VIF_LOAD_F(vi03, 5);

		vi09 = 63;

		vi10 = vf05.wi;
		vi05 = Clip(vf08.w, vf08);
		vf07 = VIF_LOAD_F(vi03, 8);
		vi11 = vf06.wi;

		vf09 = vf04 + (vf03 * vf06.z) + (vf02 * vf06.y) + (vf01 * vf06.x);

		vi06 = 0;
		vi07 = 0;

		while (vi02 > 0) {
			vi05 = vi09 & vi05;
			vi08 = vi05 | vi06;
			vi08 = vi08 | vi07;

			vi06 = Clip(vf09.w, vf09);

			vi12 = vf07.wi;
			vf10 = vf04 + (vf03 * vf07.z) + (vf02 * vf07.y) + (vf01 * vf07.x);

			if (vi08 != 0) {
				vi10 = vi10 | 0x9000;
				vi10 = vi10 | vi05;

				//MY_LOG("00002b18 0a2a1802: ISW.w        %08x (vi10), 2, %08x (vi03),", vi10, vi03);

				*VIF_AS_I(vi03, 2, VIF_REG_W) = vi10;
			}

			vi03 += 9;

			vf05 = VIF_LOAD_F(vi03, 2);

			vi06 = vi09 & vi06;
			vi08 = vi05 | vi06;
			vi08 = vi08 | vi07;

			vi07 = Clip(vf10.w, vf10);

			vi10 = vf05.wi;
			vf08 = vf04 + (vf03 * vf05.z) + (vf02 * vf05.y) + (vf01 * vf05.x);

			if (vi08 != 0) {
				vi11 = vi11 | 0x9000;
				vi11 = vi11 | vi06;
				*VIF_AS_I(vi03, -4, VIF_REG_W) = vi11;
			}

			vi02 -= 3;

			vf06 = VIF_LOAD_F(vi03, 5);

			vi07 = vi09 & vi07;
			vi08 = vi05 | vi06;
			vi08 = vi08 | vi07;

			vi05 = Clip(vf08.w, vf08);

			vi11 = vf06.wi;
			vf09 = vf04 + (vf03 * vf06.z) + (vf02 * vf06.y) + (vf01 * vf06.x);

			if (vi08 != 0) {
				vi12 = vi12 | 0x9000;
				vi12 = vi12 | vi07;
				*VIF_AS_I(vi03, -1, VIF_REG_W) = vi12;
			}

			vf07 = VIF_LOAD_F(vi03, 8);
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

	void CopyClip()
	{
		vi05 = *reinterpret_cast<int*>(&vf28.x);

		// GIF packet
		*VIF_AS_F(vi05, 0) = vf29;

		*VIF_AS_I(vi05, 0, VIF_REG_X) = vi13;
		vi05 = *reinterpret_cast<int*>(&vf28.x);

		vf20 = VIF_LOAD_F(vi05, 1);
		vf21 = VIF_LOAD_F(vi05, 2);
		vf22 = VIF_LOAD_F(vi05, 3);

		*VIF_AS_F(vi04, 0) = vf20;
		*VIF_AS_F(vi04, 1) = vf21;
		*VIF_AS_F(vi04, 2) = vf22;
		vi05 = *reinterpret_cast<int*>(&vf28.x);
		// MR32.x
		vf28.x = vf28.y;
		vf28.y = *reinterpret_cast<float*>(&vi05);
	}

	bool ClipLoopFunc(int inFlag, float inTest, int testReg) 
	{
		// _$Clipping_MinusY
		vi04 = *reinterpret_cast<int*>(&vf28.y);
		vi06 = *VIF_AS_I(vi04, 0, VIF_REG_X);
		vi01 = 0xff;
		vi06 = vi01 & vi06;

		if (vi06 <= 0) {
			return false;
		}

		vi05 = *reinterpret_cast<int*>(&vf28.y);
		vi05 += 1;
		vi04 = *reinterpret_cast<int*>(&vf28.x);
		vi04 += 1;
		vi12 = inFlag;

		// Load in from GIF packet
		vi13 = *reinterpret_cast<int*>(&vf29.x);

		vf07 = VIF_LOAD_F(vi05, 2);
		vf06 = VIF_LOAD_F(vi05, 1);
		vf05 = VIF_LOAD_F(vi05, 0);
		vi10 = *reinterpret_cast<int*>(&vf07.w);

		vi10 = vi12 & vi10;
		vi05 += 3;

		// _$Clipping_LoopMinusY
		for (; vi06 > 0; vi06 -= 1) {
			vf22 = VIF_LOAD_F(vi05, 2);
			vf21 = VIF_LOAD_F(vi05, 1);
			vf20 = VIF_LOAD_F(vi05, 0);

			vi11 = *reinterpret_cast<int*>(&vf22.w);
			vi11 = vi12 & vi11;
			vi01 = vi10 & vi11;

			if (vi01 == 0) {

				vi01 = vi10 | vi11;
				if (vi10 == 0) {
					vf07.w = (VIF_LOAD_F(vi05, -1)).w;
					vi04 += 3;
					vi13 += 1;

					(*VIF_AS_F(vi04, -3)).xyz = vf05.xyz;
					*VIF_AS_F(vi04, -2) = vf06;
					*VIF_AS_F(vi04, -1) = vf07;
				}

				// _$Clipping_LoopMinusYmid
				vi01 = vi10 | vi11;
				vf23.xyz = vf07.xyz - inTest;
				vf25.xyz = vf07.xyz - vf22.xyz;

				vf24 = vf21 - vf06;

				Q = vf23.raw[testReg] / vf25.raw[testReg];
				vf23 = vf20 - vf05;

				if (vi01 != 0) {
					vi13 += 1;
					vi04 += 3;

					ACC = vf07;
					vf25.xyz = ACC.xyz - (vf25.xyz * Q);

					ACC = vf06;
					vf24 = ACC + (vf24 * Q);

					ACC = vf05;
					vf23.xyz = ACC.xyz + (vf23.xyz * Q);

					ACC.xyz = vf30.xyz * vf25.xyz;
					vf26.xyz = ACC.xyz + (vf31.xyz * 1.0f);

					vi01 = Clip(1.0f, vf26);
					vf25.w = *reinterpret_cast<float*>(&vi01);

					(*VIF_AS_F(vi04, -3)).xyz = vf23.xyz;
					*VIF_AS_F(vi04, -2) = vf24;
					*VIF_AS_F(vi04, -1) = vf25;
				}
			}

			// _$Clipping_LoopMinusYend
			vi10 = vi11;

			vf07 = vf22;
			vf06 = vf21;
			vf05 = vf20;

			vi05 += 3;
		}

		return true;
	}

	void KickVertexFromReg(int vtxReg)
	{
		// ST (float, float) and Q (float)
		edF32VECTOR4 STQ = VIF_LOAD_F(vtxReg, 0);
		edF32VECTOR4 RGBA = VIF_LOAD_F(vtxReg, 1);
		edF32VECTOR4 XYZSkip = VIF_LOAD_F(vtxReg, 2);
		int x = XYZSkip.xi;
		int y = XYZSkip.yi;
		int z = XYZSkip.zi;

		uint skip = XYZSkip.wi;

		Renderer::SetST(STQ.x, STQ.y);

		Renderer::SetRGBAQ(
			RGBA.xi,
			RGBA.yi,
			RGBA.zi,
			RGBA.wi,
			STQ.z);

		if (x == 0xcff0 && vtxReg == 0x61) {
			skip = skip;
		}

		//Log::GetInstance().AddLog(LogLevel::Info, "Vertex", "KickVertexFromReg %x x: %x y: %x, z: %x, skip: %x S: %f, T: %f, Q: %f", vtxReg, x, y, z, skip, STQ.x, STQ.y, STQ.z);
		Log::GetInstance().AddLog(LogLevel::Info, "Vertex", "KickVertexFromReg %x x: %x y: %x, z: %x, skip: %x", vtxReg, x, y, z, skip);

		Renderer::SetVertexSkip(skip & 0x8000);
		Renderer::KickVertex(x, y, z);
	}

	void EmulateXGKICK(int primReg)
	{
		edpkt_data* pkt = reinterpret_cast<edpkt_data*>(VIF_AS_F(primReg, 0));
		HW_Gif_Tag* pGifTag = (HW_Gif_Tag*)(pkt);
		const uint prim = pGifTag->PRIM;
		Renderer::SetPrim(*reinterpret_cast<const GIFReg::PrimPacked*>(&prim));

		for (int i = 0; i < pGifTag->NLOOP; i++) {
			KickVertexFromReg(primReg + 1 + (i * 3));
		}
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

		// GIF Packet
		vf29 = VIF_LOAD_F(vi00, 1);

		// gClipMulVector
		vf30 = VIF_LOAD_F(vi00, 2);

		// gClipAddVector
		vf31 = VIF_LOAD_F(vi00, 3);

		// gClipXY
		vf27 = VIF_LOAD_F(vi00, 4);

		// Vertex count
		vi02 = vi14;

		// Vertex buffer
		vi03 = vi15 + 1;

		vf07 = VIF_LOAD_F(vi03, 2);
		vf06 = VIF_LOAD_F(vi03, 1);
		vf05 = VIF_LOAD_F(vi03, 0);

		vi12 = 32;

		vf10 = VIF_LOAD_F(vi03, 5);
		vf09 = VIF_LOAD_F(vi03, 4);
		vf08 = VIF_LOAD_F(vi03, 3);

		vi07 = VIF_F_TO_I(vf07.w);
		vi07 = vi12 & vi07;
		vf07 = vf04 + (vf03 * vf07.z) + (vf02 * vf07.y) + (vf01 * vf07.x);
		vf06 = ConvertFromInt(vf06);

		vi08 = VIF_F_TO_I(vf10.w);
		vi08 = vi12 & vi08;
		vf10 = vf04 + (vf03 * vf10.z) + (vf02 * vf10.y) + (vf01 * vf10.x);
		vf09 = ConvertFromInt(vf09);

		vi02 -= 2;
		vi03 += 6;

		vf22.w = 0.0f; //MR32.w vf22 v00

		// _$Clipping_Loop
		while (vi02 > 0) {
			vi06 = 63;
			vi12 = 32;

			vi02 -= 1;

			vf13 = VIF_LOAD_F(vi03, 2);
			vf12 = VIF_LOAD_F(vi03, 1);
			vf11 = VIF_LOAD_F(vi03, 0);

			vi09 = VIF_F_TO_I(vf13.w);
			auto tempvi09 = vi09;

			//MY_LOG("IAND         00000003 (vi01), 0000f000 (vi10), %08x (vi09),", vi09);

			vi01 = 0xf000 & vi09;
			vi09 = vi12 & vi09;

			vf13 = vf04 + (vf03 * vf13.z) + (vf02 * vf13.y) + (vf01 * vf13.x);
			vf12 = ConvertFromInt(vf12);

			if (vi01 == 0x9000) {
				// _$TestFirstEdge

				vi04 = VIF_F_TO_I(vf28.x); // From gFanBuffers
				vi04 += 1;

				vi01 = vi07 & vi08;

				vi13 = VIF_F_TO_I(vf29.x); // From GIF Packet

				auto tempvi01 = vi01;
				vi01 = vi08 | vi09;
				if (tempvi01 == 0) {

					vi01 = vi07 | vi08;
					if (vi07 == 0) {

						vf22.w = (VIF_LOAD_F(vi03, -4)).w;

						vf21 = vf06;

						Q = 1.0f / vf07.w;

						vf20.xyz = vf05.xyz * Q;
						vf22.xyz = vf07.xyz * Q;

						vi04 += 3;
						vi13 += 1;

						(*VIF_AS_F(vi04, -3)).xyz = vf20.xyz;
						*VIF_AS_F(vi04, -2) = vf21;
						*VIF_AS_F(vi04, -1) = vf22;
					}

					// _$ClipFirstEdge

					vf20.xyz = vf07.xyz - vf30.w;
					vf22.xyz = vf07.xyz - vf10.xyz;
					vf21 = vf09 - vf06;

					tempvi01 = vi01;
					vi01 = vi08 & vi09;
					if (tempvi01 != 0) {

						vf26 = vf08 - vf05;

						Q = vf20.z / vf22.z;

						vf22.w = 0.0f; //MR32.w vf22 v00

						vi13 += 1;
						vi04 += 3;

						ACC = vf07;
						vf22.xyz = ACC.xyz - (vf22.xyz * Q);

						ACC = vf06;
						vf21 = ACC + (vf21 * Q);

						ACC = vf05;
						vf26.xyz = ACC.xyz + (vf26.xyz * Q);

						ACC.xyz = vf30.xyz * vf22.xyz;
						vf23.xyz = ACC.xyz + (vf31.xyz * 1.0f);

						vi05 = Clip(1.0f, vf23);
						vi05 = vi05 & vi06;
						vf22.w = *reinterpret_cast<float*>(&vi05);

						(*VIF_AS_F(vi04, -3)).xyz = vf26.xyz;
						*VIF_AS_F(vi04, -2) = vf21;
						*VIF_AS_F(vi04, -1) = vf22;
					}
				}

				// _$TestSecondEdge

				tempvi01 = vi01;
				vi01 = vi09 | vi07;
				if (tempvi01 == 0) {

					vi01 = vi08 | vi09;
					if (vi08 == 0) {

						vf22.w = (VIF_LOAD_F(vi03, -1)).w;

						vf21 = vf09;

						Q = 1.0f / vf10.w;

						vf20.xyz = vf08.xyz * Q;
						vf22.xyz = vf10.xyz * Q;

						vi04 += 3;
						vi13 += 1;

						(*VIF_AS_F(vi04, -3)).xyz = vf20.xyz;
						*VIF_AS_F(vi04, -2) = vf21;
						*VIF_AS_F(vi04, -1) = vf22;
					}

					// _$ClipSecondEdge

					vf20.xyz = vf10.xyz - vf30.w;
					vf22.xyz = vf10.xyz - vf13.xyz;
					vf21 = vf12 - vf09;

					tempvi01 = vi01;
					vi01 = vi09 & vi07;
					if (tempvi01 != 0) {
						vf26 = vf11 - vf08;

						Q = vf20.z / vf22.z;

						vi13 += 1;
						vi04 += 3;

						ACC = vf10;
						vf22.xyz = ACC.xyz - (vf22.xyz * Q);

						ACC = vf09;
						vf21 = ACC + (vf21 * Q);

						ACC = vf08;
						vf26.xyz = ACC.xyz + (vf26.xyz * Q);

						ACC.xyz = vf30.xyz * vf22.xyz;
						vf23.xyz = ACC.xyz + (vf31.xyz * 1.0f);

						vi05 = Clip(1.0f, vf23);
						vi05 = vi05 & vi06;
						vf22.w = *reinterpret_cast<float*>(&vi05);

						(*VIF_AS_F(vi04, -3)).xyz = vf26.xyz;
						*VIF_AS_F(vi04, -2) = vf21;
						*VIF_AS_F(vi04, -1) = vf22;
					}
				}

				// _$TestThirdEdge

				if (vi01 == 0) {
					vi01 = vi09 | vi07;
					if (vi09 == 0) {

						vf22.w = (VIF_LOAD_F(vi03, 2)).w;

						vf21 = vf12;

						Q = 1.0f / vf13.w;

						vf20.xyz = vf11.xyz * Q;
						vf22.xyz = vf13.xyz * Q;

						vi04 += 3;
						vi13 += 1;

						(*VIF_AS_F(vi04, -3)).xyz = vf20.xyz;
						*VIF_AS_F(vi04, -2) = vf21;
						*VIF_AS_F(vi04, -1) = vf22;
					}

					// _$ClipThirdEdge

					vf20.xyz = vf13.xyz - vf30.w;
					vf22.xyz = vf13.xyz - vf07.xyz;
					vf21 = vf06 - vf12;

					if (vi01 != 0) {
						vf26 = vf05 - vf11;

						Q = vf20.z / vf22.z;

						vi13 += 1;
						vi04 += 3;

						ACC = vf13;
						vf22.xyz = ACC.xyz - (vf22.xyz * Q);

						ACC = vf12;
						vf21 = ACC + (vf21 * Q);

						ACC = vf11;
						vf26.xyz = ACC.xyz + (vf26.xyz * Q);

						ACC.xyz = vf30.xyz * vf22.xyz;
						vf23.xyz = ACC.xyz + (vf31.xyz * 1.0f);

						vi05 = Clip(1.0f, vf23);
						vi05 = vi05 & vi06;
						vf22.w = *reinterpret_cast<float*>(&vi05);

						(*VIF_AS_F(vi04, -3)).xyz = vf26.xyz;
						*VIF_AS_F(vi04, -2) = vf21;
						*VIF_AS_F(vi04, -1) = vf22;
					}
				}

				// _$EndOfClippingNear
				CopyClip();

				if (ClipLoopFunc(8, vf27.z, VIF_REG_Y)) {

					CopyClip();

					if (ClipLoopFunc(4, vf27.w, VIF_REG_Y)) {

						CopyClip();

						if (ClipLoopFunc(2, vf27.x, VIF_REG_X)) {

							CopyClip();

							if (ClipLoopFunc(1, vf27.y, VIF_REG_X)) {

								CopyClip();

								// _$Clipping_Convert
								vi04 = *reinterpret_cast<int*>(&vf28.y);
								vi13 = *VIF_AS_I(vi04, 0, VIF_REG_X);
								vi01 = 0xff;
								vi13 = vi01 & vi13;

								if (vi13 > 0) {
									const int vtxCount = vi13;

									vi04 += 1;

									// _$Clipping_ConvertPreLoop_NoFog 
									vf18 = VIF_LOAD_F(vi04, 1);
									vf19.xyz = VIF_LOAD_F(vi04, 2).xyz;
									vf22.w = 0.0f;

									// RGBA
									*reinterpret_cast<int*>(&vf18.x) = (int)vf18.x;
									*reinterpret_cast<int*>(&vf18.y) = (int)vf18.y;
									*reinterpret_cast<int*>(&vf18.z) = (int)vf18.z;
									*reinterpret_cast<int*>(&vf18.w) = (int)vf18.w;

									// Vertex positions (int4 x, int4 y, float z)
									*reinterpret_cast<int*>(&vf22.x) = float_to_int4(vf19.x);
									*reinterpret_cast<int*>(&vf22.y) = float_to_int4(vf19.y);

									// _$Clipping_ConvertLoop_NoFog
									while (vi13 > 0) {
										*reinterpret_cast<int*>(&vf22.z) = (int)vf19.z;

										vf14 = VIF_LOAD_F(vi04, 4);
										vf15.xyz = VIF_LOAD_F(vi04, 5).xyz;
										vf20.w = 0.0f;

										*VIF_AS_F(vi04, 1) = vf18;
										*VIF_AS_F(vi04, 2) = vf22;

										vi13 -= 3;

										// RGBA
										*reinterpret_cast<int*>(&vf14.x) = (int)vf14.x;
										*reinterpret_cast<int*>(&vf14.y) = (int)vf14.y;
										*reinterpret_cast<int*>(&vf14.z) = (int)vf14.z;
										*reinterpret_cast<int*>(&vf14.w) = (int)vf14.w;

										// Vertex positions (int4 x, int4 y, float z)
										*reinterpret_cast<int*>(&vf20.x) = float_to_int4(vf15.x);
										*reinterpret_cast<int*>(&vf20.y) = float_to_int4(vf15.y);
										*reinterpret_cast<int*>(&vf20.z) = (int)vf15.z;

										vf16 = VIF_LOAD_F(vi04, 7);
										vf17.xyz = VIF_LOAD_F(vi04, 8).xyz;
										vf21.w = 0.0f;

										*VIF_AS_F(vi04, 4) = vf14;

										vi04 += 9;

										*VIF_AS_F(vi04, -4) = vf20;

										// RGBA
										*reinterpret_cast<int*>(&vf16.x) = (int)vf16.x;
										*reinterpret_cast<int*>(&vf16.y) = (int)vf16.y;
										*reinterpret_cast<int*>(&vf16.z) = (int)vf16.z;
										*reinterpret_cast<int*>(&vf16.w) = (int)vf16.w;

										// Vertex positions (int4 x, int4 y, float z)
										*reinterpret_cast<int*>(&vf21.x) = float_to_int4(vf17.x);
										*reinterpret_cast<int*>(&vf21.y) = float_to_int4(vf17.y);
										*reinterpret_cast<int*>(&vf21.z) = (int)vf17.z;

										vf18 = VIF_LOAD_F(vi04, 1);
										vf19.xyz = VIF_LOAD_F(vi04, 2).xyz;
										vf22.w = 0.0f;

										*VIF_AS_F(vi04, -2) = vf16;

										*reinterpret_cast<int*>(&vf18.x) = (int)vf18.x;
										*reinterpret_cast<int*>(&vf18.y) = (int)vf18.y;
										*reinterpret_cast<int*>(&vf18.z) = (int)vf18.z;
										*reinterpret_cast<int*>(&vf18.w) = (int)vf18.w;

										// Vertex positions (int4 x, int4 y, float z)
										*reinterpret_cast<int*>(&vf22.x) = float_to_int4(vf19.x);
										*reinterpret_cast<int*>(&vf22.y) = float_to_int4(vf19.y);

										*VIF_AS_F(vi04, -1) = vf21;
									}

									EmulateXGKICK(vf28.yi);

									vi05 = *reinterpret_cast<int*>(&vf28.y);
									vi05 = *reinterpret_cast<int*>(&vf28.y);
									vf28.y = vf28.z;
									vf28.z = *reinterpret_cast<float*>(&vi05);
								}
							}
						}
					}
				}
			}

			// _$Clipping_LoopEnd
			vf05 = vf08;
			vf06 = vf09;
			vf07 = vf10;

			vf08 = vf11;
			vf09 = vf12;
			vf10 = vf13;

			vi07 = vi08;
			vi08 = vi09;
			vi03 += 3;
		}

		vi01 = *VIF_AS_I(vi15, -1, VIF_REG_X);

		return;
	}

	void _$GouraudMapping_No_Fog_16_2()
	{
		// Vtx addr base.
		vi03 = vi15 + 1;

		// Unused.
		vf05 = VIF_LOAD_F(vi04, 1);

		vf17 = VIF_LOAD_F(vi03, 2);

		// Obj to Screen matrix.
		vf04 = VIF_LOAD_F(vi00, 19);
		vf03 = VIF_LOAD_F(vi00, 18);
		vf02 = VIF_LOAD_F(vi00, 17);
		vf01 = VIF_LOAD_F(vi00, 16);

		vf27 = VIF_LOAD_F(vi03, 5);
		vf16 = VIF_LOAD_F(vi03, 0);

		vf18 = vf04 + (vf03 * vf17.z) + (vf02 * vf17.y) + (vf01 * vf17.x);

		// Vtx count.
		vi02 = vi14;

		vf17 = VIF_LOAD_F(vi03, 8);

		vf28 = vf04 + (vf03 * vf27.z) + (vf02 * vf27.y) + (vf01 * vf27.x);
		Q = 1.0f / vf18.w;

		vf26 = VIF_LOAD_F(vi03, 3);
		vf27 = VIF_LOAD_F(vi03, 11);

		vf20.xyz = vf16.xyz * Q;
		vf19.xyz = vf18.xyz * Q;
		Q = 1.0f / vf28.w;

		vf18 = vf04 + (vf03 * vf17.z) + (vf02 * vf17.y) + (vf01 * vf17.x);

		while (vi02 > 0) {
			vf16 = VIF_LOAD_F(vi03, 6);

			// STQ
			(*VIF_AS_F(vi03, 0)).xyz = vf20.xyz;

			// Vertex positions (int4 x, int4 y, float z)
			*reinterpret_cast<int*>(&vf21.x) = float_to_int4(vf19.x);
			*reinterpret_cast<int*>(&vf21.y) = float_to_int4(vf19.y);
			*reinterpret_cast<int*>(&vf21.z) = (int)vf19.z;

			vf30 = vf26 * Q;
			vf29 = vf28 * Q;
			Q = 1.0f / vf18.w;

			vf17 = VIF_LOAD_F(vi03, 14);

			vi02 -= 2;
			vf28 = vf04 + (vf03 * vf27.z) + (vf02 * vf27.y) + (vf01 * vf27.x);

			// XYZ Skip
			(*VIF_AS_F(vi03, 2)).xyz = vf21.xyz;

			vf26 = VIF_LOAD_F(vi03, 9);

			vi03 += 6;

			// STQ
			(*VIF_AS_F(vi03, -3)).xyz = vf30.xyz;

			// Vertex positions (int4 x, int4 y, float z)
			*reinterpret_cast<int*>(&vf31.x) = float_to_int4(vf29.x);
			*reinterpret_cast<int*>(&vf31.y) = float_to_int4(vf29.y);
			*reinterpret_cast<int*>(&vf31.z) = (int)vf29.z;

			vf20 = vf16 * Q;
			vf19 = vf18 * Q;
			Q = 1.0f / vf28.w;

			vf18 = vf04 + (vf03 * vf17.z) + (vf02 * vf17.y) + (vf01 * vf17.x);

			vf27 = VIF_LOAD_F(vi03, 11);

			// XYZ Skip
			(*VIF_AS_F(vi03, -1)).xyz = vf31.xyz;
		}

		EmulateXGKICK(vi15);
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
				//IMPLEMENTATION_GUARD();
			}

			if ((vi01 & 0x1) != 0) {
				if ((vi01 & 0x2) != 0) {
					_$Culling_12_1();
					_$ClippingRejection_12_1();
					_$Clipping();

				}
				else {
					_$NoClipCulling_12_1();
				}
			}

			// _$Clipping_End (3b20)
			_$GouraudMapping_No_Fog_16_2();
		}
	}

	void UnpackToAddr(uint addr, void* data, int size)
	{
		const auto dst = GetFakeMem() + (addr << 4);
		memcpy(dst, data, size);
	}

	void KickCallback(int reg) {
		EmulateXGKICK(reg / 0x10);
	}
}

constexpr size_t vu1BlockSize = 0x800;

void VU1Emu::SendVu1Code(unsigned char* pCode, size_t size)
{
	const edpkt_data* const pHeader = reinterpret_cast<edpkt_data*>(pCode);

	unsigned char* pBlockStart = pCode + sizeof(edpkt_data);

	unsigned char* pDst = gVu1Code;

	while (pBlockStart < (pCode + size)) {
		memcpy(pDst, pBlockStart, vu1BlockSize);

		pDst += vu1BlockSize;

		pBlockStart += 0x8;
		pBlockStart += vu1BlockSize;
	}

	pcsx2_VU::ResetVUMemory();
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
			static bool bEmulateCode = true;
			if (bEmulateCode) {
				RunCode(pRunTag->addr);
			}
			else {

				pcsx2_VU::SetKickCallback(KickCallback);
				pcsx2_VU::SetMicro(gVu1Code);
				pcsx2_VU::SetMem((unsigned char*)gFakeMem);
				pcsx2_VU::Execute(pRunTag->addr);
			}

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
	pcsx2_VU::SetItop(newItop);
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