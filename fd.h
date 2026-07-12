#ifndef FD_H
#define FD_H

#include <stdint.h>

#define MAX_OPEN_FILES 16

struct open_file {
    char name[32];
    uint8_t used;
    uint32_t read_pos;  /* Current read position */
};

void fd_init(void);
int  fd_open(const char* name);
int  fd_close(int fd);
struct open_file* fd_get(int fd);

#endif
