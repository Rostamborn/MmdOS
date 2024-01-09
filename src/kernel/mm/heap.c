#include "../lib/logger.h"
#include "../lib/panic.h"
#include "../lib/util.h"
#include "../scheduler/process.h"
#include "kheap.h"
#include "pmm.h"
#include "vmm.h"
#include <stdint.h>

void* heap_alloc(uint64_t size) {
    PageMap* pagemap = process_get_pagemap();

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
        uintptr_t metadata_size = sizeof(k_heap_metadata);
        if (arena_offset + size + metadata_size <= arena_size) {
            k_heap_metadata* metadata =
                (k_heap_metadata*) (arena_base + arena_offset);
            metadata->arena = &pagemap->arenas[i];
            result = (void*) (arena_base + arena_offset + size + metadata_size);
            pagemap->arenas[i].offset = arena_offset + size + metadata_size;
            pagemap->arenas[i].allocated++;
            return result;
        }
    }
    spinlock_release(&pagemap->lock);
end:

    // if we don't find free space among existing arenas
    void* new_alloc =
        pmm_alloc(DIV_ROUNDUP(size, PAGE_SIZE) + 1); // +1 because vm_arena
    if (new_alloc == NULL) {
        return NULL;
    }
    vm_arena* new_arena = (vm_arena*) new_alloc;
    new_arena->size =
        ALIGN_UP(size, PAGE_SIZE); // size without the meta data page
    new_arena->base = (uintptr_t) ((void*) new_arena + PAGE_SIZE);
    new_arena->id = pagemap->arena_count;
    new_arena->offset = 0;
    new_arena->allocated = 0;
    new_arena->flags = PTE_PRESENT | PTE_WRITABLE;
    new_arena->next = NULL;

    spinlock_acquire(&pagemap->lock);
    vm_arena* current = pagemap->arenas;
    if (current != NULL) {
        for (; current != NULL;) {
            if (current->next == NULL) {
                current->next = (vm_arena*) ((void*) new_arena);
                pagemap->arena_count++;
                break;
            }
            current = current->next;
        }
    } else {
        pagemap->arenas = (vm_arena*) ((void*) new_arena);
        pagemap->arena_count = 1;
    }
    spinlock_release(&pagemap->lock);

    if (!vmm_map(pagemap, (uintptr_t) new_arena, (uintptr_t) new_arena,
                 new_arena->flags)) {
        panic("Failed to map new obect");
    }

    // NOTE(Arman): we might need to map the proceeding pages as well
    // if (!vmm_map(pagemap, new_arena->base, (uintptr_t)((void*)new_arena +
    // PAGE_SIZE), new_arena->flags)) {
    //     panic("Failed to map new obect");
    // }

    k_heap_metadata* metadata = (k_heap_metadata*) (new_arena->base);
    metadata->arena = (vm_arena*) ((void*) new_arena);

    new_arena->offset = size + sizeof(k_heap_metadata);
    new_arena->allocated++;

    return (void*) (new_arena->base + (uintptr_t) sizeof(k_heap_metadata));
}

void heap_free(void* ptr) {
    k_heap_metadata* metadata =
        (k_heap_metadata*) ((uintptr_t) ptr - sizeof(k_heap_metadata));
    vm_arena* arena = metadata->arena;
    if (arena == NULL) {
        panic("k_heap_free: arena is NULL");
    }
    arena->allocated--;

    if (arena->allocated == 0) {
        // free the arena
        vm_arena* current = vmm_kernel->arenas;
        if (current == arena) {
            spinlock_acquire(&vmm_kernel->lock);
            vmm_kernel->arenas = arena->next;
            vmm_kernel->arenas = arena->next;
            pmm_free(
                (void*) vmm_virt2phys(vmm_kernel, (uintptr_t) arena, false),
                DIV_ROUNDUP(arena->size, PAGE_SIZE) + 1);
            vmm_unmap(vmm_kernel, (uintptr_t) arena, true);
            vmm_kernel->arena_count--;
            spinlock_release(&vmm_kernel->lock);
            klog("kheap ::", "freed virtual memory");
            return;
        }
        for (; current != NULL;) {
            if (current->next == arena) {
                spinlock_acquire(&vmm_kernel->lock);
                current->next = arena->next;
                pmm_free(
                    (void*) vmm_virt2phys(vmm_kernel, (uintptr_t) arena, false),
                    DIV_ROUNDUP(arena->size, PAGE_SIZE) + 1);
                vmm_unmap(vmm_kernel, (uintptr_t) arena, true);
                vmm_kernel->arena_count--;
                spinlock_release(&vmm_kernel->lock);
                klog("kheap ::", "freed virtual memory");
                return;
            }
            current = current->next;
        }
    }
    klog("kheap ::", "freed virtual memory");
}
