package cpu

@(require) foreign import cpu "../../bin/cpu.o"

@(default_calling_convention="sysv")
foreign cpu {
    enable_sse :: proc() ---
    hcf :: proc() ---
}
