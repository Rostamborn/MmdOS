%macro isr 1 ; macro to create ISRs for ones with no error code
global isr%1
isr%1:
    cli
    push 0  ; Dummy error code
    push %1 ; Push isr number
    jmp isr_stub
%endmacro

%macro isr_err 1 ; macro to create ISRs for ones with error code
global isr_err%1
isr_err%1:
    cli
    ; Error code pushed by CPU
    push %1 ; Push isr number
    jmp isr_stub
%endmacro

%macro irq 2 ; macro to create IRQs
global irq%1
irq%1:
    cli
    push 0  ; Dummy error code
    push %2 ; Push irq number
    jmp irq_stub
%endmacro

; CPU exceptions
%assign n 0
%rep 8
    isr n
%assign n n+1
%endrep

isr_err 8
isr 9
isr_err 10
isr_err 11
isr_err 12
isr_err 13
isr_err 14

%assign n 15
%rep 17
    isr n
%assign n n+1
%endrep

isr 128
isr 177

irq 0, 32
irq   1,    33
irq   2,    34
irq   3,    35
irq   4,    36
irq   5,    37
irq   6,    38
irq   7,    39
irq   8,    40
irq   9,    41
irq  10,    42
irq  11,    43
irq  12,    44
irq  13,    45
irq  14,    46
irq  15,    47

; %assign n 32
; %rep 16
;     irq n
; %assign n n+1
; %endrep

; ; User defined isrs
; %assign n 48
; %rep 208
;    isr n
; %assign n n+1
; %endrep

extern isr_handler
extern pic_eoi
isr_stub:
    push rax
    push rbx
    push rcx
    push rdx
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    push rbp
    mov rdi, rsp ; pass stack pointer to interrupt_dispatch
    call isr_handler
    mov rsp, rax
    pop rbp
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    add rsp, 16 ; remove interrupt number and error code as the stacks grows downwards

    sti
    call pic_eoi

    iretq

extern irq_handler
irq_stub:
    push rax
    push rbx
    push rcx
    push rdx
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    push rbp
    mov rdi, rsp ; pass stack pointer to interrupt_dispatch
    call irq_handler
    mov rsp, rax
    pop rbp
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    add rsp, 16 ; remove interrupt number and error code as the stacks grows downwards

    sti
    iretq
