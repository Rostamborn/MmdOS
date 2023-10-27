[bits 32]

global enable_sse
global hcf

enable_sse: ; not sure if it's correct
    mov eax, cr0
    ; clear coprocessor emulation CR0.EM
    and ax, 0xFFFB
    ; set coprocessor monitoring  CR0.MP
    or ax, 0x2
    mov cr0, eax
    mov eax, cr4
    ; set CR4.OSFXSR and CR4.OSXMMEXCPT at the same time
    or ax, 3 << 9
    mov cr4, eax
    ret

hcf:
    cli
    .loop:
        hlt
        jmp .loop
