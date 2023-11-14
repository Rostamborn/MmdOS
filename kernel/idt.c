#include <stdint.h>


struct interrupt_descriptor {
    uint16_t address_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t flags;
    uint16_t address_mid;
    uint32_t address_high;
    uint32_t reserved;
} __attribute__ ((packed));

struct idtr {
    uint16_t limit;
    uint64_t address;
} __attribute__ ((packed));

struct interrupt_descriptor idt[256];

void set_interrupt_descriptor(uint8_t vector, void *handler, uint8_t dpl) {
    // dpl is the descriptor privilage level which determines the highest
    // cpu ring that can trigger this interrupt via software(default of 0 is fine as there is no user mode).
    uint64_t handler_address = (uint64_t)handler;

    struct interrupt_descriptor *entry = &idt[vector];
    entry->address_low = handler_address & 0xffff;
    entry->address_mid = (handler_address >> 16) & 0xffff;
    entry->address_high = handler_address >> 32;
    entry->selector = 0x8; // kernel code selector is 0x8 or gdt[1]
    entry->flags = 0b1110 | ((dpl & 0b11) << 5) | (1 << 7);
    entry->ist = 0;
}

void load_idt(void *idt_address) {
    struct idtr idtr_instance; // it's ok to have it on stack as IDTR register will keep the copy.
    idtr_instance.limit = 0xfff;
    idtr_instance.address = (uint64_t)idt_address;
    asm volatile("lidt %0" : : "m"(idtr_instance));
}
