#include "util.h"
#include "logger.h"
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

void kstrcpy(char dest[], char* src, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        dest[i] = *src;
        if (*src == '\0') {
            break;
        }
        src++;
    }
}

bool kstrcmp(char* str1, char* str2, uint32_t len) {
    while ((*str1 != '\0') && (*str2 != '\0')) {
        if (*str1 != *str2) {
            return false;
        }
        str1++;
        str2++;

        if (len == 1) {
            return true;
        }
        len--;
    }

    if ((*str1 == '\0') && (*str2 == '\0')) {
        return true;
    }

    return false;
}

int64_t kpow(int64_t x, uint16_t y) {
    int64_t result = 1;
    while (y) {
        result *= x;
        y--;
    }
    return result;
}

uint64_t oct_ascii_to_dec(char* number, uint16_t size) {
    uint64_t result = 0;
    number++;

    // null terminator + the 0 predecessor which is skipped and also ending in 0
    size -= 3;
    do {
        result += ((uint64_t) (*number - '0')) * (uint64_t) kpow(8, size);
        number++;
        size--;
    } while (size != 0);
    result += ((uint64_t) (*number - '0')) * (uint64_t) kpow(8, size);
    return result;
}