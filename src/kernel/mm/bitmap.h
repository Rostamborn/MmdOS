#include <stdbool.h>
#include <stdint.h>

// Ingenius
static inline void bitmap_set(uint8_t* bitmap, uint64_t bit) {
    bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void bitmap_clear(uint8_t* bitmap, uint64_t bit) {
    bitmap[bit / 8] &= ~(1 << (bit % 8)); // basically clear the bit
}

static inline bool bitmap_get(uint8_t* bitmap, uint64_t bit) {
    return bitmap[bit / 8] & (1 << (bit % 8));
}
