#ifndef _KYA_H
#define _KYA_H

struct GlobalVideoConfig {
	int screenWidth;
	int screenHeight;
	int field_0x8;
	int isNTSC;
	int omode;
	int offsetX;
	int offsetY;
};

extern GlobalVideoConfig gVideoConfig;

#ifdef PLATFORM_WIN
int main_internal(int argc, char** argv);
#endif

void Init_edFile(void);
void MainInit(int argc, char** argv);

#endif //_KYA_H
