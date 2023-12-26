#ifndef KERNEL_PROCESS_DEF
#define KERNEL_PROCESS_DEF

#include "src/kernel/cpu/cpu.h"
#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"

#define PROCESS_NAME_MAX_LEN 64

// determine what condition the process is in
// TODO: add more states e.q. BLOCKED
typedef enum { SPAWNED, READY, RUNNING, DEAD } status_t;

typedef struct process_t {
    size_t            pid;
    status_t          status;
    void*             root_page_table;
    interrupt_frame   context;
    char              name[PROCESS_NAME_MAX_LEN];
    struct process_t* next;
} process_t;

static process_t* processes_list;
static process_t* current_process;

static size_t next_pid = 1;

// created pcb and allocates memory to process.
// does not add it to queue yet.
process_t* create_process(char* name, void* function, void* arg);

// a dummy process for when cpu has nothing else to do
void idle_process();

#endif