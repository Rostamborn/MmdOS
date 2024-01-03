#include "src/kernel/scheduler/thread.h"
#include "src/kernel/lib/alloc.h"
#include "src/kernel/lib/util.h"
#include "src/kernel/scheduler/process.h"
#include "stdbool.h"

// allocate 32kB to thread Stack.
uint64_t* alloc_stack() {
    uint64_t* stack = kalloc(32 * 1024);
    return stack;
}

// set status to DEAD
// be careful not to release stack and context here
// as they are still in use!
void thread_exit() {
    // TODO: set status to DEAD
    while (true)
        ;
    // TODO: scheduler_yield()
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
    interrupt_frame* context = kalloc(sizeof(interrupt_frame));
    thread_t*        thread = kalloc(sizeof(thread_t));

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
    thread->status = READY;
    thread->next = NULL;
    thread->context->iret_ss = 0x30;
    thread->context->iret_rsp = (uint64_t) alloc_stack();
    thread->context->iret_flags =
        0x202; // resets all bits but 2 and 9.
               // 2 for legacy reasons and 9 for interrupts.
    thread->context->iret_cs = 0x28;
    thread->context->iret_rip = (uint64_t) thread_execution_wrapper;
    thread->context->rdi = (uint64_t) function;
    thread->context->rsi = (uint64_t) arg;
    thread->context->rbp = 0;

    return thread;
}