; GDT
gdt_start:

gdt_null:  ; mandatory null discriptor
    dd 0x0 ; double word is 4 bytes; word is 2 bytes or 16 bits
    dd 0x0 ; each discriptor is 8-byte long. so 2 double words

gdt_code:  ; code segment discriptor
    dw 0xFFFF ; limit low
    dw 0x0 ; base low
    db 0x0 ; base middle
    db 10011010b ; access
    db 11001111b ; granularity
    db 0x0 ; base high

gdt_data:  ; data segment discriptor
    dw 0xFFFF ; limit low
    dw 0x0 ; base low
    db 0x0 ; base middle
    db 10010010b ; access
    db 11001111b ; granularity
    db 0x0 ; base high

gdt_end: ; end of gdt, so we can calculate the size of gdt later on

gdt_descriptor: ; should be 6-bytes in length(it's the GDT table!)
    dw gdt_end - 1 - gdt_start ; size of gdt
    dd gdt_start ; start address of gdt

; using equ is basically like defining constants(I think)
CODE_SEG equ gdt_code - gdt_start ; this is the offset of code segment (selector)
DATA_SEG equ gdt_data - gdt_start ; this is the offset of data segment (selector)
; in out GDT, the offset of null descriptor is 0x0, the offset of code segment is 0x8 
; and data segment is 0x10
; each 8 byte long, there is a descriptor
