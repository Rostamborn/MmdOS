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
    struct gdt_descriptor descriptors[11];
};

struct gdtr {
    uint16_t limit;
    uint64_t address;
} __attribute__ ((packed));

static struct gdt gdt_instance;
static struct gdtr gdtr_instance;

// void flush_gdt() {
//     // we assume kernel code selector in 0x8
//     asm volatile("\
//         mov $0x10, %ax \n\
//         mov %ax, %ds \n\
//         mov %ax, %es \n\
//         mov %ax, %fs \n\
//         mov %ax, %gs \n\
//         mov %ax, %ss \n\
//         \n\
//         pop %rdi \n\
//         push $0x8 \n\
//         push %rdi \n\
//         retfq \n\
//     ");
// }
//
// void load_gdt() {
//     asm volatile("lgdt %0" : : "m"(gdtr_instance));
//     flush_gdt();
// }
//

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
    gdt_instance.descriptors[9].limit       = 0;
    gdt_instance.descriptors[9].base_low16  = 0;
    gdt_instance.descriptors[9].base_mid8   = 0;
    gdt_instance.descriptors[9].access      = 0b11111010;
    gdt_instance.descriptors[9].granularity = 0b00100000;
    gdt_instance.descriptors[9].base_high8  = 0;

    // User data 64.
    gdt_instance.descriptors[10].limit       = 0;
    gdt_instance.descriptors[10].base_low16  = 0;
    gdt_instance.descriptors[10].base_mid8   = 0;
    gdt_instance.descriptors[10].access      = 0b11110010;
    gdt_instance.descriptors[10].granularity = 0;
    gdt_instance.descriptors[10].base_high8  = 0;

    gdtr_instance.limit = sizeof(struct gdt) - 1;
    gdtr_instance.address = (uint64_t)&gdt_instance;

    // load_gdt();
    gdt_load();
}

