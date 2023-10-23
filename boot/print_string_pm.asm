[bits 32]

VIDEO_MEMORY equ 0xb8000 ; like defining constant if I'm not mistaken(eg. the address of video memory)
WHITE_ON_BLACK equ 0x0f

print_string_pm:
    pusha
    mov edx, VIDEO_MEMORY ; we set edx to the start of video memory
    
    print_string_pm_loop:
        mov ah, WHITE_ON_BLACK ; attribute of the cell
        mov al, [ebx] ; char of the cell
        ; ah and al together make up ax which contains the character and the attributes

        cmp al, 0
        je print_string_pm_done ; if al is 0, we're done

        mov [edx], ax ; store char and attributes at current cell

        add ebx, 1 ; increment ebx to point to next char of the input string
        add edx, 2 ; increment edx to point to next cell of video memory(1 byte for char and 1 for attribute)

        jmp print_string_pm_loop

    print_string_pm_done:
        popa
        ret
