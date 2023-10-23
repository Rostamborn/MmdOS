print_string:
    pusha
    mov ah, 0x0e
    loop:
        mov al, [bx]
        cmp al, 0
        je print_string_done
        add bx, 1
        int 0x10
        jmp loop

    print_string_done:
        popa
        ret
