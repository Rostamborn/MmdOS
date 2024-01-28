#include "timer.h"
#include "../scheduler/scheduler.h"
#include "../scheduler/process.h"
#include "idt.h"
#include "../mm/vmm.h"

// I/O port     Usage
// *** Channel 0 is directly connected to IRQ0 ***
#define CH0_DATA 0x40 // Channel 0 data port (read/write)
#define CH1_DATA 0x41 // Channel 1 data port (read/write)
#define CH2_DATA 0x42 // Channel 2 data port (read/write)
#define MODE_CMD 0x43 // Mode/Command register (write only, a read is ignored)

// the amount of time the system has been running in milli-seconds.
// TODO curent algorithm has severe problems, find a better way!
uint64_t uptime = 0;

// amount of time between interrupts in milli-seconds
uint64_t quantum = 10;

execution_context* timer_handler(execution_context* frame) {
    uptime += quantum;
    frame = schedule(frame);
    // vmm_switch_pml(process_get_current_vmm());

    return frame;
}

uint64_t timer_get_uptime() { return uptime; }

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
