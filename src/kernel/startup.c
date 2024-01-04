#include "cpu/cpu.h"
#include "demo.h"
#include "mm/vmm.h"
#include "cpu/pic.h"
#include "gdt.h"
#include "interrupts/idt.h"
#include "interrupts/keyboard.h"
#include "interrupts/timer.h"
// #include "lib/logger.h"
// #include "lib/print.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "mm/slab.h"
#include "lib/alloc.h"
#include "lib/print.h"
#include "scheduler/scheduler.h"
#include "terminal/limine_term.h"
#include "terminal/prompt.h"
#include <stdbool.h>

// NOTE(Arman): *We can't use stdlib at all. We have to write our own functions*

void _start(void) {
    gdt_init();
    serial_init();
    idt_init();
    prompt_init();
    keyboard_init();
    pmm_init();
    vmm_init();
    slab_init();
    // scheduler_init();
    timer_init();
    // for demonstration ---
    // process_t* p = process_create("adder1", &add_one_to_x, NULL);
    // process_create("adder2", &add_one_to_y, NULL);
    // thread_add(p, "second thread of adder1", &add_one_to_z, NULL);
    // ---------------------
    void* ptr = kalloc(sizeof(interrupt_frame));
    void* ptr2 = kalloc(1);
    kprintf("ptr1: %p\n", ptr);
    kprintf("ptr2: %p\n", ptr2);
    kfree(ptr);
    kfree(ptr2);

    // hcf(); // halt, catch fire
    for (;;)
        ;
}
