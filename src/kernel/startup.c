#include "cpu/cpu.h"
#include "cpu/pic.h"
#include "demo/demo.h"
#include "fs/vfs.h"
#include "gdt.h"
#include "interrupts/idt.h"
#include "interrupts/keyboard.h"
#include "interrupts/timer.h"
#include "lib/alloc.h"
#include "lib/print.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "scheduler/process.h"
#include "scheduler/scheduler.h"
#include "terminal/limine_term.h"
#include "terminal/prompt.h"
#include "userland/sys.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>
#include <threads.h>
#include "../programs/ls.h"

// NOTE(Arman): *We can't use stdlib at all. We have to write our own
// functions*
char user_program[2] = {0xeb, 0xfe};


void _start(void) {
    gdt_init();
    serial_init();
    idt_init();
    pmm_init();
    vmm_init();
    vfs_init();
    syscall_init();

    // process init
    scheduler_init();
    prompt_init();
    keyboard_init();

    uint64_t* ptr1 = kalloc(9000);
    *ptr1 = 8765;
    kfree(ptr1);

    // for demonstration ---
    process_t* p = process_create("adder1", &add_one_to_x, NULL);
    process_t* p2 = process_create("adder2", &add_one_to_y, NULL);
    uint64_t* ptr2 = kalloc(9000);
    *ptr2 = 8765;
    kprintf("ptr2 addr: %p value: %d\n", ptr2, *ptr2);
    kfree(ptr2);

    // important: timer should be after all these code so that
    // it won't interrupt anything by mistake
    timer_init();

    for (;;)
        ;
}
