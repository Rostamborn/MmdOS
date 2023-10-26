%define FREE_SPACE 0x9000             ; Free space to load the long mode code.
 
ORG 0x7C00
BITS 16
 
KERNEL_OFFSET equ 0x1000 ; The memory offset in which the kernel will be loaded
mov [BOOT_DRIVE], dl ; save boot drive number
Boot:
    jmp 0x0000:.FlushCS               ; Some BIOS' may load us at 0x0000:0x7C00 while other may load us at 0x07C0:0x0000.
                                      ; Do a far jump to fix this issue, and reload CS to 0x0000.
 
.FlushCS:   
    xor ax, ax
 
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    cld
 
    call .LoadKernel                  ; Load the kernel.
    ; Point edi to a free space bracket.
    mov edi, FREE_SPACE
    ; Switch to Long Mode.
    jmp Switch_To_Long_Mode

.LoadKernel:
    mov bx, KERNEL_OFFSET
    mov dh, 15 ; read 5 sectors from disk
    mov dl, [BOOT_DRIVE] ; load boot drive number
    call disk_load

    ret

BITS 64
.Long:
    ; Blank out the screen to a blue color.
    mov edi, 0xB8000
    mov rcx, 500                      ; Since we are clearing uint64_t over here, we put the count as Count/4.
    mov rax, 0x1F201F201F201F20       ; Set the value to set the screen to: Blue background, white foreground, blank spaces.
    rep stosq                         ; Clear the entire screen. 
 
    ; Display "Hello World!"
    mov edi, 0x00b8000              
 
    mov rax, 0x1F6C1F6C1F651F48    
    mov [edi],rax
 
    mov rax, 0x1F6F1F571F201F6F
    mov [edi + 8], rax
 
    mov rax, 0x1F211F641F6C1F72
    mov [edi + 16], rax

    .end:
        hlt
        jmp .end
 
%include "boot/long_mode.asm"
%include "boot/gdt.asm"
%include "boot/disk_read.asm"
 
Print:
    pushad
.PrintLoop:
    lodsb                             ; Load the value at [@es:@si] in @al.
    test al, al                       ; If AL is the terminator character, stop printing.
    je .PrintDone                  	
    mov ah, 0x0E	
    int 0x10
    jmp .PrintLoop                    ; Loop till the null character not found.
 
.PrintDone:
    popad                             ; Pop all general purpose registers to save them.
    ret
 
BOOT_DRIVE db 0
; Pad out file.
times 510 - ($-$$) db 0
dw 0xAA55
