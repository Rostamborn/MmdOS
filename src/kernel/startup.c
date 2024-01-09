#include "cpu/cpu.h"
#include "cpu/pic.h"
#include "demo.h"
#include "gdt.h"
#include "interrupts/idt.h"
#include "interrupts/keyboard.h"
#include "interrupts/timer.h"
#include "lib/alloc.h"
#include "lib/print.h"
#include "mm/kheap.h"
#include "mm/pmm.h"
#include "mm/slab.h"
#include "mm/vmm.h"
#include "scheduler/process.h"
#include "scheduler/scheduler.h"
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
    // slab_init();
    vmm_init();
    // scheduler_init();
    timer_init();

    // for demonstration ---
    // process_t* p = process_create("adder1", &add_one_to_x, NULL);
    // process_t* p2 = process_create("adder2", &add_one_to_y, NULL);
    // thread_t* t = thread_add(p, "second thread of adder1", &add_one_to_z,
    // NULL);
    // // set thread to sleep 10 seconds
    // t->status = SLEEPING;
    // t->wake_time = timer_get_uptime() + (10 * 1000);
    // // set process to sleep 10 seconds
    // p2->status = SLEEPING;
    // p2->wake_time = timer_get_uptime() + (10 * 1000);
    // ---------------------
    execution_context* context = (execution_context*)kalloc(sizeof(execution_context));
    context->iret_cs = 0x20;
    context->iret_ss = 0x30;
    context->iret_rip = 0x40;
    thread_t*          thread = (thread_t*)kalloc(sizeof(thread_t));
    kprintf("context addr: %p\n", context);
    kprintf("cs: %x, ss: %x, rip: %x\n", context->iret_cs, context->iret_ss, context->iret_rip);
    thread->context = context;
    kprintf("thread addr: %p\n", thread);
    kprintf("cs: %x, ss: %x, rip: %x\n", thread->context->iret_cs, thread->context->iret_ss, thread->context->iret_rip);

    execution_context* context2 = (execution_context*) kalloc(sizeof(execution_context));
    kprintf("context addr: %p\n", context2);
    context2->iret_cs = 0x28;
    kprintf("context->iret_cs: %x\n", context2->iret_cs);
    kprintf("context addr: %p\n", context2);
    uint8_t* ptr = (uint8_t*) kalloc(sizeof(uint8_t) * 5000);
    ptr[0] = 12;
    kprintf("addr %p = %d\n", ptr, ptr[0]);
    kprintf("number of arenas: %d\n", vmm_kernel->arena_count);
    kfree(ptr);
    kprintf("going to allocate 6000 bytes\n");
    uint8_t* ptr1 = (uint8_t*) kalloc(sizeof(uint8_t) * 6000);
    ptr1[0] = 18;
    kprintf("addr %p = %d\n", ptr1, ptr1[0]);
    kprintf("number of arenas: %d\n", vmm_kernel->arena_count);
    kfree(ptr1);

    // hcf(); // halt, catch fire
    for (;;)
        ;
}
