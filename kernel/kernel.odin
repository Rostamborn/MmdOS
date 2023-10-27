package main

import "cpu"

// Kernel entry point
@(export)
kmain :: proc() {
    cpu.hcf()
}
