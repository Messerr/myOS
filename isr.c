#include "isr.h"
#include "idt.h"
#include "io.h"

/* Handler table: one function pointer per interrupt */
static isr_handler_t handlers[256] = {0};

/* Exception names for debugging */
static const char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Security Exception", "Reserved"
};

/* Declare all ISR/IRQ stubs from assembly */
extern void isr0(void);  extern void isr1(void);
extern void isr2(void);  extern void isr3(void);
extern void isr4(void);  extern void isr5(void);
extern void isr6(void);  extern void isr7(void);
extern void isr8(void);  extern void isr9(void);
extern void isr10(void); extern void isr11(void);
extern void isr12(void); extern void isr13(void);
extern void isr14(void); extern void isr15(void);
extern void isr16(void); extern void isr17(void);
extern void isr18(void); extern void isr19(void);
extern void isr20(void); extern void isr21(void);
extern void isr22(void); extern void isr23(void);
extern void isr24(void); extern void isr25(void);
extern void isr26(void); extern void isr27(void);
extern void isr28(void); extern void isr29(void);
extern void isr30(void); extern void isr31(void);
extern void isr128(void);

extern void irq0(void);  extern void irq1(void);
extern void irq2(void);  extern void irq3(void);
extern void irq4(void);  extern void irq5(void);
extern void irq6(void);  extern void irq7(void);
extern void irq8(void);  extern void irq9(void);
extern void irq10(void); extern void irq11(void);
extern void irq12(void); extern void irq13(void);
extern void irq14(void); extern void irq15(void);

/* Register all 32 exception handlers in the IDT */
void isr_init(void) {
    /* 0x08 = kernel code segment, 0x8E = present + ring 0 + 32-bit interrupt gate */
    idt_set_entry(0,  (uint32_t)isr0,  0x08, 0x8E);
    idt_set_entry(1,  (uint32_t)isr1,  0x08, 0x8E
);
    idt_set_entry(2,  (uint32_t)isr2,  0x08, 0x8E);
    idt_set_entry(3,  (uint32_t)isr3,  0x08, 0x8E);
    idt_set_entry(4,  (uint32_t)isr4,  0x08, 0x8E);
    idt_set_entry(5,  (uint32_t)isr5,  0x08, 0x8E);
    idt_set_entry(6,  (uint32_t)isr6,  0x08, 0x8E);
    idt_set_entry(7,  (uint32_t)isr7,  0x08, 0x8E);
    idt_set_entry(8,  (uint32_t)isr8,  0x08, 0x8E);
    idt_set_entry(9,  (uint32_t)isr9,  0x08, 0x8E);
    idt_set_entry(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_entry(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_entry(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_entry(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_entry(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_entry(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_entry(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_entry(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_entry(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_entry(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_entry(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_entry(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_entry(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_entry(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_entry(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_entry(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_entry(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_entry(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_entry(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_entry(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_entry(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_entry(31, (uint32_t)isr31, 0x08, 0x8E);
}

/* Remap the PIC and register IRQ handlers in the IDT */
void irq_init(void) {
    /* Remap the PIC: move IRQs from 0-15 to 32-47
     * This is a fixed initialization sequence — don't change the order */
    outb(0x20, 0x11);  /* Start init sequence on master PIC */
    outb(0xA0, 0x11);  /* Start init sequence on slave PIC */
    outb(0x21, 0x20);  /* Master PIC offset: IRQ 0 = interrupt 32 */
    outb(0xA1, 0x28);  /* Slave PIC offset: IRQ 8 = interrupt 40 */
    outb(0x21, 0x04);  /* Tell master: slave is on IRQ 2 */
    outb(0xA1, 0x02);  /* Tell slave: cascade identity */
    outb(0x21, 0x01);  /* 8086 mode for master */
    outb(0xA1, 0x01);  /* 8086 mode for slave */
    outb(0x21, 0x00);  /* Unmask all IRQs on master */
    outb(0xA1, 0x00);  /* Unmask all IRQs on slave */

    /* Register IRQ handlers in the IDT (interrupts 32-47) */
    idt_set_entry(32, (uint32_t)irq0,  0x08, 0x8E);
    idt_set_entry(33, (uint32_t)irq1,  0x08, 0x8E);
    idt_set_entry(34, (uint32_t)irq2,  0x08, 0x8E);
    idt_set_entry(35, (uint32_t)irq3,  0x08, 0x8E);
    idt_set_entry(36, (uint32_t)irq4,  0x08, 0x8E);
    idt_set_entry(37, (uint32_t)irq5,  0x08, 0x8E);
    idt_set_entry(38, (uint32_t)irq6,  0x08, 0x8E);
    idt_set_entry(39, (uint32_t)irq7,  0x08, 0x8E);
    idt_set_entry(40, (uint32_t)irq8,  0x08, 0x8E);
    idt_set_entry(41, (uint32_t)irq9,  0x08, 0x8E);
    idt_set_entry(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_entry(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_entry(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_entry(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_entry(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_entry(47, (uint32_t)irq15, 0x08, 0x8E);
}

/* Register a handler for interrupt N */
void register_interrupt_handler(int n, isr_handler_t handler) {
    handlers[n] = handler;
}

/* Called from isr_common in assembly — handles CPU exceptions */
void isr_handler(struct interrupt_frame* frame) {
    if (handlers[frame->int_no]) {
        handlers[frame->int_no](frame);
    } else {
        /* Default: print the exception name to serial */
        const char* msg = "Exception: ";
        for (int i = 0; msg[i]; i++) outb(0x3F8, msg[i]);

        if (frame->int_no < 32) {
            const char* name = exception_messages[frame->int_no];
            for (int i = 0; name[i]; i++) outb(0x3F8, name[i]);
        }
        outb(0x3F8, '\n');

        /* Halt on unhandled exception */
        asm volatile ("cli; hlt");
    }
}

/* Called from irq_common in assembly — handles hardware interrupts */
void irq_handler(struct interrupt_frame* frame) {
    /* Send End Of Interrupt (EOI) to PIC(s) */
    if (frame->int_no >= 40) {
        outb(0xA0, 0x20);  /* EOI to slave PIC */
    }
    outb(0x20, 0x20);      /* EOI to master PIC */

    /* Call registered handler if any */
    if (handlers[frame->int_no]) {
        handlers[frame->int_no](frame);
    }
}
