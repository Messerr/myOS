#include "pmm.h"

/* Bitmap: 1 bit per 4KB page. For 128MB of RAM, that's 4096 bytes.
 * We'll support up to 512MB (16384 bytes of bitmap). */
#define MAX_PAGES (512 * 1024 * 1024 / PAGE_SIZE)
#define BITMAP_SIZE (MAX_PAGES / 32)

static uint32_t bitmap[BITMAP_SIZE];
static uint32_t total_pages;

/* Mark a page as used */
void pmm_mark_used(uint32_t phys_addr) {
    uint32_t page = phys_addr / PAGE_SIZE;
    bitmap[page / 32] |= (1 << (page % 32));
}

/* Mark a page as free */
static void pmm_mark_free(uint32_t phys_addr) {
    uint32_t page = phys_addr / PAGE_SIZE;
    bitmap[page / 32] &= ~(1 << (page % 32));
}

/* Check if a page is used */
static int pmm_is_used(uint32_t page) {
    return bitmap[page / 32] & (1 << (page % 32));
}

void pmm_init(uint32_t mem_size_kb) {
    total_pages = (mem_size_kb * 1024) / PAGE_SIZE;
    if (total_pages > MAX_PAGES) {
        total_pages = MAX_PAGES;
    }

    /* Start with everything marked as used */
    for (uint32_t i = 0; i < BITMAP_SIZE; i++) {
        bitmap[i] = 0xFFFFFFFF;
    }

    /* Free usable memory starting at 2MB (below that is kernel + BIOS stuff)
     * This is conservative — a real OS would parse the memory map from
     * the bootloader to know exactly which regions are usable */
    for (uint32_t addr = 0x200000; addr < mem_size_kb * 1024; addr += PAGE_SIZE) {
        pmm_mark_free(addr);
    }
}

/* Allocate a single 4KB page. Returns physical address, or 0 on failure. */
uint32_t pmm_alloc_page(void) {
    for (uint32_t i = 0; i < total_pages / 32; i++) {
        if (bitmap[i] == 0xFFFFFFFF) continue;  /* All 32 pages used */

        for (int bit = 0; bit < 32; bit++) {
            if (!(bitmap[i] & (1 << bit))) {
                uint32_t page = i * 32 + bit;
                uint32_t addr = page * PAGE_SIZE;
                pmm_mark_used(addr);
                return addr;
            }
        }
    }
    return 0;  /* Out of memory */
}

/* Free a previously allocated page */
void pmm_free_page(uint32_t phys_addr) {
    pmm_mark_free(phys_addr);
}
