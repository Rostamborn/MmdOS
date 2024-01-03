#include "cpu/cpu.h"
#include "demo.h"
#include "mm/vmm.h"
#include "src/kernel/cpu/pic.h"
#include "src/kernel/gdt.h"
#include "src/kernel/interrupts/idt.h"
#include "src/kernel/interrupts/keyboard.h"
#include "src/kernel/interrupts/timer.h"
#include "src/kernel/lib/logger.h"
#include "src/kernel/lib/print.h"
#include "src/kernel/mm/pmm.h"
#include "src/kernel/mm/vmm.h"
#include "src/kernel/scheduler/scheduler.h"
#include "src/kernel/terminal/limine_term.h"
#include "src/kernel/terminal/prompt.h"
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
    thread_add(p, "second thread of adder1", &add_one_to_z, NULL);
    // ---------------------

    // hcf(); // halt, catch fire
    for (;;)
        ;
}
