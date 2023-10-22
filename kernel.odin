package main

@(require) foreign import "kernel_entry.o"

do_something :: proc() {
}

// @(export, link_name="main")
main :: proc() {
    video_memory: ^u8 = cast(^u8)cast(uintptr)0xb8000
    video_memory^ = 'X'

    do_something()    
}
