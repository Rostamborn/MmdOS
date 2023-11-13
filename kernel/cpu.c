#include <stdint.h>
#include "cpu.h"

// LIMINE_BASE_REVISION(1)
#define PORT 0x3f8
#define LOG_PREFIX "LOG :: "

// halt, catch fire
void hcf(void) {
    asm ("cli");
    for (;;) {
        asm ("hlt");
    }
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
