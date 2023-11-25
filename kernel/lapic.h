#include <stdint.h>

void lapic_write(uintptr_t offset, uint32_t value);

uint32_t lapic_read(uintptr_t offset);

void lapic_send_ipi(uint32_t id, uint8_t vector);

void lapic_init();
