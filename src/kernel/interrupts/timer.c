#include "src/kernel/interrupts/timer.h"
#include "src/kernel/interrupts/idt.h"
#include "src/kernel/scheduler/scheduler.h"
#include <stdint.h>
// I/O port     Usage
// *** Channel 0 is directly connected to IRQ0 ***
#define CH0_DATA 0x40 // Channel 0 data port (read/write)
#define CH1_DATA 0x41 // Channel 1 data port (read/write)
#define CH2_DATA 0x42 // Channel 2 data port (read/write)
#define MODE_CMD 0x43 // Mode/Command register (write only, a read is ignored)

execution_context* timer_handler(execution_context* frame) {
    frame = schedule(frame);
    return frame;
}

void timer_init() {
    irq_install_handler(0, &timer_handler);

    uint64_t freq = 100; // 100 Hz
    uint64_t divisor = 1193180 / freq;

    // There are a lot of modes for PIT, but we will use mode 3
    // for further reading refer to:
    // https://wiki.osdev.org/Programmable_Interval_Timer
    outb(MODE_CMD, 0x36);                            // 0x36 = 0011 0110
    outb(CH0_DATA, (uint8_t) divisor & 0xff);        // low byte
    outb(CH0_DATA, (uint8_t) (divisor >> 8) & 0xff); // high byte
}
