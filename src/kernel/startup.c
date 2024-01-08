#include "cpu/cpu.h"
#include "cpu/pic.h"
#include "demo.h"
#include "gdt.h"
#include "interrupts/idt.h"
#include "interrupts/keyboard.h"
#include "interrupts/timer.h"
#include "lib/print.h"
#include "mm/kheap.h"
#include "mm/pmm.h"
#include "mm/slab.h"
#include "mm/vmm.h"
#include "lib/alloc.h"
#include "terminal/limine_term.h"
#include "terminal/prompt.h"
#include <stdbool.h>
#include <stdint.h>

// NOTE(Arman): *We can't use stdlib at all. We have to write our own functions*

void _start(void) {
    gdt_init();
    serial_init();
    idt_init();
    prompt_init();
    keyboard_init();
    pmm_init();
    slab_init();
    vmm_init();
    // scheduler_init();
    timer_init();
    // for demonstration ---
    // process_t* p = process_create("adder1", &add_one_to_x, NULL);
    // process_create("adder2", &add_one_to_y, NULL);
    // thread_add(p, "second thread of adder1", &add_one_to_z, NULL);
    // ---------------------
    //
    uint8_t* ptr = (uint8_t*) kalloc(sizeof(uint8_t) * 5000);
    ptr[0] = 0x12;
    kprintf("addr %p = %x\n", ptr, ptr[0]);
    kprintf("number of arenas: %d\n", vmm_kernel->arena_count);
    uint8_t* ptr1 = (uint8_t*) kalloc(sizeof(uint8_t) * 6000);
    ptr1[0] = 0x12;
    kprintf("addr %p = %x\n", ptr1, ptr1[0]);
    kprintf("number of arenas: %d\n", vmm_kernel->arena_count);

    // hcf(); // halt, catch fire
    for (;;)
        ;
}
