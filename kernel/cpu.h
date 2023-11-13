#include <stdint.h>

#define PORT 0x3f8

extern inline unsigned char inb(int portnum);

extern inline unsigned char outb(int portnum, unsigned char value);

void hcf(void);

int init_serial();

void log_to_serial(char *string);

void print_digit(uint8_t digit);
