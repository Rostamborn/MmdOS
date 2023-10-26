[bits 32]
disable_paging:
    ; pushad
    mov eax, cr0                                   ; Set the A-register to control register 0.
    and eax, 01111111111111111111111111111111b     ; Clear the PG-bit, which is bit 31.
    mov cr0, eax                                   ; Set control register 0 to the A-register.

    ; popad
    ret

enable_paging:
    mov edi, 0x1000    ; Set the destination index to 0x1000.
    mov cr3, edi       ; Set control register 3 to the destination index.
    xor eax, eax       ; Nullify the A-register.
    mov ecx, 4096      ; Set the C-register to 4096.
    rep stosd          ; Clear the memory.
    mov edi, cr3       ; Set the destination index to control register 3

    ; PML4T - 0x1000.
    ; PDPT - 0x2000.
    ; PDT - 0x3000.
    ; PT - 0x4000.
    mov DWORD [edi], 0x2003      ; Set the uint32_t at the destination index to 0x2003. (3 (0b11) at the end means that it is present and both readable and writable)
    add edi, 0x1000              ; Add 0x1000 to the destination index.
    mov DWORD [edi], 0x3003      ; Set the uint32_t at the destination index to 0x3003.
    add edi, 0x1000              ; Add 0x1000 to the destination index.
    mov DWORD [edi], 0x4003      ; Set the uint32_t at the destination index to 0x4003.
    add edi, 0x1000              ; Add 0x1000 to the destination index.

    mov ebx, 0x00000003          ; Set the B-register to 0x00000003.
    mov ecx, 512                 ; Set the C-register to 512. In 64-bit mode, there are 512 entries(each being 4KiB) in each table.
    .SetEntry:
        mov DWORD [edi], ebx     ; Set the uint32_t at the destination index to the B-register.
        add ebx, 0x1000          ; Add 0x1000 to the B-register.
        add edi, 8               ; Add 0x8 to the destination index. In 64-bit mode, each page table entry is 8 bytes.
        loop .SetEntry           ; Loop back to .SetEntry if the C-register is not 0.

    ; enable PAE-paging by setting the PAE-bit in the 4th control register.
    mov eax, cr4                 ; Set the A-register to control register 4.
    or eax, 1 << 5               ; Set the PAE-bit, which is the 6th bit (bit 5).
    mov cr4, eax                 ; Set control register 4 to the A-register.

    ; Set the LM-bit
    mov ecx, 0xC0000080          ; Set the C-register to 0xC0000080, which is the EFER MSR.
    rdmsr                        ; Read from the model-specific register.
    or eax, 1 << 8               ; Set the LM-bit which is the 9th bit (bit 8).
    wrmsr                        ; Write to the model-specific register.

    mov eax, cr0                 ; Set the A-register to control register 0.
    or eax, 1 << 31              ; Set the PG-bit, which is the 32nd bit (bit 31).
    mov cr0, eax                 ; Set control register 0 to the A-register.

    ret
