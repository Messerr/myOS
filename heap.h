#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stddef.h>

void heap_init(void);
void* kmalloc(size_t size);
void kfree(void* ptr);

/* Debug: print heap state to serial */
void heap_dump(void);

#endif
