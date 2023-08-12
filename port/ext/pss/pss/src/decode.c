#include <stdint.h>
#include <mpeg2.h>
#include <stdio.h>
#include <stdlib.h>
#include <libfmemopen.h>
#include <mpeg2convert.h>
#include "pss.h"

#define BUFFER_SIZE 4096
static uint8_t buffer[BUFFER_SIZE];
static mpeg2dec_t* decoder;
static const mpeg2_info_t* info;
static const mpeg2_sequence_t* sequence;
static mpeg2_state_t state;
static size_t size = 0;
static int framenum = 0;

static FILE* pss = NULL, * pout = NULL, * sout = NULL, * aout = NULL, * vout = NULL;
static char* pbuffer = NULL, * sbuffer = NULL, * abuffer = NULL, * vbuffer = NULL;

int strFileOpen(char* filename)
{
	if (!(pss = fopen(filename, "rb"))) {
		printf("Couldn't open file %s\n", filename);
		return 1;
	}

	const int bufSize = 0x1000000;

	pbuffer = malloc(bufSize);
	sbuffer = malloc(bufSize);
	abuffer = malloc(bufSize);
	vbuffer = malloc(bufSize);

	pout = fmemopen(pbuffer, bufSize, "wb");
	sout = fmemopen(sbuffer, bufSize, "wb");
	aout = fmemopen(abuffer, bufSize, "wb");
	vout = fmemopen(vbuffer, bufSize, "wb");

	return pout && sout && aout && vout;
}

void playPss()
{
	// Process the PSS
	if (processPss(pss, pout, sout, aout, vout, "Dummy") != 0) {
		return;
	}

	fseek(vout, 0, SEEK_SET);

	decoder = mpeg2_init();
	if (decoder == NULL) {
		fprintf(stderr, "Could not allocate a decoder object.\n");
		return;
	}
	info = mpeg2_info(decoder);

	size = (size_t)-1;
}

char* continuePss()
{
	state = mpeg2_parse(decoder);
	sequence = info->sequence;
	switch (state) {
	case STATE_BUFFER:
		size = fread(buffer, 1, BUFFER_SIZE, vout);
		mpeg2_buffer(decoder, buffer, buffer + size);
		break;
	case STATE_SEQUENCE:
		mpeg2_convert(decoder, mpeg2convert_rgb24, NULL);
		break;
	case STATE_SLICE:
	case STATE_END:
	case STATE_INVALID_END:
		if (info->display_fbuf)
			return info->display_fbuf->buf[0];
		break;
	default:
		break;
	}

	return NULL;
}

int hasVideoEnded()
{
	return size == 0;
}

void destroyPss()
{
	mpeg2_close(decoder);
}

void strFileClose()
{
	fclose(pout);
	fclose(sout);
	fclose(aout);
	fclose(vout);

	free(pbuffer);
	free(sbuffer);
	free(abuffer);
	free(vbuffer);
}
