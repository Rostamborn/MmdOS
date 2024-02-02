#include "idt.h"
#include "../cpu/cpu.h"
#include "../cpu/pic.h"
#include "../lib/logger.h"
#include "../lib/panic.h"
#include "../terminal/limine_term.h"
#include "../userland/sys.h"
#include <stdint.h>
extern void* isr_stub_table[];

struct interrupt_descriptor { // I think it's also called a gate descriptor
    uint16_t address_low;
    uint16_t selector; // this would be the base address of the code segment
    uint8_t  ist;      // should have zero value
    uint8_t  flags;
    uint16_t address_mid;
    uint32_t address_high;
    uint32_t reserved;
} __attribute__((packed));

struct idtr {
    uint16_t limit;
    uint64_t address;
} __attribute__((packed));

struct interrupt_descriptor idt[256]; // 256 interrupts
void*                       irq_handlers[16] = {0};
void*                       isr_handlers[32] = {0};

char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "SIMD floating-point exception",
    "Virtualization exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Timer",
};

// ISRs
execution_context* isr_handler(execution_context* frame) {
    execution_context* (*handler)(execution_context* frame);
    handler = irq_handlers[frame->int_number];

    if (handler) {
        handler(frame);
    }

    if (frame->int_number < 32) {
        // log_to_serial(exception_messages[frame->int_number]);
        panic("%s", exception_messages[frame->int_number]);
    }

    return frame;
}

// IRQs
execution_context* irq_handler(execution_context* frame) {
    execution_context* (*handler)(execution_context* frame);
    handler = irq_handlers[frame->int_number - 32];
    if (handler) {
        if (frame->int_number == 32) {
            frame = handler(frame);
        } else {
            handler(frame);
        }
    }
    pic_eoi(frame->int_number);

    return frame;
}

void isr_install_handler(
    uint8_t offset, execution_context* (*handler)(execution_context* frame)) {
    isr_handlers[offset] = handler;
}

void irq_install_handler(
    uint8_t offset, execution_context* (*handler)(execution_context* frame)) {
    irq_handlers[offset] = handler;
}

void irq_uninstall_handler(uint8_t offset) { irq_handlers[offset] = 0; }

void set_interrupt_descriptor(uint8_t vector, void* handler, uint8_t dpl) {
    // dpl is the descriptor privilege level which determines the highest
    // cpu ring that can trigger this interrupt via software(default of 0 is
    // fine as there is no user mode).

    uint64_t                     handler_address = (uint64_t) handler;
    struct interrupt_descriptor* entry = &idt[vector];

    entry->address_low = handler_address & 0xffff;
    entry->address_mid = (handler_address >> 16) & 0xffff;
    entry->address_high = (handler_address >> 32) & 0xffffffff;
    entry->selector = 0x28; // kernel code selector is 0x28 (refer to GDT). the
                            // base address of the code segment
    entry->flags = 0b1110 | ((dpl & 0b11) << 5) | (1 << 7);
    entry->ist = 0;
    entry->reserved = 0;
}

void idt_load() {
    struct idtr idtr_instance; // it's ok to have it on stack as IDTR register
                               // will keep the copy.
    idtr_instance.limit = 256 * sizeof(struct interrupt_descriptor) -
                          1; // the limit is added to the base address.
                             // the minus one is because we want
                             // point to the last byte of the table.
    idtr_instance.address = (uint64_t) idt;
    asm volatile("lidt %0" : : "m"(idtr_instance) : "memory");
    asm volatile("sti");
}

// populate the idt with the interrupt stubs (no need to populate all of the
// table)
extern void idt_init() {
    set_interrupt_descriptor(0, isr0, 0);
    set_interrupt_descriptor(1, isr1, 0);
    set_interrupt_descriptor(2, isr2, 0);
    set_interrupt_descriptor(3, isr3, 0);
    set_interrupt_descriptor(4, isr4, 0);
    set_interrupt_descriptor(5, isr5, 0);
    set_interrupt_descriptor(6, isr6, 0);
    set_interrupt_descriptor(7, isr7, 0);
    set_interrupt_descriptor(8, isr_err8, 0);
    set_interrupt_descriptor(9, isr9, 0);
    set_interrupt_descriptor(10, isr_err10, 0);
    set_interrupt_descriptor(11, isr_err11, 0);
    set_interrupt_descriptor(12, isr_err12, 0);
    set_interrupt_descriptor(13, isr_err13, 0);
    set_interrupt_descriptor(14, isr_err14, 0);
    set_interrupt_descriptor(15, isr15, 0);
    set_interrupt_descriptor(16, isr16, 0);
    set_interrupt_descriptor(17, isr17, 0);
    set_interrupt_descriptor(18, isr18, 0);
    set_interrupt_descriptor(19, isr19, 0);
    set_interrupt_descriptor(20, isr20, 0);
    set_interrupt_descriptor(21, isr21, 0);
    set_interrupt_descriptor(22, isr22, 0);
    set_interrupt_descriptor(23, isr23, 0);
    set_interrupt_descriptor(24, isr24, 0);
    set_interrupt_descriptor(25, isr25, 0);
    set_interrupt_descriptor(26, isr26, 0);
    set_interrupt_descriptor(27, isr27, 0);
    set_interrupt_descriptor(28, isr28, 0);
    set_interrupt_descriptor(29, isr29, 0);
    set_interrupt_descriptor(30, isr30, 0);
    set_interrupt_descriptor(31, isr31, 0);

    // here on would be IRQs
    set_interrupt_descriptor(32, irq0, 3);
    set_interrupt_descriptor(33, irq1, 0);
    set_interrupt_descriptor(34, irq2, 0);
    set_interrupt_descriptor(35, irq3, 0);
    set_interrupt_descriptor(36, irq4, 0);
    set_interrupt_descriptor(37, irq5, 0);
    set_interrupt_descriptor(38, irq6, 0);
    set_interrupt_descriptor(39, irq7, 0);
    set_interrupt_descriptor(40, irq8, 0);
    set_interrupt_descriptor(41, irq9, 0);
    set_interrupt_descriptor(42, irq10, 0);
    set_interrupt_descriptor(43, irq11, 0);
    set_interrupt_descriptor(44, irq12, 0);
    set_interrupt_descriptor(45, irq13, 0);
    set_interrupt_descriptor(46, irq14, 0);
    set_interrupt_descriptor(47, irq15, 0);

    // used for system calls
    set_interrupt_descriptor(128, isr128, 3);
    // set_interrupt_descriptor(177, isr177, 3);

    pic_init();
    idt_load();
}
