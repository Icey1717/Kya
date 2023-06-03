#ifndef _EDCFILER_BNK_H
#define _EDCFILER_BNK_H

#include "Types.h"
#include "edCFiler.h"
#include "edCBankBuffer.h"

struct edCFiler_BNK_8 {
	struct edFILEH* pDebugBankData;
	struct FileHeaderFileData* pFileData;
};

struct edCFiler_Bnk_static {
	char* pFileData;
	char path[512];
	char diskPath[512];
};

class edCFiler_BNK : public edCFiler
{
public:
	edCFiler_BNK();

	virtual bool Init();
	virtual bool get_physical_filename(char* outFilePath, char* pathBuff);
	virtual bool mount_unit(char* filePath, char* bankPath);
	virtual bool open(edFILEH* pOutData, char* filePath);
	virtual bool close(edFILEH* pDebugBank);
	virtual uint read(edFILEH* pDebugBank, char* destination, uint requiredSize);
	virtual bool seek(edFILEH* pDebugBank);
	virtual bool isnowaitcmdend(edCFiler_28_Internal* pEdFilerInternal);

	undefined field_0x24;
	undefined field_0x25;
	undefined field_0x26;
	undefined field_0x27;
	undefined field_0x28;
	undefined field_0x29;
	undefined field_0x2a;
	undefined field_0x2b;
	undefined field_0x2c;
	undefined field_0x2d;
	undefined field_0x2e;
	undefined field_0x2f;
	undefined field_0x30;
	undefined field_0x31;
	undefined field_0x32;
	undefined field_0x33;
	undefined field_0x34;
	undefined field_0x35;
	undefined field_0x36;
	undefined field_0x37;
	undefined field_0x38;
	undefined field_0x39;
	undefined field_0x3a;
	undefined field_0x3b;
	undefined field_0x3c;
	undefined field_0x3d;
	undefined field_0x3e;
	undefined field_0x3f;
	undefined field_0x40;
	undefined field_0x41;
	undefined field_0x42;
	undefined field_0x43;
	undefined field_0x44;
	undefined field_0x45;
	undefined field_0x46;
	undefined field_0x47;
	undefined field_0x48;
	undefined field_0x49;
	undefined field_0x4a;
	undefined field_0x4b;
	undefined field_0x4c;
	undefined field_0x4d;
	undefined field_0x4e;
	undefined field_0x4f;
	undefined field_0x50;
	undefined field_0x51;
	undefined field_0x52;
	undefined field_0x53;
	undefined field_0x54;
	undefined field_0x55;
	undefined field_0x56;
	undefined field_0x57;
	undefined field_0x58;
	undefined field_0x59;
	undefined field_0x5a;
	undefined field_0x5b;
	undefined field_0x5c;
	undefined field_0x5d;
	undefined field_0x5e;
	undefined field_0x5f;
	undefined field_0x60;
	undefined field_0x61;
	undefined field_0x62;
	undefined field_0x63;
	undefined field_0x64;
	undefined field_0x65;
	undefined field_0x66;
	undefined field_0x67;
	undefined field_0x68;
	undefined field_0x69;
	undefined field_0x6a;
	undefined field_0x6b;
	undefined field_0x6c;
	undefined field_0x6d;
	undefined field_0x6e;
	undefined field_0x6f;
	undefined field_0x70;
	undefined field_0x71;
	undefined field_0x72;
	undefined field_0x73;
	undefined field_0x74;
	undefined field_0x75;
	undefined field_0x76;
	undefined field_0x77;
	undefined field_0x78;
	undefined field_0x79;
	undefined field_0x7a;
	undefined field_0x7b;
	undefined field_0x7c;
	undefined field_0x7d;
	undefined field_0x7e;
	undefined field_0x7f;
	undefined field_0x80;
	undefined field_0x81;
	undefined field_0x82;
	undefined field_0x83;
	undefined field_0x84;
	undefined field_0x85;
	undefined field_0x86;
	undefined field_0x87;
	undefined field_0x88;
	undefined field_0x89;
	undefined field_0x8a;
	undefined field_0x8b;
	undefined field_0x8c;
	undefined field_0x8d;
	undefined field_0x8e;
	undefined field_0x8f;
	undefined field_0x90;
	undefined field_0x91;
	undefined field_0x92;
	undefined field_0x93;
	undefined field_0x94;
	undefined field_0x95;
	undefined field_0x96;
	undefined field_0x97;
	undefined field_0x98;
	undefined field_0x99;
	undefined field_0x9a;
	undefined field_0x9b;
	undefined field_0x9c;
	undefined field_0x9d;
	undefined field_0x9e;
	undefined field_0x9f;
	undefined field_0xa0;
	undefined field_0xa1;
	undefined field_0xa2;
	undefined field_0xa3;
	undefined field_0xa4;
	undefined field_0xa5;
	undefined field_0xa6;
	undefined field_0xa7;
	undefined field_0xa8;
	undefined field_0xa9;
	undefined field_0xaa;
	undefined field_0xab;
	undefined field_0xac;
	undefined field_0xad;
	undefined field_0xae;
	undefined field_0xaf;
	undefined field_0xb0;
	undefined field_0xb1;
	undefined field_0xb2;
	undefined field_0xb3;
	undefined field_0xb4;
	undefined field_0xb5;
	undefined field_0xb6;
	undefined field_0xb7;
	undefined field_0xb8;
	undefined field_0xb9;
	undefined field_0xba;
	undefined field_0xbb;
	undefined field_0xbc;
	undefined field_0xbd;
	undefined field_0xbe;
	undefined field_0xbf;
	undefined field_0xc0;
	undefined field_0xc1;
	undefined field_0xc2;
	undefined field_0xc3;
	undefined field_0xc4;
	undefined field_0xc5;
	undefined field_0xc6;
	undefined field_0xc7;
	undefined field_0xc8;
	undefined field_0xc9;
	undefined field_0xca;
	undefined field_0xcb;
	undefined field_0xcc;
	undefined field_0xcd;
	undefined field_0xce;
	undefined field_0xcf;
	undefined field_0xd0;
	undefined field_0xd1;
	undefined field_0xd2;
	undefined field_0xd3;
	undefined field_0xd4;
	undefined field_0xd5;
	undefined field_0xd6;
	undefined field_0xd7;
	undefined field_0xd8;
	undefined field_0xd9;
	undefined field_0xda;
	undefined field_0xdb;
	undefined field_0xdc;
	undefined field_0xdd;
	undefined field_0xde;
	undefined field_0xdf;
	undefined field_0xe0;
	undefined field_0xe1;
	undefined field_0xe2;
	undefined field_0xe3;
	undefined field_0xe4;
	undefined field_0xe5;
	undefined field_0xe6;
	undefined field_0xe7;
	undefined field_0xe8;
	undefined field_0xe9;
	undefined field_0xea;
	undefined field_0xeb;
	undefined field_0xec;
	undefined field_0xed;
	undefined field_0xee;
	undefined field_0xef;
	undefined field_0xf0;
	undefined field_0xf1;
	undefined field_0xf2;
	undefined field_0xf3;
	undefined field_0xf4;
	undefined field_0xf5;
	undefined field_0xf6;
	undefined field_0xf7;
	undefined field_0xf8;
	undefined field_0xf9;
	undefined field_0xfa;
	undefined field_0xfb;
	undefined field_0xfc;
	undefined field_0xfd;
	undefined field_0xfe;
	undefined field_0xff;
	undefined field_0x100;
	undefined field_0x101;
	undefined field_0x102;
	undefined field_0x103;
	undefined field_0x104;
	undefined field_0x105;
	undefined field_0x106;
	undefined field_0x107;
	undefined field_0x108;
	undefined field_0x109;
	undefined field_0x10a;
	undefined field_0x10b;
	undefined field_0x10c;
	undefined field_0x10d;
	undefined field_0x10e;
	undefined field_0x10f;
	undefined field_0x110;
	undefined field_0x111;
	undefined field_0x112;
	undefined field_0x113;
	undefined field_0x114;
	undefined field_0x115;
	undefined field_0x116;
	undefined field_0x117;
	undefined field_0x118;
	undefined field_0x119;
	undefined field_0x11a;
	undefined field_0x11b;
	undefined field_0x11c;
	undefined field_0x11d;
	undefined field_0x11e;
	undefined field_0x11f;
	undefined field_0x120;
	undefined field_0x121;
	undefined field_0x122;
	undefined field_0x123;
	undefined field_0x124;
	undefined field_0x125;
	undefined field_0x126;
	undefined field_0x127;
	undefined field_0x128;
	undefined field_0x129;
	undefined field_0x12a;
	undefined field_0x12b;
	undefined field_0x12c;
	undefined field_0x12d;
	undefined field_0x12e;
	undefined field_0x12f;
	undefined field_0x130;
	undefined field_0x131;
	undefined field_0x132;
	undefined field_0x133;
	undefined field_0x134;
	undefined field_0x135;
	undefined field_0x136;
	undefined field_0x137;
	undefined field_0x138;
	undefined field_0x139;
	undefined field_0x13a;
	undefined field_0x13b;
	undefined field_0x13c;
	undefined field_0x13d;
	undefined field_0x13e;
	undefined field_0x13f;
	undefined field_0x140;
	undefined field_0x141;
	undefined field_0x142;
	undefined field_0x143;
	undefined field_0x144;
	undefined field_0x145;
	undefined field_0x146;
	undefined field_0x147;
	undefined field_0x148;
	undefined field_0x149;
	undefined field_0x14a;
	undefined field_0x14b;
	undefined field_0x14c;
	undefined field_0x14d;
	undefined field_0x14e;
	undefined field_0x14f;
	undefined field_0x150;
	undefined field_0x151;
	undefined field_0x152;
	undefined field_0x153;
	undefined field_0x154;
	undefined field_0x155;
	undefined field_0x156;
	undefined field_0x157;
	undefined field_0x158;
	undefined field_0x159;
	undefined field_0x15a;
	undefined field_0x15b;
	undefined field_0x15c;
	undefined field_0x15d;
	undefined field_0x15e;
	undefined field_0x15f;
	undefined field_0x160;
	undefined field_0x161;
	undefined field_0x162;
	undefined field_0x163;
	undefined field_0x164;
	undefined field_0x165;
	undefined field_0x166;
	undefined field_0x167;
	undefined field_0x168;
	undefined field_0x169;
	undefined field_0x16a;
	undefined field_0x16b;
	undefined field_0x16c;
	undefined field_0x16d;
	undefined field_0x16e;
	undefined field_0x16f;
	undefined field_0x170;
	undefined field_0x171;
	undefined field_0x172;
	undefined field_0x173;
	undefined field_0x174;
	undefined field_0x175;
	undefined field_0x176;
	undefined field_0x177;
	undefined field_0x178;
	undefined field_0x179;
	undefined field_0x17a;
	undefined field_0x17b;
	undefined field_0x17c;
	undefined field_0x17d;
	undefined field_0x17e;
	undefined field_0x17f;
	undefined field_0x180;
	undefined field_0x181;
	undefined field_0x182;
	undefined field_0x183;
	undefined field_0x184;
	undefined field_0x185;
	undefined field_0x186;
	undefined field_0x187;
	undefined field_0x188;
	undefined field_0x189;
	undefined field_0x18a;
	undefined field_0x18b;
	undefined field_0x18c;
	undefined field_0x18d;
	undefined field_0x18e;
	undefined field_0x18f;
	undefined field_0x190;
	undefined field_0x191;
	undefined field_0x192;
	undefined field_0x193;
	undefined field_0x194;
	undefined field_0x195;
	undefined field_0x196;
	undefined field_0x197;
	undefined field_0x198;
	undefined field_0x199;
	undefined field_0x19a;
	undefined field_0x19b;
	undefined field_0x19c;
	undefined field_0x19d;
	undefined field_0x19e;
	undefined field_0x19f;
	undefined field_0x1a0;
	undefined field_0x1a1;
	undefined field_0x1a2;
	undefined field_0x1a3;
	undefined field_0x1a4;
	undefined field_0x1a5;
	undefined field_0x1a6;
	undefined field_0x1a7;
	undefined field_0x1a8;
	undefined field_0x1a9;
	undefined field_0x1aa;
	undefined field_0x1ab;
	undefined field_0x1ac;
	undefined field_0x1ad;
	undefined field_0x1ae;
	undefined field_0x1af;
	undefined field_0x1b0;
	undefined field_0x1b1;
	undefined field_0x1b2;
	undefined field_0x1b3;
	undefined field_0x1b4;
	undefined field_0x1b5;
	undefined field_0x1b6;
	undefined field_0x1b7;
	undefined field_0x1b8;
	undefined field_0x1b9;
	undefined field_0x1ba;
	undefined field_0x1bb;
	undefined field_0x1bc;
	undefined field_0x1bd;
	undefined field_0x1be;
	undefined field_0x1bf;
	undefined field_0x1c0;
	undefined field_0x1c1;
	undefined field_0x1c2;
	undefined field_0x1c3;
	undefined field_0x1c4;
	undefined field_0x1c5;
	undefined field_0x1c6;
	undefined field_0x1c7;
	undefined field_0x1c8;
	undefined field_0x1c9;
	undefined field_0x1ca;
	undefined field_0x1cb;
	undefined field_0x1cc;
	undefined field_0x1cd;
	undefined field_0x1ce;
	undefined field_0x1cf;
	undefined field_0x1d0;
	undefined field_0x1d1;
	undefined field_0x1d2;
	undefined field_0x1d3;
	undefined field_0x1d4;
	undefined field_0x1d5;
	undefined field_0x1d6;
	undefined field_0x1d7;
	undefined field_0x1d8;
	undefined field_0x1d9;
	undefined field_0x1da;
	undefined field_0x1db;
	undefined field_0x1dc;
	undefined field_0x1dd;
	undefined field_0x1de;
	undefined field_0x1df;
	undefined field_0x1e0;
	undefined field_0x1e1;
	undefined field_0x1e2;
	undefined field_0x1e3;
	undefined field_0x1e4;
	undefined field_0x1e5;
	undefined field_0x1e6;
	undefined field_0x1e7;
	undefined field_0x1e8;
	undefined field_0x1e9;
	undefined field_0x1ea;
	undefined field_0x1eb;
	undefined field_0x1ec;
	undefined field_0x1ed;
	undefined field_0x1ee;
	undefined field_0x1ef;
	undefined field_0x1f0;
	undefined field_0x1f1;
	undefined field_0x1f2;
	undefined field_0x1f3;
	undefined field_0x1f4;
	undefined field_0x1f5;
	undefined field_0x1f6;
	undefined field_0x1f7;
	undefined field_0x1f8;
	undefined field_0x1f9;
	undefined field_0x1fa;
	undefined field_0x1fb;
	undefined field_0x1fc;
	undefined field_0x1fd;
	undefined field_0x1fe;
	undefined field_0x1ff;
	undefined field_0x200;
	undefined field_0x201;
	undefined field_0x202;
	undefined field_0x203;
	undefined field_0x204;
	undefined field_0x205;
	undefined field_0x206;
	undefined field_0x207;
	undefined field_0x208;
	undefined field_0x209;
	undefined field_0x20a;
	undefined field_0x20b;
	undefined field_0x20c;
	undefined field_0x20d;
	undefined field_0x20e;
	undefined field_0x20f;
	undefined field_0x210;
	undefined field_0x211;
	undefined field_0x212;
	undefined field_0x213;
	byte bnkInUse[4];
	struct edCFiler_BNK_8 field_0x218[4];
	undefined4 field_0x238;
	undefined4 field_0x23c;
	float field_0x240;
	undefined field_0x244;
	undefined field_0x245;
	undefined field_0x246;
	undefined field_0x247;
	undefined field_0x248;
	undefined field_0x249;
	undefined field_0x24a;
	undefined field_0x24b;
	undefined field_0x24c;
	undefined field_0x24d;
	undefined field_0x24e;
	undefined field_0x24f;
	undefined field_0x250;
	undefined field_0x251;
	undefined field_0x252;
	undefined field_0x253;
	undefined field_0x254;
	undefined field_0x255;
	undefined field_0x256;
	undefined field_0x257;
	undefined field_0x258;
	undefined field_0x259;
	undefined field_0x25a;
	undefined field_0x25b;
	undefined field_0x25c;
	undefined field_0x25d;
	undefined field_0x25e;
	undefined field_0x25f;
	undefined field_0x260;
	undefined field_0x261;
	undefined field_0x262;
	undefined field_0x263;
	undefined field_0x264;
	undefined field_0x265;
	undefined field_0x266;
	undefined field_0x267;
	undefined field_0x268;
	undefined field_0x269;
	undefined field_0x26a;
	undefined field_0x26b;
	undefined field_0x26c;
	undefined field_0x26d;
	undefined field_0x26e;
	undefined field_0x26f;
	undefined field_0x270;
	undefined field_0x271;
	undefined field_0x272;
	undefined field_0x273;
	undefined field_0x274;
	undefined field_0x275;
	undefined field_0x276;
	undefined field_0x277;
	undefined field_0x278;
	undefined field_0x279;
	undefined field_0x27a;
	undefined field_0x27b;
	undefined field_0x27c;
	undefined field_0x27d;
	undefined field_0x27e;
	undefined field_0x27f;
	undefined field_0x280;
	undefined field_0x281;
	undefined field_0x282;
	undefined field_0x283;
	undefined field_0x284;
	undefined field_0x285;
	undefined field_0x286;
	undefined field_0x287;
	undefined field_0x288;
	undefined field_0x289;
	undefined field_0x28a;
	undefined field_0x28b;
	undefined field_0x28c;
	undefined field_0x28d;
	undefined field_0x28e;
	undefined field_0x28f;
	undefined field_0x290;
	undefined field_0x291;
	undefined field_0x292;
	undefined field_0x293;
	undefined field_0x294;
	undefined field_0x295;
	undefined field_0x296;
	undefined field_0x297;
	undefined field_0x298;
	undefined field_0x299;
	undefined field_0x29a;
	undefined field_0x29b;
	undefined field_0x29c;
	undefined field_0x29d;
	undefined field_0x29e;
	undefined field_0x29f;
	undefined field_0x2a0;
	undefined field_0x2a1;
	undefined field_0x2a2;
	undefined field_0x2a3;
	undefined field_0x2a4;
	undefined field_0x2a5;
	undefined field_0x2a6;
	undefined field_0x2a7;
	undefined field_0x2a8;
	undefined field_0x2a9;
	undefined field_0x2aa;
	undefined field_0x2ab;
	undefined field_0x2ac;
	undefined field_0x2ad;
	undefined field_0x2ae;
	undefined field_0x2af;
	undefined field_0x2b0;
	undefined field_0x2b1;
	undefined field_0x2b2;
	undefined field_0x2b3;
	undefined field_0x2b4;
	undefined field_0x2b5;
	undefined field_0x2b6;
	undefined field_0x2b7;
	undefined field_0x2b8;
	undefined field_0x2b9;
	undefined field_0x2ba;
	undefined field_0x2bb;
	undefined field_0x2bc;
	undefined field_0x2bd;
	undefined field_0x2be;
	undefined field_0x2bf;
	undefined field_0x2c0;
	undefined field_0x2c1;
	undefined field_0x2c2;
	undefined field_0x2c3;
	undefined field_0x2c4;
	undefined field_0x2c5;
	undefined field_0x2c6;
	undefined field_0x2c7;
	undefined field_0x2c8;
	undefined field_0x2c9;
	undefined field_0x2ca;
	undefined field_0x2cb;
	undefined field_0x2cc;
	undefined field_0x2cd;
	undefined field_0x2ce;
	undefined field_0x2cf;
};

void Link_00247700(void);

#endif //_EDCFILER_BNK_H
