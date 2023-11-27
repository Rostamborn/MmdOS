#include <stdint.h>
#include "cpu.h"

void pit_periodic_init(uint16_t c) {
    uint32_t divisor = 1193180 / c;
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xff));
    outb(0x40, (uint8_t)(divisor >> 8 & 0xff));
    // outb(0x40, c & 0xff); // lower byte
}
// NOTE: We need to set up I/O APIC to receive interrupts from the PIT
// as it is a system wide timer and not a processor specific timer.
