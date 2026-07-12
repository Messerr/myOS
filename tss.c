#include "tss.h"
#include "gdt.h"
#include "io.h"

static struct tss_entry tss;

static void serial_write(const char* str) {
    for (int i = 0; str[i]; i++) outb(0x3F8, str[i]);
}

void tss_init(uint32_t kernel_ss, uint32_t kernel_esp) {
    /* Zero out the TSS */
    uint8_t* p = (uint8_t*)&tss;
    for (uint32_t i = 0; i < sizeof(struct tss_entry); i++) {
        p[i] = 0;
    }

    /* Set the kernel stack for ring 0 */
    tss.ss0 = kernel_ss;
    tss.esp0 = kernel_esp;

    /* Set the I/O map base beyond the TSS limit to disable I/O in user mode */
    tss.iomap_base = sizeof(struct tss_entry);

    /* Add TSS descriptor to GDT at index 5
     * Base = address of our tss struct
     * Limit = size of the struct - 1
     * Access = 0x89: present, ring 0, TSS (non-busy)
     * Granularity = 0x00: byte granularity, 16-bit */
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = sizeof(struct tss_entry) - 1;
    gdt_set_entry(5, base, limit, 0x89, 0x00);

    /* Load the TSS selector into the task register
     * Index 5 * 8 bytes = 0x28, with ring 0 = 0x28 */
    asm volatile ("ltr %%ax" : : "a"(0x28));

    serial_write("TSS initialized.\n");
}

void tss_set_kernel_stack(uint32_t esp) {
    tss.esp0 = esp;
}
