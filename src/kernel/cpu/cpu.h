#ifndef KERNEL_CPU_DEF
#define KERNEL_CPU_DEF

#include <stdint.h>

#define PORT 0x3f8

// to communicate with ports(using inb & outb asm instructions)
static inline uint8_t inb(int portnum) {
    unsigned char data = 0;
    __asm__ __volatile__("inb %%dx, %%al" : "=a"(data) : "d"(portnum));

    return data;
}

static inline uint8_t outb(int portnum, unsigned char value) {
    __asm__ __volatile__("outb %%al, %%dx" : : "a"(value), "d"(portnum));

    return value;
}

// extern inline void io_wait(void);

// halt, catch fire
static inline void hcf(void) {
    asm("cli");
    for (;;) {
        asm("hlt");
    }
}

static inline uint64_t read_cr3() {
    uint64_t cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

static inline void write_cr3(uint64_t cr3) {
    asm volatile("mov %0, %%cr3" : : "r"(cr3));
}

int serial_init();

void log_to_serial(char* string);

void log_to_serial_digit(uint64_t digit);

void disable_pic();

uint64_t rdmsr(uint32_t msr);

static inline void enable_interrupts() { asm volatile("sti"); }

static inline void disable_interrupts() { asm volatile("cli"); }

// Note(Arman): This is nothing short of genius. We pass the stack pointer to
// interrupt_dispatch in interrupt_stub. To make it easier to access the stack
// contents, we create a struct which resembles the pushed content and thus we
// can access the stack contents easily.
typedef struct {
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
    uint64_t error_code; // This is pushed by the cpu if the interrupt is an
                         // error interrupt. If not, then we push a dummy value
                         // of 0(in the macro)
    uint64_t iret_rip;   // iret prefix means that the cpu pushed this
                         // automatically and we didn't
    uint64_t iret_cs;
    uint64_t iret_flags;
    uint64_t iret_rsp;
    uint64_t iret_ss;
} __attribute__((packed)) execution_context;

typedef struct {
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
} __attribute__((packed)) tss_frame;
#endif
