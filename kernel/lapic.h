#include <stdint.h>

void lapic_write(uintptr_t offset, uint32_t value);

uint32_t lapic_read(uintptr_t offset);

void lapic_init();
