#include "thread.h"
#include "../lib/alloc.h"
#include "../lib/logger.h"
#include "../lib/panic.h"
#include "../lib/spinlock.h"
#include "../lib/util.h"
#include "process.h"
#include "scheduler.h"
#include "stdbool.h"

#define STACK_SIZE (64 * 1024)

size_t next_tid = 1;

// allocate 64KB to thread Stack.
void* alloc_stack() {
    void* stack = kalloc(STACK_SIZE);
    return stack + STACK_SIZE;
}

// set status to DEAD
// be careful not to release stack and context here
// as they are still in use!
void thread_exit() {
    process_t* current_process = process_get_current();
    current_process->running_thread->status = DEAD;
    scheduler_yield();
}

void thread_execution_wrapper(void (*function)(void*), void* arg) {
    function(arg);
    thread_exit();
}

void thread_sleep(thread_t* thread, size_t millis) {
    thread->status = SLEEPING;
    // TODO: thread->wake_time = current_uptime_ms() + millis;
    //  TODO: scheduler_yield();
}

thread_t* thread_add(process_t* restrict process, char* restrict name,
                     void* restrict function(void*), void* restrict arg) {
    spinlock_t lock = SPINLOCK_INIT;
    spinlock_acquire(&lock);

    execution_context* context =
        (execution_context*) kalloc(sizeof(execution_context));
    thread_t* thread = kalloc(sizeof(thread_t));

    thread->context = context;

    if (process->threads == NULL) {
        process->threads = thread;
        process->running_thread = thread;
    } else {
        for (thread_t* scan = process->threads; scan != NULL;
             scan = scan->next) {
            if (scan->next != NULL)
                continue;
            scan->next = thread;
            break;
        }
    }
    process->threads_count++;

    kstrcpy(thread->name, name, THREAD_NAME_MAX_LEN);
    thread->tid = next_tid++;
    // TODO check if process should move to ready or blocked state
    thread->status = READY;
    thread->next = NULL;
    thread->stack = alloc_stack();
    thread->context->iret_ss = 0x30;
    thread->context->iret_rsp = (uint64_t) thread->stack;
    thread->context->iret_flags =
        0x202; // resets all bits but 2 and 9.
               // 2 for legacy reasons and 9 for interrupts.
    thread->context->iret_cs = 0x28;
    thread->context->iret_rip = (uint64_t) thread_execution_wrapper;
    thread->context->rdi = (uint64_t) function;
    thread->context->rsi = (uint64_t) arg;
    thread->context->rbp = 0;

    spinlock_release(&lock);
    return thread;
}

void thread_delete(process_t* process, thread_t* thread) {
    if (thread == NULL) {
        return;
    }
    spinlock_t lock = SPINLOCK_INIT;
    spinlock_acquire(&lock);
    // remove thread from linked list
    if (process->threads == thread) {
        if (thread->next == NULL) {
            process->status = DEAD;
            process->threads = NULL;
        }
        process->threads = thread->next;
    } else {
        thread_t* scan = process->threads;
        while (scan->next != thread) {
            scan = scan->next;
            if (scan == NULL) {
                panic("thread with tid: %d could not be removed from process "
                      "with pid: %d as it was not part of it threads ",
                      thread->tid, process->pid);
            }
        }
        scan->next = thread->next;
    }
    if (thread->next == NULL) {
        process->running_thread = process->threads;
    } else {
        process->running_thread = thread->next;
    }

    process->threads_count--;
    spinlock_release(&lock);
    // release resources
    // kfree(thread->stack);
    // kfree(thread->context);
    // kfree(thread);

    return;
}
