#include "cpu/cpu.h"
#include "cpu/pic.h"
#include "demo.h"
#include "gdt.h"
#include "interrupts/idt.h"
#include "interrupts/keyboard.h"
#include "interrupts/timer.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
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
    scheduler_init();
    timer_init();
    // for demonstration ---
    process_t* p = process_create("adder1", &add_one_to_x, NULL);
    process_create("adder2", &add_one_to_y, NULL);
    thread_t* t = thread_add(p, "second thread of adder1", &add_one_to_z, NULL);
    // set thread to sleep 10 seconds
    t->status = SLEEPING;
    t->wake_time = timer_get_uptime() + (10 * 1000);
    // ---------------------

    // hcf(); // halt, catch fire
    for (;;)
        ;
}
