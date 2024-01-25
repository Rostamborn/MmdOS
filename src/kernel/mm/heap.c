#include "kheap.h"
#include "../lib/logger.h"
#include "../lib/panic.h"
#include "../scheduler/process.h"
#include "pmm.h"
#include "vmm.h"
#include <stdint.h>

void* u_alloc(uint64_t size) {
    vmm_t* curr_vmm = process_get_current_vmm();

    arena_t* current = curr_vmm->arena;
    if (current != NULL) {
        for (arena_t* arena = current; arena != NULL; arena = arena->next) {
            if ((arena->size - arena->offset) >=
                size + sizeof(arena_metadata)) {
                arena_metadata* metadata =
                    (arena_metadata*) ((uintptr_t) arena->base + arena->offset);
                metadata->arena_base = arena->base;

                void* ret = (void*) ((uintptr_t) arena->base + arena->offset +
                                     sizeof(arena_metadata));

                arena->offset =
                    ALIGN_UP(arena->offset + size + sizeof(arena_metadata),
                             DEFAULT_ALIGNMENT);
                arena->allocated++;

                return ret;
            }
            current = arena; // last one that is not NULL
        }
    }

    // no arena or no space in current arena
    void*    new_alloc = pmm_alloc(DIV_ROUNDUP(size, PAGE_SIZE));
    arena_t* new_arena = (arena_t*) (new_alloc + HHDM_OFFSET);

    if (!vmm_map_page(curr_vmm, (uintptr_t) new_arena, (uintptr_t) new_alloc,
                      PTE_PRESENT | PTE_WRITABLE)) {
        panic("k_alloc: failed to map new allocated page");
    }

    new_arena->next = NULL;
    new_arena->base = (uintptr_t) new_arena;
    new_arena->size = ALIGN_UP(size, PAGE_SIZE);
    new_arena->offset = sizeof(arena_t);
    new_arena->allocated = 0;

    if (current == NULL) {
        curr_vmm->arena = new_arena;
    } else {
        current->next = new_arena;
    }

    arena_metadata* metadata =
        (arena_metadata*) (new_arena->base + new_arena->offset);
    metadata->arena_base = (uint64_t) new_arena;

    void* ret =
        (void*) (new_arena->base + new_arena->offset + sizeof(arena_metadata));

    new_arena->offset = ALIGN_UP(
        new_arena->offset + size + sizeof(arena_metadata), DEFAULT_ALIGNMENT);
    new_arena->allocated++;

    return ret;
}

void u_free(void* addr) {
    arena_metadata* metadata =
        (arena_metadata*) (addr - sizeof(arena_metadata));
    arena_t* arena = (arena_t*) metadata->arena_base;

    arena->allocated--;

    if (arena->allocated <= 0) {
        // free arena
        arena_t* curr = process_get_current_vmm()->arena;
        if (curr == arena) {
            curr = arena->next;
        } else {
            while (curr->next != arena) {
                curr = curr->next;
            }
            curr->next = arena->next;
        }

        pmm_free((void*) arena - HHDM_OFFSET,
                 DIV_ROUNDUP(arena->size, PAGE_SIZE));

        for (uint64_t i = 0; i < arena->size / PAGE_SIZE; i++) {
            vmm_unmap_page(process_get_current_vmm(),
                           (uintptr_t) arena + i * PAGE_SIZE, true);
        }
    }

    return;
}
