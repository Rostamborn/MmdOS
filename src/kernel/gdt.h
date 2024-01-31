#ifndef GDT_H
#define GDT_H

#include "cpu/cpu.h"
#include <stdint.h>

struct tss_t {
    uint32_t resereved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t resereved1;
    uint64_t resereved2;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t resereved3;
    uint16_t resereved4;
    uint16_t iomap_offset;
} __attribute__((packed));

void gdt_init();

void tss_init();

void gdt_load();

void tss_set_rsp0(uint64_t rsp0);

#endif
