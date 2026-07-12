#include "user.h"
#include "tss.h"
#include "pmm.h"
#include "syscall.h"
#include "io.h"

static uint8_t user_stack_mem[16384] __attribute__((aligned(4096)));

static void serial_write_kern(const char* str) {
    for (int i = 0; str[i]; i++) outb(0x3F8, str[i]);
}

/* === Userspace syscall wrappers === */

static int sys_write(const char* buf, int len) {
    int ret;
    asm volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_WRITE), "b"((uint32_t)buf), "c"((uint32_t)len)
        : "memory"
    );
    return ret;
}

static void sys_exit(int code) {
    asm volatile (
        "int $0x80"
        :
        : "a"(SYS_EXIT), "b"((uint32_t)code)
    );
}

static int sys_read(char* buf, int max_len) {
    int ret;
    asm volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_READ), "b"((uint32_t)buf), "c"((uint32_t)max_len)
        : "memory"
    );
    return ret;
}

static int sys_open(const char* name) {
    int ret;
    asm volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_OPEN), "b"((uint32_t)name)
        : "memory"
    );
    return ret;
}

static int sys_fread(int fd, char* buf, int max_len) {
    int ret;
    asm volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_FREAD), "b"((uint32_t)fd), "c"((uint32_t)buf), "d"((uint32_t)max_len)
        : "memory"
    );
    return ret;
}

static int sys_close(int fd) {
    int ret;
    asm volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_CLOSE), "b"((uint32_t)fd)
    );
    return ret;
}

static int sys_fwrite(const char* name, const char* content, int len) {
    int ret;
    asm volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_FWRITE), "b"((uint32_t)name), "c"((uint32_t)content), "d"((uint32_t)len)
        : "memory"
    );
    return ret;
}

static void sys_list(void) {
    asm volatile (
        "int $0x80"
        :
        : "a"(SYS_LIST)
    );
}

static int sys_exec(const char* name) {
    int ret;
    asm volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(10), "b"((uint32_t)name)
        : "memory"
    );
    return ret;
}

static void sys_gfx_init(void) {
    asm volatile ("int $0x80" : : "a"(11));
}

static void sys_gfx_pixel(int x, int y, int color) {
    asm volatile ("int $0x80" : : "a"(12), "b"(x), "c"(y), "d"(color));
}

static void sys_gfx_swap(void) {
    asm volatile ("int $0x80" : : "a"(13));
}

static void sys_gfx_clear(int color) {
    asm volatile ("int $0x80" : : "a"(14), "b"(color));
}

static void sys_gfx_exit(void) {
    asm volatile ("int $0x80" : : "a"(15));
}

/* === String helpers (no kernel functions available!) === */

static int u_strlen(const char* s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

static void u_print(const char* str) {
    sys_write(str, u_strlen(str));
}

static int u_streq(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

static int u_starts_with(const char* str, const char* prefix) {
    while (*prefix) {
        if (*str != *prefix) return 0;
        str++; prefix++;
    }
    return 1;
}

static const char* u_skip_word(const char* s) {
    while (*s && *s != ' ') s++;
    if (*s == ' ') s++;
    return s;
}

/* === The user-mode shell === */

static void user_shell(void) {
    char cmd[256];

    u_print("\n");
    u_print("  +============================+\n");
    u_print("  |   MyOS User-Mode Shell     |\n");
    u_print("  |   Ring 3 - Syscalls Only   |\n");
    u_print("  |   Type 'help' for cmds     |\n");
    u_print("  +============================+\n");
    u_print("\n");

    while (1) {
        u_print("user> ");
        int len = sys_read(cmd, 256);

        if (len <= 0) continue;

        if (u_streq(cmd, "help")) {
            u_print("Commands:\n");
            u_print("  ls                    - list files\n");
            u_print("  cat <file>            - display file contents\n");
            u_print("  write <file> <text>   - write text to a file\n");
            u_print("  rm <file>             - delete a file (via write empty)\n");
            u_print("  echo <text>           - print text\n");
            u_print("  whoami                - show privilege level\n");
            u_print("  exit                  - exit shell\n");
            u_print("  help                  - show this message\n");
	    u_print("  exec <program>        - run a program\n");
	    u_print("  draw                  - graphics demo\n");
        }
        else if (u_streq(cmd, "ls")) {
            sys_list();
        }
        else if (u_streq(cmd, "draw")) {
            u_print("Switching to graphics mode...\n");
            sys_gfx_init();
            /* Draw something fun from userspace */
            sys_gfx_clear(0);

            /* Draw rainbow bars */
            for (int i = 0; i < 200; i++) {
                for (int x = 0; x < 320; x++) {
                    sys_gfx_pixel(x, i, 32 + ((x + i) % 224));
                }
            }
            sys_gfx_swap();

            /* Wait for a keypress */
            char tmp[4];
            sys_read(tmp, 2);

            /* Return to text mode */
            sys_gfx_exit();
            u_print("Back to text mode.\n");
        }
        else if (u_starts_with(cmd, "cat ")) {
            const char* filename = u_skip_word(cmd);
            if (filename[0] == '\0') {
                u_print("Usage: cat <filename>\n");
                continue;
            }
            int fd = sys_open(filename);
            if (fd < 0) {
                u_print("File not found: ");
                u_print(filename);
                u_print("\n");
                continue;
            }
            char buf[4096];
            int bytes = sys_fread(fd, buf, 4095);
            if (bytes > 0) {
                buf[bytes] = '\0';
                u_print(buf);
                u_print("\n");
            }
            sys_close(fd);
        }
        else if (u_starts_with(cmd, "write ")) {
            const char* rest = u_skip_word(cmd);       /* skip "write" */
            const char* content = u_skip_word(rest);    /* skip filename */

            /* Extract filename */
            char fname[32];
            int i = 0;
            while (rest[i] && rest[i] != ' ' && i < 31) {
                fname[i] = rest[i];
                i++;
            }
            fname[i] = '\0';

            if (fname[0] == '\0' || content[0] == '\0') {
                u_print("Usage: write <filename> <content>\n");
                continue;
            }

            int result = sys_fwrite(fname, content, u_strlen(content));
            if (result == 0) {
                u_print("Written to ");
                u_print(fname);
                u_print("\n");
            } else {
                u_print("Write failed\n");
            }
        }
        else if (u_starts_with(cmd, "echo ")) {
            const char* text = u_skip_word(cmd);
            u_print(text);
            u_print("\n");
        }
        else if (u_starts_with(cmd, "exec ")) {
            const char* progname = u_skip_word(cmd);
            if (progname[0] == '\0') {
                u_print("Usage: exec <program>\n");
                continue;
            }
            u_print("Loading program: ");
            u_print(progname);
            u_print("\n");
            int result = sys_exec(progname);
            if (result < 0) {
                u_print("Failed to execute: ");
                u_print(progname);
                u_print("\n");
            }
        }
        else if (u_streq(cmd, "whoami")) {
            u_print("Running in Ring 3 (user mode)\n");
            u_print("I cannot access hardware directly.\n");
            u_print("All I/O goes through int 0x80 syscalls.\n");
        }
        else if (u_streq(cmd, "exit")) {
            u_print("Goodbye!\n");
            sys_exit(0);
        }
        else if (cmd[0] != '\0') {
            u_print("Unknown command: ");
            u_print(cmd);
            u_print("\nType 'help' for available commands.\n");
        }
    }
}

/* === Kernel-side: jump to user mode === */

void user_mode_init(void) {
    serial_write_kern("Jumping to user mode...\n");

    uint32_t user_stack_top = (uint32_t)(user_stack_mem + 16384);

    asm volatile (
        "cli\n"
        "mov $0x23, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "push $0x23\n"
        "push %0\n"
        "pushf\n"
        "pop %%eax\n"
        "or $0x200, %%eax\n"
        "push %%eax\n"
        "push $0x1B\n"
        "push %1\n"
        "iret\n"
        :
        : "r"(user_stack_top),
          "r"((uint32_t)user_shell)
        : "eax", "memory"
    );
}
