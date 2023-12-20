#include "src/kernel/cpu/pic.h"
#include "src/kernel/gdt.h"
#include "src/kernel/interrupts/idt.h"
#include "src/kernel/interrupts/keyboard.h"
#include "src/kernel/interrupts/timer.h"
#include "src/kernel/lib/logger.h"
#include "src/kernel/lib/print.h"
#include "src/kernel/mm/pmm.h"
#include "src/kernel/terminal/limine_term.h"
#include "src/kernel/terminal/prompt.h"
#include <stdbool.h>
// NOTE(Arman): *We can't use stdlib at all. We have to write our own functions*

void _start(void) {
    gdt_init();
    idt_init();
    timer_init();
    prompt_init();
    keyboard_init();
    pmm_init();

    init_serial();

    void* ptr = pmm_alloc(1);
    klog(0, "ptr: %p", ptr);
    void* ptr2 = pmm_alloc(1);
    klog(0, "ptr2: %p", ptr2);

    // uint8_t a = 1 / 0; // I can not belive the interrupt system works

    // hcf(); // halt, catch fire
    for (;;)
        ;
}
