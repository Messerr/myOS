#include "fs.h"
#include "heap.h"
#include "io.h"

static struct fs_file file_table[FS_MAX_FILES];

/* Serial helpers */
static void serial_write(const char* str) {
    for (int i = 0; str[i]; i++) outb(0x3F8, str[i]);
}

static void serial_write_dec(uint32_t val) {
    if (val == 0) { outb(0x3F8, '0'); return; }
    char buf[12];
    int i = 0;
    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }
    for (int j = i - 1; j >= 0; j--) outb(0x3F8, buf[j]);
}

static int str_eq(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

static void str_copy(char* dest, const char* src, int max) {
    int i = 0;
    while (src[i] && i < max - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

static int str_len(const char* s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

void fs_init(void) {
    for (int i = 0; i < FS_MAX_FILES; i++) {
        file_table[i].used = 0;
        file_table[i].data = 0;
        file_table[i].size = 0;
        file_table[i].name[0] = '\0';
    }

    /* Create a welcome file */
    const char* welcome = "Welcome to MyOS!\nThis file lives in RAM.\n";
    fs_create("welcome.txt", welcome, str_len(welcome));

    serial_write("Filesystem initialized.\n");
}

int fs_create(const char* name, const char* content, uint32_t size) {
    /* Check if file already exists */
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (file_table[i].used && str_eq(file_table[i].name, name)) {
            serial_write("fs: file already exists: ");
            serial_write(name);
            serial_write("\n");
            return -1;
        }
    }

    /* Find a free slot */
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (!file_table[i].used) {
            str_copy(file_table[i].name, name, FS_MAX_FILENAME);
            file_table[i].data = kmalloc(size + 1);
            if (!file_table[i].data) {
                serial_write("fs: out of memory\n");
                return -1;
            }
            for (uint32_t j = 0; j < size; j++) {
                file_table[i].data[j] = content[j];
            }
            file_table[i].data[size] = '\0';
            file_table[i].size = size;
            file_table[i].used = 1;
            return 0;
        }
    }

    serial_write("fs: file table full\n");
    return -1;
}

int fs_write(const char* name, const char* content, uint32_t size) {
    /* Find existing file and overwrite, or create new */
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (file_table[i].used && str_eq(file_table[i].name, name)) {
            /* Free old data */
            if (file_table[i].data) kfree(file_table[i].data);
            /* Allocate new */
            file_table[i].data = kmalloc(size + 1);
            if (!file_table[i].data) {
                serial_write("fs: out of memory\n");
                file_table[i].used = 0;
                return -1;
            }
            for (uint32_t j = 0; j < size; j++) {
                file_table[i].data[j] = content[j];
            }
            file_table[i].data[size] = '\0';
            file_table[i].size = size;
            return 0;
        }
    }
    /* File doesn't exist — create it */
    return fs_create(name, content, size);
}

int fs_read(const char* name, char* buffer, uint32_t buf_size) {
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (file_table[i].used && str_eq(file_table[i].name, name)) {
            uint32_t copy_size = file_table[i].size;
            if (copy_size >= buf_size) copy_size = buf_size - 1;
            for (uint32_t j = 0; j < copy_size; j++) {
                buffer[j] = file_table[i].data[j];
            }
            buffer[copy_size] = '\0';
            return copy_size;
        }
    }
    return -1;  /* Not found */
}

int fs_delete(const char* name) {
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (file_table[i].used && str_eq(file_table[i].name, name)) {
            if (file_table[i].data) kfree(file_table[i].data);
            file_table[i].used = 0;
            file_table[i].data = 0;
            file_table[i].size = 0;
            file_table[i].name[0] = '\0';
            return 0;
        }
    }
    return -1;  /* Not found */
}

void fs_list(void) {
    int count = 0;
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (file_table[i].used) {
            serial_write("  ");
            serial_write(file_table[i].name);
            serial_write("  (");
            serial_write_dec(file_table[i].size);
            serial_write(" bytes)\n");
            count++;
        }
    }
    if (count == 0) {
        serial_write("  (no files)\n");
    }
}
