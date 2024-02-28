#include "process.h"
#include "../lib/alloc.h"
#include "../lib/logger.h"
#include "../lib/panic.h"
#include "../lib/print.h"
#include "../lib/spinlock.h"
#include "../lib/util.h"
#include "../scheduler/process.h"
#include "../userland/sys.h"
#include "stdbool.h"
#include <stdint.h>

#define STACK_SIZE (64 * 1024)

process_t* processes_list;
process_t* current_process;
size_t     next_pid = 1;

void* alloc_stack(uint8_t mode) {
    return kalloc(STACK_SIZE) + STACK_SIZE;
    // if (mode == KERNEL_MODE) {
    //     return kalloc(STACK_SIZE) + STACK_SIZE;
    // } else if (mode == USER_MODE) {
    //     return malloc(STACK_SIZE) + STACK_SIZE;
    // } else {
    //     panic("alloc_stack: invalid mode");
    //     return NULL;
    // }
}

// set status to DEAD
// be careful not to release stack and context here
// as they are still in use!
void process_exit() {
    process_t* current_process = process_get_current();
    current_process->status = DEAD;
    scheduler_yield();
}

void process_execution_wrapper(void (*function)(void*), void* arg) {
    function(arg);
    process_exit();
}

void process_idle() {
    klog("SCHEDULER::", "from idle process");
    uint64_t c = 0;
    while (true) {
        asm("hlt");
    }
}

// created pcb and allocates memory to process.
// also add it to queue.
process_t* process_create(char* restrict name, void* restrict function(void*),
                          void* restrict arg) {
    // disable_interrupts();
    process_t*         process = (process_t*) kalloc(sizeof(process_t));
    execution_context* context =
        (execution_context*) kalloc(sizeof(execution_context));

    process->context = context;

    process->kstack = alloc_stack(KERNEL_MODE);
    // thread->ustack = alloc_stack(USER_MODE);
    process->ustack = NULL;
    process->context->iret_ss = 0x30;
    process->context->iret_rsp = (uint64_t) process->kstack;
    process->context->iret_flags =
        0x202; // resets all bits but 2 and 9.
               // 2 for legacy reasons and 9 for interrupts.
    process->context->iret_cs = 0x28;
    process->context->iret_rip = (uint64_t) process_execution_wrapper;
    process->context->rdi = (uint64_t) function;
    process->context->rsi = (uint64_t) arg;
    process->context->rbp = 0;

    kstrcpy(process->name, name, PROCESS_NAME_MAX_LEN);
    process->pid = next_pid++;
    process->status = SPAWNED;

    process->vmm = vmm_new();

    process_add(process);
    enable_interrupts();

    klog("PROCESS::", "process created");
    return process;
}

void process_add(process_t* process) {
    process_t* iterator = processes_list;

    // TODO check if process should move to ready or blocked state
    process->status = READY;

    if (iterator == NULL) {
        processes_list = process;
        return;
    }

    while (iterator->next != NULL) {
        iterator = iterator->next;
    }
    iterator->next = process;

    return;
}

process_t* process_get_list() { return processes_list; }

process_t* process_get_current() { return current_process; }

vmm_t* process_get_current_vmm() { return process_get_current()->vmm; }

void process_set_current(process_t* p) { current_process = p; }

void process_delete(process_t* process) {

    if (process == NULL) {
        return;
    }

    spinlock_t lock = SPINLOCK_INIT;

    spinlock_acquire(&lock);
    // removing process from queue
    if (processes_list == process) {
        processes_list = process->next;
    } else {
        process_t* scan = processes_list;
        while (scan->next != process) {
            scan = scan->next;
            if (scan == NULL) {
                panic("deleting a process with pid: %d that is not part of "
                      "linked list",
                      process->pid);
            }
        }
        scan->next = process->next;
    }

    // setting current_process
    if (current_process->next != NULL) {
        current_process = current_process->next;
    } else {
        current_process = processes_list;
    }

    spinlock_release(&lock);
    // freeing resources
    // TODO: free process->resources
    // TODO: free root page table if not used by other processes
    // TODO: free stacks
    kfree(process);

    return;
}
