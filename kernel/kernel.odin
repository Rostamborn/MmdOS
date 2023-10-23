package main

@(require) foreign import "../bin/kernel_entry.o"
@(require) foreign import sse "../bin/sse_enable.o"

foreign sse {
    enable_sse :: proc() ---
}


@(export, link_name="m_main")
m_main :: proc() {
    enable_sse()
    video_memory: ^u8 = cast(^u8)cast(uintptr)0xb8000
    video_memory^ = 'A'
}
