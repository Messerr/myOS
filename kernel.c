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
#include "tss.h"
#include "syscall.h"
#include "user.h"
#include "io.h"
#include "loader.h"

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

static uint8_t kernel_stack[4096] __attribute__((aligned(16)));

void kernel_main(void) {
    const uint8_t color = 0x0F;

    gdt_init();
    idt_init();
    isr_init();
    irq_init();
    pmm_init(32 * 1024);
    paging_init();
    heap_init();
    tss_init(0x10, (uint32_t)(kernel_stack + 4096));
    syscall_init();
    fs_init();
    loader_init();
    keyboard_init();

    asm volatile ("sti");

    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_MEMORY[i] = vga_entry(' ', color);
    }

    const char* str = "MyOS v0.9 - User-mode shell";
    for (size_t i = 0; str[i] != '\0'; i++) {
        VGA_MEMORY[i] = vga_entry(str[i], color);
    }
    serial_write(str);
    serial_write("\n");

    /* Drop into user mode — runs the shell in ring 3 */
    user_mode_init();

    for (;;) {
        asm volatile ("hlt");
    }
}
