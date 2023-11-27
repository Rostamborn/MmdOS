#include "cpu.h"
#include "lapic.h"
#include "timer.h"
#include <stdint.h>

#define LAPIC_BASE_ADDR 0xfee00000 // APIC base address

// enabling x2APIC by setting the bit 10 of IA32_APIC_BASE_MSR
// #define IA32_APIC_BASE_MSR 0x1B | (1 << 10) // APIC base MSR
#define IA32_APIC_BASE_MSR 0x1B
// above address might be invalid

// Local APIC registers(offsets)
#define LAPIC_REG_ID 0x20 // LAPIC ID
#define LAPIC_REG_EOI 0xb0 // End of interrupt
#define LAPIC_REG_SPURIOUS 0xf0
#define LAPIC_REG_CMCI 0x2f0 // LVT Corrected machine check interrupt
#define LAPIC_REG_ICR0 0x300 // Interrupt command register
#define LAPIC_REG_ICR1 0x310 // /ICR1 should be written first and then ICR0
#define LAPIC_REG_LVT_TIMER 0x320
#define LAPIC_REG_TIMER_INITCNT 0x380 // Initial count register
#define LAPIC_REG_TIMER_CURCNT 0x390 // Current count register(readonly)
#define LAPIC_REG_TIMER_DIV 0x3e0
#define LAPIC_EOI_ACK 0x00

// I/O APIC registers(offsets)
#define IOAPIC_BASE 0xfec00000
#define IOREGSEL 0x00 // I/O register select: We put the offset to select the desired register(ID, VER, etc.)
#define IOWIN 0x10 // I/O APIC window: after IOREGSEL is set, we read/write the value from this register
                   // read or write heppens when IOWIN is accessed.
#define IOAPIC_REG_ID 0x00 // ID
#define IOAPIC_REG_VER 0x01 // version
#define IOAPIC_REG_ARB 0x02 // arbitration ID
#define IOAPIC_REG_TABLE 0x10 // redirection table

void lapic_write(uintptr_t offset, uint32_t value) {
    *(uintptr_t*)(LAPIC_BASE_ADDR + offset) = value;
}

uint32_t lapic_read(uintptr_t offset) {
    return *(uintptr_t*)(LAPIC_BASE_ADDR + offset);
}

void ioapic_write(uintptr_t offset, uint32_t value) {
    *(uintptr_t*)(IOAPIC_BASE + IOREGSEL) = offset;
    *(uintptr_t*)(IOAPIC_BASE + IOWIN) = value;
}

uint32_t ioapic_read(uintptr_t offset) {
    *(uintptr_t*)(IOAPIC_BASE + IOREGSEL) = offset;
    return *(uintptr_t*)(IOAPIC_BASE + IOWIN);
}

void lapic_eoi() {
    lapic_write(LAPIC_REG_EOI, LAPIC_EOI_ACK);
    // be sure to send EOI before iret
}

// CUATION: hasn't been tested yet
void lapic_send_ipi(uint32_t id, uint8_t vector) { // IPI: we can send an interrupt to the target
                                                   // core(id) with the given offset(index) 
                                                   // in the interrupt vector(vector)
    lapic_write(LAPIC_REG_ICR1, id << 24);
    lapic_write(LAPIC_REG_ICR0, vector); // by writing to the lower half, we send the interrupt
}

// TIMER (IGNORE THIS, I/O APIC needs to be implemented first)
void lapic_timer_stop() {
    lapic_write(LAPIC_REG_TIMER_INITCNT, 0);
    // lapic_write(LAPIC_REG_LVT_TIMER, 1 << 16);
}

// void lapic_one_shot_timer(uint32_t ticks) {
//     lapic_write(LAPIC_REG_TIMER_INITCNT, ticks);
//     lapic_write(LAPIC_REG_LVT_TIMER, 32 & (0 << 17)); // 
// }
//
void install_redirection_table(uint8_t index, uint8_t vector) {
    uint32_t low = vector;
    uint32_t high = 0x10000; // enable the interrupt
    ioapic_write(IOAPIC_REG_TABLE + index * 2, low);
    ioapic_write(IOAPIC_REG_TABLE + index * 2 + 1, high);
}

void lapic_timer_periodic(uint32_t ticks) {
    lapic_timer_stop();
    // not sure about 16(might be 17)
    lapic_write(LAPIC_REG_LVT_TIMER, 32 | (1 << 16)); // setting periodic mode
    lapic_write(LAPIC_REG_TIMER_DIV, 0x0); // divide by 2
    lapic_write(LAPIC_REG_TIMER_INITCNT, ticks);
}

// NOTE: IDK what the return value is...
uint32_t lapic_reg_id() {
    return lapic_read(LAPIC_REG_ID);
}

void ioapic_init() {
    install_redirection_table(0, 32);
    pit_periodic_init(1);

}

void lapic_init() {
    // enabling apic
    lapic_write(LAPIC_REG_SPURIOUS, 0x1ff | (1 << 8)); // enable spurious interrupt vector
    ioapic_init();
}
