#include "cpu/cpu.h"
#include "cpu/pic.h"
#include "demo/demo.h"
#include "fs/vfs.h"
#include "gdt.h"
#include "interrupts/idt.h"
#include "interrupts/keyboard.h"
#include "interrupts/timer.h"
#include "lib/alloc.h"
#include "lib/logger.h"
#include "lib/print.h"
#include "mm/kheap.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "scheduler/process.h"
#include "scheduler/scheduler.h"
#include "terminal/limine_term.h"
#include "terminal/prompt.h"
#include "userland/sys.h"
#include "userland/user.h"
#include <stdbool.h>
#include <stdint.h>
// #include "limine.h"
#include <stdbool.h>
#include <stdint.h>
#include <threads.h>
// #include <stdint.h>
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

    // TODO: crashes because of the absence of the lower half mappings
    //  which includes the framebuffer. we should fix this.
    //  mapping in vmm_init didn't fix
    // vmm_t* new_vmm = vmm_new();
    // vmm_switch_pml(new_vmm);
    // vmm_switch_pml(vmm_kernel);

    uint64_t* ptr1 = kalloc(9000);
    *ptr1 = 8765;
    // kprintf("ptr1 addr: %p value: %d\n", ptr1, *ptr1);
    kfree(ptr1);

    // // NOTE: you should also map the importted program to user space. just
    // like the above mapping.
    //  meaning give the virtual addresses in the lower half
    //
    //
    //
    // // when to call jmp_user(&user_program, user_stack)
    // jmp_user(&user_program, user_stack);

    // for demonstration ---
    //  game of life process
    process_t* p = process_create("adder1", &add_one_to_x, NULL);
    process_t* p2 = process_create("adder2", &add_one_to_y, NULL);
    uint64_t* ptr2 = kalloc(9000);
    *ptr2 = 8765;
    kprintf("ptr2 addr: %p value: %d\n", ptr2, *ptr2);
    kfree(ptr2);
    // vfs_execute("/a.out");


    // important: timer should be after all these code so that
    // it won't interrupt anything by mistake
    timer_init();

    for (;;)
        ;
}
