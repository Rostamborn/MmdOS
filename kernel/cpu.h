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
