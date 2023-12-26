#include "cpu/cpu.h"
#include "mm/vmm.h"
#include "src/kernel/cpu/pic.h"
#include "src/kernel/gdt.h"
#include "src/kernel/interrupts/idt.h"
#include "src/kernel/interrupts/keyboard.h"
#include "src/kernel/interrupts/timer.h"
#include "src/kernel/lib/logger.h"
#include "src/kernel/lib/print.h"
#include "src/kernel/mm/pmm.h"
#include "src/kernel/scheduler/scheduler.h"
#include "src/kernel/terminal/limine_term.h"
#include "src/kernel/terminal/prompt.h"
#include "src/kernel/mm/vmm.h"
#include <stdbool.h>

// NOTE(Arman): *We can't use stdlib at all. We have to write our own functions*

void _start(void) {
    gdt_init();
    serial_init();
    idt_init();
    timer_init();
    prompt_init();
    keyboard_init();
    pmm_init();
    vmm_init();


    // void* ptr = pmm_alloc(1000);
    // klog(0, "ptr: %p", ptr);
    // void* ptr2 = pmm_alloc(1000);
    // klog(0, "ptr2: %p", ptr2);

    scheduler_init();
    // hcf(); // halt, catch fire
    for (;;)
        ;
}
