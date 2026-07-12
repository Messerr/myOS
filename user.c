#include "user.h"
#include "tss.h"
#include "pmm.h"
#include "io.h"

static void serial_write(const char* str) {
    for (int i = 0; str[i]; i++) outb(0x3F8, str[i]);
}

/* --- The actual user program --- */

/* These functions will run in ring 3. They can ONLY talk to the kernel
 * via int 0x80. Any attempt to use outb, cli, hlt, etc. will GPF. */

static void sys_write(const char* str, int len) {
    asm volatile (
        "int $0x80"
        :
        : "a"(1),           /* syscall number: SYS_WRITE */
          "b"((uint32_t)str), /* arg1: string pointer */
          "c"((uint32_t)len)  /* arg2: length */
        : "memory"
    );
}

static void sys_exit(int code) {
    asm volatile (
        "int $0x80"
        :
        : "a"(2),              /* syscall number: SYS_EXIT */
          "b"((uint32_t)code)  /* arg1: exit code */
    );
}

static int user_strlen(const char* s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

/* Helper: write a null-terminated string */
static void user_print(const char* str) {
    sys_write(str, user_strlen(str));
}

/* This is the user program entry point */
static void user_program(void) {
    user_print("Hello from user space! (ring 3)\n");
    user_print("I can only talk to the kernel via syscalls.\n");
    user_print("Calling sys_exit(0)...\n");
    sys_exit(0);
}

/* --- Kernel-side setup to jump to user mode --- */

void user_mode_init(void) {
    serial_write("Jumping to user mode...\n");

    /* Allocate a user-mode stack */
    uint32_t user_stack = pmm_alloc_page();
    uint32_t user_stack_top = user_stack + 4096;

    /* The user code segment selector: GDT index 3, ring 3 = 0x18 | 3 = 0x1B
     * The user data segment selector: GDT index 4, ring 3 = 0x20 | 3 = 0x23 */

    /* To enter ring 3, we fake an iret.
     * iret expects the stack to contain (bottom to top):
     *   SS, ESP, EFLAGS, CS, EIP
     * We push these manually and then iret. */

    asm volatile (
        "cli\n"
        "mov $0x23, %%ax\n"       /* User data segment selector */
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"

        "push $0x23\n"            /* SS: user data segment */
        "push %0\n"               /* ESP: user stack */
        "pushf\n"                 /* EFLAGS */
        "pop %%eax\n"             /* Get EFLAGS */
        "or $0x200, %%eax\n"      /* Set IF (interrupt flag) so interrupts work */
        "push %%eax\n"            /* Push modified EFLAGS */
        "push $0x1B\n"            /* CS: user code segment */
        "push %1\n"               /* EIP: user program entry */
        "iret\n"
        :
        : "r"(user_stack_top),
          "r"((uint32_t)user_program)
        : "eax", "memory"
    );
}
