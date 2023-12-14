#include <stdint.h>

#define PAGE_SIZE 4096

uint64_t page_align(uint64_t addr) {
    return (addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
}
