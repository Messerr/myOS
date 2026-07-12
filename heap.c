#include "heap.h"
#include "pmm.h"
#include "io.h"

/* Each block (free or allocated) has this header */
struct block_header {
    uint32_t size;             /* Size of usable area (not including header) */
    uint8_t  is_free;          /* 1 = free, 0 = allocated */
    struct block_header* next; /* Next block in the list */
};

#define HEADER_SIZE sizeof(struct block_header)

/* Minimum block size to avoid tiny useless fragments */
#define MIN_BLOCK_SIZE 16

/* Head of the block list */
static struct block_header* heap_start = NULL;

/* Serial debug helpers */
static void serial_write(const char* str) {
    for (int i = 0; str[i]; i++) {
        outb(0x3F8, str[i]);
    }
}

static void serial_write_hex(uint32_t val) {
    const char hex[] = "0123456789ABCDEF";
    char buf[11] = "0x00000000";
    for (int i = 9; i >= 2; i--) {
        buf[i] = hex[val & 0xF];
        val >>= 4;
    }
    serial_write(buf);
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

/* Request a new page from the PMM and add it to the heap */
static struct block_header* heap_grow(void) {
    uint32_t page = pmm_alloc_page();
    if (page == 0) return NULL;  /* Out of memory */

    struct block_header* new_block = (struct block_header*) page;
    new_block->size = PAGE_SIZE - HEADER_SIZE;
    new_block->is_free = 1;
    new_block->next = NULL;

    /* Append to the end of the list */
    if (heap_start == NULL) {
        heap_start = new_block;
    } else {
        struct block_header* current = heap_start;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_block;
    }

    return new_block;
}

void heap_init(void) {
    /* Start with one page */
    heap_grow();
    serial_write("Heap initialized.\n");
}

/* Split a block if it's much larger than needed */
static void split_block(struct block_header* block, size_t size) {
    /* Only split if there's room for a header + minimum usable space */
    if (block->size >= size + HEADER_SIZE + MIN_BLOCK_SIZE) {
        struct block_header* new_block =
            (struct block_header*)((uint8_t*)block + HEADER_SIZE + size);
        new_block->size = block->size - size - HEADER_SIZE;
        new_block->is_free = 1;
        new_block->next = block->next;

        block->size = size;
        block->next = new_block;
    }
}

void* kmalloc(size_t size) {
    if (size == 0) return NULL;

    /* Align to 4 bytes for safety */
    if (size % 4 != 0) {
        size += 4 - (size % 4);
    }

    /* Search for a free block that's big enough */
    struct block_header* current = heap_start;
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            split_block(current, size);
            current->is_free = 0;
            /* Return pointer to memory AFTER the header */
            return (void*)((uint8_t*)current + HEADER_SIZE);
        }
        current = current->next;
    }

    /* No block found — grow the heap */
    struct block_header* new_block = heap_grow();
    if (new_block == NULL) return NULL;

    /* If one page isn't enough, keep growing */
    while (new_block->size < size) {
        struct block_header* extra = heap_grow();
        if (extra == NULL) return NULL;
        /* Merge if the new page is adjacent */
        if ((uint8_t*)new_block + HEADER_SIZE + new_block->size == (uint8_t*)extra) {
            new_block->size += HEADER_SIZE + extra->size;
            new_block->next = extra->next;
        } else {
            /* Not adjacent — try the new block instead */
            new_block = extra;
        }
    }

    split_block(new_block, size);
    new_block->is_free = 0;
    return (void*)((uint8_t*)new_block + HEADER_SIZE);
}

/* Merge adjacent free blocks to reduce fragmentation */
static void merge_free_blocks(void) {
    struct block_header* current = heap_start;
    while (current != NULL && current->next != NULL) {
        if (current->is_free && current->next->is_free) {
            /* Check if they're physically adjacent */
            uint8_t* end_of_current =
                (uint8_t*)current + HEADER_SIZE + current->size;
            if (end_of_current == (uint8_t*)current->next) {
                current->size += HEADER_SIZE + current->next->size;
                current->next = current->next->next;
                continue;  /* Check again — might merge with the next one too */
            }
        }
        current = current->next;
    }
}

void kfree(void* ptr) {
    if (ptr == NULL) return;

    /* The header is right before the pointer we returned */
    struct block_header* block =
        (struct block_header*)((uint8_t*)ptr - HEADER_SIZE);
    block->is_free = 1;

    merge_free_blocks();
}

/* Debug: dump the entire heap state to serial */
void heap_dump(void) {
    serial_write("\n--- Heap Dump ---\n");
    struct block_header* current = heap_start;
    int i = 0;
    while (current != NULL) {
        serial_write("  Block ");
        serial_write_dec(i);
        serial_write(": addr=");
        serial_write_hex((uint32_t)current);
        serial_write(" size=");
        serial_write_dec(current->size);
        serial_write(current->is_free ? " [FREE]" : " [USED]");
        serial_write("\n");
        current = current->next;
        i++;
    }
    serial_write("--- End Dump ---\n\n");
}
