/* Auto-generated - do not edit */
#ifndef BUILTIN_PROGRAMS_H
#define BUILTIN_PROGRAMS_H

#include <stdint.h>
#include <stddef.h>

struct builtin_program {
    const char* name;
    const uint8_t* data;
    uint32_t size;
};

extern const struct builtin_program builtin_programs[];
extern const int num_builtin_programs;

#endif
