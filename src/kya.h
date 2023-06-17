#ifndef _KYA_H
#define _KYA_H

#ifdef PLATFORM_WIN
int main_internal(int argc, char** argv);
#endif

void Init_edFile(void);
void MainInit(int argc, char** argv);

#endif //_KYA_H
