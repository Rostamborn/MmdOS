#include <stdint.h>
#include <stdbool.h>

extern struct limine_memmap_request memmap_req;

extern struct limine_hhdm_request hhdm_req;

#define HHDM_OFFSET (hhdm_req.response->offset)

// #define KERNEL_BASE 0xffffffff80000000;
// #define KERNEL_TOP (KERNEL_BASE + 128*1024*1024); // 128 MB for kernel

void pmm_init();

void* pmm_alloc(uint64_t n_pages);

void* pmm_alloc_nozero(uint64_t n_pages);

void pmm_free(void* addr, uint64_t n_pages);

bool pmm_check_alloc(void* addr);
