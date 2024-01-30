global do_syscall
do_syscall:
    mov rdi, rdi
    mov rsi, rsi
    mov rdx, rdx
    mov rcx, rcx
    mov r8, r8

    int 0x80
    
    mov rax, rax

    ret
