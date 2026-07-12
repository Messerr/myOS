#include "gdt.h"

/* Our GDT: 5 entries */
static struct gdt_entry gdt[5];
static struct gdt_ptr gp;

/* Defined in gdt_flush.s — loads the GDT into the CPU */
extern void gdt_flush(uint32_t gdt_ptr_addr);

/* Helper: fill in one GDT entry */
static void gdt_set_entry(int index, uint32_t base, uint32_t limit,
                          uint8_t access, uint8_t granularity) {
    gdt[index].base_low    = base & 0xFFFF;
    gdt[index].base_middle = (base >> 16) & 0xFF;
    gdt[index].base_high   = (base >> 24) & 0xFF;

    gdt[index].limit_low   = limit & 0xFFFF;
    gdt[index].granularity  = (limit >> 16) & 0x0F;  /* upper 4 bits of limit */
    gdt[index].granularity |= granularity & 0xF0;     /* flags in upper nibble */

    gdt[index].access = access;
}

void gdt_init(void) {
    /* Set up the GDT pointer */
    gp.limit = (sizeof(struct gdt_entry) * 5) - 1;
    gp.base  = (uint32_t) &gdt;

    /* Entry 0: Null segment (required) */
    gdt_set_entry(0, 0, 0, 0, 0);

    /* Entry 1: Kernel Code segment
     * Base=0, Limit=0xFFFFF, Access=0x9A, Gran=0xCF
     *
     * Access 0x9A = 1 0 01 1 01 0
     *   Present=1, Ring=0, Type=1(code/data), Executable=1, Direction=0,
     *   Readable=1, Accessed=0
     *
     * Granularity 0xCF = 1 1 0 0 | 1111
     *   Granularity=1(4KB pages), Size=1(32-bit), upper limit=0xF
     */
    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xCF);

    /* Entry 2: Kernel Data segment
     * Same as above but Access=0x92 (not executable, writable)
     */
    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xCF);

    /* Entry 3: User Code segment
     * Access=0xFA — same as kernel code but Ring=3
     */
    gdt_set_entry(3, 0, 0xFFFFF, 0xFA, 0xCF);

    /* Entry 4: User Data segment
     * Access=0xF2 — same as kernel data but Ring=3
     */
    gdt_set_entry(4, 0, 0xFFFFF, 0xF2, 0xCF);

    /* Load the GDT into the CPU */
    gdt_flush((uint32_t) &gp);
}
