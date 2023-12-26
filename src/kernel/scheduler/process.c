#include "src/kernel/scheduler/process.h"
#include "src/kernel/lib/alloc.h"
#include "src/kernel/lib/logger.h"
#include "src/kernel/lib/print.h"
#include "src/kernel/lib/util.h"

void idle_process() {
    kprintf("from idle process");
    while (true) {
        asm("hlt");
    }
}

// allocate 32kB to process Stack.
uint64_t* alloc_stack() {
    uint64_t* stack = kalloc(32 * 1024);
    return stack;
}

// created pcb and allocates memory to process.
// does not add it to queue yet.
process_t* create_process(char* name, void* function, void* arg) {
    process_t* process = kalloc(sizeof(process_t));

    kstrcpy(process->name, name, PROCESS_NAME_MAX_LEN);
    process->pid = next_pid++;
    process->status = SPAWNED;
    process->context.iret_ss = 0x30;
    process->context.iret_rsp = alloc_stack();
    process->context.iret_flags =
        0x202; // resets all bits but 2 and 9.
               // 2 for legacy reasons and 9 for interrupts.
    process->context.iret_cs = 0x28;
    process->context.iret_rip = (uint64_t) function;
    process->context.rdi = (uint64_t) arg;
    process->context.rbp = 0;

    return process;
}
