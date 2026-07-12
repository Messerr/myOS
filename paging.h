#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

/* Page directory/table entry flags */
#define PAGE_PRESENT   0x001
#define PAGE_WRITABLE  0x002
#define PAGE_USER      0x004

void paging_init(void);

#endif
