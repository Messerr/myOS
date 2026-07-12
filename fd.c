#include "fd.h"

static struct open_file open_files[MAX_OPEN_FILES];

static void str_copy(char* dest, const char* src, int max) {
    int i = 0;
    while (src[i] && i < max - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

void fd_init(void) {
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        open_files[i].used = 0;
    }
}

int fd_open(const char* name) {
    /* Find a free slot */
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (!open_files[i].used) {
            str_copy(open_files[i].name, name, 32);
            open_files[i].used = 1;
            open_files[i].read_pos = 0;
            return i;
        }
    }
    return -1;  /* No free slots */
}

int fd_close(int fd) {
    if (fd < 0 || fd >= MAX_OPEN_FILES) return -1;
    if (!open_files[fd].used) return -1;
    open_files[fd].used = 0;
    return 0;
}

struct open_file* fd_get(int fd) {
    if (fd < 0 || fd >= MAX_OPEN_FILES) return 0;
    if (!open_files[fd].used) return 0;
    return &open_files[fd];
}
