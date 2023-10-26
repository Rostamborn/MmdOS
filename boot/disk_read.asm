disk_load:
    push dx ; store dx on the stack which contains the number of sectors we want
    
    mov ah, 0x02 ; read disk function
    mov al, dh ; number of sectors to read
    mov ch, 0x00 ; cylinder number
    mov dh, 0x00 ; head number
    mov cl, 0x02 ; choose second sector which would be after boot sector

    int 0x13 ; call interrupt for disk read

    jc disk_error ; if carry flag is set, there was an error

    pop dx ; restore dx from the stack
    cmp dh, al ; compare the number of sectors we want to read with the number of sectors we read
    jne disk_error ; if they are not equal, there was an error
    ret

disk_error:
    jmp $
