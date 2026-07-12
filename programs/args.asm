BITS 32
org 0x00800000

    mov eax, 1
    mov ebx, prompt
    mov ecx, prompt_len
    int 0x80

    mov eax, 3
    mov ebx, buffer
    mov ecx, 128
    int 0x80
    mov esi, eax

    mov eax, 1
    mov ebx, reply
    mov ecx, reply_len
    int 0x80

    mov eax, 1
    mov ebx, buffer
    mov ecx, esi
    int 0x80

    mov eax, 1
    mov ebx, newline
    mov ecx, 1
    int 0x80

    mov eax, 2
    mov ebx, 0
    int 0x80

prompt:     db "Type something: "
prompt_len equ $ - prompt
reply:      db "You said: "
reply_len  equ $ - reply
newline:    db 10
buffer:     times 128 db 0
