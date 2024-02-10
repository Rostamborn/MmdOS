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
    // read_file_x();
    //   read_file_y();

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

    // process_t* p = process_create("user_program", &user_program, NULL);
    // p->vmm = vmm_kernel;
    // //
    // //
    // // Note: use vmm_switch_pml(p->vmm) to switch mapping directory instantly
    // //
    // void* user_stack = pmm_alloc(16) + 16*PAGE_SIZE;
    // vmm_map_page(vmm_kernel, (uintptr_t)user_stack, (uintptr_t)user_stack,
    // PTE_PRESENT | PTE_WRITABLE | PTE_USER); vmm_map_page(p->vmm,
    // (uintptr_t)&user_program, (uintptr_t)&user_program, PTE_PRESENT |
    // PTE_WRITABLE | PTE_USER); p->threads->ustack = user_stack;
    // tss_set_rsp0((uint64_t)p->threads->kstack);
    // p->threads->context->iret_rip = (uint64_t)&user_program;
    // p->threads->context->iret_rsp = (uint64_t)user_stack;
    // klog("startup ::", "allocated everything");

    // jmp_user(/* the addr that will be put in rip*/ , /* the user_stack addr
    // */);
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
    // // thread_t* t = thread_add(p, "second thread of adder1", &add_one_to_z,
    // // NULL); set thread to sleep 10 seconds t->status = SLEEPING;
    // t->wake_time
    // // = timer_get_uptime() + (5 * 1000); set process to sleep 10 seconds
    // p2->status = SLEEPING;
    // p2->wake_time = timer_get_uptime() + (5 * 1000);
    // // ---------------------
    //
    uint64_t* ptr2 = kalloc(9000);
    *ptr2 = 8765;
    // kprintf("ptr2 addr: %p value: %d\n", ptr2, *ptr2);
    kfree(ptr2);
    // vfs_execute("/a.out");

    // important: timer should be after all these code so that
    // it won't interrupt anything by mistake
    timer_init();

    for (;;)
        ;
}
