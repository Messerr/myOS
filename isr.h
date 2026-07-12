#ifndef ISR_H
#define ISR_H

#include <stdint.h>

/* The struct the CPU + our stub pushes onto the stack */
struct interrupt_frame {
    /* Pushed by our stub: */
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    /* Pushed by the CPU: */
    uint32_t eip, cs, eflags, user_esp, user_ss;
};

/* Handler function type */
typedef void (*isr_handler_t)(struct interrupt_frame* frame);

void isr_init(void);
void irq_init(void);
void register_interrupt_handler(int n, isr_handler_t handler);

#endif
