BITS 32
org 0x00800000

mov eax, 1
mov ebx, msg
mov ecx, msg_len
int 0x80

mov eax, 2
mov ebx, 0
int 0x80

msg: db "Hello! I'm a program loaded from the filesystem!", 10
msg_len equ $ - msg
