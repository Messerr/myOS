#include "shell.h"
#include "fs.h"
#include "io.h"

#define CMD_MAX 256

static char cmd_buffer[CMD_MAX];
static int  cmd_pos = 0;

/* String helpers */
static int str_eq(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

static int str_starts_with(const char* str, const char* prefix) {
    while (*prefix) {
        if (*str != *prefix) return 0;
        str++; prefix++;
    }
    return 1;
}

static int str_len(const char* s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

static void serial_write(const char* str) {
    for (int i = 0; str[i]; i++) outb(0x3F8, str[i]);
}

static void print_prompt(void) {
    serial_write("myos> ");
}

/* Find the first space in a string, return pointer to char after it */
static const char* find_arg(const char* cmd) {
    while (*cmd && *cmd != ' ') cmd++;
    if (*cmd == ' ') cmd++;  /* skip the space */
    return cmd;
}

/* Find the second argument (after two spaces) */
static const char* find_second_arg(const char* cmd) {
    const char* first = find_arg(cmd);
    return find_arg(first);
}

static void execute_command(const char* cmd) {
    if (cmd[0] == '\0') {
        print_prompt();
        return;
    }

    if (str_eq(cmd, "help")) {
        serial_write("Available commands:\n");
        serial_write("  ls              - list all files\n");
        serial_write("  cat <file>      - display file contents\n");
        serial_write("  write <file> <text> - write text to file\n");
        serial_write("  rm <file>       - delete a file\n");
        serial_write("  clear           - clear screen\n");
        serial_write("  help            - show this message\n");
    }
    else if (str_eq(cmd, "ls")) {
        fs_list();
    }
    else if (str_starts_with(cmd, "cat ")) {
        const char* filename = find_arg(cmd);
        if (filename[0] == '\0') {
            serial_write("Usage: cat <filename>\n");
        } else {
            char buffer[FS_MAX_FILESIZE];
            int result = fs_read(filename, buffer, FS_MAX_FILESIZE);
            if (result < 0) {
                serial_write("File not found: ");
                serial_write(filename);
                serial_write("\n");
            } else {
                serial_write(buffer);
                serial_write("\n");
            }
        }
    }
    else if (str_starts_with(cmd, "write ")) {
        const char* filename = find_arg(cmd);
        const char* content = find_second_arg(cmd);
        if (filename[0] == '\0' || content == filename) {
            serial_write("Usage: write <filename> <content>\n");
        } else {
            /* We need to extract just the filename (up to the space before content) */
            /* Copy filename to a temp buffer */
            char fname[FS_MAX_FILENAME];
            int i = 0;
            while (filename[i] && filename[i] != ' ' && i < FS_MAX_FILENAME - 1) {
                fname[i] = filename[i];
                i++;
            }
            fname[i] = '\0';

            if (content[0] == '\0') {
                serial_write("Usage: write <filename> <content>\n");
            } else {
                int len = str_len(content);
                int result = fs_write(fname, content, len);
                if (result == 0) {
                    serial_write("Written ");
                    char dec_buf[12];
                    int di = 0;
                    int tmp = len;
                    if (tmp == 0) { dec_buf[di++] = '0'; }
                    else { while (tmp > 0) { dec_buf[di++] = '0' + (tmp % 10); tmp /= 10; } }
                    for (int j = di - 1; j >= 0; j--) outb(0x3F8, dec_buf[j]);
                    serial_write(" bytes to ");
                    serial_write(fname);
                    serial_write("\n");
                }
            }
        }
    }
    else if (str_starts_with(cmd, "rm ")) {
        const char* filename = find_arg(cmd);
        if (filename[0] == '\0') {
            serial_write("Usage: rm <filename>\n");
        } else {
            int result = fs_delete(filename);
            if (result < 0) {
                serial_write("File not found: ");
                serial_write(filename);
                serial_write("\n");
            } else {
                serial_write("Deleted: ");
                serial_write(filename);
                serial_write("\n");
            }
        }
    }
    else if (str_eq(cmd, "clear")) {
        /* Send ANSI clear sequence (works in most terminals) */
        serial_write("\033[2J\033[H");
    }
    else {
        serial_write("Unknown command: ");
        serial_write(cmd);
        serial_write("\nType 'help' for available commands.\n");
    }

    print_prompt();
}

void shell_init(void) {
    cmd_pos = 0;
    cmd_buffer[0] = '\0';
    serial_write("\n");
    serial_write("  +========================+\n");
    serial_write("  |    Welcome to MyOS!    |\n");
    serial_write("  |  Type 'help' for cmds  |\n");
    serial_write("  +========================+\n");
    serial_write("\n");
    print_prompt();
}

void shell_handle_char(char c) {
    if (c == '\n' || c == '\r') {
        serial_write("\n");
        cmd_buffer[cmd_pos] = '\0';
        execute_command(cmd_buffer);
        cmd_pos = 0;
    }
    else if (c == '\b' || c == 127) {
        /* Backspace */
        if (cmd_pos > 0) {
            cmd_pos--;
            serial_write("\b \b");
        }
    }
    else if (cmd_pos < CMD_MAX - 1) {
        cmd_buffer[cmd_pos++] = c;
        /* Echo the character */
        outb(0x3F8, c);
    }
}
