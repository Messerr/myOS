#include "keyboard.h"
#include "isr.h"
#include "io.h"
#include "shell.h"
#include "kbuf.h"

static const char scancode_to_ascii[128] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;
static int cursor_x = 0;
static int cursor_y = 1;

static void vga_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        return;
    }
    if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            VGA_MEMORY[cursor_y * 80 + cursor_x] = (uint16_t) ' ' | (0x0F << 8);
        }
        return;
    }
    if (cursor_x >= 80) {
        cursor_x = 0;
        cursor_y++;
    }
    if (cursor_y >= 25) {
        cursor_y = 1;
    }
    VGA_MEMORY[cursor_y * 80 + cursor_x] = (uint16_t) c | (0x0F << 8);
    cursor_x++;
}

static void keyboard_handler(struct interrupt_frame* frame) {
    (void)frame;
    uint8_t scancode = inb(0x60);
    if (scancode & 0x80) return;
    char c = scancode_to_ascii[scancode];
    if (c) {
        vga_putchar(c);
        kbuf_put(c);
    }
}

static void serial_handler(struct interrupt_frame* frame) {
    (void)frame;
    char c = inb(0x3F8);
    if (c) {
        kbuf_put(c);
    }
}

void keyboard_init(void) {
    kbuf_init();
    register_interrupt_handler(33, keyboard_handler);
    outb(0x3F9, 0x01);
    outb(0x3FC, 0x0B);
    register_interrupt_handler(36, serial_handler);
}
