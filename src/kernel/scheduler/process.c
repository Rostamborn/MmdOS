#include "process.h"
#include "../lib/alloc.h"
#include "../lib/logger.h"
#include "../lib/print.h"
#include "../lib/spinlock.h"
#include "../lib/util.h"
#include "../mm/vmm.h"
#include "../scheduler/process.h"
#include "stdbool.h"

process_t* processes_list;
process_t* current_process;
size_t     next_pid = 1;

void process_idle() {
    klog("SCHEDULER::", "from idle process");
    while (true) {
        asm("hlt");
    }
}

// created pcb and allocates memory to process.
// also add it to queue.
process_t* process_create(char* restrict name, void* restrict function(void*),
                          void* restrict arg) {
    disable_interrupts();

    process_t* process = kalloc(sizeof(process_t));

    kstrcpy(process->name, name, PROCESS_NAME_MAX_LEN);
    process->pid = next_pid++;
    process->status = SPAWNED;

    thread_t* thread = thread_add(process, name, &(*function), arg);

    // TODO: vmm_create()
    process->pagemap = vmm_new_pagemap();

    process_add(process);
    enable_interrupts();

    return process;
}

void process_add(process_t* process) {
    process_t* iterator = processes_list;

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

PageMap* process_get_pagemap() { return process_get_current()->pagemap; }

void process_set_current(process_t* p) { current_process = p; }

void process_delete(process_t* process) {
    if (process == NULL) {
        return;
    }

    spinlock_t lock = SPINLOCK_INIT;
    spinlock_acquire(&lock);

    // removing threads
    thread_t* next;
    for (thread_t* scan = process->threads; scan != NULL; scan = next) {
        next = scan->next;
        thread_delete(process, scan);
    }

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

    // freeing resources
    // TODO: free process->resources
    // TODO: free root page table if not used by other processes
    kfree(process);

    spinlock_release(&lock);
    return;
}
