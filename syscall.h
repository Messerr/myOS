#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

#define SYS_WRITE   1
#define SYS_EXIT    2
#define SYS_READ    3
#define SYS_OPEN    4
#define SYS_FREAD   5
#define SYS_CLOSE   6
#define SYS_FWRITE  7
#define SYS_MALLOC  8
#define SYS_LIST    9
#define SYS_EXEC   10
#define SYS_GFX_INIT  11
#define SYS_GFX_PIXEL  12
#define SYS_GFX_SWAP  13
#define SYS_GFX_CLEAR  14
#define SYS_GFX_EXIT  15

void syscall_init(void);

#endif
