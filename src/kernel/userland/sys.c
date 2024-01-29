#include "../interrupts/idt.h"
#include "../cpu/cpu.h"
#include "../cpu/pic.h"
#include <stdint.h>

void* syscall_handlers[32] = {0};

execution_context* syscall_handler(execution_context* frame) {
    execution_context* (*handler)(execution_context * frame);
    handler = syscall_handlers[frame->rdi];
    if (handler) {
        frame = handler(frame);
    }
    pic_eoi(frame->rdi);

    return frame;
}

void syscall_install_handler(
    uint8_t offset, execution_context* (*handler)(execution_context* frame)) {
    syscall_handlers[offset] = handler;
}

void syscall_init(void) {
    set_interrupt_descriptor(0x80, syscall_handler, 3);
}

// uint64_t syscall_do(uint64_t syscall_number, uint64_t arg1, uint64_t arg2,
//                     uint64_t arg3, uint64_t arg4, uint64_t arg5) {
//     uint64_t ret;
//     asm volatile("mov %1, %%rax\n\t"
//                  "mov %2, %%rdi\n\t"
//                  "mov %3, %%rsi\n\t"
//                  "mov %4, %%rdx\n\t"
//                  "mov %5, %%r10\n\t"
//                  "mov %6, %%r8\n\t"
//                  "mov %7, %%r9\n\t"
//                  "syscall\n\t"
//                  "mov %%rax, %0\n\t"
//                  : "=r"(ret)
//                  : "r"(syscall_number), "r"(arg1), "r"(arg2), "r"(arg3),
//                    "r"(arg4), "r"(arg5)
//                  : "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9");
//     return ret;
// }

// __attribute__((naked))
// uint64_t syscall_do(uint64_t syscall_num, uint64_t arg) {
//     asm volatile("int $0x80"
//                     : "S"(arg)
//                     : "D"(syscall_num), "S"(arg)
//     );
//
//     return arg;
// }
