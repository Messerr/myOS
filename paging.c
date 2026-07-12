#include "paging.h"
#include "pmm.h"
#include "isr.h"
#include "io.h"

/* Page directory: 1024 entries, each pointing to a page table.
 * Must be 4KB-aligned, so we use __attribute__((aligned)). */
static uint32_t page_directory[1024] __attribute__((aligned(4096)));

/* We'll statically allocate page tables for the first 16MB.
 * That's 4 page tables (each covers 4MB). */
static uint32_t page_tables[4][1024] __attribute__((aligned(4096)));

/* Serial output for debug messages */
static void serial_write(const char* str) {
    for (int i = 0; str[i]; i++) {
        outb(0x3F8, str[i]);
    }
}

/* Print a hex number to serial (for debugging) */
static void serial_write_hex(uint32_t val) {
    const char hex[] = "0123456789ABCDEF";
    char buf[11] = "0x00000000";
    for (int i = 9; i >= 2; i--) {
        buf[i] = hex[val & 0xF];
        val >>= 4;
    }
    serial_write(buf);
}

/* Page fault handler */
static void page_fault_handler(struct interrupt_frame* frame) {
    /* The faulting address is stored in CR2 */
    uint32_t fault_addr;
    asm volatile ("mov %%cr2, %0" : "=r"(fault_addr));

    serial_write("PAGE FAULT at address ");
    serial_write_hex(fault_addr);
    serial_write(" | error code: ");
    serial_write_hex(frame->err_code);

    /* Decode the error code */
    serial_write(" [");
    if (!(frame->err_code & 0x1)) serial_write("not-present ");
    if (frame->err_code & 0x2)    serial_write("write ");
    else                          serial_write("read ");
    if (frame->err_code & 0x4)    serial_write("user-mode");
    else                          serial_write("kernel-mode");
    serial_write("]\n");

    /* Halt — can't recover from this yet */
    asm volatile ("cli; hlt");
}

void paging_init(void) {
    /* Zero out the page directory */
    for (int i = 0; i < 1024; i++) {
        page_directory[i] = 0;
    }

    /* Identity-map the first 16MB (4 page tables)
     * Virtual address X = Physical address X */
    for (int t = 0; t < 4; t++) {
        for (int i = 0; i < 1024; i++) {
            uint32_t phys_addr = (t * 1024 + i) * PAGE_SIZE;
            page_tables[t][i] = phys_addr | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
        }

        /* Point the page directory entry at this page table */
        page_directory[t] = (uint32_t)&page_tables[t] | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
    }

    /* Register page fault handler (interrupt 14) */
    register_interrupt_handler(14, page_fault_handler);

    /* Load the page directory into CR3 */
    asm volatile ("mov %0, %%cr3" : : "r"(&page_directory));

    /* Enable paging by setting bit 31 of CR0 */
    uint32_t cr0;
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile ("mov %0, %%cr0" : : "r"(cr0));

    serial_write("Paging enabled. First 16MB identity-mapped.\n");
}
