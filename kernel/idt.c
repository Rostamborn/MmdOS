#include <stdint.h>
#include "idt.h"
#include "cpu.h"

extern void *isr_stub_table[];

struct interrupt_descriptor { // I think it's also called a gate descriptor
    uint16_t address_low;
    uint16_t selector; // this would be the base address of the code segment
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

struct interrupt_descriptor idt[256]; // 256 interrupts

// Note(Arman): This is nothing short of genius. We pass the stack pointer to
// interrupt_dispatch in interrupt_stub. To make it easier to access the stack
// contents, we create a struct which resembles the pushed content and thus we
// can access the stack contents easily.
struct interrupt_frame {
    uint64_t rbp;
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    uint64_t int_number; // this is pushed in the macro (Hopefully it's 8 bytes)
    uint64_t error_code; // This is pushed by the cpu if the interrupt is an error interrupt.
                         // If not, then we push a dummy value of 0(in the macro)
    uint64_t iret_rip; // iret prefix means that the cpu pushed this automatically and we didn't
    uint64_t iret_cs;
    uint64_t iret_flags;
    uint64_t iret_rsp;
    uint64_t iret_ss;

} __attribute__ ((packed));

// because we have the interrupt number, we can send all interrupts here and then call
// the appropriate interrupt handler using the switch cases
extern struct interrupt_frame* interrupt_dispatch(struct interrupt_frame *frame) {
    // log_to_serial("running interrupt_dispatch\n");
    switch (frame->int_number) {
        case 0:
            log_to_serial("Divide by zero error\n");
            break;
        case 1:
            log_to_serial("Debug\n");
            break;
        case 2:
            log_to_serial("Non-maskable interrupt\n");
            break;
        case 3:
            log_to_serial("Breakpoint\n");
            break;
        case 4:
            log_to_serial("Overflow\n");
            break;
        case 5:
            log_to_serial("Bound range exceeded\n");
            break;
        case 6:
            log_to_serial("Invalid opcode\n");
            break;
        case 7:
            log_to_serial("Device not available\n");
            break;
        case 8:
            log_to_serial("Double fault\n");
            break;
        case 9:
            log_to_serial("Coprocessor segment overrun\n");
            break;
        case 10:
            log_to_serial("Invalid TSS\n");
            break;
        case 11:
            log_to_serial("Segment not present\n");
            break;
        case 12:
            log_to_serial("Stack-segment fault\n");
            break;
        case 13:
            log_to_serial("General protection fault\n");
            break;
        case 14:
            log_to_serial("Page fault\n");
            break;
        case 15:
            log_to_serial("Reserved\n");
            break;
        case 16:
            log_to_serial("x87 floating-point exception\n");
            break;
        case 17:
            log_to_serial("Alignment check\n");
            break;
        case 18:
            log_to_serial("Machine check\n");
            break;
        case 19:
            log_to_serial("SIMD floating-point exception\n");
            break;
        case 20:
            log_to_serial("Virtualization exception\n");
            break;
        case 21:
            log_to_serial("Reserved\n");
            break;
        case 22:
            // do something 
            break;
        case 23:
            // do something
            break;
        case 24:
            // do something
            break;
        case 25:
            // do something
            break;
        case 26:
            // do something
            break;
        case 27:
            // do something
            break;
        case 28:
            // do something
        case 29:
            // do something
            break;
        case 30:
            // do something
            break;
        case 31:
            // do something
            break;
        default:
            // do something
            break;
    }

    return frame;
}

// the handler parameter is the interrupt stub. The interrupt stub is a function
// TODO: make handlers meaningful and actually do something.
void set_interrupt_descriptor(uint8_t vector, void *handler, uint8_t dpl) {
    // dpl is the descriptor privilage level which determines the highest
    // cpu ring that can trigger this interrupt via software(default of 0 is fine as there is no user mode).
    uint64_t handler_address = (uint64_t)handler;

    struct interrupt_descriptor *entry = &idt[vector];
    entry->address_low = handler_address & 0xffff;
    entry->address_mid = (handler_address >> 16) & 0xffff;
    entry->address_high = (handler_address >> 32) & 0xffffffff;
    entry->selector = 0x28; // kernel code selector is 0x28 (refer to GDT). the base address of the code segment
    entry->flags = 0b1110 | ((dpl & 0b11) << 5) | (1 << 7);
    entry->ist = 0;
    entry->reserved = 0;
}

void idt_load() {
    struct idtr idtr_instance; // it's ok to have it on stack as IDTR register will keep the copy.
    idtr_instance.limit = 256 * sizeof(struct interrupt_descriptor) - 1; // the limit is added to the base address.
                                                                         // the minus one is because we want
                                                                         // point to the last byte of the table.
    idtr_instance.address = (uint64_t)idt;
    asm volatile("lidt %0" : : "m"(idtr_instance) : "memory");
    asm volatile("sti");
}

// populate the idt with the interrupt stubs (no need to populate all of the table)
extern void idt_init() {
    // for (int i = 0; i < 256; i++) {
    //     set_interrupt_descriptor(i, isr_stub_table[i], 0);
    // }
    set_interrupt_descriptor(0, isr0, 0);
    set_interrupt_descriptor(0, isr1, 0);
    set_interrupt_descriptor(0, isr2, 0);
    set_interrupt_descriptor(0, isr3, 0);
    set_interrupt_descriptor(0, isr4, 0);
    set_interrupt_descriptor(0, isr5, 0);
    set_interrupt_descriptor(0, isr6, 0);
    set_interrupt_descriptor(0, isr7, 0);
    set_interrupt_descriptor(0, isr_err8, 0);
    set_interrupt_descriptor(0, isr9, 0);
    set_interrupt_descriptor(0, isr_err10, 0);
    set_interrupt_descriptor(0, isr_err11, 0);
    set_interrupt_descriptor(0, isr_err12, 0);
    set_interrupt_descriptor(0, isr_err13, 0);
    set_interrupt_descriptor(0, isr_err14, 0);
    set_interrupt_descriptor(0, isr15, 0);
    set_interrupt_descriptor(0, isr16, 0);
    set_interrupt_descriptor(0, isr17, 0);
    set_interrupt_descriptor(0, isr18, 0);
    set_interrupt_descriptor(0, isr19, 0);
    set_interrupt_descriptor(0, isr20, 0);
    set_interrupt_descriptor(0, isr21, 0);
    set_interrupt_descriptor(0, isr22, 0);
    set_interrupt_descriptor(0, isr23, 0);
    set_interrupt_descriptor(0, isr24, 0);
    set_interrupt_descriptor(0, isr25, 0);
    set_interrupt_descriptor(0, isr26, 0);
    set_interrupt_descriptor(0, isr27, 0);
    set_interrupt_descriptor(0, isr28, 0);
    set_interrupt_descriptor(0, isr29, 0);
    set_interrupt_descriptor(0, isr30, 0);
    set_interrupt_descriptor(0, isr31, 0);
    set_interrupt_descriptor(0, isr32, 0);
    set_interrupt_descriptor(0, isr33, 0);
    set_interrupt_descriptor(0, isr34, 0);
    set_interrupt_descriptor(0, isr35, 0);
    set_interrupt_descriptor(0, isr36, 0);
    set_interrupt_descriptor(0, isr37, 0);
    set_interrupt_descriptor(0, isr38, 0);
    set_interrupt_descriptor(0, isr39, 0);
    set_interrupt_descriptor(0, isr40, 0);
    set_interrupt_descriptor(0, isr41, 0);
    set_interrupt_descriptor(0, isr42, 0);
    set_interrupt_descriptor(0, isr43, 0);
    set_interrupt_descriptor(0, isr44, 0);
    set_interrupt_descriptor(0, isr45, 0);
    set_interrupt_descriptor(0, isr46, 0);
    set_interrupt_descriptor(0, isr47, 0);

    idt_load();
}
