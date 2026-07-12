#include <stdint.h>
#include <stddef.h>
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "keyboard.h"
#include "pmm.h"
#include "paging.h"
#include "heap.h"
#include "timer.h"
#include "task.h"
#include "fs.h"
#include "shell.h"
#include "tss.h"
#include "syscall.h"
#include "user.h"
#include "io.h"

static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;
static const int VGA_WIDTH = 80;
static const int VGA_HEIGHT = 25;

static inline uint16_t vga_entry(unsigned char c, uint8_t color) {
    return (uint16_t) c | (uint16_t) color << 8;
}

static void serial_write(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        outb(0x3F8, str[i]);
    }
}

/* Kernel stack for TSS (used when returning from user mode) */
static uint8_t kernel_stack[4096] __attribute__((aligned(16)));

void kernel_main(void) {
    const uint8_t color = 0x0F;

    /* Initialize all subsystems */
    gdt_init();
    idt_init();
    isr_init();
    irq_init();
    pmm_init(32 * 1024);
    paging_init();
    heap_init();

    /* TSS must be set up after GDT, before entering user mode */
    tss_init(0x10, (uint32_t)(kernel_stack + 4096));

    syscall_init();
    tasking_init();
    fs_init();
    keyboard_init();
    timer_init(100);

    /* Enable interrupts */
    asm volatile ("sti");

    /* Clear screen */
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_MEMORY[i] = vga_entry(' ', color);
    }

    const char* str = "MyOS v0.8 - Userspace support";
    for (size_t i = 0; str[i] != '\0'; i++) {
        VGA_MEMORY[i] = vga_entry(str[i], color);
    }
    serial_write(str);
    serial_write("\n\n");

    serial_write("=== Entering User Mode ===\n");
    user_mode_init();

    /* We won't reach here — user_mode_init jumps to ring 3
     * and sys_exit halts the system */
    for (;;) {
        asm volatile ("hlt");
    }
}
