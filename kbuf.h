#ifndef KBUF_H
#define KBUF_H

#include <stdint.h>

#define KBUF_SIZE 256

void kbuf_init(void);
void kbuf_put(char c);
char kbuf_get(void);       /* Returns 0 if empty */
int  kbuf_empty(void);

#endif
