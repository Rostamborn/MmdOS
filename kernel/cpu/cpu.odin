package cpu

@(require) foreign import cpu "../../bin/cpu.o"

@(default_calling_convention="sysv")
foreign cpu {
    call_main :: proc() ---
    enable_sse :: proc() ---
}
