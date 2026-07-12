#ifndef LOADER_H
#define LOADER_H

#include <stdint.h>

/* Fixed address where user programs are loaded */
#define PROGRAM_LOAD_ADDR 0x00800000

void loader_init(void);
int  loader_exec(const char* name);

#endif
