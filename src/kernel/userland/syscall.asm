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

; switch_usermode:
;     mov rax, 0x202
;     mov rflags, rax
;     mov rax, 0x38
;     or rax, 3
;     mov cs, rax ; move to usermode code segment
;     mov rax, 0x40
;     or rax, 3
;     mov ss, rax ; move to usermode data segment
;     ; rsp ???
;     ; rip ???
;     push 0x40
;
;
;     iretq
