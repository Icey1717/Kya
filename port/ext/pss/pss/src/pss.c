/*
	written by Missingno_force a.k.a. Missingmew
	Copyright (c) 2014-2018
	see LICENSE for details
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <io.h>
#include "libfmemopen.h"

#ifdef ENABLE_PSS_LOG
#define printf
#else
#define log(...)
#endif

typedef struct {
	unsigned char id[4];
	unsigned char sysClockRef[6];
	unsigned char progMuxRate[3];
	uint8_t reservedStuffing;
}__attribute__((packed)) mpegPackHeader;

typedef struct {
	unsigned char id[4];
	unsigned char size[2];
}__attribute__((packed)) mpegPesHeader;

typedef struct {
	unsigned char flags[2];
	uint8_t size; // size of extension data
}__attribute__((packed)) mpegPrivate1ExtensionHead;

/* PACK, PES and the PRIVATE1 extension seem to be actual PSS stuff, the rest seems to be konami specific */

typedef struct {
	unsigned char unknown[3]; // seems to be FF 90 00
	uint8_t streamId;
}__attribute__((packed)) mpegPrivate1ExtensionKonami;

typedef struct {
	uint32_t unknown1;
	uint16_t size; // only for subtitle streams, seems to be 7F 00 for audio
	char samplerate[2]; // only for audio streams, subtitles have this set to 0
	uint8_t channels; // mono files have the next byte also set to 01, stereo files have 0 there
	uint8_t unknown2;
	unsigned char null[6];
}__attribute__((packed)) konamiPrivate1Header;

typedef struct {
	char magic[4];
	unsigned char version[4];
	unsigned char reserved1[4];
	unsigned char size[4];
	char samplerate[4];
	unsigned char reserved2[12];
	char name[0x10];
}__attribute__((packed)) vagHeader;

enum PesType {
	MPEG_SEQUENCE_HEAD =		0x000001B3,
	MPEG_SEQUENCE_END =		0x000001B7,
	MPEG_PROGRAM_END =		0x000001B9,
	MPEG_PACK_HEAD =		0x000001BA,
	MPEG_SYSTEM_HEAD =		0x000001BB,
	MPEG_PRIVATE_STREAM1 =		0x000001BD,
	MPEG_PADDING =			0x000001BE,
	MPEG_VIDEO = 			0x000001E0,
};

enum Private1Type {
	ZOEPT_SUB_JP,
	ZOEPT_VAG,
	ZOEPT_DMX = 0x05,
	ZOEPT_SUB_EN = 0x07,
	ZOEPT_SUB_FR,
	ZOEPT_SUB_DE,
	ZOEPT_SUB_IT,
};

char audioSamplerate[4] = {0};
uint8_t audioNumChannels;
unsigned char vagEnd[0x10] = { 0x00, 0x01, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77 };

int writeFile( FILE *input, unsigned int length, FILE *output ) {
	unsigned char dataBuffer[1024];
	unsigned int bytesLeft = length;
	
	while(bytesLeft) {
		unsigned int wantedRead;
		if(bytesLeft >= sizeof(dataBuffer))
			wantedRead = sizeof(dataBuffer);
		else
			wantedRead = bytesLeft;
		unsigned int haveRead = fread(dataBuffer, 1, wantedRead, input);
		if(haveRead != wantedRead) {
			log("haveRead != wantedRead: %d != %d\n", haveRead, wantedRead);
			perror("This broke");
			return 0;
		}

		unsigned int haveWrite = fwrite(dataBuffer, 1, haveRead, output);
		if(haveWrite != haveRead) {
			log("haveWrite != haveRead: %d != %d\n", haveWrite, haveRead);
			return 0;
		}
		
		bytesLeft -= haveRead;
	}
	return 1;
}

void mpegDumpVideoStream( FILE *f, unsigned int offset, FILE *o ) {
	fseek( f, offset, SEEK_SET );
	uint32_t streamSize, headSize;
	mpegPesHeader pesHead;
	mpegPrivate1ExtensionHead extHead;
	fread( &pesHead, sizeof(pesHead), 1, f );
	headSize = pesHead.size[0] << 8 | pesHead.size[1];
	fread( &extHead, sizeof(extHead), 1, f );
	fseek( f, extHead.size, SEEK_CUR );
	streamSize = headSize - 3 - extHead.size; // subtract 3 for extHead size that is included in pesHead but not extHead
	writeFile( f, streamSize, o );
	fseek( f, offset+headSize+6, SEEK_SET );
	return;
}

void mpegDetectAndDumpPrivateStream( FILE *f, unsigned int offset, FILE *aout, FILE *sout, FILE *pout ) {
	fseek( f, offset, SEEK_SET );
	uint32_t streamSize, headSize;
	char *outfilename = NULL;
	
	mpegPesHeader pesHead;
	mpegPrivate1ExtensionHead extHead;
	
	
	mpegPrivate1ExtensionKonami sonyExtHead;
	konamiPrivate1Header sonyPrivateHead;
	
	
	fread( &pesHead, sizeof(pesHead), 1, f );
	headSize = pesHead.size[0] << 8 | pesHead.size[1];
	fread( &extHead, sizeof(extHead), 1, f );
	fseek( f, extHead.size, SEEK_CUR );
	
	fread( &sonyExtHead, sizeof(sonyExtHead), 1, f );
	fread( &sonyPrivateHead, sizeof(sonyPrivateHead), 1, f );
	
	switch(sonyExtHead.streamId) {
		case ZOEPT_VAG: { // stream 0x01
			if( ftell(aout) == 0 ) {
				streamSize = headSize - 3 - extHead.size - 4 - 16 - (0x800-0x10); // subtract 3 for extHead size that is included in pesHead but not extHead, subtract 4 for sony extension, subtract 16 for sony header subtract 0x800-0x10 for alignment
				fseek( f, 0x800-0x10, SEEK_CUR ); // alignment stuffs
				audioSamplerate[2] = sonyPrivateHead.samplerate[0]; audioSamplerate[3] = sonyPrivateHead.samplerate[1];
				audioNumChannels = sonyPrivateHead.channels;
				unsigned char null[0x40] = {0};
				if( audioNumChannels == 1 ) fwrite( null, 1, 0x30, aout );
				else if( audioNumChannels == 2 ) fwrite( null, 1, 0x40, aout );
			}
			else {
				streamSize = headSize - 3 - extHead.size - 4;
				fseek( f, -0x10, SEEK_CUR ); // seek back size of sonyExtHead because only the first audio block contains it
			}
			writeFile( f, streamSize, aout );
			break;
		}
		case ZOEPT_DMX: { // stream 0x05
			fseek( f, -0x10, SEEK_CUR );
			writeFile( f, headSize, pout );
			break;
		}
		default: { // subtitle streams, always seem to fit into one PES
			streamSize = sonyPrivateHead.size; // for subs this is the actual size, so we can use it here
			//outfilename = malloc( strlen( name ) + 7 + 2 + 4 + 1 );
			//sprintf( outfilename, "%s-stream%02x.bin", name, sonyExtHead.streamId );
			//if( !(sout = fopen( outfilename, "wb" ))) {
			//	printf("Couldnt open file %s\n", outfilename);
			//	return;
			//}
			//writeFile( f, streamSize, sout );
			//fclose(sout);
			//free(outfilename);
			//outfilename = NULL;
			break;
		}
	}
	fseek( f, offset+headSize+6, SEEK_SET );
	return;
}

void vagPrepareHeader( vagHeader *header, uint32_t size, char *name ) {
	header->version[3] = 0x20;
	strncpy( header->name, name, 0x10 );
	header->size[0] = (size >> 24) & 0xFF; header->size[1] = (size >> 16) & 0xFF; header->size[2] = (size >> 8) & 0xFF; header->size[3] = size & 0xFF;
	header->samplerate[0] = 0; header->samplerate[1] = 0; header->samplerate[2] = audioSamplerate[2]; header->samplerate[3] = audioSamplerate[3];
	memset( header->reserved1, 0, 4 );
	memset( header->reserved2, 0, 12 );
	if( audioNumChannels == 1 ) strncpy( header->magic, "VAGp", 4 );
	else if( audioNumChannels == 2 ) strncpy( header->magic, "VAG2", 4 );
	return;
}

void vagWriteHeader( vagHeader header, FILE *o ) {
	fwrite( header.magic, 4, 1, o );
	fwrite( header.version, 4, 1, o );
	fwrite( header.reserved1, 4, 1, o );
	fwrite( header.size, 4, 1, o );
	fwrite( header.samplerate, 4, 1, o );
	fwrite( header.reserved2, 12, 1, o );
	fwrite( header.name, 0x10, 1, o );
	return;
}

int processPss(FILE* pss, FILE* pout, FILE* sout, FILE* aout, FILE* vout, char* name) {
	mpegPackHeader packHead;
	mpegPesHeader tempHead;

	uint32_t curId = 0, headSize = 0;
	unsigned char temp[4];

	fread(temp, 4, 1, pss);
	fseek(pss, -4, SEEK_CUR);
	curId = temp[0] << 24 | temp[1] << 16 | temp[2] << 8 | temp[3];

	while (curId != MPEG_PROGRAM_END) {
		switch (curId) {
		case MPEG_VIDEO: {
			log("Encountered VIDEO header at %08x\n", (unsigned int)ftell(pss));
			mpegDumpVideoStream(pss, ftell(pss), vout);
			break;
		}
		case MPEG_PRIVATE_STREAM1: {
			log("Encountered PRIVATE1 header at %08x\n", (unsigned int)ftell(pss));
			mpegDetectAndDumpPrivateStream(pss, ftell(pss), aout, sout, pout);
			break;
		}
								 // SYSTEM, PADDING and PACKs are ignored for now
		case MPEG_SYSTEM_HEAD: {
			log("Encountered SYSTEM header at %08x", (unsigned int)ftell(pss));
			fread(&tempHead, sizeof(tempHead), 1, pss);
			headSize = tempHead.size[0] << 8 | tempHead.size[1];
			log(" of %04x bytes\n", headSize);
			fseek(pss, headSize, SEEK_CUR);
			break;
		}
		case MPEG_PADDING: {
			log("Encountered padding stream at %08x", (unsigned int)ftell(pss));
			fread(&tempHead, sizeof(tempHead), 1, pss);
			headSize = tempHead.size[0] << 8 | tempHead.size[1];
			log(" of %04x bytes\n", headSize);
			fseek(pss, headSize, SEEK_CUR);
			break;
		}
		case MPEG_PACK_HEAD: {
			log("Encountered PACK header at %08x", (unsigned int)ftell(pss));
			fread(&packHead, sizeof(packHead), 1, pss);
			log(" with %1x bytes padding\n", packHead.reservedStuffing & 0x07);
			fseek(pss, packHead.reservedStuffing & 0x07, SEEK_CUR);
			break;
		}
		default: {
			log("Encountered unknown ID %08x at %08x, stopping...\n", curId, (unsigned int)ftell(pss));
			return 1;
		}
		}
		fread(temp, 4, 1, pss);
		fseek(pss, -4, SEEK_CUR);
		curId = temp[0] << 24 | temp[1] << 16 | temp[2] << 8 | temp[3];
	}

	if (ftell(aout) > 0) {
		unsigned int headSize;
		if (audioNumChannels == 1) headSize = 0x30;
		else if (audioNumChannels == 2) headSize = 0x40;
		fseek(aout, headSize, SEEK_SET);
		unsigned char readbuf[0x10] = { 0 };
		vagHeader header;
		while (fread(readbuf, 0x10, 1, aout)) {
			if (!memcmp(readbuf, vagEnd, 0x10)) break;
		}
		uint32_t numBytes = ftell(aout) / audioNumChannels - headSize;
		vagPrepareHeader(&header, numBytes, name);
		fseek(aout, 0, SEEK_SET);
		vagWriteHeader(header, aout);
	}

	return 0;
}

int processToFile(FILE* pss, char* name) {
	FILE* pout = NULL, * sout = NULL, * aout = NULL, * vout = NULL;

	char* outfilename = malloc(strlen(name) + 7 + 2 + 4 + 1);

	sprintf(outfilename, "%s-stream00.m2v", name);
	if (!(vout = fopen(outfilename, "wb"))) {
		log("Couldnt open file %s\n", outfilename);
		return 1;
	}
	sprintf(outfilename, "%s-stream01.vag", name);
	if (!(aout = fopen(outfilename, "wb+"))) {
		log("Couldnt open file %s\n", outfilename);
		return 1;
	}
	sprintf(outfilename, "%s-stream05.dmx", name);
	if (!(pout = fopen(outfilename, "wb+"))) {
		log("Couldnt open file %s\n", outfilename);
		return 1;
	}

	if (processPss(pss, pout, sout, aout, vout, name) != 0) {
		return 1;
	}

	unsigned int haveAudio = ftell(aout), haveVideo = ftell(vout), havePoly = ftell(pout);

	fclose(aout);
	fclose(vout);
	fclose(pout);
	fflush(stdout);
	if (!haveAudio) {
		sprintf(outfilename, "%s-stream01.vag", name);
		if (remove(outfilename)) log("Couldnt delete unused audio stream %s\n", outfilename);
	}
	if (!haveVideo) {
		sprintf(outfilename, "%s-stream00.m2v", name);
		if (remove(outfilename)) log("Couldnt delete unused video file %s\n", outfilename);
	}
	if (!havePoly) {
		sprintf(outfilename, "%s-stream05.dmx", name);
		if (remove(outfilename)) log("Couldnt delete unused poly file %s\n", outfilename);
	}


	free(outfilename);

	return 0;
}

int processToMemory(FILE* pss, char* name) {
	FILE* pout = NULL, * sout = NULL, * aout = NULL, * vout = NULL;

	const int bufSize = 0x10000;

	char* pbuffer = malloc(bufSize);
	char* sbuffer = malloc(bufSize);
	char* abuffer = malloc(bufSize);
	char* vbuffer = malloc(bufSize);

	{
		pout = fmemopen(pbuffer, bufSize, "wb");
		sout = fmemopen(sbuffer, bufSize, "wb");
		aout = fmemopen(abuffer, bufSize, "wb");
		vout = fmemopen(vbuffer, bufSize, "wb");
	}

	if (processPss(pss, pout, sout, aout, vout, name) != 0) {
		return 1;
	}

	unsigned int haveAudio = ftell(aout), haveVideo = ftell(vout), havePoly = ftell(pout);

	fclose(aout);
	fclose(pout);
	fflush(stdout);

	return 0;
}

//int main( int argc, char **argv ) {
//	FILE* pss = NULL;
//	
//	if( argc < 2 ) {
//		printf("Not enough args!\nUse: %s PSS-file\n", argv[0]);
//		return 1;
//	}
//	
//	if( !(pss = fopen( argv[1], "rb" ))) {
//		printf("Couldnt open file %s\n", argv[1]);
//		return 1;
//	}
//	
//	//return processToFile(pss, argv[1]);
//	return processToMemory(pss, argv[1]);
//}