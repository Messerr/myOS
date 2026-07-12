BITS 32
org 0x00800000

    ; sys_gfx_init
    mov eax, 11
    int 0x80

    ; sys_gfx_clear(0) — black background
    mov eax, 14
    mov ebx, 0
    int 0x80

    ; Draw a smiley face!

    ; Face (yellow circle) - approximate with filled rows
    ; Center at 160,100 radius 40
    mov esi, 60        ; y start (100-40)
    .face_y:
        cmp esi, 140   ; y end (100+40)
        jg .face_done

        ; Calculate x range for this y
        mov edi, 120   ; x start (160-40)
        .face_x:
            cmp edi, 200
            jg .face_x_done

            ; Check if inside circle: (x-160)^2 + (y-100)^2 <= 40^2
            mov eax, edi
            sub eax, 160
            imul eax, eax   ; (x-160)^2
            mov ebx, esi
            sub ebx, 100
            imul ebx, ebx   ; (y-100)^2
            add eax, ebx
            cmp eax, 1600   ; 40^2
            jg .skip_face

            ; Draw yellow pixel
            mov eax, 12     ; SYS_GFX_PIXEL
            mov ebx, edi    ; x
            mov ecx, esi    ; y
            mov edx, 14     ; yellow
            int 0x80

            .skip_face:
            inc edi
            jmp .face_x
        .face_x_done:

        inc esi
        jmp .face_y
    .face_done:

    ; Left eye (black)
    mov esi, 85
    .leye_y:
        cmp esi, 95
        jg .leye_done
        mov edi, 145
        .leye_x:
            cmp edi, 155
            jg .leye_x_done
            mov eax, edi
            sub eax, 150
            imul eax, eax
            mov ebx, esi
            sub ebx, 90
            imul ebx, ebx
            add eax, ebx
            cmp eax, 25
            jg .skip_leye
            mov eax, 12
            mov ebx, edi
            mov ecx, esi
            mov edx, 0
            int 0x80
            .skip_leye:
            inc edi
            jmp .leye_x
        .leye_x_done:
        inc esi
        jmp .leye_y
    .leye_done:

    ; Right eye (black)
    mov esi, 85
    .reye_y:
        cmp esi, 95
        jg .reye_done
        mov edi, 165
        .reye_x:
            cmp edi, 175
            jg .reye_x_done
            mov eax, edi
            sub eax, 170
            imul eax, eax
            mov ebx, esi
            sub ebx, 90
            imul ebx, ebx
            add eax, ebx
            cmp eax, 25
            jg .skip_reye
            mov eax, 12
            mov ebx, edi
            mov ecx, esi
            mov edx, 0
            int 0x80
            .skip_reye:
            inc edi
            jmp .reye_x
        .reye_x_done:
        inc esi
        jmp .reye_y
    .reye_done:

    ; Mouth (black arc) — simple horizontal lines
    mov esi, 110
    .mouth_y:
        cmp esi, 120
        jg .mouth_done
        ; Mouth from x=145 to x=175, curved
        mov eax, esi
        sub eax, 115   ; center of mouth arc
        imul eax, eax  ; y offset squared
        mov ebx, 25
        sub ebx, eax   ; radius^2 - y^2 = x range
        ; Simple approach: draw wider at bottom
        mov edi, 148
        .mouth_x:
            cmp edi, 172
            jg .mouth_x_done
            mov eax, 12
            mov ebx, edi
            mov ecx, esi
            mov edx, 0
            int 0x80
            inc edi
            jmp .mouth_x
        .mouth_x_done:
        inc esi
        jmp .mouth_y
    .mouth_done:

    ; Swap buffer to display
    mov eax, 13
    int 0x80

    ; Wait for keypress (sys_read)
    sub esp, 8
    mov eax, 3
    mov ebx, esp
    mov ecx, 2
    int 0x80
    add esp, 8

    ; Exit graphics mode
    mov eax, 15
    int 0x80

    ; Exit program
    mov eax, 2
    mov ebx, 0
    int 0x80
