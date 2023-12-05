#include "cpu.h"
#include <stdint.h>

// IRQ default constants
#define IRQ_BASE            0x20
#define IRQ0_TIMER          0x00
#define IRQ1_KEYBOARD       0x01
#define IRQ2_CASCADE        0x02
#define IRQ3_SERIAL_PORT2   0x03
#define IRQ4_SERIAL_PORT1   0x04
#define IRQ5_RESERVED       0x05
#define IRQ6_DISKETTE_DRIVE 0x06
#define IRQ7_PARALLEL_PORT  0x07
#define IRQ8_CMOS_CLOCK     0x08
#define IRQ9_CGA            0x09
#define IRQ10_RESERVED      0x0A
#define IRQ11_RESERVED      0x0B
#define IRQ12_AUXILIARY     0x0C
#define IRQ13_FPU           0x0D
#define IRQ14_HARD_DISK     0x0E
#define IRQ15_RESERVED      0x0F

#define PIC1            0x20  /* IO base address for master PIC */
#define PIC2            0xA0  /* IO base address for slave PIC */
#define PIC1_COMMAND    PIC1
#define PIC1_DATA       (PIC1+1)    /* master data */
#define PIC2_COMMAND    PIC2
#define PIC2_DATA       (PIC2+1)    /* slave data */

#define PIC_EOI         0x20   /* end of interrupt */

#define ICW1            0x11    /* interrupt control command word PIC for initialization */
#define ICW4       0x01    /* 8086/88 (MCS-80/85) mode */


void pic_init() {
    // uint8_t a1, a2;

    // save mask registers
    // a1 = inb(PIC1_DATA);
    // a2 = inb(PIC2_DATA);

    // send commands to pic to initialize both master & slave
    outb(PIC1_COMMAND, ICW1);
    outb(PIC2_COMMAND, ICW1);

    // map vector offset of all default IRQ's from 0x20 to 0x27 in master(ICW2)
    outb(PIC1_DATA, 0x20);
    // map vector offset of all default IRQ's from 0x28 to 0x2F in slave(ICW2)
    outb(PIC2_DATA, 0x28);

    // ICW3: tell master PIC that there is a slave PIC at IRQ2 (0000 0100)
    outb(PIC1_DATA, 4);
    // ICW3: tell slave PIC its cascade identity (0000 0010)
    outb(PIC2_DATA, 2);

    // ICW4, set x86 mode
    outb(PIC1_DATA, ICW4);
    outb(PIC2_DATA, ICW4);

    // restore the mask registers
    outb(PIC1_DATA, 0x0);
    outb(PIC2_DATA, 0x0);
    // outb(PIC1_DATA, a1);
    // outb(PIC2_DATA, a2);
}

void pic_eoi(uint8_t offset) {
    if (offset >= 40) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

