#include "cpu/cpu.h"
#include "cpu/pic.h"
#include "demo.h"
#include "gdt.h"
#include "interrupts/idt.h"
#include "interrupts/keyboard.h"
#include "interrupts/timer.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "mm/slab.h"
#include "lib/print.h"
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
    PageMap* pagemap = vmm_new_pagemap();
    execution_context* ptr = (execution_context*)vmm_alloc(pagemap, sizeof(execution_context), PTE_PRESENT | PTE_WRITABLE, NULL);
    if (ptr == NULL) {
        kprintf("ptr is null\n");
    }
    kprintf("ptr: %p\n", ptr);
    ptr->int_number = 22; 
    kprintf("ptr number: %d\n", ptr->int_number);
    vmm_free(pagemap, ptr);

    int* ptr2 = (int*)vmm_alloc(pagemap, sizeof(int), PTE_PRESENT | PTE_WRITABLE, NULL);
    kprintf("ptr2: %p\n", ptr2);
    *ptr2 = 1234321;
    kprintf("ptr2 number: %d\n", *ptr2);
    vmm_free(pagemap, ptr2);

    // hcf(); // halt, catch fire
    for (;;)
        ;
}
