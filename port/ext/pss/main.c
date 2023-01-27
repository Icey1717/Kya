#include "decode.h"
#include <stdio.h>

int main(int argc, char** argv) {
	FILE* pss = NULL;

	if (argc < 2) {
		printf("Not enough args!\nUse: %s PSS-file\n", argv[0]);
		return 1;
	}

	strFileOpen(argv[1]);
	playPss();

	while (!hasVideoEnded()) {
		continuePss();
	}

	destroyPss();
	strFileClose();
}