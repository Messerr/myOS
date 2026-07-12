#include "task.h"
#include "heap.h"
#include "io.h"

#define TASK_STACK_SIZE 4096  /* 4KB stack per task */

static struct task* current_task = 0;
static struct task* task_list = 0;
static uint32_t next_id = 1;

/* Serial debug */
static void serial_write(const char* str) {
    for (int i = 0; str[i]; i++) {
        outb(0x3F8, str[i]);
    }
}

static void serial_write_dec(uint32_t val) {
    if (val == 0) { outb(0x3F8, '0'); return; }
    char buf[12];
    int i = 0;
    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }
    for (int j = i - 1; j >= 0; j--) {
        outb(0x3F8, buf[j]);
    }
}

/* Called when a task function returns */
static void task_exit(void) {
    serial_write("Task ");
    serial_write_dec(current_task->id);
    serial_write(" finished.\n");
    current_task->state = 2;  /* Mark as finished */
    /* Yield to next task — we'll never come back */
    task_yield();
    /* Should never reach here */
    for (;;) asm volatile ("hlt");
}

/* Assembly-defined context switch */
extern void switch_context(uint32_t* old_esp, uint32_t new_esp);

void tasking_init(void) {
    /* Create a task struct for the current (kernel_main) execution context.
     * It's already running, so we just need a struct to save its state into. */
    struct task* kernel_task = kmalloc(sizeof(struct task));
    kernel_task->id = 0;
    kernel_task->esp = 0;
    kernel_task->eip = 0;
    kernel_task->stack = 0;  /* Uses the original boot stack */
    kernel_task->state = 1;  /* Running */
    kernel_task->next = kernel_task;  /* Points to itself (circular list) */

    current_task = kernel_task;
    task_list = kernel_task;

    serial_write("Tasking initialized.\n");
}

struct task* task_create(task_func_t function) {
    struct task* new_task = kmalloc(sizeof(struct task));
    uint32_t* stack = kmalloc(TASK_STACK_SIZE);

    new_task->id = next_id++;
    new_task->stack = stack;
    new_task->state = 0;  /* Ready */

    /* Set up the stack so it looks like the task was interrupted.
     * When we switch to it, switch_context will pop these values. */
    uint32_t* sp = (uint32_t*)((uint8_t*)stack + TASK_STACK_SIZE);

    /* Push the return address for when the task function returns */
    *(--sp) = (uint32_t)task_exit;

    /* Push the function address — this is where execution starts */
    *(--sp) = (uint32_t)function;

    /* Push fake saved registers (what switch_context will pop):
     * ebp, ebx, esi, edi */
    *(--sp) = 0;  /* edi */
    *(--sp) = 0;  /* esi */
    *(--sp) = 0;  /* ebx */
    *(--sp) = 0;  /* ebp */

    new_task->esp = (uint32_t)sp;

    /* Insert into circular list */
    new_task->next = current_task->next;
    current_task->next = new_task;

    serial_write("Created task ");
    serial_write_dec(new_task->id);
    serial_write("\n");

    return new_task;
}

void schedule(void) {
    if (current_task == 0) return;

    /* Find the next ready task in the circular list */
    struct task* next = current_task->next;
    int count = 0;
    while (next->state == 2 && count < 20) {
        /* Skip finished tasks */
        next = next->next;
        count++;
    }

    /* If we looped all the way around and only found finished tasks, just return */
    if (next == current_task || next->state == 2) return;

    /* If the next task is the same as current, nothing to switch */
    if (next == current_task) return;

    struct task* prev = current_task;
    current_task = next;

    if (prev->state == 1) prev->state = 0;  /* Was running, now ready */
    current_task->state = 1;  /* Now running */

    /* Perform the context switch */
    switch_context(&prev->esp, current_task->esp);
    asm volatile ("sti");
}

void task_yield(void) {
    schedule();
}
