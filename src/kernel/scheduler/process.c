#include "process.h"
#include "../lib/alloc.h"
#include "../lib/logger.h"
#include "../lib/print.h"
#include "../lib/util.h"
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
    process->root_page_table = NULL;

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

void process_set_current(process_t* p) { current_process = p; }
