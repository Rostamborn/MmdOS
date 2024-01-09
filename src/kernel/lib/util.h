#include <stdint.h>

#define NULL (void*) 0

#define PAGE_SIZE 4096

#define VEC_TYPE(type)                                                         \
    struct {                                                                   \
        type*    data;                                                         \
        uint64_t size;                                                         \
        uint64_t capacity;                                                     \
    }

#define MIN(A, B)                                                              \
    ({                                                                         \
        __auto_type MIN_a = A;                                                 \
        __auto_type MIN_b = B;                                                 \
        MIN_a < MIN_b ? MIN_a : MIN_b;                                         \
    })

#define MAX(A, B)                                                              \
    ({                                                                         \
        __auto_type MAX_a = A;                                                 \
        __auto_type MAX_b = B;                                                 \
        MAX_a > MAX_b ? MAX_a : MAX_b;                                         \
    })

#define DIV_ROUNDUP(VALUE, DIV)                                                \
    ({                                                                         \
        __auto_type DIV_ROUNDUP_value = VALUE;                                 \
        __auto_type DIV_ROUNDUP_div = DIV;                                     \
        (DIV_ROUNDUP_value + (DIV_ROUNDUP_div - 1)) / DIV_ROUNDUP_div;         \
    })

#define ALIGN_UP(VALUE, ALIGN)                                                 \
    ({                                                                         \
        __auto_type ALIGN_UP_value = VALUE;                                    \
        __auto_type ALIGN_UP_align = ALIGN;                                    \
        DIV_ROUNDUP(ALIGN_UP_value, ALIGN_UP_align) * ALIGN_UP_align;          \
    })

#define ALIGN_DOWN(VALUE, ALIGN)                                               \
    ({                                                                         \
        __auto_type ALIGN_DOWN_value = VALUE;                                  \
        __auto_type ALIGN_DOWN_align = ALIGN;                                  \
        (ALIGN_DOWN_value / ALIGN_DOWN_align) * ALIGN_DOWN_align;              \
    })

#define SIZEOF_ARRAY(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))

uint64_t div_round_up(uint64_t addr, uint64_t align);

uint64_t page_align_up(uint64_t addr);

uint64_t page_align_down(uint64_t addr);

uint64_t max(uint64_t a, uint64_t b);

void memset(void* ptr, uint8_t value, uint64_t size);

void memcpy(void* restrict dest, const void* restrict src, uint64_t size);

void kstrcpy(char*restrict dest, char*restrict src, uint32_t size);
