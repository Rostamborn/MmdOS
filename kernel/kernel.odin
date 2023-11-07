package main

import "cpu"

// Kernel entry point
@(export)
kmain :: proc "contextless"() {
    char : ^u8 = cast(^u8)cast(uintptr)0xb8000
    char^ = byte('A')
    cpu.hcf()
}
