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
#include "io.h"

static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;
static const int VGA_WIDTH = 80;
static const int VGA_HEIGHT = 25;

static inline uint16_t vga_entry(unsigned char c, uint8_t color) {
    return (uint16_t) c | (uint16_t) color << 8;
}

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

    const char* str = "MyOS v0.7 - Type in the serial console";
    for (size_t i = 0; str[i] != '\0'; i++) {
        VGA_MEMORY[i] = vga_entry(str[i], color);
    }

    /* Launch the shell */
    shell_init();

    /* Idle loop */
    for (;;) {
        asm volatile ("hlt");
    }
}
