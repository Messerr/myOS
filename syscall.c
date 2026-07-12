#include "syscall.h"
#include "isr.h"
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

static void syscall_handler(struct interrupt_frame* frame) {
    /* Syscall number is in eax, arguments in ebx, ecx, edx */
    uint32_t syscall_num = frame->eax;
    uint32_t arg1 = frame->ebx;
    uint32_t arg2 = frame->ecx;

    switch (syscall_num) {
        case SYS_WRITE: {
            /* arg1 = pointer to string, arg2 = length */
            const char* str = (const char*)arg1;
            for (uint32_t i = 0; i < arg2; i++) {
                outb(0x3F8, str[i]);
            }
            frame->eax = arg2;  /* Return bytes written */
            break;
        }
        case SYS_EXIT: {
            serial_write("[Kernel] User program exited with code ");
            serial_write_dec(arg1);
            serial_write("\n");
            /* For now, just halt. With full multitasking you'd kill the task. */
            serial_write("[Kernel] Returning to kernel idle loop.\n");
            /* We can't easily return to kernel_main from here, so halt */
            asm volatile ("cli; hlt");
            break;
        }
        default:
            serial_write("[Kernel] Unknown syscall: ");
            serial_write_dec(syscall_num);
            serial_write("\n");
            frame->eax = (uint32_t)-1;
            break;
    }
}

void syscall_init(void) {
    /* Register int 0x80 as the syscall interrupt
     * Flags = 0xEE: present, ring 3 (so user code can trigger it), 32-bit interrupt gate
     * The ring 3 flag is critical — without it, int 0x80 from user mode = GPF */
    idt_set_entry(0x80, 0, 0, 0);  /* Clear first */

    /* We need to register via the IDT directly with ring 3 access */
    extern void isr128(void);
    idt_set_entry(0x80, (uint32_t)isr128, 0x08, 0xEE);

    register_interrupt_handler(0x80, syscall_handler);

    serial_write("Syscalls initialized (int 0x80).\n");
}
