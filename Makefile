CC = i686-elf-gcc
AS = i686-elf-as
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra

OBJS = boot.o gdt_flush.o gdt.o idt.o isr_stubs.o isr.o keyboard.o pmm.o paging.o heap.o timer.o task.o switch.o fs.o tss.o syscall.o fd.o kbuf.o loader.o builtin_programs.o user.o kernel.o

all: programs myos.bin

programs:
	./programs/build_programs.sh

boot.o: boot.s
	$(AS) boot.s -o boot.o

gdt_flush.o: gdt_flush.s
	$(AS) gdt_flush.s -o gdt_flush.o

isr_stubs.o: isr_stubs.s
	$(AS) isr_stubs.s -o isr_stubs.o

switch.o: switch.s
	$(AS) switch.s -o switch.o

kernel.o: kernel.c gdt.h idt.h isr.h keyboard.h pmm.h paging.h heap.h timer.h task.h fs.h tss.h syscall.h user.h loader.h io.h
	$(CC) -c kernel.c -o kernel.o $(CFLAGS)

gdt.o: gdt.c gdt.h
	$(CC) -c gdt.c -o gdt.o $(CFLAGS)

idt.o: idt.c idt.h
	$(CC) -c idt.c -o idt.o $(CFLAGS)

isr.o: isr.c isr.h idt.h io.h
	$(CC) -c isr.c -o isr.o $(CFLAGS)

keyboard.o: keyboard.c keyboard.h isr.h io.h kbuf.h
	$(CC) -c keyboard.c -o keyboard.o $(CFLAGS)

pmm.o: pmm.c pmm.h
	$(CC) -c pmm.c -o pmm.o $(CFLAGS)

paging.o: paging.c paging.h pmm.h isr.h io.h
	$(CC) -c paging.c -o paging.o $(CFLAGS)

heap.o: heap.c heap.h pmm.h io.h
	$(CC) -c heap.c -o heap.o $(CFLAGS)

timer.o: timer.c timer.h isr.h io.h
	$(CC) -c timer.c -o timer.o $(CFLAGS)

task.o: task.c task.h heap.h io.h
	$(CC) -c task.c -o task.o $(CFLAGS)

fs.o: fs.c fs.h heap.h io.h
	$(CC) -c fs.c -o fs.o $(CFLAGS)

tss.o: tss.c tss.h gdt.h io.h
	$(CC) -c tss.c -o tss.o $(CFLAGS)

syscall.o: syscall.c syscall.h isr.h idt.h io.h fs.h fd.h kbuf.h heap.h loader.h
	$(CC) -c syscall.c -o syscall.o $(CFLAGS)

fd.o: fd.c fd.h
	$(CC) -c fd.c -o fd.o $(CFLAGS)

kbuf.o: kbuf.c kbuf.h
	$(CC) -c kbuf.c -o kbuf.o $(CFLAGS)

loader.o: loader.c loader.h fs.h pmm.h paging.h tss.h builtin_programs.h io.h
	$(CC) -c loader.c -o loader.o $(CFLAGS)

builtin_programs.o: builtin_programs.c builtin_programs.h
	$(CC) -c builtin_programs.c -o builtin_programs.o $(CFLAGS)

user.o: user.c user.h tss.h pmm.h syscall.h io.h
	$(CC) -c user.c -o user.o $(CFLAGS)

myos.bin: $(OBJS) linker.ld
	$(CC) -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib $(OBJS) -lgcc

clean:
	rm -f *.o myos.bin programs/*.bin builtin_programs.c builtin_programs.h
