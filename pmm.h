#ifndef PMM_H
#define PMM_H

#include <stdint.h>

#define PAGE_SIZE 4096

void pmm_init(uint32_t mem_size_kb);
uint32_t pmm_alloc_page(void);
void pmm_free_page(uint32_t phys_addr);
void pmm_mark_used(uint32_t phys_addr);

#endif
