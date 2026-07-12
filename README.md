# myOS
 
A bare-metal x86 operating system written from scratch in C and assembly. No libraries, no frameworks, no existing OS underneath — just raw hardware access running on QEMU.
 
## Features
 
- **Custom bootloader integration** — Multiboot-compliant kernel loaded directly by QEMU
- **GDT** — flat memory model with kernel and user segments
- **IDT + interrupt handling** — 32 CPU exception handlers + 16 hardware IRQ handlers
- **PS/2 keyboard & serial drivers** — interrupt-driven input
- **Physical memory manager** — bitmap-based page frame allocator
- **Virtual memory (paging)** — identity-mapped page tables with page fault handler
- **Heap allocator** — `kmalloc`/`kfree` with block splitting, reuse, and coalescing
- **Preemptive multitasking** — timer-driven context switching between tasks via PIT at 100Hz
- **RAM filesystem** — create, read, write, list, and delete files in memory
- **Interactive shell** — command-line interface with `ls`, `cat`, `write`, `rm`, `help`, `clear`
## Architecture
 
```
Shell
 ├── Filesystem (RAM-backed, heap-allocated)
 ├── Keyboard / Serial (interrupt-driven I/O)
 │    └── IDT + ISRs
 │         └── GDT
 ├── Timer (100Hz PIT)
 │    └── Preemptive Scheduler + Context Switch
 └── Heap (kmalloc / kfree)
      └── PMM (physical page allocator)
           └── Paging (virtual memory)
```
 
## Building
 
### Prerequisites
 
An `i686-elf` cross-compiler toolchain and QEMU. On Ubuntu/Debian:
 
```bash
sudo apt install build-essential bison flex libgmp3-dev libmpc-dev \
    libmpfr-dev texinfo libisl-dev nasm qemu-system-x86 xorriso mtools wget
```
 
Then build the [i686-elf cross-compiler](https://wiki.osdev.org/GCC_Cross-Compiler) (GCC + binutils targeting bare-metal x86).
 
### Build and run
 
```bash
make clean
make
qemu-system-i386 -kernel myos.bin -nographic
```
 
Press `Ctrl-A` then `X` to exit QEMU.
 
## Shell Commands
 
```
myos> help
  ls              - list all files
  cat <file>      - display file contents
  write <file> <text> - write text to file
  rm <file>       - delete a file
  clear           - clear screen
  help            - show this message
```
 
## Screenshot
 
```
  +========================+
  |    Welcome to MyOS!    |
  |  Type 'help' for cmds  |
  +========================+
 
myos> ls
  welcome.txt  (41 bytes)
myos> cat welcome.txt
Welcome to MyOS!
This file lives in RAM.
 
myos> write hello.txt Hello from my own operating system!
Written 35 bytes to hello.txt
myos> ls
  welcome.txt  (41 bytes)
  hello.txt  (35 bytes)
```
 
## File Structure
 
| File | Purpose |
|------|---------|
| `boot.s` | Multiboot header, stack setup, entry point |
| `gdt.h/c`, `gdt_flush.s` | Global Descriptor Table |
| `idt.h/c` | Interrupt Descriptor Table |
| `isr.h/c`, `isr_stubs.s` | Exception + IRQ handlers, PIC remapping |
| `keyboard.h/c` | PS/2 keyboard + serial input driver |
| `pmm.h/c` | Physical memory manager (bitmap allocator) |
| `paging.h/c` | Virtual memory setup (identity-mapped paging) |
| `heap.h/c` | Kernel heap allocator (kmalloc/kfree) |
| `timer.h/c` | Programmable Interval Timer driver |
| `task.h/c`, `switch.s` | Multitasking + context switching |
| `fs.h/c` | RAM filesystem |
| `shell.h/c` | Interactive command-line shell |
| `io.h` | Port I/O helpers (inb/outb) |
| `kernel.c` | Kernel entry point, subsystem init |
| `linker.ld` | Linker script (memory layout) |
 
## Built With
 
- C (freestanding, no stdlib)
- x86 assembly (GAS syntax)
- i686-elf-gcc cross-compiler
- QEMU for testing
## License
 
MIT
