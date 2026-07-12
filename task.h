#ifndef TASK_H
#define TASK_H

#include <stdint.h>

/* Saved CPU state for a task */
struct task {
    uint32_t id;
    uint32_t esp;          /* Saved stack pointer */
    uint32_t eip;          /* Saved instruction pointer */
    uint32_t* stack;       /* Base of allocated stack */
    uint8_t  state;        /* 0=ready, 1=running, 2=finished */
    struct task* next;     /* Next task in circular list */
};

/* Task function signature */
typedef void (*task_func_t)(void);

void tasking_init(void);
struct task* task_create(task_func_t function);
void schedule(void);
void task_yield(void);

#endif
