BITS 32
org 0x00800000

    mov esi, 1

.loop:
    cmp esi, 6
    jge .done

    mov eax, 1
    mov ebx, prefix
    mov ecx, prefix_len
    int 0x80

    mov eax, 1
    lea ebx, [digits + esi]
    mov ecx, 1
    int 0x80

    mov eax, 1
    mov ebx, newline
    mov ecx, 1
    int 0x80

    inc esi
    jmp .loop

.done:
    mov eax, 1
    mov ebx, bye
    mov ecx, bye_len
    int 0x80

    mov eax, 2
    mov ebx, 0
    int 0x80

digits:  db "0123456789"
prefix:  db "Count: "
prefix_len equ 7
newline: db 10
bye:     db "Counting complete!", 10
bye_len equ $ - bye
