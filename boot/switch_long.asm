%define PAGE_PRESENT (1 << 0)
%define PAGE_WRITE (1 << 1)

switch_to_long:
    call enable_paging

    mov al, 0xFF                      ; Out 0xFF to 0xA1 and 0x21 to disable all IRQs.
    out 0xA1, al
    out 0x21, al
 
    nop
    nop
 
    lidt [IDT] 

     ; Enter long mode.
    mov eax, 10100000b                ; Set the PAE and PGE bit.
    mov cr4, eax
 
    mov edx, edi                      ; Point CR3 at the PML4.
    mov cr3, edx
 
    mov ecx, 0xC0000080               ; Read from the EFER MSR. 
    rdmsr    
 
    or eax, 0x00000100                ; Set the LME bit.
    wrmsr
 
    mov ebx, cr0                      ; Activate long mode -
    or ebx,0x80000001                 ; - by enabling paging and protection simultaneously.
    mov cr0, ebx
    
    lgdt [GDT.Pointer] ; load the gdt descriptor
    jmp GDT.Code:init_long ; jump to the code segment

[64 bits]
init_long:

    mov ax, GDT.Data ; load the data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; mov ebp, 0x900000 ; set the stack pointer to 0x900000
    ; mov esp, ebp
    mov rbp, 0x90000000 ; set the stack pointer
    mov rbp, rsp ; set the base pointer

    call BEGIN_LONG
