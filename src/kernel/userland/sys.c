#include "sys.h"
#include "../interrupts/idt.h"
#include "../cpu/pic.h"
#include <stdint.h>

void* syscall_handlers[32] = {0};

execution_context* syscall_handler(execution_context* frame) {
    uint64_t (*handler)(execution_context * frame
                        , uint64_t a0
                        , uint64_t a1
                        , uint64_t a2
                        , uint64_t a3);
    handler = syscall_handlers[frame->rdi];
    
    frame->rax = handler(frame
                         , frame->rsi
                         , frame->rdx
                         , frame->rcx
                         , frame->r8);
    pic_eoi(128);
    return frame;
}

void syscall_install_handler(
    uint8_t offset, execution_context* (*handler)(execution_context* frame)) {
    syscall_handlers[offset] = handler;
}

void syscall_init(void) {
    set_interrupt_descriptor(0x80, syscall_handler, 3);
}

__attribute__((naked))
uint64_t do_syscall(uint64_t num, uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3) {
asm ("int $0x80" ::: "rdi", "rsi", "rdx", "rcx", "r8", "memory");
}
