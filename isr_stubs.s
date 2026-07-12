/* --- CPU Exceptions (0-31) --- */

/* Some exceptions push an error code, some don't.
 * For the ones that don't, we push a dummy 0 to keep
 * the stack layout consistent. */

.macro ISR_NO_ERR num
.global isr\num
isr\num:
    push $0          /* dummy error code */
    push $\num       /* interrupt number */
    jmp isr_common
.endm

.macro ISR_ERR num
.global isr\num
isr\num:
    /* CPU already pushed error code */
    push $\num       /* interrupt number */
    jmp isr_common
.endm

/* Exceptions 0-31 */
ISR_NO_ERR 0
ISR_NO_ERR 1
ISR_NO_ERR 2
ISR_NO_ERR 3
ISR_NO_ERR 4
ISR_NO_ERR 5
ISR_NO_ERR 6
ISR_NO_ERR 7
ISR_ERR    8
ISR_NO_ERR 9
ISR_ERR    10
ISR_ERR    11
ISR_ERR    12
ISR_ERR    13
ISR_ERR    14
ISR_NO_ERR 15
ISR_NO_ERR 16
ISR_ERR    17
ISR_NO_ERR 18
ISR_NO_ERR 19
ISR_NO_ERR 20
ISR_NO_ERR 21
ISR_NO_ERR 22
ISR_NO_ERR 23
ISR_NO_ERR 24
ISR_NO_ERR 25
ISR_NO_ERR 26
ISR_NO_ERR 27
ISR_NO_ERR 28
ISR_NO_ERR 29
ISR_ERR    30
ISR_NO_ERR 31

/* --- IRQs (32-47) --- */

.macro IRQ irq_num int_num
.global irq\irq_num
irq\irq_num:
    push $0
    push $\int_num
    jmp irq_common
.endm

IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

/* --- Common ISR handler --- */

.extern isr_handler

isr_common:
    pusha            /* push all general-purpose registers */
    mov %ds, %ax     /* save data segment */
    push %eax

    mov $0x10, %ax   /* load kernel data segment */
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    push %esp        /* push pointer to stack (becomes interrupt_frame*) */
    call isr_handler
    add $4, %esp     /* clean up pushed argument */

    pop %eax         /* restore original data segment */
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    popa             /* restore all general-purpose registers */
    add $8, %esp     /* clean up error code and interrupt number */
    iret             /* return from interrupt */

/* --- Common IRQ handler --- */

.extern irq_handler

irq_common:
    pusha
    mov %ds, %ax
    push %eax

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    push %esp
    call irq_handler
    add $4, %esp

    pop %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    popa
    add $8, %esp
    iret

ISR_NO_ERR 128
