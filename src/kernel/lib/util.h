#include <stdint.h>

#define NULL (void*) 0

#define PAGE_SIZE 4096

uint64_t div_round_up(uint64_t addr, uint64_t align);

uint64_t page_align_up(uint64_t addr);

uint64_t page_align_down(uint64_t addr);

uint64_t max(uint64_t a, uint64_t b);

void memset(void* ptr, uint8_t value, uint64_t size);
