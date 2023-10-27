package main

import "cpu"
// import "../drivers"

@(export, link_name="m_main")
m_main :: proc "contextless" () {
    cpu.enable_sse()
    video_memory: ^u8 = cast(^u8)cast(uintptr)0xb8000
    video_memory^ = 'A'
    // drivers.print()
}
