#ifndef SYS_H
#define SYS_H

#include "../cpu/cpu.h"
#include <stdint.h>

#define KERNEL_CS 0x28 | 0
#define KERNEL_SS 0x30 | 0
#define USER_CS 0x38 | 3
#define USER_SS 0x40 | 3

#define RFLAGS 0x202

extern uint64_t do_syscall(uint64_t frame
                           , uint64_t a0
                           , uint64_t a1
                           , uint64_t a2
                           , uint64_t a3);

void syscall_init(void);

void syscall_install_handler(
    uint8_t offset, uint64_t (*handler)(uint64_t frame
                                                  , uint64_t a0
                                                  , uint64_t a1
                                                  , uint64_t a2
                                                  , uint64_t a3));

uint64_t syscall_memcpy(void* dest, void* src, uint64_t size);

#endif
