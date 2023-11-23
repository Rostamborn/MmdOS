#include <stdint.h>
#include "gdt.h"

struct gdt_descriptor {
    uint16_t limit;
    uint16_t base_low16;
    uint8_t  base_mid8;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high8;
};

struct gdt {
    struct gdt_descriptor descriptors[9];
};

struct gdtr {
    uint16_t limit;
    uint64_t address;
} __attribute__ ((packed));

static struct gdt gdt_instance;
static struct gdtr gdtr_instance;

// TODO: implement this later
struct tss {
    uint32_t unused0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t unused1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t unused2;
    uint32_t iopb;
} __attribute__((packed));

void gdt_load(void) {
    asm volatile (
        "lgdt %0\n\t"
        "push $0x28\n\t"
        "lea 1f(%%rip), %%rax\n\t"
        "push %%rax\n\t"
        "lretq\n\t"
        "1:\n\t"
        "mov $0x30, %%eax\n\t"
        "mov %%eax, %%ds\n\t"
        "mov %%eax, %%es\n\t"
        "mov %%eax, %%fs\n\t"
        "mov %%eax, %%gs\n\t"
        "mov %%eax, %%ss\n\t"
        :
        : "m"(gdtr_instance)
        : "rax", "memory"
    );
}

void gdt_init() {
    // Null descriptor.
    gdt_instance.descriptors[0].limit       = 0;
    gdt_instance.descriptors[0].base_low16  = 0;
    gdt_instance.descriptors[0].base_mid8   = 0;
    gdt_instance.descriptors[0].access      = 0;
    gdt_instance.descriptors[0].granularity = 0;
    gdt_instance.descriptors[0].base_high8  = 0;

    // Kernel code 16.
    gdt_instance.descriptors[1].limit       = 0xffff;
    gdt_instance.descriptors[1].base_low16  = 0;
    gdt_instance.descriptors[1].base_mid8   = 0;
    gdt_instance.descriptors[1].access      = 0b10011010;
    gdt_instance.descriptors[1].granularity = 0b00000000;
    gdt_instance.descriptors[1].base_high8  = 0;

    // Kernel data 16.
    gdt_instance.descriptors[2].limit       = 0xffff;
    gdt_instance.descriptors[2].base_low16  = 0;
    gdt_instance.descriptors[2].base_mid8   = 0;
    gdt_instance.descriptors[2].access      = 0b10010010;
    gdt_instance.descriptors[2].granularity = 0b00000000;
    gdt_instance.descriptors[2].base_high8  = 0;

    // Kernel code 32.
    gdt_instance.descriptors[3].limit       = 0xffff;
    gdt_instance.descriptors[3].base_low16  = 0;
    gdt_instance.descriptors[3].base_mid8   = 0;
    gdt_instance.descriptors[3].access      = 0b10011010;
    gdt_instance.descriptors[3].granularity = 0b11001111;
    gdt_instance.descriptors[3].base_high8  = 0;

    // Kernel data 32.
    gdt_instance.descriptors[4].limit       = 0xffff;
    gdt_instance.descriptors[4].base_low16  = 0;
    gdt_instance.descriptors[4].base_mid8   = 0;
    gdt_instance.descriptors[4].access      = 0b10010010;
    gdt_instance.descriptors[4].granularity = 0b11001111;
    gdt_instance.descriptors[4].base_high8  = 0;

    // Kernel code 64.
    gdt_instance.descriptors[5].limit       = 0;
    gdt_instance.descriptors[5].base_low16  = 0;
    gdt_instance.descriptors[5].base_mid8   = 0;
    gdt_instance.descriptors[5].access      = 0b10011010;
    gdt_instance.descriptors[5].granularity = 0b00100000;
    gdt_instance.descriptors[5].base_high8  = 0;

    // Kernel data 64.
    gdt_instance.descriptors[6].limit       = 0;
    gdt_instance.descriptors[6].base_low16  = 0;
    gdt_instance.descriptors[6].base_mid8   = 0;
    gdt_instance.descriptors[6].access      = 0b10010010;
    gdt_instance.descriptors[6].granularity = 0;
    gdt_instance.descriptors[6].base_high8  = 0;

    // User code 64.
    gdt_instance.descriptors[7].limit       = 0;
    gdt_instance.descriptors[7].base_low16  = 0;
    gdt_instance.descriptors[7].base_mid8   = 0;
    gdt_instance.descriptors[7].access      = 0b11111010;
    gdt_instance.descriptors[7].granularity = 0b00100000;
    gdt_instance.descriptors[7].base_high8  = 0;

    // User data 64.
    gdt_instance.descriptors[8].limit       = 0;
    gdt_instance.descriptors[8].base_low16  = 0;
    gdt_instance.descriptors[8].base_mid8   = 0;
    gdt_instance.descriptors[8].access      = 0b11110010;
    gdt_instance.descriptors[8].granularity = 0;
    gdt_instance.descriptors[8].base_high8  = 0;

    gdtr_instance.limit = sizeof(struct gdt) - 1;
    gdtr_instance.address = (uint64_t)&gdt_instance;

    // load_gdt();
    gdt_load();
}

