#include "cpu/cpu.h"
#include "cpu/pic.h"
#include "demo.h"
#include "gdt.h"
#include "interrupts/idt.h"
#include "interrupts/keyboard.h"
#include "interrupts/timer.h"
#include "lib/logger.h"
#include "lib/print.h"
#include "mm/kheap.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "scheduler/scheduler.h"
#include "terminal/limine_term.h"
#include "terminal/prompt.h"
// #include "limine.h"
#include <stdbool.h>
#include <stdint.h>
// #include <stdint.h>

// NOTE(Arman): *We can't use stdlib at all. We have to write our own functions*

void _start(void) {

    gdt_init();
    serial_init();
    idt_init();
    prompt_init();
    keyboard_init();
    pmm_init();
    vmm_init();

    // vmm_t* new_vmm = vmm_new();
    // // TODO: crashes because of the absence of the lower half mappings
    // vmm_switch_pml(new_vmm);
    // vmm_switch_pml(vmm_kernel);

    // this crashes because I think the stack is no longer valid because of pml
    // switch and we should do something about it. it also seems that the stack
    // resides somewhere in the lower half becuase copying the higher half of
    // vmm_kernel is not enough and the program crashes and when I copy the
    // lower half too, it works.

    uint64_t* ptr1 = k_alloc(sizeof(uint64_t));
    *ptr1 = 8765;
    kprintf("ptr1 addr: %p value: %d\n", ptr1, *ptr1);
    k_free(ptr1);

    // kprintf("cr3 content %x\n", read_cr3());
    // slab_init();
    scheduler_init();

    // for demonstration ---
    process_t* p = process_create("adder1", &add_one_to_x, NULL);
    process_t* p2 = process_create("adder2", &add_one_to_y, NULL);
    thread_t* t = thread_add(p, "second thread of adder1", &add_one_to_z, NULL);
    // set thread to sleep 10 seconds
    t->status = SLEEPING;
    t->wake_time = timer_get_uptime() + (5 * 1000);
    // set process to sleep 10 seconds
    p2->status = SLEEPING;
    p2->wake_time = timer_get_uptime() + (5 * 1000);
    // ---------------------

    timer_init();
    // hcf(); // halt, catch fire
    for (;;)
        ;
}
