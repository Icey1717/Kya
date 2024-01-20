#include "vu1_emu.h"

#include "port/pointer_conv.h"
#include "port.h"

#include "PCSX2_VU.h"
#include "log.h"

#include <BS_thread_pool.hpp>
#include <recycle/shared_pool.hpp>
#include "edVideo/VideoD.h"

#include "hardware_draw.h"

#define VU_VTX_TRACE_LOG(format, ...) if (VU1Emu::bTracingVtx) { MY_LOG_CATEGORY("vtx_trace", LogLevel::Verbose, format, ##__VA_ARGS__); }

#define GET_VIF_CMD(cmd) (((unsigned int)cmd >> (3 << 3)) & 0xff)

namespace VU1Emu {

#define FAKE_VU1_MEM_SIZE 0x4000 // 16kb

	char gFakeMem[FAKE_VU1_MEM_SIZE] = {};
	unsigned char gVu1Code[FAKE_VU1_MEM_SIZE] = {};
	int gItop;

	bool bEnableHardwareDraw = false;
	bool bEnableInterpreter = false;
	bool bRunSingleThreaded = true;
	bool bRunSimplifiedCode = true;
	bool bTraceVtx = false;

	bool bTracingVtx = false;

	namespace Debug {
		constexpr bool bUseJobPool = true;
		constexpr bool bDelayCoreExecution = true;
		constexpr bool bUsePoolForDrawJobs = true;
		constexpr bool bUsePoolForCores = true;
	}

	BS::thread_pool gJobPool(12);

	bool bStartedWork = false;

	std::mutex gKickVertexLock;

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

	struct alignas(32) ExecTag {
		ushort count;
		byte pad;
		byte cmd;
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

#define VIF_LOAD_I(reg, row, off) *(int*)(GetLocalFakeMem() + (reg << 4) + (row << 4) + (off << 2))
#define VIF_LOAD_F(reg, off) (*(edF32VECTOR4*)(GetLocalFakeMem() + (reg << 4) + (off << 4)))
#define VIF_LOAD_M3(reg, off) (*(edF32MATRIX3*)(GetLocalFakeMem() + (reg << 4) + (off << 4)))
#define VIF_LOAD_M4(reg, off) (*(edF32MATRIX4*)(GetLocalFakeMem() + (reg << 4) + (off << 4)))

#define VIF_AS_I(reg, row, off) (int*)(GetLocalFakeMem() + (reg << 4) + (row << 4) + (off << 2))
#define VIF_AS_F(reg, off) ((edF32VECTOR4*)(GetLocalFakeMem() + (reg << 4) + (off << 4)))

#define float_to_int4(x)	(s32)((float)x * (1.0f / 0.0625f))
#define float_to_int12(x)	(s32)((float)x * (1.0f / 0.000244140625f))
#define float_to_int15(x)	(s32)((float)x * (1.0f / 0.000030517578125))

#define int4_to_float(x)	(float)((float)x * 0.0625f)
#define int12_to_float(x)	(float)((float)x * 0.000244140625f)
#define int15_to_float(x)	(float)((float)x * 0.000030517578125)

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

	edF32VECTOR3 ConvertFromInt(const edF32VECTOR3& reg) {
		edF32VECTOR3 ret = {
			(float)(VIF_F_TO_I(reg.x)),
			(float)(VIF_F_TO_I(reg.y)),
			(float)(VIF_F_TO_I(reg.z))
		};
		return ret;
	}

	class Vu1Core
	{
		PS2::DrawBufferData<Renderer::GSVertex, uint16_t>* pDrawBuffer;

		char fakeMem[FAKE_VU1_MEM_SIZE] = {};

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
		int addr;

		const edF32VECTOR4 vf00 = { 0.0f, 0.0f, 0.0f, 1.0f };
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

		int I;

		uint gClipflag = 0;

		char* GetLocalFakeMem() {
			return fakeMem;
		}

		void _$DrawingStart_Shared()
		{
			vi15 = itop;
			vi14 = VIF_LOAD_I(vi15, 0, VIF_REG_X);
			vi14 = (vi14 & 0xFF);
			if (vi14 == 0) {
				// JUMP
				// _$StopStrip
				assert(false);
			}

			vi01 = VIF_LOAD_I(vi15, -1, 0);
		}

		void _$DrawingStart_XYZ32() 
		{
			_$DrawingStart_Shared();
		}

		void _$DrawingStart_XYZ32_Normal()
		{
			_$DrawingStart_Shared();
		}

		void _$DrawingBones_Rigid()
		{
			_$DrawingStart_Shared();
		}

		void _$DrawingBones_Rigid_noNormal()
		{
			_$DrawingStart_Shared();
		}

		void _$UnpackData_XYZ32_Loop(int count, edF32VECTOR4* pStartAddr) 
		{
			for (int i = 0; i < count; i++) {
				pStartAddr[i].x = int12_to_float(pStartAddr[i].xi);
				pStartAddr[i].y = int12_to_float(pStartAddr[i].yi);
				pStartAddr += 2;
			}
		}

		void _$UnpackData_XYZ32() 
		{
			vi03 = vi15 + 1;
			_$UnpackData_XYZ32_Loop(vi14, VIF_AS_F(vi03, 0));
		}

		void _$UnpackData_XYZ32_Normal()
		{
			vi02 = vi14;
			vi03 = vi15 + 1;
			vi04 = vi03 + 216;

			// Load ST 0
			vf22 = VIF_LOAD_F(vi03, 0);

			// Load RGBA 0
			vf23 = VIF_LOAD_F(vi03, 1);

			// Load Normal 0
			vf24 = VIF_LOAD_F(vi04++, 0);

			// ST 0 to float
			vf25.x = int12_to_float(vf22.xi);
			vf25.y = int12_to_float(vf22.yi);

			// RGBA 0 to float
			vf26.x = vf23.xi;
			vf26.y = vf23.yi;
			vf26.z = vf23.zi;
			vf26.w = vf23.wi;

			// Normal 0 to float
			vf27.x = int15_to_float(vf24.xi);
			vf27.y = int15_to_float(vf24.yi);
			vf27.z = int15_to_float(vf24.zi);

			// Load ST 1
			vf04 = VIF_LOAD_F(vi03, 3);

			// Load RGBA 1
			vf05 = VIF_LOAD_F(vi03, 4);

			// Load Normal 1
			vf06 = VIF_LOAD_F(vi04++, 0);

			// _$UnpackData_XYZ32_Normal_Loop

			while (vi02 > 0) {
				// Load ST 2
				vf10 = VIF_LOAD_F(vi03, 6);

				// Load RGBA 2
				vf11 = VIF_LOAD_F(vi03, 7);

				// Load Normal 2
				vf12 = VIF_LOAD_F(vi04++, 0);

				// ST 1 to float
				vf07.x = int12_to_float(vf04.xi);
				vf07.y = int12_to_float(vf04.yi);

				// RGBA 1 to float
				vf08.x = vf05.xi;
				vf08.y = vf05.yi;
				vf08.z = vf05.zi;
				vf08.w = vf05.wi;

				// Normal 1 to float
				vf09.x = int15_to_float(vf06.xi);
				vf09.y = int15_to_float(vf06.yi);
				vf09.z = int15_to_float(vf06.zi);

				// Store ST 0
				VIF_AS_F(vi03, 0)->xy = vf25.xy;

				// Store RGBA 0
				VIF_AS_F(vi03, 1)->xyz = vf26.xyz;

				// Store Normal 0
				VIF_AS_F(vi04, -3)->xyz = vf27.xyz;

				vi03 += 12;

				// Load ST 3
				vf16 = VIF_LOAD_F(vi03, -3);

				// Load RGBA 3
				vf17 = VIF_LOAD_F(vi03, -2);

				// Load Normal 3
				vf18 = VIF_LOAD_F(vi04++, 0);

				// ST 2 to float
				vf13.x = int12_to_float(vf10.xi);
				vf13.y = int12_to_float(vf10.yi);

				// RGBA 2 to float
				vf14.x = vf11.xi;
				vf14.y = vf11.yi;
				vf14.z = vf11.zi;
				vf14.w = vf11.wi;

				// Normal 2 to float
				vf15.x = int15_to_float(vf12.xi);
				vf15.y = int15_to_float(vf12.yi);
				vf15.z = int15_to_float(vf12.zi);

				// Store ST 1
				VIF_AS_F(vi03, -9)->xy = vf07.xy;

				// Store RGBA 1
				VIF_AS_F(vi03, -8)->xyz = vf08.xyz;

				// Store Normal 1
				VIF_AS_F(vi04, -3)->xyz = vf09.xyz;

				vi02 -= 4;

				// Load ST 0
				vf22 = VIF_LOAD_F(vi03, 0);

				// Load RGBA 0
				vf23 = VIF_LOAD_F(vi03, 1);

				// Load Normal 0
				vf24 = VIF_LOAD_F(vi04++, 0);

				// ST 3 to float
				vf19.x = int12_to_float(vf16.xi);
				vf19.y = int12_to_float(vf16.yi);

				// RGBA 3 to float
				vf20.x = vf17.xi;
				vf20.y = vf17.yi;
				vf20.z = vf17.zi;
				vf20.w = vf17.wi;

				// Normal 3 to float
				vf21.x = int15_to_float(vf18.xi);
				vf21.y = int15_to_float(vf18.yi);
				vf21.z = int15_to_float(vf18.zi);

				// Store ST 2
				VIF_AS_F(vi03, -6)->xy = vf13.xy;

				// Store RGBA 2
				VIF_AS_F(vi03, -5)->xyz = vf14.xyz;

				// Store Normal 2
				VIF_AS_F(vi04, -3)->xyz = vf15.xyz;

				// Load ST 1
				vf04 = VIF_LOAD_F(vi03, 3);

				// Load RGBA 1
				vf05 = VIF_LOAD_F(vi03, 4);

				// Load Normal 1
				vf06 = VIF_LOAD_F(vi04++, 0);

				// ST 0 to float
				vf25.x = int12_to_float(vf22.xi);
				vf25.y = int12_to_float(vf22.yi);

				// RGBA 0 to float
				vf26.x = vf23.xi;
				vf26.y = vf23.yi;
				vf26.z = vf23.zi;
				vf26.w = vf23.wi;

				// Normal 0 to float
				vf27.x = int15_to_float(vf24.xi);
				vf27.y = int15_to_float(vf24.yi);
				vf27.z = int15_to_float(vf24.zi);

				// Store ST 3
				VIF_AS_F(vi03, -3)->xy = vf19.xy;

				// Store RGBA 3
				VIF_AS_F(vi03, -2)->xyz = vf20.xyz;

				// Store Normal 3
				VIF_AS_F(vi04, -3)->xyz = vf21.xyz;
			}
		}

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
				}

				vi06 = Clip(vf09.w, vf09);
				vi06 = vi09 & vi06;
				vi08 = vi05 & vi06;
				vi08 = vi08 & vi07;

				if (vi08 != 0) {
					vi11 = vi11 | 0xa000;
					*VIF_AS_I(vi03, 5, VIF_REG_W) = vi11;
				}

				vi07 = Clip(vf10.w, vf10);
				vi07 = vi09 & vi07;
				vi08 = vi05 & vi06;
				vi08 = vi08 & vi07;

				if (vi08 != 0) {
					vi12 = vi12 | 0xa000;
					*VIF_AS_I(vi03, 8, VIF_REG_W) = vi12;
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

		void KickVertexFromReg(int vtxReg, uint prim)
		{
			// ST (float, float) and Q (float)
			edF32VECTOR4 STQ = VIF_LOAD_F(vtxReg, 0);
			edF32VECTOR4 RGBA = VIF_LOAD_F(vtxReg, 1);
			edF32VECTOR4 XYZSkip = VIF_LOAD_F(vtxReg, 2);
			int x = XYZSkip.xi;
			int y = XYZSkip.yi;
			int z = XYZSkip.zi;

			//assert(x < 5000000);
			//assert(y < 5000000);
			//assert(z < 5000000);

			uint skip = XYZSkip.wi;

			Renderer::SetST(STQ.x, STQ.y);

			Renderer::SetRGBAQ(
				RGBA.xi,
				RGBA.yi,
				RGBA.zi,
				RGBA.wi,
				STQ.z);

			VU_VTX_TRACE_LOG("KickVertexFromReg 0x{:x} | 0x{:x} S: {}, T: {}, Q: {}", vtxReg, vtxReg + 0, STQ.x, STQ.y, STQ.z);
			VU_VTX_TRACE_LOG("KickVertexFromReg 0x{:x} | 0x{:x} r: 0x{:x} g: 0x{:x}, b: 0x{:x}, a: 0x{:x}", vtxReg, vtxReg + 1, RGBA.xi, RGBA.yi, RGBA.zi, RGBA.wi);
			VU_VTX_TRACE_LOG("KickVertexFromReg 0x{:x} | 0x{:x} x: 0x{:x} y: 0x{:x}, z: 0x{:x}, skip: 0x{:x}", vtxReg, vtxReg + 2, x, y, z, skip);

			Renderer::SetVertexSkip(skip & 0x8000);
			Renderer::GSVertex vtx{ { STQ.x, STQ.y }, {RGBA.xi, RGBA.yi, RGBA.zi, RGBA.wi }, STQ.z, { x, y }, z, 0, 0 };

			const GIFReg::GSPrimPacked pPrimPacked = *reinterpret_cast<const GIFReg::GSPrimPacked*>(&prim);
			Renderer::KickVertex(vtx, pPrimPacked, skip & 0x8000, *pDrawBuffer);
		}

		void _$Clipping()
		{
			// Obj to Screen matrix
			vf04 = VIF_LOAD_F(vi00, 0x13);
			vf03 = VIF_LOAD_F(vi00, 0x12);
			vf02 = VIF_LOAD_F(vi00, 0x11);
			vf01 = VIF_LOAD_F(vi00, 0x10);

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
					if (tempvi01 == 0) { // Goto _$ClipSecondEdge

						vi01 = vi07 | vi08;
						if (vi07 == 0) { // Goto _$ClipFirstEdge

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
						if (tempvi01 != 0) { // Goto _$TestSecondEdge

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

						// _$TestSecondEdge

						tempvi01 = vi01;
						vi01 = vi09 | vi07;
						if (tempvi01 == 0) { // Goto _$ClipThirdEdge

							vi01 = vi08 | vi09;
							if (vi08 == 0) { // Goto _$ClipSecondEdge

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
						}
						else {
							goto _$ClipThirdEdge;
						}
					}

					// _$ClipSecondEdge

					vf20.xyz = vf10.xyz - vf30.w;
					vf22.xyz = vf10.xyz - vf13.xyz;
					vf21 = vf12 - vf09;

					tempvi01 = vi01;
					vi01 = vi09 & vi07;
					if (tempvi01 != 0) { // Goto _$TestThirdEdge
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

					// _$TestThirdEdge

					if (vi01 == 0) { // Goto _$EndOfClippingNear
						vi01 = vi09 | vi07;
						if (vi09 == 0) { // Goto _$ClipThirdEdge

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

	_$ClipThirdEdge:
						// _$ClipThirdEdge

						vf20.xyz = vf13.xyz - vf30.w;
						vf22.xyz = vf13.xyz - vf07.xyz;
						vf21 = vf06 - vf12;

						if (vi01 != 0) { // Goto _$EndOfClippingNear
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

		void _$GouraudMapping_No_Fog_16_2_Simple()
		{
			const int vtxCount = vi14;
			int vtxReg = vi15 + 1;

			const edF32MATRIX4 objToScreen = VIF_LOAD_M4(vi00, 0x10);
			VU_VTX_TRACE_LOG("GouraudMapping_No_Fog objToScreen matrix: \n{}", objToScreen.ToString());

			for (int vtxIndex = 0; vtxIndex < vtxCount; vtxIndex++) {
				// Vtx data
				edF32VECTOR4* pSTQ = VIF_AS_F(vtxReg, 0);
				edF32VECTOR4* pRGBA = VIF_AS_F(vtxReg, 1);
				edF32VECTOR4* pXYZ = VIF_AS_F(vtxReg, 2);

				VU_VTX_TRACE_LOG("GouraudMapping_No_Fog XYZ  0x{:x} x: {} y: {} z: {} w: 0x{:x}", vtxReg, pXYZ->x, pXYZ->y, pXYZ->z, pXYZ->wi);
				VU_VTX_TRACE_LOG("GouraudMapping_No_Fog RGBA 0x{:x} r: {} g: {} b: {}", vtxReg, pRGBA->xi, pRGBA->yi, pRGBA->zi);
				VU_VTX_TRACE_LOG("GouraudMapping_No_Fog STQ  0x{:x} S: {} T: {} Q: {}", vtxReg, pSTQ->x, pSTQ->y, pSTQ->z);

#if 0
				{
					edF32VECTOR4 a = objToScreen.rowX * pXYZ->x;
					VU_VTX_TRACE_LOG("GouraudMapping_No_Fog XYZ  0x{:x} a x: {} y: {} z: {} w: {}", vtxReg, a.x, a.y, a.z, a.w);

					a = objToScreen.rowY * pXYZ->y;
					VU_VTX_TRACE_LOG("GouraudMapping_No_Fog XYZ  0x{:x} b x: {} y: {} z: {} w: {}", vtxReg, a.x, a.y, a.z, a.w);

					a = objToScreen.rowZ * pXYZ->z;
					VU_VTX_TRACE_LOG("GouraudMapping_No_Fog XYZ  0x{:x} c x: {} y: {} z: {} w: {}", vtxReg, a.x, a.y, a.z, a.w);
				}
#endif

				edF32VECTOR4 screenVtx = *pXYZ * objToScreen;

				VU_VTX_TRACE_LOG("GouraudMapping_No_Fog XYZ to screen 0x{:x} x: {} y: {} z: {} w: {}", vtxReg, screenVtx.x, screenVtx.y, screenVtx.z, screenVtx.w);

				float invW = 1.0f / fabs(screenVtx.w);

				VU_VTX_TRACE_LOG("GouraudMapping_No_Fog Q 0x{:x} DIVQ: {}", vtxReg, invW);

				pSTQ->xyz = pSTQ->xyz * invW;
				VU_VTX_TRACE_LOG("GouraudMapping_No_Fog ST * Q 0x{:x} x: {} y: {} z: {}", vtxReg, pSTQ->x, pSTQ->y, pSTQ->z);

				screenVtx.xyz = screenVtx.xyz * invW;
				VU_VTX_TRACE_LOG("GouraudMapping_No_Fog XYZ * Q 0x{:x} x: {} y: {} z: {}", vtxReg, screenVtx.x, screenVtx.y, screenVtx.z);

				// Vertex positions (int4 x, int4 y, float z)
				*reinterpret_cast<int*>(&screenVtx.x) = float_to_int4(screenVtx.x);
				*reinterpret_cast<int*>(&screenVtx.y) = float_to_int4(screenVtx.y);
				*reinterpret_cast<int*>(&screenVtx.z) = (int)screenVtx.z;

				VU_VTX_TRACE_LOG("GouraudMapping_No_Fog XYZ 0x{:x} x: 0x{:x} y: 0x{:x} z: 0x{:x}", vtxReg, screenVtx.xi, screenVtx.yi, screenVtx.zi);

				pXYZ->xyz = screenVtx.xyz;

				VU_VTX_TRACE_LOG("GouraudMapping_No_Fog transformed XYZ  0x{:x} x: {} y: {} z: {} w: 0x{:x}", vtxReg, pXYZ->xi, pXYZ->yi, pXYZ->zi, pXYZ->wi);
				VU_VTX_TRACE_LOG("GouraudMapping_No_Fog transformed RGBA 0x{:x} r: {} g: {} b: {} a: {}", vtxReg, pRGBA->xi, pRGBA->yi, pRGBA->zi, pRGBA->zi);
				VU_VTX_TRACE_LOG("GouraudMapping_No_Fog transformed STQ  0x{:x} S: {} T: {} Q: {}", vtxReg, pSTQ->x, pSTQ->y, pSTQ->z);

				vtxReg += 3;
			}

			EmulateXGKICK(vi15);
		}

		void _$GouraudMapping_No_Fog_16_2()
		{
			if (bRunSimplifiedCode) {
				_$GouraudMapping_No_Fog_16_2_Simple();
				return;
			}

			// Vtx addr base.
			vi03 = vi15 + 1;

			// Unused.
			//vf05 = VIF_LOAD_F(vi04, 1);

			// XYZ (1)
			vf17 = VIF_LOAD_F(vi03, 2);

			// Obj to Screen matrix.
			vf04 = VIF_LOAD_F(vi00, 0x13);
			vf03 = VIF_LOAD_F(vi00, 0x12);
			vf02 = VIF_LOAD_F(vi00, 0x11);
			vf01 = VIF_LOAD_F(vi00, 0x10);

			VU_VTX_TRACE_LOG("GouraudMapping_No_Fog obj to screen \n{}\n{}\n{}\n{}", vf01.ToString(), vf02.ToString(), vf03.ToString(), vf04.ToString());

			// Just putting this down here for neatness of output.
			VU_VTX_TRACE_LOG("GouraudMapping_No_Fog XYZ vf17 0x{:x} x: {} y: {} z: {} w: 0x{:x}", vi03, vf17.x, vf17.y, vf17.z, vf17.wi);

			// XYZ (2)
			vf27 = VIF_LOAD_F(vi03, 5);
			VU_VTX_TRACE_LOG("GouraudMapping_No_Fog XYZ vf27 0x{:x} x: {} y: {} z: {} w: 0x{:x}", vi03 + 3, vf27.x, vf27.y, vf27.z, vf27.wi);

			// STQ (1)
			vf16 = VIF_LOAD_F(vi03, 0);
			VU_VTX_TRACE_LOG("GouraudMapping_No_Fog STQ vf16 0x{:x} S: {} T: {} Q: {}", vi03, vf16.x, vf16.y, vf16.z);

			// XYZ to Screen
			vf18 = vf04 + (vf03 * vf17.z) + (vf02 * vf17.y) + (vf01 * vf17.x);
			VU_VTX_TRACE_LOG("GouraudMapping_No_Fog XYZ to screen vf18 0x{:x} x: {} y: {} z: {} w: {}", vi03, vf18.x, vf18.y, vf18.z, vf18.w);

			// Vtx count.
			vi02 = vi14;

			// XYZ (1)
			vf17 = VIF_LOAD_F(vi03, 8);
			VU_VTX_TRACE_LOG("GouraudMapping_No_Fog XYZ vf17 0x{:x} x: {} y: {} z: {} w: 0x{:x}", vi03 + 6, vf17.x, vf17.y, vf17.z, vf17.wi);

			// XYZ to Screen
			vf28 = vf04 + (vf03 * vf27.z) + (vf02 * vf27.y) + (vf01 * vf27.x);
			VU_VTX_TRACE_LOG("GouraudMapping_No_Fog XYZ to screen vf28 0x{:x} x: {} y: {} z: {} w: {}", vi03 + 3, vf28.x, vf28.y, vf28.z, vf28.w);

			Q = 1.0f / vf18.w;
			VU_VTX_TRACE_LOG("GouraudMapping_No_Fog Q 0x{:x} DIVQ: {}", vi03, Q);

			// STQ (2)
			vf26 = VIF_LOAD_F(vi03, 3);
			VU_VTX_TRACE_LOG("GouraudMapping_No_Fog STQ vf26 0x{:x} S: {} T: {} Q: {}", vi03 + 3, vf26.x, vf26.y, vf26.z);

			// XYZ (2)
			vf27 = VIF_LOAD_F(vi03, 11);
			VU_VTX_TRACE_LOG("GouraudMapping_No_Fog XYZ vf27 0x{:x} x: {} y: {} z: {} w: 0x{:x}", vi03 + 9, vf27.x, vf27.y, vf27.z, vf27.wi);

			vf20.xyz = vf16.xyz * Q;
			vf19.xyz = vf18.xyz * Q;
			VU_VTX_TRACE_LOG("GouraudMapping_No_Fog XYZ * Q vf19 0x{:x} x: {} y: {} z: {}", vi03, vf19.x, vf19.y, vf19.z);

			Q = 1.0f / vf28.w; // ?? IS THIS A BUG?
			VU_VTX_TRACE_LOG("GouraudMapping_No_Fog Q 0x{:x} DIVQ: {}", vi03 + 3, Q);

			// XYZ to Screen
			vf18 = vf04 + (vf03 * vf17.z) + (vf02 * vf17.y) + (vf01 * vf17.x);
			VU_VTX_TRACE_LOG("GouraudMapping_No_Fog XYZ to screen vf18 0x{:x} x: {} y: {} z: {} w: {}", vi03 + 6, vf18.x, vf18.y, vf18.z, vf18.w);

			while (vi02 > 0) {

				// STQ (1)
				vf16 = VIF_LOAD_F(vi03, 6);

				// STQ (1)
				(*VIF_AS_F(vi03, 0)).xyz = vf20.xyz;
				VU_VTX_TRACE_LOG("GouraudMapping_No_Fog STQ vf20 0x{:x} S: {} T: {} Q: {}", vi03, vf20.x, vf20.y, vf20.z);

				// Vertex positions (int4 x, int4 y, float z)
				*reinterpret_cast<int*>(&vf21.x) = float_to_int4(vf19.x);
				*reinterpret_cast<int*>(&vf21.y) = float_to_int4(vf19.y);
				*reinterpret_cast<int*>(&vf21.z) = (int)vf19.z;

				vf30.xyz = vf26.xyz * Q;
				vf29.xyz = vf28.xyz * Q;
				VU_VTX_TRACE_LOG("GouraudMapping_No_Fog XYZ * Q vf29 0x{:x} x: {} y: {} z: {}", vi03 + 3, vf29.x, vf29.y, vf29.z);
				Q = 1.0f / vf18.w;

				// XYZ (1)
				vf17 = VIF_LOAD_F(vi03, 14);
				VU_VTX_TRACE_LOG("GouraudMapping_No_Fog XYZ vf17 0x{:x} x: {} y: {} z: {} w: 0x{:x}", vi03 + 12, vf17.x, vf17.y, vf17.z, vf17.wi);

				vi02 -= 2;

				// XYZ to Screen
				vf28 = vf04 + (vf03 * vf27.z) + (vf02 * vf27.y) + (vf01 * vf27.x);
				VU_VTX_TRACE_LOG("GouraudMapping_No_Fog XYZ to screen vf28 0x{:x} x: {} y: {} z: {} w: {}", vi03 + 9, vf28.x, vf28.y, vf28.z, vf28.w);

				// XYZ Skip
				(*VIF_AS_F(vi03, 2)).xyz = vf21.xyz;
				VU_VTX_TRACE_LOG("GouraudMapping_No_Fog XYZ vf21 0x{:x} x: 0x{:x} y: 0x{:x} z: 0x{:x}", vi03, vf21.xi, vf21.yi, vf21.zi);

				vf26 = VIF_LOAD_F(vi03, 9);

				vi03 += 6;

				// STQ (2)
				(*VIF_AS_F(vi03, -3)).xyz = vf30.xyz;
				VU_VTX_TRACE_LOG("GouraudMapping_No_Fog STQ vf30 0x{:x} S: {} T: {} Q: {}", vi03 - 3, vf30.x, vf30.y, vf30.z);

				// Vertex positions (int4 x, int4 y, float z)
				*reinterpret_cast<int*>(&vf31.x) = float_to_int4(vf29.x);
				*reinterpret_cast<int*>(&vf31.y) = float_to_int4(vf29.y);
				*reinterpret_cast<int*>(&vf31.z) = (int)vf29.z;

				vf20.xyz = vf16.xyz * Q;
				vf19.xyz = vf18.xyz * Q;
				VU_VTX_TRACE_LOG("GouraudMapping_No_Fog XYZ * Q vf19 0x{:x} x: {} y: {} z: {}", vi03, vf19.x, vf19.y, vf19.z);
				Q = 1.0f / vf28.w;

				// XYZ to Screen
				vf18 = vf04 + (vf03 * vf17.z) + (vf02 * vf17.y) + (vf01 * vf17.x);
				VU_VTX_TRACE_LOG("GouraudMapping_No_Fog XYZ to screen vf18 0x{:x} x: {} y: {} z: {} w: {}", vi03 + 6, vf18.x, vf18.y, vf18.z, vf18.w);

				// XYZ (2)
				vf27 = VIF_LOAD_F(vi03, 11);
				VU_VTX_TRACE_LOG("GouraudMapping_No_Fog XYZ vf27 0x{:x} x: {} y: {} z: {} w: 0x{:x}", vi03 + 9, vf27.x, vf27.y, vf27.z, vf27.wi);

				// XYZ Skip
				(*VIF_AS_F(vi03, -1)).xyz = vf31.xyz;
				VU_VTX_TRACE_LOG("GouraudMapping_No_Fog XYZ vf31 0x{:x} x: 0x{:x} y: 0x{:x} z: 0x{:x}", vi03 - 3, vf31.xi, vf31.yi, vf31.zi);
			}

			EmulateXGKICK(vi15);
		}

		void _$XYZW_16_ConvBones_Rigid_Simple()
		{
			if (vi01 & 0x400) {
				vi01 = 0x7bff & vi01;

				const int vtxCount = vi14;
				int vtxReg = vi15 + 1;

				for (int vtxIndex = 0; vtxIndex < vtxCount; vtxIndex++) {
					edF32VECTOR4* pXYZ = VIF_AS_F(vtxReg, 2);

					VU_VTX_TRACE_LOG("_$XYZW_16_ConvBones_Rigid_Simple XYZ original 0x{:x} x: 0x{:x} y: 0x{:x} z: 0x{:x}", vi03, pXYZ->xi, pXYZ->yi, pXYZ->zi);

					pXYZ->x = int12_to_float(pXYZ->xi);
					pXYZ->y = int12_to_float(pXYZ->yi);
					pXYZ->z = int12_to_float(pXYZ->zi);

					VU_VTX_TRACE_LOG("_$XYZW_16_ConvBones_Rigid_Simple XYZ transformed 0x{:x} x: {} y: {} z: {}", vi03, pXYZ->x, pXYZ->y, pXYZ->z);

					vtxReg += 3;
				}
			}
		}

		void _$XYZW_16_ConvBones_Rigid()
		{
			if (bRunSimplifiedCode) {
				_$XYZW_16_ConvBones_Rigid_Simple();
				return;
			}

			vi04 = 0x400;
			vi04 = vi04 & vi01;

			if (vi04 != 0) {

				// vtx data reg
				vi03 = vi15;

				// Vtx count
				vi02 = vi14;

				vi04 = 0x7bff;

				// Load XYZ 0
				vf14 = VIF_LOAD_F(vi03, 2);

				// Load XYZ 1
				vf24 = VIF_LOAD_F(vi03, 5);

				// Conv XYZ 0
				vf15.x = int12_to_float(vf14.xi);
				vf15.y = int12_to_float(vf14.yi);
				vf15.z = int12_to_float(vf14.zi);

				vi01 = vi04 & vi01;

				// _$XYZW_16_Conv_LoopBones_Rigid
				while (vi02 > 0) {
					// Load XYZ 2
					vf04 = VIF_LOAD_F(vi03, 8);

					// Conv XYZ 1
					vf25.x = int12_to_float(vf24.xi);
					vf25.y = int12_to_float(vf24.yi);
					vf25.z = int12_to_float(vf24.zi);

					vi02 -= 3;

					// Store XYZ 0
					VIF_AS_F(vi03, 2)->xyz = vf15.xyz;

					// Load XYZ 0
					vf14 = VIF_LOAD_F(vi03, 11);

					vi03 += 9;

					// Conv XYZ 2
					vf05.x = int12_to_float(vf04.xi);
					vf05.y = int12_to_float(vf04.yi);
					vf05.z = int12_to_float(vf04.zi);

					// Store XYZ 1
					VIF_AS_F(vi03, -4)->xyz = vf25.xyz;

					// Load XYZ 1
					vf24 = VIF_LOAD_F(vi03, 5);

					// Conv XYZ 0
					vf15.x = int12_to_float(vf14.xi);
					vf15.y = int12_to_float(vf14.yi);
					vf15.z = int12_to_float(vf14.zi);

					// Store XYZ 2
					VIF_AS_F(vi03, -1)->xyz = vf05.xyz;
				}
			}
		}

		void _$XYZW_16_Conv_EndBones_Rigid_Simple()
		{
			const int vtxCount = vi14;
			int vtxReg = vi15 + 1;

			int normalReg = vtxReg + 0xd8;

			const int animMatrixMask = 0x7ff;
			const int flagsMask = 0xc000;

			for (int vtxIndex = 0; vtxIndex < vtxCount; vtxIndex++) {
				// Vtx data
				edF32VECTOR4* pSTQ = VIF_AS_F(vtxReg, 0);
				edF32VECTOR4* pRGBA = VIF_AS_F(vtxReg, 1);
				edF32VECTOR4* pXYZ = VIF_AS_F(vtxReg, 2);
				edF32VECTOR4* pNormal = VIF_AS_F(normalReg++, 0);

				const int animMatrixReg = pXYZ->wi & animMatrixMask;
				const edF32MATRIX4 animMatrix = VIF_LOAD_M4(animMatrixReg, 0);


				VU_VTX_TRACE_LOG("Bones_Rigid original XYZ  0x{:x} x: {} y: {} z: {} w: 0x{:x}", vtxReg, pXYZ->x, pXYZ->y, pXYZ->z, pXYZ->wi);
				VU_VTX_TRACE_LOG("Bones_Rigid original RGBA 0x{:x} r: {} g: {} b: {}", vtxReg, pRGBA->xi, pRGBA->yi, pRGBA->zi);
				VU_VTX_TRACE_LOG("Bones_Rigid original STQ  0x{0:x} S: {1} (0x{1:x}) T: {2} (0x{2:x}) Q: {3:.2f}", vtxReg, pSTQ->xi, pSTQ->yi, pSTQ->z);

				VU_VTX_TRACE_LOG("Bones_Rigid anim matrix addr: 0x{:x}\n{}", animMatrixReg, animMatrix.ToString());

				// Drop the animation matrix from the flags.
				pXYZ->wi = pXYZ->wi & flagsMask;

				// Transform other to float.
				pNormal->x = int15_to_float(pNormal->xi);
				pNormal->y = int15_to_float(pNormal->yi);
				pNormal->z = int15_to_float(pNormal->zi);

				pNormal->xyz = (*pNormal * animMatrix).xyz;
				pXYZ->xyz = (*pXYZ * animMatrix).xyz;

				pSTQ->x = int12_to_float(pSTQ->xi);
				pSTQ->y = int12_to_float(pSTQ->yi);

				pRGBA->xyz = ConvertFromInt(pRGBA->xyz);

				VU_VTX_TRACE_LOG("Bones_Rigid transformed XYZ  0x{:x} x: {} y: {} z: {} w: 0x{:x}", vtxReg, pXYZ->x, pXYZ->y, pXYZ->z, pXYZ->wi);
				VU_VTX_TRACE_LOG("Bones_Rigid transformed RGBA 0x{:x} r: {} g: {} b: {}", vtxReg, pRGBA->x, pRGBA->y, pRGBA->z);
				VU_VTX_TRACE_LOG("Bones_Rigid transformed STQ  0x{:x} S: {} T: {} Q: {}", vtxReg, pSTQ->x, pSTQ->y, pSTQ->z);

				vtxReg += 3;
			}
		}

		void _$XYZW_16_Conv_EndBones_Rigid()
		{
			if (bRunSimplifiedCode) {
				_$XYZW_16_Conv_EndBones_Rigid_Simple();
				return;
			}

			// Vtx count
			vi02 = vi14;

			// vtx data reg
			vi03 = vi15 + 1;

			vi04 = vi03 + 0xd8;
			vi05 = 0x7ff;
			vi06 = 0xc000;

			vi09 = VIF_LOAD_I(vi03, 2, VIF_REG_W);

			// Vtx data
			vf25 = VIF_LOAD_F(vi03, 0); // STQ
			vf27 = VIF_LOAD_F(vi03, 1); // RGBA
			vf29 = VIF_LOAD_F(vi03, 2); // XYZ Skip

			VU_VTX_TRACE_LOG("Bones_Rigid original XYZ 0x{:x} vf29 x: {} y: {} z: {} w: 0x{:x}", vi03, vf29.x, vf29.y, vf29.z, vf29.wi);

			vf31 = VIF_LOAD_F(vi04++, 0);

			vi10 = vi09 & vi06;
			vi09 = vi09 & vi05;

			// Anim matrix.
			vf21 = VIF_LOAD_F(vi09++, 0);
			vf22 = VIF_LOAD_F(vi09++, 0);
			vf23 = VIF_LOAD_F(vi09++, 0);
			vf24 = VIF_LOAD_F(vi09++, 0);

			VU_VTX_TRACE_LOG("Bones_Rigid anim matrix addr: 0x{:x} \n{}\n{}\n{}\n{}", vi09 - 4, vf21.ToString(), vf22.ToString(), vf23.ToString(), vf24.ToString());

			vf19.x = int15_to_float(vf31.xi);
			vf19.y = int15_to_float(vf31.yi);
			vf19.z = int15_to_float(vf31.zi);

			vf26.x = int12_to_float(vf25.xi);
			vf26.y = int12_to_float(vf25.yi);

			vf28 = ConvertFromInt(vf27);

			vf31 = vf24 + (vf23 * vf19.z) + (vf22 * vf19.y) + (vf21 * vf19.x);

			while (vi02 > 0) {
				// XYZ * anim matrix
				vf30 = vf24 + (vf23 * vf29.z) + (vf22 * vf29.y) + (vf21 * vf29.x);

				vi07 = VIF_LOAD_I(vi03, 5, VIF_REG_W);

				// Vtx data
				vf05 = VIF_LOAD_F(vi03, 3); // STQ
				vf07 = VIF_LOAD_F(vi03, 4); // RGBA
				vf09 = VIF_LOAD_F(vi03, 5); // XYZ Skip

				VU_VTX_TRACE_LOG("Bones_Rigid original XYZ 0x{:x} vf09 x: {} y: {} z: {} w: 0x{:x}", vi03 + 3, vf09.x, vf09.y, vf09.z, vf09.wi);

				vf11 = VIF_LOAD_F(vi04++, 0);

				*VIF_AS_I(vi03, 2, VIF_REG_W) = vi10;

				(*VIF_AS_F(vi03, 0)).xy = vf26.xy;
				(*VIF_AS_F(vi03, 1)).xyz = vf28.xyz;
				(*VIF_AS_F(vi03, 2)).xyz = vf30.xyz;

				(*VIF_AS_F(vi04, -2)).xyz = vf31.xyz;

				VU_VTX_TRACE_LOG("Bones_Rigid transformed STQ  0x{:x} vf06 S: {} T: {} Q: {}", vi03, vf26.x, vf26.y, vf25.z);
				VU_VTX_TRACE_LOG("Bones_Rigid transformed RGBA 0x{:x} vf08 r: {} g: {} b: {}", vi03, vf28.x, vf28.y, vf28.z);
				VU_VTX_TRACE_LOG("Bones_Rigid transformed XYZ  0x{:x} vf10 x: {} y: {} z: {}", vi03, vf30.x, vf30.y, vf30.z);

				//VU_VTX_TRACE_LOG("vf11 x: {} y: {} z: {}", vf31.x, vf31.y, vf31.z);

				vi08 = vi07 & vi06;
				vi07 = vi07 & vi05;

				// Anim matrix.
				vf01 = VIF_LOAD_F(vi07++, 0);
				vf02 = VIF_LOAD_F(vi07++, 0);
				vf03 = VIF_LOAD_F(vi07++, 0);
				vf04 = VIF_LOAD_F(vi07++, 0);

				VU_VTX_TRACE_LOG("Bones_Rigid anim matrix addr: 0x{:x} \n{}\n{}\n{}\n{}", vi07 - 4, vf01.ToString(), vf02.ToString(), vf03.ToString(), vf04.ToString());

				vi09 = VIF_LOAD_I(vi03, 8, VIF_REG_W);

				// Vtx data
				vf25 = VIF_LOAD_F(vi03, 6);
				vf27 = VIF_LOAD_F(vi03, 7);
				vf29 = VIF_LOAD_F(vi03, 8);

				VU_VTX_TRACE_LOG("Bones_Rigid original XYZ 0x{:x} vf29 x: {} y: {} z: {} w: 0x{:x}", vi03 + 6, vf29.x, vf29.y, vf29.z, vf29.wi);

				vf31 = VIF_LOAD_F(vi04++, 0);

				vf12.x = int15_to_float(vf11.xi);
				vf12.y = int15_to_float(vf11.yi);
				vf12.z = int15_to_float(vf11.zi);

				// ST
				vf06.x = int12_to_float(vf05.xi);
				vf06.y = int12_to_float(vf05.yi);

				vf08 = ConvertFromInt(vf07);

				vi10 = vi09 & vi06;
				vi09 = vi09 & vi05;

				// Normal * anim matrix
				vf11 = vf04 + (vf03 * vf12.z) + (vf02 * vf12.y) + (vf01 * vf12.x);

				// XYZ * anim matrix
				vf10 = vf04 + (vf03 * vf09.z) + (vf02 * vf09.y) + (vf01 * vf09.x);

				// Anim matrix.
				vf21 = VIF_LOAD_F(vi09++, 0);
				vf22 = VIF_LOAD_F(vi09++, 0);
				vf23 = VIF_LOAD_F(vi09++, 0);
				vf24 = VIF_LOAD_F(vi09++, 0);

				VU_VTX_TRACE_LOG("Bones_Rigid anim matrix addr: 0x{:x} \n{}\n{}\n{}\n{}", vi09 - 4, vf21.ToString(), vf22.ToString(), vf23.ToString(), vf24.ToString());

				vf19.x = int15_to_float(vf31.xi);
				vf19.y = int15_to_float(vf31.yi);
				vf19.z = int15_to_float(vf31.zi);

				vf26.x = int12_to_float(vf25.xi);
				vf26.y = int12_to_float(vf25.yi);

				vf28 = ConvertFromInt(vf27);

				vi03 += 6;
				vi02 -= 2;

				*VIF_AS_I(vi03, -1, VIF_REG_W) = vi08;

				(*VIF_AS_F(vi03, -3)).xy = vf06.xy;
				(*VIF_AS_F(vi03, -2)).xyz = vf08.xyz;
				(*VIF_AS_F(vi03, -1)).xyz = vf10.xyz;

				(*VIF_AS_F(vi04, -2)).xyz = vf11.xyz;

				VU_VTX_TRACE_LOG("Bones_Rigid transformed STQ  0x{:x} vf06 S: {} T: {} Q: {}", vi03 - 3, vf06.x, vf06.y, vf05.z);
				VU_VTX_TRACE_LOG("Bones_Rigid transformed RGBA 0x{:x} vf08 r: {} g: {} b: {}", vi03 - 3, vf08.x, vf08.y, vf08.z);
				VU_VTX_TRACE_LOG("Bones_Rigid transformed XYZ  0x{:x} vf10 x: {} y: {} z: {}", vi03 - 3, vf10.x, vf10.y, vf10.z);

				//VU_VTX_TRACE_LOG("vf11 x: {} y: {} z: {}", vf11.x, vf11.y, vf11.z);

				vf31 = vf24 + (vf23 * vf19.z) + (vf22 * vf19.y) + (vf21 * vf19.x);
			}
		}

		void _$Alpha_Object()
		{
			if ((vi01 & 0x20) != 0) {

				// vtx data reg
				vi03 = vi15 + 1;

				// Vtx count
				vi02 = vi14;

				vf05 = VIF_LOAD_F(vi00, 61);

				I = 128;

				vf07 = vf00 * I;

				vf06.x = vf05.xi;
				vf06.y = vf05.yi;
				vf06.z = vf05.zi;
				vf06.w = vf05.wi;

				Q = 1.0f / vf07.w;

				// _$Alpha_Object_Loop
				while (vi02 > 0) {
					// Load RGBA
					vf10 = VIF_LOAD_F(vi03, 1);

					vf11.x = vf10.xi;
					vf11.y = vf10.yi;
					vf11.z = vf10.zi;
					vf11.w = vf10.wi;

					vf12.w = vf11.w * vf06.x;

					vf13 = vf12 * Q;

					vf14.xi = vf13.x;
					vf14.yi = vf13.y;
					vf14.zi = vf13.z;
					vf14.wi = vf13.w;

					vi03 += 3;
					vi02 -= 1;

					// Store RGBA
					VIF_AS_F(vi03, -2)->w = vf14.w;
				}
			}
		}

		void _$XYZW_16_Conv()
		{
			if ((vi01 & 0x400) != 0) {
				// Vtx data reg
				vi03 = vi15 + 1;

				// Vtx count
				vi02 = vi14;

				// Load XYZ 0
				vf14 = VIF_LOAD_F(vi03, 2);

				// Load XYZ 1
				vf24 = VIF_LOAD_F(vi03, 5);

				// Conv XYZ 0
				vf15.x = int12_to_float(vf14.xi);
				vf15.y = int12_to_float(vf14.yi);
				vf15.z = int12_to_float(vf14.zi);

				// _$XYZW_16_Conv_Loop
				while (vi02 > 0) {
					// Load XYZ 2
					vf04 = VIF_LOAD_F(vi03, 8);

					// Conv XYZ 1
					vf25.x = int12_to_float(vf24.xi);
					vf25.y = int12_to_float(vf24.yi);
					vf25.z = int12_to_float(vf24.zi);

					vi02 -= 3;

					// Store XYZ 0
					VIF_AS_F(vi03, 2)->xyz = vf15.xyz;

					// XYZ 0
					vf14 = VIF_LOAD_F(vi03, 11);

					vi03 += 9;

					// Conv XYZ 2
					vf05.x = int12_to_float(vf04.xi);
					vf05.y = int12_to_float(vf04.yi);
					vf05.z = int12_to_float(vf04.zi);

					// Store XYZ 1
					VIF_AS_F(vi03, -4)->xyz = vf25.xyz;

					// Load XYZ 1
					vf24 = VIF_LOAD_F(vi03, 5);

					// Conv XYZ 0
					vf15.x = int12_to_float(vf14.xi);
					vf15.y = int12_to_float(vf14.yi);
					vf15.z = int12_to_float(vf14.zi);

					// Store XYZ 2
					VIF_AS_F(vi03, -1)->xyz = vf05.xyz;
				}
			}
		}

		void _$ParallelLightning_addcolor_Simple()
		{
			if (vi01 & 16) {
				const int vtxCount = vi14;
				int vtxReg = vi15 + 1;
				int normalReg = vtxReg + 0xd8;

				const edF32MATRIX3 lightDirections = VIF_LOAD_M3(vi00, 0x18);
				const edF32MATRIX4 lightColors = VIF_LOAD_M4(vi00, 0x1b);

				edF32VECTOR4 lightAmbient = VIF_LOAD_F(vi00, 0x20);
				const edF32VECTOR4 lightAmbientAdjusted = { lightAmbient.x, lightAmbient.y, lightAmbient.z, 0.0f };

				for (int vtxIndex = 0; vtxIndex < vtxCount; vtxIndex++) {
					edF32VECTOR4 normal = VIF_LOAD_F(normalReg++, 0);
					VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor Normal 0x{:x} x: {} y: {} z: {} w: {}", vtxReg, normal.x, normal.y, normal.z, normal.w);

					normal = (lightDirections.rowZ * normal.z) + (lightDirections.rowY * normal.y) + (lightDirections.rowX * normal.x);
					VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor transformed Normal 0x{:x} vf12 x: {} y: {} z: {} w: {}", vtxReg, normal.x, normal.y, normal.z, normal.w);

					normal.x = std::max<float>(normal.x, 0.0f);
					normal.y = std::max<float>(normal.y, 0.0f);
					normal.z = std::max<float>(normal.z, 0.0f);
					normal.w = std::max<float>(normal.w, 0.0f);
					VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor transformed Normal maxed 0x{:x} vf12 x: {} y: {} z: {} w: {}", vtxReg, normal.x, normal.y, normal.z, normal.w);

					normal = (lightAmbientAdjusted + vf00) + (lightColors.rowT * normal.w) + (lightColors.rowZ * normal.z) + (lightColors.rowY * normal.y) + (lightColors.rowX * normal.x);

					normal.x = std::min<float>(normal.x, 255);
					normal.y = std::min<float>(normal.y, 255);
					normal.z = std::min<float>(normal.z, 255);
					normal.w = std::min<float>(normal.w, 255);

					edF32VECTOR4 rgba = VIF_LOAD_F(vtxReg, 1);

					rgba = rgba * lightAmbient.w;

					rgba = rgba * normal;

					// RGBA 0 result
					rgba.xi = rgba.x;
					rgba.yi = rgba.y;
					rgba.zi = rgba.z;
					rgba.wi = rgba.w;

					(*VIF_AS_F(vtxReg, 1)).xyz = rgba.xyz;

					vtxReg += 3;
				}
			}
		}

		void _$ParallelLightning_addcolor()
		{
			if (bRunSimplifiedCode) {
				_$ParallelLightning_addcolor_Simple();
				return;
			}

			vi04 = 16;
			vi04 = vi04 & vi01;

			if (vi04 != 0x0) {
				
				// Light directions matrix
				vf06 = VIF_LOAD_F(vi00, 0x18);
				vf07 = VIF_LOAD_F(vi00, 0x19);
				vf08 = VIF_LOAD_F(vi00, 0x1a);

				vf06.x = -0.6192f;
				vf06.y = 0.6087f;
				vf06.z = 0.0f;
				vf06.w = 0.0f;

				vf07.x = -0.4442f;
				vf07.y = 0.7061f;
				vf07.z = 0.0f;
				vf07.w = 0.0f;

				vf08.x = 0.6475f;
				vf08.y = 0.3618f;
				vf08.z = 0.0f;
				vf08.w = 0.0f;

				VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor light directions \n{}\n{}\n{}\n", vf06.ToString(), vf07.ToString(), vf08.ToString());

				// Adjusted light ambient
				vf09 = VIF_LOAD_F(vi00, 0x20);

				vf09.x = 70.0568f;
				vf09.y = 44.5816f;
				vf09.z = 62.0958f;
				vf09.w = 0.0078f;

				VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor adjusted light ambient vf09 x: {} y: {} z: {} w: {}", vf09.x, vf09.y, vf09.z, vf09.w);

				// Vtx count
				vi02 = vi14;

				// Vtx data reg
				vi03 = vi15 + 1;

				// Normal reg
				vi04 = vi03 + 216;

				// Unused
				vf28 = vf09;

				// Adjusted light ambient copy
				vf01 = vf09;

				I = 255;
				vf09.w = 0.0f;

				// Load Normal 0
				vf11 = VIF_LOAD_F(vi04++, 0);
				vf11.x = 0.3259f;
				vf11.y = -0.6334f;
				vf11.z = -0.1107f;
				vf11.w = 0.0f;
				VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor Normal 0x{:x} vf11 x: {} y: {} z: {} w: {}", vi04 - 1, vf11.x, vf11.y, vf11.z, vf11.w);

				// Light colour matrix
				vf02 = VIF_LOAD_F(vi00, 0x1b);
				vf03 = VIF_LOAD_F(vi00, 0x1c);
				vf04 = VIF_LOAD_F(vi00, 0x1d);
				vf05 = VIF_LOAD_F(vi00, 0x1e);

				vf02.x = 79.61f;
				vf02.y = 84.3866f;
				vf02.z = 85.9788f;
				vf02.w = 0.0f;

				vf03.x = 103.493f;
				vf03.y = 103.493f;
				vf03.z = 103.493f;
				vf03.w = 0.0f;

				vf04.x = 0.0f;
				vf04.y = 0.0f;
				vf04.z = 0.0f;
				vf04.w = 0.0f;

				vf05.x = 0.0f;
				vf05.y = 0.0f;
				vf05.z = 0.0f;
				vf05.w = 0.0f;

				VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor light color \n{}\n{}\n{}\n{}", vf02.ToString(), vf03.ToString(), vf04.ToString(), vf05.ToString());

				// Normal 0 * Light directions
				vf12 = (vf08 * vf11.z) + (vf07 * vf11.y) + (vf06 * vf11.x);
				VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor transformed Normal 0x{:x} vf12 x: {} y: {} z: {} w: {}", vi04 - 1, vf12.x, vf12.y, vf12.z, vf12.w);

				// Normal 0 MAXx.xyzw
				vf13.x = std::max<float>(vf12.x, 0.0f);
				vf13.y = std::max<float>(vf12.y, 0.0f);
				vf13.z = std::max<float>(vf12.z, 0.0f);
				vf13.w = std::max<float>(vf12.w, 0.0f);
				VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor maxed Normal 0x{:x} vf13 x: {} y: {} z: {} w: {}", vi04 - 1, vf13.x, vf13.y, vf13.z, vf13.w);

				// Load Normal 1
				vf11 = VIF_LOAD_F(vi04++, 0);
				VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor normal 0x{:x} vf11 x: {} y: {} z: {} w: {}", vi04 - 1, vf11.x, vf11.y, vf11.z, vf11.w);

				// Normal 1 * Light directions
				vf12 = (vf08 * vf11.z) + (vf07 * vf11.y) + (vf06 * vf11.x);
				VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor transformed Normal 0x{:x} vf12 x: {} y: {} z: {} w: {}", vi04 - 1, vf12.x, vf12.y, vf12.z, vf12.w);

				// Load RGBA 0
				vf10 = VIF_LOAD_F(vi03, 1);

				vf10.x = 25.0f;
				vf10.y = 25.0f;
				vf10.z = 25.0f;
				vf10.w = 0.0f;

				VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor RGBA 0x{:x} vf10 r: {} g: {} b: {} a: 0x{:x}", vi03, vf10.x, vf10.y, vf10.z, vf10.wi);

				// Maxed Normal 0 * Light colour
				vf14 = (vf09 + vf00) + (vf05 * vf13.w) + (vf04 * vf13.z) + (vf03 * vf13.y) + (vf02 * vf13.x);
				VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor transformed light color 0x{:x} vf06 x: {} y: {} z: {} w: {}", vi04 - 1, vf14.x, vf14.y, vf14.z, vf14.w);

				// RGBA 0 * Light ambient w
				vf18 = vf10 * vf01.w;
				VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor RGBA 0x{:x} vf18 r: {} g: {} b: {} a: {}", vi03, vf18.x, vf18.y, vf18.z, vf18.w);

				// Load Normal 2
				vf11 = VIF_LOAD_F(vi04++, 0);
				VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor Normal 0x{:x} vf11 x: {} y: {} z: {} w: {}", vi04 - 1, vf11.x, vf11.y, vf11.z, vf11.w);

				// Normal 1 MAXx.xyzw
				vf13.x = std::max<float>(vf12.x, 0.0f);
				vf13.y = std::max<float>(vf12.y, 0.0f);
				vf13.z = std::max<float>(vf12.z, 0.0f);
				vf13.w = std::max<float>(vf12.w, 0.0f);
				VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor maxed Normal 0x{:x} vf13 x: {} y: {} z: {} w: {}", vi04 - 1, vf13.x, vf13.y, vf13.z, vf13.w);

				// Normal 2 * Light directions
				vf12 = (vf08 * vf11.z) + (vf07 * vf11.y) + (vf06 * vf11.x);
				VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor transformed Normal 0x{:x} vf12 x: {} y: {} z: {} w: {}", vi04 - 1, vf12.x, vf12.y, vf12.z, vf12.w);

				// _$ParallelLightning_mulcolor_Loop
				while (vi02 > 0) {
					// MINIi Light Color 0
					vf15.x = std::min<float>(vf14.x, I);
					vf15.y = std::min<float>(vf14.y, I);
					vf15.z = std::min<float>(vf14.z, I);
					vf15.w = std::min<float>(vf14.w, I);
					VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor MINIi 0x{:x} vf15 x: {} y: {} z: {} w: {}", vi03, vf15.x, vf15.y, vf15.z, vf15.w);

					// Load RGBA 1
					vf10 = VIF_LOAD_F(vi03, 4);
					VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor RGBA 0x{:x} vf10 r: {} g: {} b: {} a: 0x{:x}", vi03 + 3, vf10.x, vf10.y, vf10.z, vf10.wi);

					// Normal 1 * Light colour
					vf14 = (vf09 + vf00) + (vf05 * vf13.w) + (vf04 * vf13.z) + (vf03 * vf13.y) + (vf02 * vf13.x);
					VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor transformed light color 0x{:x} vf06 x: {} y: {} z: {} w: {}", vi04 - 1, vf14.x, vf14.y, vf14.z, vf14.w);

					// MINIi * RGBA 0
					vf16 = vf15 * vf18;
					VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor Final RGBA 0x{:x} vf16 r: {} g: {} b: {} a: {}", vi03, vf16.x, vf16.y, vf16.z, vf16.w);

					// RGBA 1 * Light ambient w
					vf18 = vf10 * vf01.w;
					VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor RGBA 0x{:x} vf18 r: {} g: {} b: {} a: {}", vi03, vf18.x, vf18.y, vf18.z, vf18.w);

					// Load Normal 0
					vf11 = VIF_LOAD_F(vi04++, 0);
					VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor Normal 0x{:x} vf11 x: {} y: {} z: {} w: {}", vi04 - 1, vf11.x, vf11.y, vf11.z, vf11.w);

					vi02 -= 1;
					vi03 += 3;

					// RGBA 0 result
					vf17.xi = vf16.x;
					vf17.yi = vf16.y;
					vf17.zi = vf16.z;
					vf17.wi = vf16.w;
					VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor Final RGBA integer 0x{:x} vf16 R: {} G: {} B: {} A: {}", vi03 - 3, vf17.xi, vf17.yi, vf17.zi, vf17.wi);

					// Normal 2 MAXx.xyzw
					vf13.x = std::max<float>(vf12.x, 0.0f);
					vf13.y = std::max<float>(vf12.y, 0.0f);
					vf13.z = std::max<float>(vf12.z, 0.0f);
					vf13.w = std::max<float>(vf12.w, 0.0f);
					VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor maxed Normal 0x{:x} vf13 x: {} y: {} z: {} w: {}", vi04 - 1, vf13.x, vf13.y, vf13.z, vf13.w);

					// Normal 0 * Light directions
					vf12 = (vf08 * vf11.z) + (vf07 * vf11.y) + (vf06 * vf11.x);
					VU_VTX_TRACE_LOG("_$ParallelLightning_addcolor transformed Normal 0x{:x} vf12 x: {} y: {} z: {} w: {}", vi04 - 1, vf12.x, vf12.y, vf12.z, vf12.w);

					// Store RGBA 0
					(*VIF_AS_F(vi03, -2)).xyz = vf17.xyz;

					VU_VTX_TRACE_LOG("\n");
				}
			}
			else {
				_$Lightning_Ambiant();
				_$Lightning_Repack();
			}
		}

		void _$Lightning_Ambiant() {
			vi04 = 128;
			vi04 = vi04 & vi01;

			if (vi04 != 0) {
				IMPLEMENTATION_GUARD();
			}
		}

		void _$Lightning_Repack()
		{
			vi03 = vi15 + 1;
			vi02 = vi14;

			vf01 = VIF_LOAD_F(vi00, 32);

			while (vi02 > 0) {
				vf03 = VIF_LOAD_F(vi03, 1);

				vi03 += 3;
				vi02--;

				*reinterpret_cast<int*>(&vf06.x) = (int)vf03.x;
				*reinterpret_cast<int*>(&vf06.y) = (int)vf03.y;
				*reinterpret_cast<int*>(&vf06.z) = (int)vf03.z;
				*reinterpret_cast<int*>(&vf06.w) = (int)vf03.w;

				(*VIF_AS_F(vi03, -2)).xyz = vf06.xyz;

				VU_VTX_TRACE_LOG("_$Lightning_Repack 0x{:x} vf08 r: {:x} g: {:x} b: {:x}", vi03 - 3, vf06.xi, vf06.yi, vf06.zi);
			}
		}

		void _$Env_Mapping()
		{
			vi04 = 64;
			vi04 = vi04 & vi01;

			if (vi04 != 0) {
				IMPLEMENTATION_GUARD();
			}
		}

		void _$XYZW_16_ConvBones_Rigid_noNormal()
		{
			vi04 = 0x400;
			vi04 = vi04 & vi01;

			if (vi04 != 0)
			{
				assert(false);
			}
		}

		void _$XYZW_16_Conv_EndBones_Rigid_noNormal()
		{
			vi02 = vi14;
			vi03 = vi15 + 1;

			vi05 = 0x7ff;
			vi06 = 0xc000;

			vi12 = VIF_LOAD_I(vi03, 2, VIF_REG_W);

			// Vtx data
			vf15 = VIF_LOAD_F(vi03, 0);
			vf19 = VIF_LOAD_F(vi03, 2);

			vi13 = vi12 & vi06;
			vi12 = vi12 & vi05;

			// Anim matrix.
			vf11 = VIF_LOAD_F(vi12++, 0);
			vf12 = VIF_LOAD_F(vi12++, 0);
			vf13 = VIF_LOAD_F(vi12++, 0);
			vf14 = VIF_LOAD_F(vi12++, 0);

			while (vi02 > 0) {
				vi07 = VIF_LOAD_I(vi03, 5, VIF_REG_W);

				// Vtx data
				vf05 = VIF_LOAD_F(vi03, 3);
				vf09 = VIF_LOAD_F(vi03, 5);

				vf16.x = int12_to_float(vf15.xi);
				vf16.y = int12_to_float(vf15.yi);

				vi02 -= 2;

				vi08 = vi07 & vi06;
				vi07 = vi07 & vi05;

				vf20 = vf14 + (vf13 * vf19.z) + (vf12 * vf19.y) + (vf11 * vf19.x);

				// Anim matrix.
				vf01 = VIF_LOAD_F(vi07++, 0);
				vf02 = VIF_LOAD_F(vi07++, 0);
				vf03 = VIF_LOAD_F(vi07++, 0);
				vf04 = VIF_LOAD_F(vi07++, 0);

				*VIF_AS_I(vi03, 2, VIF_REG_W) = vi13;

				(*VIF_AS_F(vi03, 0)).xy = vf16.xy;
				(*VIF_AS_F(vi03, 2)).xyz = vf20.xyz;

				vi12 = VIF_LOAD_I(vi03, 8, VIF_REG_W);

				// Vtx data
				vf15 = VIF_LOAD_F(vi03, 6);
				vf19 = VIF_LOAD_F(vi03, 8);

				vf06.x = int12_to_float(vf05.xi);
				vf06.y = int12_to_float(vf05.yi);

				vi13 = vi12 & vi06;
				vi12 = vi12 & vi05;

				vf10 = vf04 + (vf03 * vf09.z) + (vf02 * vf09.y) + (vf01 * vf09.x);

				// Anim matrix.
				vf11 = VIF_LOAD_F(vi12++, 0);
				vf12 = VIF_LOAD_F(vi12++, 0);
				vf13 = VIF_LOAD_F(vi12++, 0);
				vf14 = VIF_LOAD_F(vi12++, 0);

				vi03 += 6;

				*VIF_AS_I(vi03, -1, VIF_REG_W) = vi08;

				(*VIF_AS_F(vi03, -3)).xy = vf06.xy;
				(*VIF_AS_F(vi03, -1)).xyz = vf10.xyz;
			}
		}
		void _$BonesRigid_Ambiant_Test()
		{
			vi04 = 128;
			vi04 = vi04 & vi01;

			if (vi04 != 0) {
				IMPLEMENTATION_GUARD();
			}
		}

	public:

		void Prepare(char* pMem, int inItop, int inAddr)
		{
			memcpy(fakeMem, pMem, FAKE_VU1_MEM_SIZE);
			itop = inItop;
			addr = inAddr;
		}

		void RunCode()
		{
			RunCode(addr);
		}

		void RunCode(int addr) 
		{
			VU_VTX_TRACE_LOG("RunCode 0x{:x}", addr);

			bool bTestConv16 = true;

			if (addr == 0x3 /*_$DrawingStart_XYZ32 (0x0018)*/) {
				VU_VTX_TRACE_LOG("_$DrawingStart_XYZ32");
				_$DrawingStart_XYZ32();
				_$UnpackData_XYZ32();
			}
			else if (addr == 0xfc /*_$DrawingBones_Rigid (0x07e0)*/) {
				VU_VTX_TRACE_LOG("_$DrawingBones_Rigid");
				_$DrawingBones_Rigid();

				_$XYZW_16_ConvBones_Rigid();

				_$XYZW_16_Conv_EndBones_Rigid();

				_$ParallelLightning_addcolor();

				_$Env_Mapping();

				// _$No_Env_Mapping
			}
			else if (addr == 0x156 /*_$DrawingBones_Rigid_noNormal (0x0ab0)*/) {
				VU_VTX_TRACE_LOG("_$DrawingBones_Rigid_noNormal");
				bTestConv16 = false;

				_$DrawingBones_Rigid_noNormal();
				_$XYZW_16_ConvBones_Rigid_noNormal();
				_$XYZW_16_Conv_EndBones_Rigid_noNormal();

				_$BonesRigid_Ambiant_Test();
			}
			else if (addr == 0x19) {
				_$DrawingStart_XYZ32_Normal();

				_$UnpackData_XYZ32_Normal();

				_$ParallelLightning_addcolor();

				_$Env_Mapping();
			}
			else {
				IMPLEMENTATION_GUARD();
			}

			if (bTestConv16) {
				_$XYZW_16_Conv();

				vi02 = VIF_LOAD_I(vi15, -1, VIF_REG_Y);

				if ((vi02 & 0x1) != 0) {
					// ???
					IMPLEMENTATION_GUARD();
				}
			}

			if ((vi01 & 0x8) != 0) {
				// _$BF_Culling
				IMPLEMENTATION_GUARD();
			}

			_$Alpha_Object();

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

		void EmulateXGKICK(int primReg)
		{
			const std::lock_guard<std::mutex> lock(gKickVertexLock);

			edpkt_data* pkt = reinterpret_cast<edpkt_data*>(VIF_AS_F(primReg, 0));
			HW_Gif_Tag* pGifTag = (HW_Gif_Tag*)(pkt);
			const uint prim = pGifTag->PRIM;
			Renderer::SetPrim(*reinterpret_cast<const GIFReg::GSPrimPacked*>(&prim), pDrawBuffer);

			if (pGifTag->NLOOP > 0x60) {
				Log::GetInstance().ForceFlush();
				assert(false);
			}

			for (int i = 0; i < pGifTag->NLOOP; i++) {
				KickVertexFromReg(primReg + 1 + (i * 3), prim);
			}
		}

		void SetDrawBuffer(PS2::DrawBufferData<Renderer::GSVertex, uint16_t>* pNewDrawBuffer) {
			pDrawBuffer = pNewDrawBuffer;
		}

		Vu1Core(PS2::DrawBufferData<Renderer::GSVertex, uint16_t>* pNewDrawBuffer) : pDrawBuffer(pNewDrawBuffer) { }
		Vu1Core() : pDrawBuffer(nullptr) { }
	};

	void UnpackToAddr(uint addr, const void* data, int size)
	{
		const auto dst = GetFakeMem() + (addr << 4);
		memcpy(dst, data, size);
	}


	void KickCallback(int reg) {
		static Vu1Core interpreterCore(&Renderer::GetDefaultDrawBuffer());
		interpreterCore.Prepare(GetFakeMem(), 0, 0);
		interpreterCore.EmulateXGKICK(reg / 0x10);
	}

	using CorePtr = std::shared_ptr<Vu1Core>;

	struct DrawJob
	{
		using JobPtr = std::shared_ptr<DrawJob>;

		std::vector<CorePtr> cores;
		PS2::DrawBufferData<Renderer::GSVertex, uint16_t> buffer;
		Renderer::TextureData textureData;
		PS2::GSState state;

		void Run() {
			for (auto& core : cores) {
				core->RunCode();
			}

			cores.clear();
		}

		void Draw() {
			Renderer::Draw(buffer, textureData, state);
		}
	};

	struct lock_policy
	{
		using mutex_type = std::mutex;
		using lock_type = std::lock_guard<mutex_type>;
	};

	recycle::shared_pool<DrawJob, lock_policy> gDrawJobPool;
	recycle::shared_pool<Vu1Core, lock_policy> gCorePool;

	DrawJob::JobPtr gNextJob;

	std::vector<DrawJob::JobPtr> gFrameDraws;

	constexpr size_t vu1BlockSize = 0x800;

	struct UnpackOperation {
		VU1Emu::WriteTag tag;
		char* dst;
	};

	std::vector<UnpackOperation> gDelayedFlagWrites;
}

void VU1Emu::BeginFrame()
{
	// Toggle trace on.
	if (bTraceVtx && !bTracingVtx) {
		bTracingVtx = true;
		VU_VTX_TRACE_LOG("Toggling vertex trace! - Using Interpreter: {}", bEnableInterpreter);
	}
}

void VU1Emu::EndFrame()
{
	if (bTracingVtx) {
		bTraceVtx = false;
		bTracingVtx = false;
		Log::GetInstance().ForceFlush();
	}
}

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

static void LogUpdate(const VU1Emu::WriteTag* const pTag) {
	const uint shift = *reinterpret_cast<const uint*>(pTag) >> 15;
	const bool flg = shift & 1;

	const uint offset = (pTag->addr + VU1Emu::gItop) & 0x3ff;
	VU_VTX_TRACE_LOG("VU1Emu::UpdateMemory type: 0x{:x} addr: 0x{:x} (0x{:x}) size: 0x{:x} itop: 0x{:x}", pTag->type, pTag->addr, flg ? offset : pTag->addr, pTag->count, VU1Emu::gItop);
};

void VU1Emu::ProcessVifList(edpkt_data* pVifPkt, bool bRunCode /*= true*/)
{
	int stcl = 0;
	int stwl = 0;
	edF32VECTOR4 strow = { 0.0f, 0.0f, 0.0f, 0.0f };
	uint stmask = 0;
	char* pFakeMem = GetFakeMem();

	VU_VTX_TRACE_LOG("ProcessVifList Begin");

	for (auto& op : gDelayedFlagWrites) {
		int addr = op.tag.addr;
		addr += gItop;
		addr = addr & 0x3ff;
		UnpackToAddr(addr, op.dst, op.tag.count * 0x10);
	}

	gDelayedFlagWrites.clear();

	while (pVifPkt->cmdA != 0x60000000) {
		RunTag* pRunTag = (RunTag*)(&(pVifPkt->asU32[3]));

		if (pVifPkt->asU32[3] == VIF_NOP) {
			ExecTag* pOther = (ExecTag*)(&(pVifPkt->asU32[0]));

			if (pOther->cmd == 0x30) {
				edpkt_data* pExecPktBase = (edpkt_data*)LOAD_SECTION(pVifPkt->asU32[1]);

				for (int i = 0; i < pOther->count; i++) {
					edpkt_data* pExecPkt = pExecPktBase + i;
					for (int j = 0; j < 4; j++) {
						uint vifCmd = GET_VIF_CMD(pExecPkt->asU32[j]);

						CyclePacket* pCycleMaskCmd = (CyclePacket*)pExecPkt;

						switch (vifCmd) {
						case 0x0:
							// NOP
							break;
						case 0x1:
							// Execute commands at addr
							stcl = pCycleMaskCmd->cycle.cl;
							stwl = pCycleMaskCmd->cycle.wl;
							break;
						case 0x20:
							j++;
							stmask = pExecPkt->asU32[j];
							break;
						case 0x30:
							i++;
							strow = pExecPkt[i].asVector;
							i++;
							break;
						}
					}
				}
			}
			else {
				// Shouldn't end up here?
				IMPLEMENTATION_GUARD();
			}
		}
		else if (pRunTag->cmd == VIF_MSCAL) {
			if (!bRunCode) {
				pVifPkt++;
				continue;
			}

			VU_VTX_TRACE_LOG("ProcessVifList VIF_MSCAL - Using Interpreter: {} (addr: 0x{:x})", bEnableInterpreter, pRunTag->addr);

			if (bEnableHardwareDraw) {
				// Just add the vertices here.
				HardwareRenderer::ProcessVertices(GetVertexDataStart());
			}
			else if (bEnableInterpreter) {
				auto GetLocalFakeMem = [=]() {return pFakeMem; };
				edF32VECTOR4* pV = VIF_AS_F(gItop, 4);
				pcsx2_VU::SetKickCallback(KickCallback);
				pcsx2_VU::SetMicro(gVu1Code);
				pcsx2_VU::SetMem((unsigned char*)pFakeMem);
				pcsx2_VU::Execute(pRunTag->addr);
			}
			else {
				if (bRunSingleThreaded) {
					static Vu1Core singleThreadCore(&Renderer::GetDefaultDrawBuffer());
					singleThreadCore.Prepare(pFakeMem, gItop, pRunTag->addr);
					singleThreadCore.RunCode();
				}
				else {
					if (!gNextJob) {

						gNextJob = Debug::bUsePoolForDrawJobs ? gDrawJobPool.allocate() : DrawJob::JobPtr(new DrawJob());
						assert(gNextJob->cores.size() == 0);
					}

					auto core = Debug::bUsePoolForCores ? gCorePool.allocate() : CorePtr(new Vu1Core());
					core->SetDrawBuffer(&gNextJob->buffer);
					core->Prepare(pFakeMem, gItop, pRunTag->addr);

					if (Debug::bDelayCoreExecution) {
						gNextJob->cores.push_back(core);
					}
					else {
						core->RunCode();
					}

					bStartedWork = true;
				}
			}

		}
		else {
			const WriteTag* const pTag = (WriteTag*)(&(pVifPkt->asU32[3]));

			const uint shift = pVifPkt->asU32[3] >> 15;
			const bool flg = shift & 1;

			// Need to handle non flagged packets?
			assert(flg);

			const uint offset = (((pTag->addr + gItop) << 4) & 0x3ff0);

			char* pWriteStart = pFakeMem + offset;

			const int dataSize = 0x10;

			switch (pTag->type) {
			case UNPACK_V4_32_MASKED:
			case UNPACK_V4_32:
			{
				LogUpdate(pTag);

				char* pUnpack = (char*)LOAD_SECTION(pVifPkt->asU32[1]);

				for (int i = 0; i < pTag->count; i += 1) {
					void* const pStart = pWriteStart + (i * dataSize * stcl);
					memcpy(pStart, pUnpack + (i * 0x10), 0x10);
				}
			}
			break;
			case UNPACK_V2_16_MASKED:
			{
				LogUpdate(pTag);

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
			case UNPACK_V4_16_MASKED:
			{
				LogUpdate(pTag);

				short* pUnpack = (short*)LOAD_SECTION(pVifPkt->asU32[1]);

				for (int i = 0; i < pTag->count; i += 1) {
					void* const pStart = pWriteStart + (i * dataSize * stcl);

					edF32VECTOR4* pV = (edF32VECTOR4*)pStart;
					*pV = strow;

					int* pI = (int*)pStart;
					short* pU = pUnpack + (i * 4);
					pI[0] = pU[0];
					pI[1] = pU[1];
					pI[2] = pU[2];
					pI[3] = pU[3];
				}
			}
			break;
			case UNPACK_V4_8_MASKED:
			{
				LogUpdate(pTag);

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
			case UNPACK_S32:
			case UNPACK_S16:
			case UNPACK_S8:
			case UNPACK_V2_32:
			case UNPACK_V2_16:
			case UNPACK_V2_8:
			case UNPACK_V3_32:
			case UNPACK_V3_16:
			case UNPACK_V3_8:
			case UNPACK_V4_16:
			case UNPACK_V4_8:
			case UNPACK_V4_5:
			case UNPACK_S32_MASKED:
			case UNPACK_S16_MASKED:
			case UNPACK_S8_MASKED:
			case UNPACK_V2_32_MASKED:
			case UNPACK_V2_8_MASKED:
			case UNPACK_V3_32_MASKED:
			case UNPACK_V3_16_MASKED:
			case UNPACK_V3_8_MASKED:
			case UNPACK_V4_5_MASKED:
				IMPLEMENTATION_GUARD();
			break;
			}
		}

		pVifPkt++;
	}
}

void VU1Emu::SetVifItop(uint newItop)
{
	pcsx2_VU::SetItop(newItop);
	gItop = newItop;
}

#define CHECK_VIF_CMD(number, cmd) (((unsigned int)number >> (3 << 3)) & 0xff) == cmd
#define GET_UNPACK_LENGTH(number) ((unsigned int)number >> (2 << 3)) & 0xff

void VU1Emu::UpdateMemory(const edpkt_data* pVifPkt, const edpkt_data* pEnd)
{
	VU_VTX_TRACE_LOG("VU1Emu::UpdateMemory Begin 0x{:x} 0x{:x}", (uintptr_t)pVifPkt, (uintptr_t)pEnd);

	while (pVifPkt < pEnd) {
		WriteTag* pTag = (WriteTag*)(&(pVifPkt->asU32[3]));

		if (pTag->type == UNPACK_V4_32 || pTag->type == UNPACK_V4_32_MASKED) {

			LogUpdate(pTag);

			if (pVifPkt->asU32[1] == 0xe) {
				UnpackToAddr(pTag->addr, pVifPkt + 1, pTag->count * 0x10);

				// Jump forward count packets, since they can't be tags.
				pVifPkt += pTag->count;
			}
			else {
				uint addr = pTag->addr;
				uint shift = pVifPkt->asU32[3] >> 15;
				bool flg = shift & 1;

				if (flg) {
					// Flag writes not supported yet, itop isn't correct!
					addr += gItop;
					addr = addr & 0x3ff;
					gDelayedFlagWrites.push_back({ *pTag, (char*)LOAD_SECTION(pVifPkt->asU32[1]) });
				}
				else {
					UnpackToAddr(addr, LOAD_SECTION(pVifPkt->asU32[1]), pTag->count * 0x10);
				}
			}
		}
		else if (pTag->type == UNPACK_S32) {
			IMPLEMENTATION_GUARD();
		}
		else if (pTag->type == UNPACK_V4_8_MASKED) {

			LogUpdate(pTag);

			if (pVifPkt->asU32[1] == 0xe) {
				UnpackToAddr(pTag->addr, pVifPkt + 1, pTag->count * 0x4);

				IMPLEMENTATION_GUARD();
				// NEED TO DO SAME AS V4 32
				// Jump forward count packets, since they can't be tags.
				pVifPkt += pTag->count - 1;
			}
			else {
				uint addr = pTag->addr;
				uint shift = pVifPkt->asU32[3] >> 15;
				bool flg = shift & 1;

				if (flg) {
					addr += gItop;
					addr = addr & 0x3ff;
				}

				UnpackToAddr(addr, LOAD_SECTION(pVifPkt->asU32[1]), pTag->count * 0x4);
			}
		}
		else if (pTag->type == UNPACK_V2_16_MASKED) {
			IMPLEMENTATION_GUARD();
		}
		else if (pTag->type == UNPACK_V4_16_MASKED) {
			IMPLEMENTATION_GUARD();
		}
		pVifPkt++;
	}
}

bool& VU1Emu::GetHardwareDrawEnabled()
{
	return bEnableHardwareDraw;
}

bool& VU1Emu::GetInterpreterEnabled()
{
	return bEnableInterpreter;
}

bool& VU1Emu::GetRunSingleThreaded()
{
	return bRunSingleThreaded;
}

bool& VU1Emu::GetRunSimplifiedCode()
{
	return bRunSimplifiedCode;
}

bool& VU1Emu::GetTraceVtx()
{
	return bTraceVtx;
}

char* VU1Emu::GetVertexDataStart()
{
	return GetFakeMem() + ((gItop) << 4);
}

void VU1Emu::QueueDraw()
{
	auto TraceVertex = [](auto& drawBuffer) {
		if (bTracingVtx) {
			VU_VTX_TRACE_LOG("VU1Emu::QueueDraw");
			VU_VTX_TRACE_LOG("VU1Emu::QueueDraw Vertex count: {} Index count: {}", drawBuffer.vertex.tail, drawBuffer.index.tail);

			VU_VTX_TRACE_LOG("VU1Emu::QueueDraw Vertices:");

			for (int i = 0; i < drawBuffer.vertex.tail; i++) {
				auto* pVertex = &drawBuffer.vertex.buff[i];
				VU_VTX_TRACE_LOG("VU1Emu::QueueDraw 0x{:x} S: {} T: {} Q: {}", i, pVertex->ST[0], pVertex->ST[1], pVertex->Q);
				VU_VTX_TRACE_LOG("VU1Emu::QueueDraw 0x{:x} X: {} Y: {} Z: {}", i, pVertex->XY[0], pVertex->XY[1], pVertex->Z);
				VU_VTX_TRACE_LOG("VU1Emu::QueueDraw 0x{:x} R: {} G: {} B: {} A: {}", i, pVertex->RGBA[0], pVertex->RGBA[1], pVertex->RGBA[2], pVertex->RGBA[3]);
			}

			VU_VTX_TRACE_LOG("");
			VU_VTX_TRACE_LOG("VU1Emu::QueueDraw Indices:");

			for (int i = 0; i < drawBuffer.index.tail; i++) {
				auto* pIndex = &drawBuffer.index.buff[i];
				VU_VTX_TRACE_LOG("VU1Emu::QueueDraw 0x{:x} {}", i, *pIndex);
			}
		}
	};

	if (bEnableHardwareDraw) {
		Renderer::Draw(Renderer::GetHardwareDrawBuffer());
	}
	else if (bRunSingleThreaded) {
		TraceVertex(Renderer::GetDefaultDrawBuffer());

		Renderer::Draw();
	}
	else {
		gNextJob->textureData = Renderer::GetImagePointer();
		gNextJob->state = PS2::GetGSState();

		if (Debug::bUseJobPool) {
			gFrameDraws.push_back(gNextJob);
			gJobPool.push_task([pJob = std::move(gNextJob)]() {
				pJob->Run();
				});
		}
		else {
			gNextJob->Run();		
			gNextJob->Draw();
			gNextJob.reset();
		}
	}
}

void VU1Emu::Wait()
{
	if (bRunSingleThreaded) {
		return;
	}
	
	if (bStartedWork) {
		gJobPool.wait_for_tasks();

		for (auto& pJob : gFrameDraws) {
			pJob->Draw();
		}

		gFrameDraws.clear();

		bStartedWork = false;
	}
}
