#include "scheduler.h"
#include "../cpu/cpu.h"
#include "../interrupts/timer.h"
#include "../lib/logger.h"
#include "../lib/panic.h"
#include "../lib/spinlock.h"
#include "stdbool.h"

void scheduler_init() { process_create("idle process", &process_idle, NULL); }

// return control to scheduler by firing interrupt
void scheduler_yield() { asm("int $0x20"); }

// private method for schedule
bool switch_threads(process_t* process) {

    for (int i = 0; i < process->threads_count + 1; i++) {

        if (process->threads == NULL) {
            process->status = DEAD;
            return false;
        }

        switch (process->running_thread->status) {
        case READY:
            return true;

        case RUNNING:
            process->running_thread->status = READY;
            break;

        case SLEEPING:
            uint64_t uptime = timer_get_uptime();

            if (uptime > process->running_thread->wake_time) {
                process->running_thread->wake_time = 0;

                // TODO: in future check if it should be blocked
                process->running_thread->status = READY;
                return true;
            }
            break;

        case DEAD:
            thread_delete(process, process->running_thread);
            continue;

        default:
            panic("SCHEDULER ::",
                  "unhandled status when switching thread! tid: %d, status: %d",
                  process->running_thread->tid,
                  process->running_thread->status);
            break;
        }

        if (process->running_thread->next != NULL) {
            process->running_thread = process->running_thread->next;
        } else {
            process->running_thread = process->threads;
        }
    }
    return false;
}

bool stay_idle(process_t* process) {
    process = process->next;
    while (process != NULL) {
        if (process->status != SLEEPING) {
            return false;
        }
        for (thread_t* scan = process->threads; scan != NULL;
             scan = scan->next) {
            if (scan->status != SLEEPING) {
                return false;
            }
        }
        process = process->next;
    }

    return true;
}

execution_context* schedule(execution_context* restrict context) {
    // making sure there are processes in the list
    process_t* processes_list = process_get_list();
    if (processes_list == NULL) {
        return context;
    }

    // making sure current_process is not NULL
    process_t* current_process = process_get_current();
    if (current_process == NULL) {
        current_process = processes_list;
        process_set_current(current_process);
    }

    bool still_scheduling = true;
    bool process_switched = false;
    bool thread_switched = false;

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
            current_process->running_thread->remaining_quantum--;
            current_process->running_thread->context = context;

            if (current_process->running_thread->status == DEAD) {
                thread_delete(current_process, current_process->running_thread);
                thread_switched = true;
            }

            if (current_process->status == DEAD) {
                // let the next iteration of loop handle dead
                break;
            }

            // check if current thread should keep running
            if (current_process->running_thread->remaining_quantum > 0) {
                still_scheduling = false;
                break;
            }

            // check if current process should keep running
            if (current_process->remaining_quantum > 0) {
                bool success = switch_threads(current_process);
                thread_switched = true;
                if (success) {
                    still_scheduling = false;
                    break;
                }
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
            klog("schedule ::", "process with pid: %d is dead",
                 current_process->pid);
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
            if (current_process->threads == NULL) {
                current_process->status = DEAD;
                still_scheduling = true;
                continue;
            }

            if ((current_process->pid == 1) && stay_idle(current_process)) {
                still_scheduling = false;
            }

            if (current_process->running_thread->status == DEAD ||
                current_process->running_thread->status == SLEEPING) {
                bool success = switch_threads(current_process);
                thread_switched = true;
                if (!success) {
                    still_scheduling = true;
                }
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

    process_set_current(current_process);
    current_process->status = RUNNING;
    current_process->running_thread->status = RUNNING;

    if (process_switched) {
        current_process->remaining_quantum = DEFAULT_PROCESS_RUNNING_QUANTUM;
        current_process->running_thread->remaining_quantum =
            DEFAULT_THREAD_RUNNING_QUANTUM;
    } else if (thread_switched) {
        current_process->running_thread->remaining_quantum =
            DEFAULT_THREAD_RUNNING_QUANTUM;
    }

    if (current_process->pid != 1) {
        klog("SCHEDULER ::",
             "thread with tid: %d from process with pid: %d will run next",
             current_process->running_thread->tid, current_process->pid);
    }

    return current_process->running_thread->context;
}
