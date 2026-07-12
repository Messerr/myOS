#!/bin/bash
set -e

cd "$(dirname "$0")"

# Assemble each program as a flat binary at origin 0x00800000
for src in *.asm; do
    name="${src%.asm}"
    nasm -f bin -o "${name}.bin" -DORG=0x00800000 "$src"
    echo "Assembled ${name}.bin ($(wc -c < ${name}.bin) bytes)"
done

# Generate a C file with all programs as byte arrays
echo "/* Auto-generated - do not edit */" > ../builtin_programs.c
echo '#include "builtin_programs.h"' >> ../builtin_programs.c
echo "" >> ../builtin_programs.c

echo "/* Auto-generated - do not edit */" > ../builtin_programs.h
echo "#ifndef BUILTIN_PROGRAMS_H" >> ../builtin_programs.h
echo "#define BUILTIN_PROGRAMS_H" >> ../builtin_programs.h
echo "" >> ../builtin_programs.h
echo "#include <stdint.h>" >> ../builtin_programs.h
echo "#include <stddef.h>" >> ../builtin_programs.h
echo "" >> ../builtin_programs.h
echo "struct builtin_program {" >> ../builtin_programs.h
echo "    const char* name;" >> ../builtin_programs.h
echo "    const uint8_t* data;" >> ../builtin_programs.h
echo "    uint32_t size;" >> ../builtin_programs.h
echo "};" >> ../builtin_programs.h
echo "" >> ../builtin_programs.h

count=0
for bin in *.bin; do
    name="${bin%.bin}"
    echo "static const uint8_t prog_${name}[] = {" >> ../builtin_programs.c
    xxd -i < "$bin" >> ../builtin_programs.c
    echo "};" >> ../builtin_programs.c
    echo "" >> ../builtin_programs.c
    count=$((count + 1))
done

echo "extern const struct builtin_program builtin_programs[];" >> ../builtin_programs.h
echo "extern const int num_builtin_programs;" >> ../builtin_programs.h
echo "" >> ../builtin_programs.h
echo "#endif" >> ../builtin_programs.h

# Write the program table
echo "const struct builtin_program builtin_programs[] = {" >> ../builtin_programs.c
for bin in *.bin; do
    name="${bin%.bin}"
    echo "    { \"${name}\", prog_${name}, sizeof(prog_${name}) }," >> ../builtin_programs.c
done
echo "};" >> ../builtin_programs.c
echo "const int num_builtin_programs = ${count};" >> ../builtin_programs.c

echo "Generated builtin_programs.c and builtin_programs.h with ${count} programs"
