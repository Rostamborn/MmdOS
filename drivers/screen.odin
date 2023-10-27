package drivers

print :: proc "contextless" () {
    video_memory: ^u8 = cast(^u8)cast(uintptr)0xb8002
    video_memory^ = 1
}
