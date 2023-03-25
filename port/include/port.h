#pragma once

struct sceCdCLOCK {
	unsigned int second;
};

void sceScfGetLocalTimefromRTC(sceCdCLOCK* localTime);

typedef unsigned short u_short;
typedef unsigned char u_char;
typedef unsigned int u_int;
typedef unsigned long long ulong;

#define settag(t,i,a,q) (t)->id=(i),(t)->next=(sceDmaTag *)(a),(t)->qwc=(u_short)(q)
#define LINK_CHAR(a) ((ulong)0x4b4e494c << 0x20) | a

/*
 * DMA Tag
 */
typedef struct _sceDmaTag {
	u_short             qwc;    /* transfer count */
	u_char              mark;   /* mark */
	u_char              id;     /* tag */
	u_int next;  /* next tag */
	u_int               p[2];   /* padding */
} sceDmaTag __attribute__((aligned(16)));

inline void* sceDmaAddEnd(sceDmaTag** tag, int qwc, void* addr)
{
	void* ret;
	ulong* atag = (ulong*)*tag;
	atag[0] = 0x70000000;
	atag[1] = 0;
	ret = *tag + 1;
	(*tag) += qwc + 1;
	return(ret);
}

inline void* sceDmaAddCall(sceDmaTag** tag, int qwc, void* ctag)
{
	void* ret;
	//settag(*tag, 0x50, ctag, qwc);
	ulong* atag = (ulong*)*tag;
	atag[0] = LINK_CHAR(0x50000000);
	atag[1] = (ulong)ctag;
	ret = *tag + 1;
	(*tag) += qwc + 1;
	return(ret);
}

inline void* sceDmaAddRef(sceDmaTag** tag, int qwc, void* addr)
{
	void* ret;
	//settag(*tag, 0x50, ctag, qwc);
	ulong* atag = (ulong*)*tag;
	atag[0] = LINK_CHAR(qwc | 0x30000000);
	atag[1] = (ulong)addr;
	(*tag)++;
	return(addr);
}

#define SCE_GIF_PACKED      0
#define SCE_GIF_REGLIST     1
#define SCE_GIF_IMAGE       2
#define SCE_GIF_PACKED_AD 0x0e

#define	SCE_GS_FALSE			(0)
#define	SCE_GS_TRUE			(1)

#define SCE_GIF_SET_TAG(nloop, eop, pre, prim, flg, nreg) \
    ((ulong)(nloop) | ((ulong)(eop)<<15) | ((ulong)(pre) << 46) | \
    ((ulong)(prim)<<47) | ((ulong)(flg)<<58) | ((ulong)(nreg)<<60))

#include <stdint.h>
#include <algorithm>

typedef char s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

enum GIF_FLG {
	GIF_FLG_PACKED = 0,
	GIF_FLG_REGLIST = 1,
	GIF_FLG_IMAGE = 2,
	GIF_FLG_IMAGE2 = 3
};

enum GIF_REG {
	GIF_REG_PRIM = 0x00,
	GIF_REG_RGBA = 0x01,
	GIF_REG_STQ = 0x02,
	GIF_REG_UV = 0x03,
	GIF_REG_XYZF2 = 0x04,
	GIF_REG_XYZ2 = 0x05,
	GIF_REG_TEX0_1 = 0x06,
	GIF_REG_TEX0_2 = 0x07,
	GIF_REG_CLAMP_1 = 0x08,
	GIF_REG_CLAMP_2 = 0x09,
	GIF_REG_FOG = 0x0a,
	GIF_REG_INVALID = 0x0b,
	GIF_REG_XYZF3 = 0x0c,
	GIF_REG_XYZ3 = 0x0d,
	GIF_REG_A_D = 0x0e,
	GIF_REG_NOP = 0x0f,
};

static const char GifTag_ModeStr[4][16] = {
	"Packed", "Reglist", "Image", "Image2"
};

static const char GifTag_RegStr[16][16] = {
	"PRIM",    "RGBA",    "STQ",    "UV",
	"XYZF2",   "XYZ2",    "TEX0_1", "TEX0_2",
	"CLAMP_1", "CLAMP_2", "FOG",    "INVALID",
	"XYZF3",   "XYZ3",    "A+D",    "NOP"
};


struct Gif_Tag {
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
	} tag;

	u32  nLoop;		// NLOOP left to process
	u32  nRegs;		// NREG (1~16)
	u32  nRegIdx;	// Current nReg Index (packed mode processing)
	u32  len;		// Packet Length in Bytes (not including tag)
	u32  cycles;    // Time needed to process packet data in ee-cycles
	u8   regs[16];	// Regs
	bool hasAD;		// Has an A+D Write
	bool isValid;	// Tag is valid

	Gif_Tag() { Reset(); }
	Gif_Tag(u8* pMem, bool analyze = false) {
		setTag(pMem, analyze);
	}

	void Reset() { memset(this, 0, sizeof(Gif_Tag)); }
	u8   curReg() { return regs[nRegIdx & 0xf]; }

	void packedStep() {
		if (nLoop > 0) {
			nRegIdx++;
			if (nRegIdx >= nRegs) {
				nRegIdx = 0;
				nLoop--;
			}
		}
	}

	void setTag(u8* pMem, bool analyze = false) {
		tag = *(HW_Gif_Tag*)pMem;
		nLoop = tag.NLOOP;
		hasAD = false;
		nRegIdx = 0;
		isValid = 1;
		len = 0; // avoid uninitialized compiler warning
		switch (tag.FLG) {
		case GIF_FLG_PACKED:
			nRegs = ((tag.NREG - 1) & 0xf) + 1;
			len = (nRegs * tag.NLOOP) * 16;
			cycles = len << 1; // Packed Mode takes 2 ee-cycles
			if (analyze) analyzeTag();
			break;
		case GIF_FLG_REGLIST:
			nRegs = ((tag.NREG - 1) & 0xf) + 1;
			len = ((nRegs * tag.NLOOP + 1) >> 1) * 16;
			cycles = len << 2; // Reg-list Mode takes 4 ee-cycles
			break;
		case GIF_FLG_IMAGE:
		case GIF_FLG_IMAGE2:
			nRegs = 0;
			len = tag.NLOOP * 16;
			cycles = len << 2; // Image Mode takes 4 ee-cycles
			tag.FLG = GIF_FLG_IMAGE;
			break;
			//jNO_DEFAULT;
		}
	}

	void analyzeTag() {
		hasAD = false;
		u32 t = tag.REGS[0];
		u32 i = 0;
		u32 j = std::min<u32>(nRegs, 8);
		for (; i < j; i++) {
			regs[i] = t & 0xf;
			hasAD |= (regs[i] == GIF_REG_A_D);
			t >>= 4;
		}
		t = tag.REGS[1];
		j = nRegs;
		for (; i < j; i++) {
			regs[i] = t & 0xf;
			hasAD |= (regs[i] == GIF_REG_A_D);
			t >>= 4;
		}
	}
};

#define SCE_GS_PSMCT32			(0)
#define SCE_GS_PSMCT24			(1)
#define SCE_GS_PSMCT16			(2)

#define SCE_GS_PSMZ32			(48)
#define SCE_GS_PSMZ24			(49)
#define SCE_GS_PSMZ16			(50)
#define SCE_GS_PSMZ16S			(58)

/* GS registers address */
	/*-- vertex info. reg--*/
#define SCE_GS_PRIM         0x00
#define SCE_GS_RGBAQ        0x01
#define SCE_GS_ST           0x02
#define SCE_GS_UV           0x03
#define SCE_GS_XYZF2        0x04
#define SCE_GS_XYZ2         0x05
#define SCE_GS_FOG          0x0a
#define SCE_GS_XYZF3        0x0c
#define SCE_GS_XYZ3         0x0d
#define SCE_GS_XYOFFSET_1   0x18
#define SCE_GS_XYOFFSET_2   0x19
#define SCE_GS_PRMODECONT   0x1a

	/*-- drawing attribute reg. --*/
#define SCE_GS_PRMODE       0x1b
#define SCE_GS_TEX0_1       0x06
#define SCE_GS_TEX0_2       0x07
#define SCE_GS_TEX1_1       0x14
#define SCE_GS_TEX1_2       0x15
#define SCE_GS_TEX2_1       0x16
#define SCE_GS_TEX2_2       0x17
#define SCE_GS_TEXCLUT      0x1c
#define SCE_GS_SCANMSK      0x22
#define SCE_GS_MIPTBP1_1    0x34
#define SCE_GS_MIPTBP1_2    0x35
#define SCE_GS_MIPTBP2_1    0x36
#define SCE_GS_MIPTBP2_2    0x37
#define SCE_GS_CLAMP_1      0x08
#define SCE_GS_CLAMP_2      0x09
#define SCE_GS_TEXA         0x3b
#define SCE_GS_FOGCOL       0x3d
#define SCE_GS_TEXFLUSH     0x3f

/*-- pixel operation reg. --*/
#define SCE_GS_SCISSOR_1    0x40
#define SCE_GS_SCISSOR_2    0x41
#define SCE_GS_ALPHA_1      0x42
#define SCE_GS_ALPHA_2      0x43
#define SCE_GS_DIMX         0x44
#define SCE_GS_DTHE         0x45
#define SCE_GS_COLCLAMP     0x46
#define SCE_GS_TEST_1       0x47
#define SCE_GS_TEST_2       0x48
#define SCE_GS_PABE         0x49
#define SCE_GS_FBA_1        0x4a
#define SCE_GS_FBA_2        0x4b

	/*-- buffer reg. --*/
#define SCE_GS_FRAME_1      0x4c
#define SCE_GS_FRAME_2      0x4d
#define SCE_GS_ZBUF_1       0x4e
#define SCE_GS_ZBUF_2       0x4f

	/*-- inter-buffer transfer reg. --*/
#define SCE_GS_BITBLTBUF    0x50
#define SCE_GS_TRXPOS       0x51
#define SCE_GS_TRXREG       0x52
#define SCE_GS_TRXDIR       0x53
#define SCE_GS_HWREG        0x54

	/*-- other reg. --*/
#define SCE_GS_SIGNAL       0x60
#define SCE_GS_FINISH       0x61
#define SCE_GS_LABEL        0x62
#define SCE_GS_NOP          0x7f

#define SCE_GS_SET_ALPHA_1  SCE_GS_SET_ALPHA
#define SCE_GS_SET_ALPHA_2  SCE_GS_SET_ALPHA
#define SCE_GS_SET_ALPHA(a, b, c, d, fix) \
    ((ulong)(a)       | ((ulong)(b) << 2)     | ((ulong)(c) << 4) | \
    ((ulong)(d) << 6) | ((ulong)(fix) << 32))

#define SCE_GS_SET_FRAME(fbp, fbw, psm, fbmask) \
    ((ulong)(fbp)        | ((ulong)(fbw) << 16) | \
    ((ulong)(psm) << 24) | ((ulong)(fbmask) << 32))

#define SCE_GS_SET_TEST(ate, atst, aref, afail, date, datm, zte, ztst) \
    ((ulong)(ate)         | ((ulong)(atst) << 1) | \
    ((ulong)(aref) << 4)  | ((ulong)(afail) << 12) | \
    ((ulong)(date) << 14) | ((ulong)(datm) << 15) | \
    ((ulong)(zte) << 16)  | ((ulong)(ztst) << 17))

#define SCE_GS_SET_SCANMSK(msk) ((ulong)(msk))

#define SCE_GS_SET_ZBUF(zbp, psm, zmsk) \
    ((ulong)(zbp) | ((ulong)(psm) << 24) | \
    ((ulong)(zmsk) << 32))

#define SCE_GS_SET_XYOFFSET(ofx, ofy) ((ulong)(ofx) | ((ulong)(ofy) << 32))

#define SCE_GS_SET_SCISSOR(scax0, scax1, scay0, scay1) \
    ((ulong)(scax0)        | ((ulong)(scax1) << 16) | \
    ((ulong)(scay0) << 32) | ((ulong)(scay1) << 48))

#define SCE_GS_ALPHA_CS			(0)
#define SCE_GS_ALPHA_CD			(1)
#define SCE_GS_ALPHA_ZERO		(2)
#define SCE_GS_ALPHA_AS			(0)
#define SCE_GS_ALPHA_AD			(1)
#define SCE_GS_ALPHA_FIX		(2)

#define SCE_GS_PRIM_POINT		(0)
#define SCE_GS_PRIM_LINE		(1)
#define SCE_GS_PRIM_LINESTRIP		(2)
#define SCE_GS_PRIM_TRI			(3)
#define SCE_GS_PRIM_TRISTRIP		(4)
#define SCE_GS_PRIM_TRIFAN		(5)
#define SCE_GS_PRIM_SPRITE		(6)
#define SCE_GS_PRIM_IIP			(1<<3)
#define SCE_GS_PRIM_TME			(1<<4)
#define SCE_GS_PRIM_FGE			(1<<5)
#define SCE_GS_PRIM_ABE			(1<<6)
#define SCE_GS_PRIM_AA1			(1<<7)
#define SCE_GS_PRIM_FST			(1<<8)
#define SCE_GS_PRIM_CTXT1		(0)
#define SCE_GS_PRIM_CTXT2		(1<<9)
#define SCE_GS_PRIM_FIX			(1<<10)

#define SCE_GS_SET_PRIM(prim, iip, tme, fge, abe, aa1, fst, ctxt, fix) \
    ((ulong)(prim)      | ((ulong)(iip) << 3)  | ((ulong)(tme) << 4) | \
    ((ulong)(fge) << 5) | ((ulong)(abe) << 6)  | ((ulong)(aa1) << 7) | \
    ((ulong)(fst) << 8) | ((ulong)(ctxt) << 9) | ((ulong)(fix) << 10))

#define SCE_GS_SET_PRMODECONT(ac) ((ulong)(ac))
#define SCE_GS_SET_COLCLAMP(clamp) ((ulong)(clamp))
#define SCE_GS_SET_DTHE(dthe) ((ulong)(dthe))

#define SCE_GS_NOINTERLACE		(0)
#define SCE_GS_INTERLACE		(1)
#define	SCE_GS_FIELD			(0)
#define	SCE_GS_FRAME			(1)
#define SCE_GS_NTSC			(0x2)
#define SCE_GS_PAL			(0x3)

#define DUMP_TAG(a) { Gif_Tag gifTag; \
gifTag.setTag((u8*)&a, 1); \
MY_LOG("--Gif Tag [mode=%s][pre=%d][prim=%d][nregs=%d][nloop=%d][qwc=%d][EOP=%d]\n", \
	GifTag_ModeStr[gifTag.tag.FLG], gifTag.tag.PRE, gifTag.tag.PRIM, \
	gifTag.nRegs, gifTag.nLoop, gifTag.len / 16, gifTag.tag.EOP); }

/* DMAC */
#define D0_CHCR         ((volatile u_int *)(0x10008000))
#define D0_MADR         ((volatile u_int *)(0x10008010))
#define D0_QWC          ((volatile u_int *)(0x10008020))
#define D0_TADR         ((volatile u_int *)(0x10008030))
#define D0_ASR0         ((volatile u_int *)(0x10008040))
#define D0_ASR1         ((volatile u_int *)(0x10008050))

#define D1_CHCR         ((volatile u_int *)(0x10009000))
#define D1_MADR         ((volatile u_int *)(0x10009010))
#define D1_QWC          ((volatile u_int *)(0x10009020))
#define D1_TADR         ((volatile u_int *)(0x10009030))
#define D1_ASR0         ((volatile u_int *)(0x10009040))
#define D1_ASR1         ((volatile u_int *)(0x10009050))

#define D2_CHCR         ((volatile u_int *)(0x1000a000))
#define D2_MADR         ((volatile u_int *)(0x1000a010))
#define D2_QWC          ((volatile u_int *)(0x1000a020))
#define D2_TADR         ((volatile u_int *)(0x1000a030))
#define D2_ASR0         ((volatile u_int *)(0x1000a040))
#define D2_ASR1         ((volatile u_int *)(0x1000a050))

#define D3_CHCR         ((volatile u_int *)(0x1000b000))
#define D3_MADR         ((volatile u_int *)(0x1000b010))
#define D3_QWC          ((volatile u_int *)(0x1000b020))

#define D4_CHCR         ((volatile u_int *)(0x1000b400))
#define D4_MADR         ((volatile u_int *)(0x1000b410))
#define D4_QWC          ((volatile u_int *)(0x1000b420))
#define D4_TADR         ((volatile u_int *)(0x1000b430))

#define D5_CHCR         ((volatile u_int *)(0x1000c000))
#define D5_MADR         ((volatile u_int *)(0x1000c010))
#define D5_QWC          ((volatile u_int *)(0x1000c020))

#define D6_CHCR         ((volatile u_int *)(0x1000c400))
#define D6_MADR         ((volatile u_int *)(0x1000c410))
#define D6_QWC          ((volatile u_int *)(0x1000c420))
#define D6_TADR         ((volatile u_int *)(0x1000c430))

#define D7_CHCR         ((volatile u_int *)(0x1000c800))
#define D7_MADR         ((volatile u_int *)(0x1000c810))
#define D7_QWC          ((volatile u_int *)(0x1000c820))

#define D8_CHCR         ((volatile u_int *)(0x1000d000))
#define D8_MADR         ((volatile u_int *)(0x1000d010))
#define D8_QWC          ((volatile u_int *)(0x1000d020))
#define D8_SADR         ((volatile u_int *)(0x1000d080))

#define D9_CHCR         ((volatile u_int *)(0x1000d400))
#define D9_MADR         ((volatile u_int *)(0x1000d410))
#define D9_QWC          ((volatile u_int *)(0x1000d420))
#define D9_TADR         ((volatile u_int *)(0x1000d430))
#define D9_SADR         ((volatile u_int *)(0x1000d480))