[org 0x7c00]
    KERNEL_OFFSET equ 0x1000 ; The memory offset in which the kernel will be loaded

    mov [BOOT_DRIVE], dl ; BIOS stores boot drive in dl

    mov bp, 0x9000 ; put stack in a safe place so it will not overwrite anything important
    mov sp, bp

    mov bx, MSG_REAL_MODE
    call print_string

    call load_kernel

    

    call switch_to_pm ; we never return back here

    jmp $

%include "boot/print_string.asm"
%include "boot/print_string_pm.asm"
%include "boot/disk_read.asm"
%include "boot/gdt.asm"
%include "boot/switch_pm.asm"
%include "boot/A20_line.asm"

[bits 16]
load_kernel:
    mov bx, MSG_LOAD_KERNEL
    call print_string

    mov bx, KERNEL_OFFSET
    mov dh, 15 ; read 5 sectors from disk
    mov dl, [BOOT_DRIVE] ; load boot drive number
    call disk_load

    ret

[bits 32]
BEGIN_PM:
    mov ebx, MSG_PROT_MODE
    call print_string_pm

    call check_A20 ; check if A20 address line is enabled, if not enable it.

    call KERNEL_OFFSET ; call the kernel


BOOT_DRIVE db 0
MSG_REAL_MODE db "Started in 16-bit Real Mode", 0
MSG_PROT_MODE db "Switched to 32-bit Protected Mode", 0
MSG_LOAD_KERNEL db "Loading Kernel into memory" , 0
MSG_A20_on db "A20 is ON" , 0
MSG_A20_off db "A20 is OFF" , 0

; padding
times 510-($-$$) db 0 ; pad with zeros until 510 bytes
dw 0xaa55 ; boot signature to compelete the 512 byte long section
