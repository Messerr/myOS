.global gdt_flush
.type gdt_flush, @function

gdt_flush:
    mov 4(%esp), %eax    /* get the GDT pointer argument from the stack */
    lgdt (%eax)          /* load the GDT */

    /* Reload data segment registers with kernel data selector (0x10)
     * 0x10 = index 2 * 8 bytes per entry = offset 16 = 0x10
     */
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    /* Reload CS (code segment) by doing a far jump
     * 0x08 = index 1 * 8 bytes per entry = offset 8 = 0x08
     * This is the only way to change CS — you can't mov into it
     */
    ljmp $0x08, $flush_done

flush_done:
    ret
