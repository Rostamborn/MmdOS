#include "sys.h"
#include "../cpu/pic.h"
#include "../fs/vfs.h"
#include "../interrupts/idt.h"
#include "../lib/logger.h"
#include "../lib/print.h"
#include <stdint.h>

void* syscall_handlers[32] = {0};

execution_context* syscall_handler(execution_context* frame) {
    uint64_t (*handler)(uint64_t frame, uint64_t a0, uint64_t a1, uint64_t a2,
                        uint64_t a3);

    handler = syscall_handlers[frame->rdi];

    frame->rax = handler((uint64_t) frame, frame->rsi, frame->rdx, frame->rcx,
                         frame->r8);

    pic_eoi(128);
    return frame;
}

void syscall_install_handler(uint8_t offset,
                             uint64_t (*handler)(uint64_t frame, uint64_t a0,
                                                 uint64_t a1, uint64_t a2,
                                                 uint64_t a3)) {
    syscall_handlers[offset] = handler;
}

uint64_t _syscall_memcpy(uint64_t frame, uint64_t dest, uint64_t src,
                         uint64_t size, uint64_t unused) {
    uint8_t* d = (uint8_t*) dest;
    uint8_t* s = (uint8_t*) src;
    for (uint64_t i = 0; i < size; i++) {
        d[i] = s[i];
    }

    return size;
}

void syscall_init(void) {
    syscall_install_handler(0, &_syscall_memcpy);
    syscall_install_handler(1, &vfs_open_syscall);
    syscall_install_handler(2, &vfs_close_syscall);
    syscall_install_handler(3, &vfs_read_syscall);
    syscall_install_handler(4, &print_syscall);
}

// __attribute__((naked))
// uint64_t do_syscall(uint64_t num, uint64_t a0, uint64_t a1, uint64_t a2,
// uint64_t a3) { asm ("int $128" : :: "rdi", "rsi", "rdx", "rcx", "r8",
// "memory");
// }

uint64_t syscall_memcpy(void* dest, void* src, uint64_t size) {
    uint64_t res =
        do_syscall(0, (uint64_t) dest, (uint64_t) src, (uint64_t) size, 0);
    return res;
}
