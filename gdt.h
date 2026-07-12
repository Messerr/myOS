#ifndef GDT_H
#define GDT_H

#include <stdint.h>

/* A single GDT entry (8 bytes) */
struct gdt_entry {
    uint16_t limit_low;    /* Lower 16 bits of the limit */
    uint16_t base_low;     /* Lower 16 bits of the base */
    uint8_t  base_middle;  /* Next 8 bits of the base */
    uint8_t  access;       /* Access flags */
    uint8_t  granularity;  /* Granularity + upper 4 bits of limit */
    uint8_t  base_high;    /* Last 8 bits of the base */
} __attribute__((packed));

/* The GDT pointer structure — tells the CPU where the GDT is */
struct gdt_ptr {
    uint16_t limit;   /* Size of the GDT minus 1 */
    uint32_t base;    /* Address of the first GDT entry */
} __attribute__((packed));

/* Set up and install the GDT */
void gdt_init(void);

void gdt_set_entry(int index, uint32_t base, uint32_t limit,
                   uint8_t access, uint8_t granularity);

#endif
