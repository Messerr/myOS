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

void syscall_init(void);

#endif
