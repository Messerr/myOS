#include "kbuf.h"

static char buffer[KBUF_SIZE];
static int head = 0;
static int tail = 0;

void kbuf_init(void) {
    head = 0;
    tail = 0;
}

void kbuf_put(char c) {
    int next = (head + 1) % KBUF_SIZE;
    if (next != tail) {  /* Don't overflow */
        buffer[head] = c;
        head = next;
    }
}

char kbuf_get(void) {
    if (head == tail) return 0;  /* Empty */
    char c = buffer[tail];
    tail = (tail + 1) % KBUF_SIZE;
    return c;
}

int kbuf_empty(void) {
    return head == tail;
}
