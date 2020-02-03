
#ifndef DBG__H
#define DBG__H

#include "emul.h"

// Needs to be implemented by user of this dbg util
extern int disasm(char** str, unsigned (*fn)(void*, int), void* userp);


extern unsigned CODE_LEN;

void bp_init();

void set_bp(int);


void run_prompt(Machine* m);

#endif
