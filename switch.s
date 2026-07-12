.global switch_context
.type switch_context, @function

switch_context:
    /* Arguments: switch_context(uint32_t* old_esp, uint32_t new_esp)
     * old_esp is at 4(%esp), new_esp is at 8(%esp)
     */

    /* Save callee-saved registers of the OLD task */
    push %ebp
    push %ebx
    push %esi
    push %edi

    /* Save the current stack pointer into *old_esp */
    mov 20(%esp), %eax     /* old_esp pointer (offset 20 because we pushed 4 regs) */
    mov %esp, (%eax)       /* *old_esp = esp */

    /* Load the new task's stack pointer */
    mov 24(%esp), %esp     /* esp = new_esp (offset 24 for same reason) */

    /* Restore callee-saved registers of the NEW task */
    pop %edi
    pop %esi
    pop %ebx
    pop %ebp

    /* ret pops the return address — for a new task, this is the task function.
     * For a resuming task, this is wherever it was when it got switched out. */
    ret
