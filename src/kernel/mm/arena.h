#include <cstdint>
#include <stdint.h>


#define IS_POWER_2(num) (((num) & (num-1)) == 0)

typedef struct {
    uint8_t* buf;
    uint64_t buf_size;
    uint64_t curr_offset;
    uint64_t prev_offset;
} Arena;

typedef struct {
    Arena* arena;
    uint64_t curr_offset;
    uint64_t prev_offset;
} Temp_Arena;

uintptr_t align_forward(uintptr_t ptr, uint64_t align);

void* arena_alloc(Arena* arena, uint64_t size, uint64_t align);

void arena_init(Arena* arena, void* buf, uint64_t buf_size);

void arena_free_all(Arena* arena);
