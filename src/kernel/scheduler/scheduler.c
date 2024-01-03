#include "src/kernel/scheduler/scheduler.h"
#include "src/kernel/cpu/cpu.h"
#include "src/kernel/lib/logger.h"
#include "stdbool.h"

void scheduler_init() { process_create("idle process", &process_idle, NULL); }

interrupt_frame* schedule(interrupt_frame* restrict context) {
    // making sure current_process and processes_list are valid
    process_t* list_of_processes = process_list();
    if (current_process == NULL) {
        if (list_of_processes == NULL) {
            return context;
        }
        current_process = list_of_processes;
        current_process->remaining_quantum = DEFAULT_PROCESS_RUNNING_QUANTUM;
        current_process->running_thread->remaining_quantum =
            DEFAULT_THREAD_RUNNING_QUANTUM;
        return current_process->running_thread->context;
    }
    current_process->remaining_quantum--;
    current_process->running_thread->remaining_quantum--;
    current_process->running_thread->context = context;

    if (current_process->running_thread->remaining_quantum > 0) {
        // continue running current thread
        klog("SCHEDULER ::",
             "thread with tid: %d from process with pid: %d will run next",
             current_process->running_thread->tid, current_process->pid);
        return current_process->running_thread->context;
    }

    if (current_process->remaining_quantum > 0) {
        // switch thread
        current_process->running_thread->status = READY;

        for (int i = 0; i < current_process->threads_count; i++) {
            if (current_process->running_thread->next != NULL) {
                current_process->running_thread =
                    current_process->running_thread->next;
            } else {
                current_process->running_thread = current_process->threads;
            }

            if (current_process->running_thread == NULL) {
                // no thread left in process
                // TODO: delete process
                continue;
            }

            if (current_process->status == SLEEPING) {
                // TODO: handle sleeping
                continue;
            }

            if (current_process->running_thread->status == DEAD) {
                // TODO: delete thread
                continue;
            }
            current_process->running_thread->status = RUNNING;
            current_process->running_thread->remaining_quantum =
                DEFAULT_THREAD_RUNNING_QUANTUM;
            klog("SCHEDULER ::",
                 "thread with tid: %d from process with pid: %d will run next",
                 current_process->running_thread->tid, current_process->pid);
            return current_process->running_thread->context;
        }
    }

    // switching process
    current_process->running_thread->status = READY;

    // TODO: in the future check if there is no other running thread,
    // TODO: before setting status to READY
    current_process->status = READY;

    while (true) {
        if (current_process->next != NULL) {
            current_process = current_process->next;
        } else {
            current_process = list_of_processes;
        }

        if (current_process != NULL && current_process->status == DEAD) {
            // delete process
            continue;
        }
        break;
    }

    current_process->status = RUNNING;
    current_process->remaining_quantum = DEFAULT_PROCESS_RUNNING_QUANTUM;
    current_process->running_thread->status = RUNNING;
    current_process->running_thread->remaining_quantum =
        DEFAULT_THREAD_RUNNING_QUANTUM;
    klog("SCHEDULER ::",
         "thread with tid: %d from process with pid: %d will run next",
         current_process->running_thread->tid, current_process->pid);
    return current_process->running_thread->context;
}