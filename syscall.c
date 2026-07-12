#include "syscall.h"
#include "isr.h"
#include "idt.h"
#include "io.h"
#include "fs.h"
#include "fd.h"
#include "kbuf.h"
#include "heap.h"
#include "loader.h"
#include "user.h"
#include "vga.h"

static void serial_write(const char* str) {
    for (int i = 0; str[i]; i++) outb(0x3F8, str[i]);
}

static void serial_write_dec(uint32_t val) {
    if (val == 0) { outb(0x3F8, '0'); return; }
    char buf[12];
    int i = 0;
    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }
    for (int j = i - 1; j >= 0; j--) outb(0x3F8, buf[j]);
}

static void syscall_handler(struct interrupt_frame* frame) {
    uint32_t num  = frame->eax;
    uint32_t arg1 = frame->ebx;
    uint32_t arg2 = frame->ecx;
    uint32_t arg3 = frame->edx;

    switch (num) {
        case SYS_WRITE: {
            const char* str = (const char*)arg1;
            for (uint32_t i = 0; i < arg2; i++) {
                outb(0x3F8, str[i]);
            }
            frame->eax = arg2;
            break;
        }
        case SYS_GFX_INIT: {
            vga_init();
            frame->eax = 0;
            break;
        }
        case SYS_GFX_PIXEL: {
            /* arg1=x, arg2=y, arg3=color */
            vga_putpixel(arg1, arg2, arg3);
            frame->eax = 0;
            break;
        }
        case SYS_GFX_SWAP: {
            vga_swap();
            frame->eax = 0;
            break;
        }
        case SYS_GFX_CLEAR: {
            vga_clear(arg1);
            frame->eax = 0;
            break;
        }
        case SYS_GFX_EXIT: {
            vga_exit();
            frame->eax = 0;
            break;
        }
        case SYS_EXIT: {
            serial_write("[Kernel] User program exited with code ");
            serial_write_dec(arg1);
            serial_write("\n");
            user_mode_init();
            break;
        }

        case SYS_READ: {
            /* arg1 = buffer, arg2 = max length
             * Blocking read: wait until we have a full line (Enter pressed) */
            char* buf = (char*)arg1;
            uint32_t max_len = arg2;
            uint32_t pos = 0;

            /* Wait for input, character by character */
            while (pos < max_len - 1) {
                /* Spin until a character is available */
                while (kbuf_empty()) {
                    asm volatile ("sti; hlt");  /* Sleep until interrupt */
                }
                char c = kbuf_get();

                if (c == '\n' || c == '\r') {
                    buf[pos] = '\0';
                    outb(0x3F8, '\n');  /* Echo newline */
                    frame->eax = pos;
                    return;
                }
                else if (c == '\b' || c == 127) {
                    if (pos > 0) {
                        pos--;
                        serial_write("\b \b");
                    }
                }
                else {
                    buf[pos++] = c;
                    outb(0x3F8, c);  /* Echo */
                }
            }
            buf[pos] = '\0';
            frame->eax = pos;
            break;
        }

        case SYS_OPEN: {
            const char* name = (const char*)arg1;
            /* Check file exists in filesystem first */
            char tmp[64];
            int result = fs_read(name, tmp, 64);
            if (result < 0) {
                frame->eax = (uint32_t)-1;
            } else {
                frame->eax = fd_open(name);
            }
            break;
        }

        case SYS_FREAD: {
            int fd_num = (int)arg1;
            char* buf = (char*)arg2;
            uint32_t max_len = arg3;
            struct open_file* f = fd_get(fd_num);
            if (!f) {
                frame->eax = (uint32_t)-1;
                break;
            }
            char file_data[FS_MAX_FILESIZE];
            int file_size = fs_read(f->name, file_data, FS_MAX_FILESIZE);
            if (file_size < 0) {
                frame->eax = (uint32_t)-1;
                break;
            }
            /* Copy from read_pos */
            uint32_t remaining = file_size - f->read_pos;
            if (remaining == 0) {
                frame->eax = 0;
                break;
            }
            uint32_t to_copy = remaining < max_len ? remaining : max_len;
            for (uint32_t i = 0; i < to_copy; i++) {
                buf[i] = file_data[f->read_pos + i];
            }
            buf[to_copy] = '\0';
            f->read_pos += to_copy;
            frame->eax = to_copy;
            break;
        }

        case SYS_CLOSE: {
            frame->eax = fd_close((int)arg1);
            break;
        }

        case SYS_FWRITE: {
            const char* name = (const char*)arg1;
            const char* content = (const char*)arg2;
            uint32_t len = arg3;
            frame->eax = fs_write(name, content, len);
            break;
        }

        case SYS_MALLOC: {
            void* ptr = kmalloc(arg1);
            frame->eax = (uint32_t)ptr;
            break;
        }

        case SYS_LIST: {
            fs_list();
            frame->eax = 0;
            break;
        }

        case SYS_EXEC: {
            const char* name = (const char*)arg1;
            serial_write("[Kernel] sys_exec: ");
            serial_write(name);
            serial_write("\n");
            int result = loader_exec(name);
            frame->eax = result;
            break;
        }

        default:
            serial_write("[Kernel] Unknown syscall: ");
            serial_write_dec(num);
            serial_write("\n");
            frame->eax = (uint32_t)-1;
            break;
    }
}

void syscall_init(void) {
    extern void isr128(void);
    idt_set_entry(0x80, (uint32_t)isr128, 0x08, 0xEE);
    register_interrupt_handler(0x80, syscall_handler);
    fd_init();
    serial_write("Syscalls initialized (int 0x80).\n");
}
