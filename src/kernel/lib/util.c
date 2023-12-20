#include "util.h"
#include <stdint.h>

uint64_t div_round_up(uint64_t addr, uint64_t align) {
    return (addr + align - 1) / align;
}

uint64_t page_align_up(uint64_t addr) {
    // return (addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    return div_round_up(addr, PAGE_SIZE) * PAGE_SIZE;
}

uint64_t page_align_down(uint64_t addr) {
    // return addr & ~(PAGE_SIZE - 1);
    return (addr / PAGE_SIZE) * PAGE_SIZE;
}

uint64_t max(uint64_t a, uint64_t b) { return a > b ? a : b; }

void memset(void* ptr, uint8_t value, uint64_t size) {
    uint8_t* tmp = (uint8_t*) ptr;
    for (; size > 0; size--) {
        *tmp = value;
        tmp++;
    }
}

void memcpy(void* restrict dest, const void* restrict src, uint64_t size) {
    uint8_t* d = (uint8_t*) dest;
    uint8_t* s = (uint8_t*) src;
    for (; size > 0; size--) {
        *d = *s;
        d++;
        s++;
    }
}
