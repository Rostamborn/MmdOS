#ifndef KERNEL_THREAD_DEF
#define KERNEL_THREAD_DEF

#ifndef KERNEL_STATUS_T_DEF
#define KERNEL_STATUS_T_DEF
// determine what condition the process or thread is in
// TODO: add more states e.q. BLOCKED
typedef enum { SPAWNED, READY, RUNNING, SLEEPING, DEAD } status_t;
#endif

#include "../cpu/cpu.h"
#include "process.h"
#include "stddef.h"
#include "stdint.h"

#define THREAD_NAME_MAX_LEN 64
#define DEFAULT_THREAD_RUNNING_QUANTUM 1

typedef struct thread_t {
    size_t           tid;
    status_t         status;
    interrupt_frame* context;
    char             name[THREAD_NAME_MAX_LEN];
    struct thread_t* next;
    uint64_t         wake_time;
    int16_t          remaining_quantum;
} thread_t;

// create a thread but will not add it to process yet
thread_t* thread_add(struct process_t* restrict process, char* restrict name,
                     void* restrict function(void*), void* restrict arg);

#endif
