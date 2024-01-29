#ifndef SYS_H
#define SYS_H

#include "../cpu/cpu.h"

#define KERNEL_CS 0x28 | 0
#define KERNEL_SS 0x30 | 0
#define USER_CS 0x38 | 3
#define USER_SS 0x40 | 3

#define RFLAGS 0x202

void syscall_init(void);

void syscall_install_handler(
    uint8_t offset, execution_context* (*handler)(execution_context* frame));


#endif
