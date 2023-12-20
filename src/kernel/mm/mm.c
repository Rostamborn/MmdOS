#include <stdint.h>

#define VM_FLAG_NONE 0x0
#define VM_FLAG_WRITE (1 << 0)
#define VM_FLAG_EXEC (1 << 1)
#define VM_FLAG_USER (1 << 2)
#define VM_FLAG_MMIO (1 << 3)

struct vm_object {
    uintptr_t         base;
    uint64_t          size;  // not sure if it's 64-bits
    uint64_t          falgs; // to make flags platform-agnostic
    struct vm_object* next;
};

typedef struct vm_object vm_object_t;

// IDK why this would be useful
uint64_t convert_x86_vm_flags(uint64_t flags) {
    uint64_t res = 0;
    if (flags & VM_FLAG_WRITE) {
        res |= VM_FLAG_WRITE;
    }
    if (flags & VM_FLAG_USER) {
        res |= VM_FLAG_USER;
    }

    return res;
}
