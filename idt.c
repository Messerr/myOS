#include "idt.h"

static struct idt_entry idt[256];
static struct idt_ptr ip;

void idt_set_entry(int index, uint32_t base, uint16_t selector, uint8_t flags) {
    idt[index].base_low  = base & 0xFFFF;
    idt[index].base_high = (base >> 16) & 0xFFFF;
    idt[index].selector  = selector;
    idt[index].zero      = 0;
    idt[index].flags     = flags;
}

void idt_init(void) {
    ip.limit = (sizeof(struct idt_entry) * 256) - 1;
    ip.base  = (uint32_t) &idt;

    for (int i = 0; i < 256; i++) {
        idt_set_entry(i, 0, 0, 0);
    }

    asm volatile ("lidt %0" : : "m"(ip));
}
