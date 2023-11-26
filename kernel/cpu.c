#include <stdint.h>
#include "cpu.h"

// LIMINE_BASE_REVISION(1)
#define PORT 0x3f8
#define LOG_PREFIX "LOG :: "

#define PIC_COMMAND_MASTER 0x20
#define PIC_DATA_MASTER 0x21
#define PIC_COMMAND_SLAVE 0xa0
#define PIC_DATA_SLAVE 0xa1

#define ICW_1 0x11
#define ICW_2_MASTER 0x20
#define ICW_2_SLAVE 0x28
#define ICW_3_MASTER 0x4 // 0x04 ??
#define ICW_3_SLAVE 0x2 // 0x02 ??
#define ICW_4 0x1 // not sure


// halt, catch fire
void hcf(void) {
    asm ("cli");
    for (;;) {
        asm ("hlt");
    }
}

inline void enable_interrupts() {
    asm volatile("sti");
}

inline void disable_interrupts() {
    asm volatile("cli");
}

// to communicate with the serial port
extern inline unsigned char inb(int portnum) {
    unsigned char data = 0;
    __asm__ __volatile__("inb %%dx, %%al" : "=a" (data) : "d" (portnum));
    // "=a" is the output operand and "d" is the input operand
    return data;
}

extern inline unsigned char outb(int portnum, unsigned char value) {
    __asm__ __volatile__("outb %%al, %%dx" : : "a" (value), "d" (portnum));
    return value;
}

// https://wiki.osdev.org/Serial_Ports
int init_serial() {
   outb(PORT + 1, 0x00);    // Disable all interrupts
   outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(PORT + 1, 0x00);    //                  (hi byte)
   outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
   outb(PORT + 4, 0x1E);    // Set in loopback mode, test the serial chip
   outb(PORT + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)
 
   // Check if serial is faulty (i.e: not same byte as sent)
   if(inb(PORT + 0) != 0xAE) {
      return 1;
   }
 
   // If serial is not faulty set it in normal operation mode
   // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
   outb(PORT + 4, 0x0F);
   return 0;
}

// PORT + 5 is the transmit buffer
void log_to_serial(char *string) {
    char *log = LOG_PREFIX;
    while (*log != '\0'){
        while ((inb(PORT + 5) & 0x20) == 0); // *check if transmit buffer is empty*
        outb(PORT, *log); // write char to serial port
        log++;
    }

    while (*string != '\0') {
        while ((inb(PORT + 5) & 0x20) == 0); // *check if transmit buffer is empty*
        outb(PORT, *string); // write char to serial port
        string++;
    }
}

// NOTE(Arman): I made this in a hurry, so it's not the best
void print_digit(uint8_t digit) {
    uint8_t dig = digit;
    uint8_t len = 0;
    while(dig > 0) {
        dig /= 10;
        len++;
    }
    dig = digit;
    char str[len + 2];
    uint8_t remainder = 0;
    for(uint8_t i = 0; i < len; i++) {
        remainder = dig % 10;
        dig /= 10;
        str[i] = remainder + '0';
    }
    str[len] = '\n';
    str[len+1] = '\0'; // null termination

    // reverse string
    for(uint8_t i = 0; i < len; i++) {
        char tmp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = tmp;
    }

    log_to_serial(str);
}

void disable_pic() {
    // ICW: https://wiki.osdev.org/8259_PIC#Initialisation
    outb(PIC_COMMAND_MASTER, ICW_1);
    outb(PIC_COMMAND_SLAVE, ICW_1);
    outb(PIC_DATA_MASTER, ICW_2_MASTER);
    outb(PIC_DATA_SLAVE, ICW_2_SLAVE);
    outb(PIC_DATA_MASTER, ICW_3_MASTER);
    outb(PIC_DATA_SLAVE, ICW_3_SLAVE);
    outb(PIC_DATA_MASTER, ICW_4);
    outb(PIC_DATA_SLAVE, ICW_4);
    outb(PIC_DATA_MASTER, 0xff);
    outb(PIC_DATA_SLAVE, 0xff);
}

uint64_t rdmsr(uint32_t msr) { // read model specific register insctruction
    uint32_t low = 0, high = 0; // low: eax, high: edx, input: ecx
    asm volatile("rdmsr" : "=a" (low), "=d" (high) : "c" (msr) : "memory");
    return ((uint64_t)high << 32) | low; // return the 64-bit result
}

void memset_k(void *ptr, uint8_t value, uint64_t size) {
    uint8_t *tmp = (uint8_t*)ptr;
    for (; size > 0; size--) {
        *tmp = value;
        tmp++;
    }
}
