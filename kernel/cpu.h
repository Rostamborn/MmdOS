#include <stdint.h>

#define PORT 0x3f8

extern unsigned char inb(int portnum);

extern unsigned char outb(int portnum, unsigned char value);

// extern inline void io_wait(void);

void hcf(void);

int init_serial();

void log_to_serial(char *string);

void log_to_serial_digit(uint64_t digit);

void disable_pic();

uint64_t rdmsr(uint32_t msr);

inline void enable_interrupts();

inline void enable_interrupts();

void memset_k(void *ptr, uint8_t value, uint64_t size);

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
    uint64_t error_code; // This is pushed by the cpu if the interrupt is an error interrupt.
                         // If not, then we push a dummy value of 0(in the macro)
    uint64_t iret_rip; // iret prefix means that the cpu pushed this automatically and we didn't
    uint64_t iret_cs;
    uint64_t iret_flags;
    uint64_t iret_rsp;
    uint64_t iret_ss;
} __attribute__ ((packed)) interrupt_frame;

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
