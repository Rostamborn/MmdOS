#include "cpu/cpu.h"
#include "cpu/pic.h"
#include "demo.h"
#include "gdt.h"
#include "interrupts/idt.h"
#include "interrupts/keyboard.h"
#include "interrupts/timer.h"
#include "mm/kheap.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "scheduler/scheduler.h"
#include "lib/print.h"
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
    // ---------------------------------------------------------
    // uint64_t* ptr = pmm_alloc(1);
    // kprintf("allocated a page at: %p\n", ptr);
    // *ptr = 234;
    // kprintf("value at page: %d\n", *ptr);
    //
    // uint8_t cond = pmm_check_alloc(ptr);
    // kprintf("check alloc ptr: %d\n", cond);
    //
    // uint64_t* ptr2 = pmm_alloc(2);
    // kprintf("allocated a page at: %p\n", ptr2);
    // *ptr2 = 567;
    // kprintf("value at page: %d\n", *ptr2);
    // pmm_free(ptr2, 2);
    // cond = pmm_check_alloc(ptr2);
    // kprintf("check alloc ptr2: %d\n", cond);
    //
    // kprintf("freed 2 pages at: %p\n", ptr2);
    // uint64_t* ptr3 = pmm_alloc(1);
    // kprintf("allocated a page at: %p\n", ptr3);
    // *ptr3 = 8910;
    // kprintf("value at page: %d\n", *ptr3);
    //
    // kprintf("%x\n", 0x0);
    // --------------------------------------------------------
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


    timer_init();
    // hcf(); // halt, catch fire
    for (;;)
        ;
}
