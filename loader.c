#include "loader.h"
#include "fs.h"
#include "pmm.h"
#include "paging.h"
#include "tss.h"
#include "builtin_programs.h"
#include "io.h"

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

static void serial_write_hex(uint32_t val) {
    const char hex[] = "0123456789ABCDEF";
    char buf[11] = "0x00000000";
    for (int i = 9; i >= 2; i--) {
        buf[i] = hex[val & 0xF];
        val >>= 4;
    }
    serial_write(buf);
}

static int str_eq(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

void loader_init(void) {
    /* Install builtin programs into the filesystem */
    for (int i = 0; i < num_builtin_programs; i++) {
        fs_create(builtin_programs[i].name, 
                  (const char*)builtin_programs[i].data,
                  builtin_programs[i].size);
        serial_write("  Installed program: ");
        serial_write(builtin_programs[i].name);
        serial_write(" (");
        serial_write_dec(builtin_programs[i].size);
        serial_write(" bytes)\n");
    }
}

int loader_exec(const char* name) {
    /* Read the program from the filesystem */
    char program_data[8192];  /* Max program size: 8KB */
    int size = fs_read(name, program_data, sizeof(program_data));

    if (size <= 0) {
        serial_write("loader: program not found: ");
        serial_write(name);
        serial_write("\n");
        return -1;
    }

    serial_write("loader: loading '");
    serial_write(name);
    serial_write("' (");
    serial_write_dec(size);
    serial_write(" bytes) at ");
    serial_write_hex(PROGRAM_LOAD_ADDR);
    serial_write("\n");

    /* Copy program to the load address */
    uint8_t* dest = (uint8_t*)PROGRAM_LOAD_ADDR;
    for (int i = 0; i < size; i++) {
        dest[i] = program_data[i];
    }

    /* Set up a fresh user stack */
    static uint8_t prog_stack[16384] __attribute__((aligned(4096)));
    uint32_t prog_stack_top = (uint32_t)(prog_stack + 16384);

    serial_write("loader: jumping to program...\n");

    /* Jump to the program in ring 3 */
    asm volatile (
        "cli\n"
        "mov $0x23, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "push $0x23\n"
        "push %0\n"
        "pushf\n"
        "pop %%eax\n"
        "or $0x200, %%eax\n"
        "push %%eax\n"
        "push $0x1B\n"
        "push %1\n"
        "iret\n"
        :
        : "r"(prog_stack_top),
          "r"((uint32_t)PROGRAM_LOAD_ADDR)
        : "eax", "memory"
    );

    return 0;  /* Won't reach here */
}
