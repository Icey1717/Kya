#ifndef _ED_SYSTEM_H
#define _ED_SYSTEM_H

#ifdef PLATFORM_PS2
void _edSystemPS2SpecificInit(void);
void _edSystemPS2SpecificAtMonInit(int argc, char** argv);
#endif

#endif //_ED_SYSTEM_H