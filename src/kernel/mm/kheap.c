#include "kheap.h"
#include "vmm.h"
#include "pmm.h"
#include "../lib/logger.h"
#include "../lib/panic.h"
#include <stdint.h>
#include "../lib/util.h"


void* k_heap_alloc(uint64_t size) {
    PageMap* pagemap = vmm_kernel;
    
    void* result = NULL;

    // if we find free space among existing arenas
    if (pagemap->arena_count == 0 || pagemap->arenas == NULL) {
        goto end;
    }
    spinlock_release(&pagemap->lock);
    for (int i = 0; i < pagemap->arena_count; i++) {
        uintptr_t arena_size = pagemap->arenas[i].size;
        uintptr_t arena_offset = pagemap->arenas[i].offset;
        uintptr_t arena_base = pagemap->arenas[i].base;
        if (arena_offset + size <= arena_size) {
            result = (void*)(arena_base + arena_offset + size);
            pagemap->arenas[i].offset = arena_offset + size;
            return result;
        }
    }
    spinlock_release(&pagemap->lock);
    klog("kheap", "No free space among existing arenas");
end:

    // if we don't find free space among existing arenas
    void* new_alloc = pmm_alloc(DIV_ROUNDUP(size, PAGE_SIZE) + 1); // +1 because vm_arena
    if (new_alloc == NULL) {
        return NULL;
    }
    vm_arena* new_arena = (vm_arena*) new_alloc;
    new_arena->size = ALIGN_UP(size, PAGE_SIZE); // size without the meta data page
    new_arena->base = (uintptr_t)((void*) new_arena + PAGE_SIZE + HHDM_OFFSET);
    new_arena->offset = 0;
    new_arena->flags = PTE_PRESENT | PTE_WRITABLE;
    new_arena->next = NULL;

    spinlock_acquire(&pagemap->lock);
    vm_arena* current = pagemap->arenas;
    if (current != NULL) {
        klog("kheap ::", "current != NULL");
        for (;current != NULL;) {
            if (current->next == NULL) {
                klog("kheap ::", "adding to arena linked list");
                current->next = new_arena;
                pagemap->arena_count++;
                break;
            }
            current = current->next;
        }
    } else {
        pagemap->arenas = new_arena;
        pagemap->arena_count = 1;
    }
    spinlock_release(&pagemap->lock); 
        
    if (!vmm_map(pagemap, new_arena->base, (uintptr_t)((void*)new_arena + PAGE_SIZE), new_arena->flags)) {
        panic("Failed to map new object");
    }

    new_arena->offset = size;
    return (void*) new_arena->base;
}

void k_heap_free(void* ptr) {

}
