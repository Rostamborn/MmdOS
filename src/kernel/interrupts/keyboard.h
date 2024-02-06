#ifndef KERNEL_KEYBOARD_DEF
#define KERNEL_KEYBOARD_DEF

#include <stdint.h>

void keyboard_init();

uint8_t keyboard_getch();

uint64_t keyboard_getch_syscall(uint64_t frame, uint64_t p1, uint64_t p2,
                                uint64_t p3, uint64_t p4);
#endif
