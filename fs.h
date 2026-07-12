#ifndef FS_H
#define FS_H

#include <stdint.h>
#include <stddef.h>

#define FS_MAX_FILES    32
#define FS_MAX_FILENAME 32
#define FS_MAX_FILESIZE 4096

struct fs_file {
    char     name[FS_MAX_FILENAME];
    uint8_t* data;
    uint32_t size;
    uint8_t  used;  /* 1 = this slot has a file */
};

void fs_init(void);
int  fs_create(const char* name, const char* content, uint32_t size);
int  fs_read(const char* name, char* buffer, uint32_t buf_size);
int  fs_delete(const char* name);
void fs_list(void);
int  fs_write(const char* name, const char* content, uint32_t size);

#endif
