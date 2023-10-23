[bits 16]

switch_to_pm:
    cli ; we turn interrupts off

    lgdt [gdt_descriptor] ; load the gdt descriptor

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax ; set the PE bit in CR0 to 1

    jmp CODE_SEG:init_pm ; jump to the code segment which is a long
                        ; jump and will flush the prefetch queue(CPU pipeline)

[bits 32]
; init registers and stack in PM

init_pm:
    mov ax, DATA_SEG ; as our old segments are meaningless, we make the 
    mov ds, ax  ; segment registers to point to the new data descriptor defined in GDT
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000 ; set the stack pointer to 0x90000
    mov esp, ebp
    
    call BEGIN_PM
