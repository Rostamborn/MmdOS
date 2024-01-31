global jmp_user
jmp_user:
    ; 0x38 is user code segment
    ; 0x40 is user data segment
    mov ax, 0x43
    mov ds, ax

    push 0x43
    push rsi
    push 0x202 ; suggested by osdev notes
    push 0x3b
    push rdi

    ireqt
