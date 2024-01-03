#ifndef KERNEL_PROCESS_DEF
#define KERNEL_PROCESS_DEF

#include "src/kernel/cpu/cpu.h"
#include "src/kernel/scheduler/thread.h"
#include "stddef.h"
#include "stdint.h"

#define PROCESS_NAME_MAX_LEN 64
#define MAX_RESOURCE_IDS 255
#define DEFAULT_PROCESS_RUNNING_QUANTUM 3

typedef struct process_t {
    size_t   pid;
    status_t status;
    void*    root_page_table;
    // stores system-wide ids to keep track of resources
    size_t            resources[MAX_RESOURCE_IDS];
    char              name[PROCESS_NAME_MAX_LEN];
    struct thread_t*  threads;
    struct thread_t*  running_thread;
    int16_t           remaining_quantum;
    int8_t            threads_count;
    struct process_t* next;
} process_t;

static process_t* processes_list;
static process_t* current_process;

static size_t next_pid = 1;

// created pcb and allocates memory to process.
// does not add it to queue yet.
process_t* process_create(char* restrict name, void* restrict function(void*),
                          void* restrict arg);

// add created process to queue
void process_add(process_t* process);

// a dummy process for when cpu has nothing else to do
void process_idle();

process_t* process_list();

#endif