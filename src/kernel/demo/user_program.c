#include <stdint.h>

extern uint64_t do_syscall(uint64_t frame, uint64_t a0, uint64_t a1,
                           uint64_t a2, uint64_t a3);

void main() {
    do_syscall(4, "\nfrom user program\n", 0, 0, 0);
    do_syscall(4, "it actually worked\n", 0, 0, 0);
    do_syscall(4, "that like magic!!\n", 0, 0, 0);
}