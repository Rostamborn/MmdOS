#include "src/kernel/scheduler/scheduler.h"
#include "src/kernel/cpu/cpu.h"
#include "src/kernel/lib/logger.h"
#include "src/kernel/lib/panic.h"
#include "src/kernel/lib/spinlock.h"
#include "stdbool.h"

void scheduler_init() { process_create("idle process", &process_idle, NULL); }

// return control to scheduler by firing interrupt
void scheduler_yield() { asm("int $0x20"); }

interrupt_frame* schedule(interrupt_frame* restrict context) {
    // making sure current_process and processes_list are valid
    process_t* processes_list = process_get_list();
    process_t* current_process = process_get_current();
    if (current_process == NULL) {
        if (processes_list == NULL) {
            return context;
        }
        current_process = processes_list;
        process_set_current(current_process);

        current_process->remaining_quantum = DEFAULT_PROCESS_RUNNING_QUANTUM;
        current_process->running_thread->remaining_quantum =
            DEFAULT_THREAD_RUNNING_QUANTUM;
        return current_process->running_thread->context;
    }
    current_process->remaining_quantum--;
    current_process->running_thread->remaining_quantum--;
    current_process->running_thread->context = context;

    if (current_process->running_thread->status == DEAD) {
        thread_delete(current_process, current_process->running_thread);
    }

    if (current_process->status == DEAD) {
        process_delete(current_process);
        current_process = process_get_current();
        processes_list = process_get_list();
    }

    if (current_process->running_thread->remaining_quantum > 0 &&
        (current_process->running_thread->status == RUNNING)) {
        // continue running current thread
        klog("SCHEDULER ::",
             "thread with tid: %d from process with pid: %d will run next",
             current_process->running_thread->tid, current_process->pid);
        return current_process->running_thread->context;
    }

    if (current_process->remaining_quantum > 0) {
        // switch thread
        if (current_process->running_thread->status == RUNNING) {
            current_process->running_thread->status = READY;
        }

        for (int i = 0; i < current_process->threads_count; i++) {
            if (current_process->running_thread->next != NULL) {
                current_process->running_thread =
                    current_process->running_thread->next;
            } else {
                current_process->running_thread = current_process->threads;
            }

            if (current_process->running_thread == NULL) {
                // no thread left in process
                process_delete(current_process);
                processes_list = process_get_list();
                current_process = process_get_current();
                break;
            }

            if (current_process->status == SLEEPING) {
                // TODO: handle sleeping
                continue;
            }

            if (current_process->running_thread->status == DEAD) {
                thread_delete(current_process, current_process->running_thread);
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
    if (current_process->running_thread != NULL &&
        current_process->running_thread->status == RUNNING) {
        current_process->running_thread->status = READY;
    }

    // TODO: in the future check if there is no other running thread,
    // TODO: before setting status to READY
    if (current_process->status == RUNNING) {
        current_process->status = READY;
    }

    if (current_process->status == DEAD) {
        process_delete(current_process);
        current_process = process_get_current();
        processes_list = process_get_list();
    }

    while (true) {
        if (current_process->next != NULL) {
            current_process = current_process->next;
        } else {
            current_process = processes_list;
        }

        if (current_process == NULL) {
            panic("no process in queue");
        }

        if (current_process->status == DEAD ||
            current_process->threads == NULL) {
            process_delete(current_process);
            current_process = process_get_current();
            processes_list = process_get_list();
            continue;
        }

        break;
    }
    process_set_current(current_process);
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