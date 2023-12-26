#include "src/kernel/scheduler/scheduler.h"
#include "src/kernel/cpu/cpu.h"
#include "src/kernel/lib/logger.h"
#include "stdbool.h"
void add_process(process_t* process) {
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
}

void scheduler_init() {
    process_t* idle_p = create_process("idle process", &idle_process, NULL);
    add_process(idle_p);
}

interrupt_frame* schedule(interrupt_frame* context) {
    // making sure current_process and processes_list are valid
    if (current_process == NULL) {
        if (processes_list == NULL) {
            return context;
        }
        current_process = processes_list;
        return current_process->context;
    }

    current_process->context = context;
    current_process->status = READY;

    while (true) {
        process_t* prev_process = current_process;
        if (current_process->next != NULL) {
            current_process = current_process->next;
        } else {
            current_process = processes_list;
        }

        if (current_process != NULL && current_process->status == DEAD) {
            // delete process

        } else {
            current_process->status = RUNNING;
            break;
        }
    }

    klog("SCHEDULER::", "pid: %d | name: %s will run next.",
         current_process->pid, current_process->name);
    return current_process->context;
}