#include "scheduler.h"
#include "../cpu/cpu.h"
#include "../gdt.h"
#include "../lib/panic.h"
#include "process.h"
#include "stdbool.h"

void scheduler_init() { process_create("idle process", &process_idle, NULL); }

// return control to scheduler by firing interrupt
void scheduler_yield() { asm("int $0x20"); }

execution_context* scheduler_context_switch(execution_context* context) {
    // rsp0 contains kernel stack
    tss_set_rsp0((uint64_t) process_get_current()->kstack);

    context = process_get_current()->context;

    vmm_switch_pml(process_get_current_vmm());

    return context;
}

bool stay_idle(process_t* process) {
    process = process->next;
    while (process != NULL) {
        if (process->status != SLEEPING) {
            return false;
        }

        process = process->next;
    }

    return true;
}

execution_context* schedule(execution_context* restrict context) {
    // making sure there are processes in the list
    process_t* processes_list = process_get_list();
    if (processes_list == NULL) {
        panic("SCHEDULER ::", "process list empty!");
    }

    // making sure current_process is not NULL
    process_t* current_process = process_get_current();
    if (current_process == NULL) {
        current_process = processes_list;
        process_set_current(current_process);
    }

    bool still_scheduling = true;
    bool process_switched = false;

    while (still_scheduling) {

        if (processes_list == NULL) {
            panic("SCHEDULER ::", "no process found");
        }

        switch (current_process->status) {
        case SPAWNED:
            // TODO check if it should be moved to blocked queue
            current_process->status = READY;
            still_scheduling = false;
            process_switched = true;
            break;

        case READY:
            still_scheduling = false;
            process_switched = true;
            break;

        case RUNNING:
            current_process->remaining_quantum--;
            current_process->context = context;

            // check if current process should keep running
            if (current_process->remaining_quantum > 0) {
                still_scheduling = false;
                break;
            }

            current_process->status = READY;
            process_switched = true;
            break;

        case SLEEPING:
            uint64_t uptime = timer_get_uptime();

            if (uptime > current_process->wake_time) {
                current_process->wake_time = 0;

                // TODO: in future check if it should be blocked
                current_process->status = READY;
                still_scheduling = false;
            }
            break;

        case DEAD:
            process_delete(current_process);
            current_process = process_get_current();
            processes_list = process_get_list();
            process_switched = true;

            continue;

        default:
            panic(
                "SCHEDULER ::",
                "unhandled status when switching process! pid: %d, status: %d",
                current_process->pid, current_process->status);
            break;
        }

        if (!still_scheduling) {

            if ((current_process->pid == 1) && stay_idle(current_process)) {
                still_scheduling = false;
            }
        }

        if (still_scheduling) {
            if (current_process->next != NULL) {
                current_process = current_process->next;
            } else {
                current_process = processes_list;
            }
        }
    }

    // if (current_process->pid != 1) {
    //     klog("SCHEDULER ::",
    //          "thread with tid: %d from process with pid: %d will run next",
    //          current_process->running_thread->tid, current_process->pid);
    // }

    process_set_current(current_process);
    current_process->status = RUNNING;

    if (process_switched) {
        current_process->remaining_quantum = DEFAULT_PROCESS_RUNNING_QUANTUM;

        return scheduler_context_switch(process_get_current()->context);
    }

    return current_process->context;
}
