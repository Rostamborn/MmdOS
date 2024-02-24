#ifndef KERNEL_PROCESS_DEF
#define KERNEL_PROCESS_DEF

#include "../cpu/cpu.h"
#include "../mm/vmm.h"
#include "stddef.h"
#include "stdint.h"

#define PROCESS_NAME_MAX_LEN 64
#define MAX_RESOURCE_IDS 255
#define DEFAULT_PROCESS_RUNNING_QUANTUM 3

#ifndef KERNEL_STATUS_T_DEF
#define KERNEL_STATUS_T_DEF
// determine what condition the process or thread is in
// TODO: add more states e.q. BLOCKED
typedef enum { SPAWNED, READY, RUNNING, SLEEPING, DEAD } status_t;
#endif

typedef struct process_t {
    size_t   pid;
    status_t status;
    vmm_t*   vmm;
    // stores system-wide ids to keep track of resources
    size_t             resources[MAX_RESOURCE_IDS];
    char               name[PROCESS_NAME_MAX_LEN];
    int16_t            remaining_quantum;
    uint64_t           wake_time;
    struct process_t*  next;
    execution_context* context;
    // keeping pointer to stack so to free it easily later on
    void* ustack;
    void* kstack;
} process_t;

// created pcb and allocates memory to process.
// does not add it to queue yet.
process_t* process_create(char* restrict name, void* restrict function(void*),
                          void* restrict arg);

// add created process to queue
void process_add(process_t* process);

// a dummy process for when cpu has nothing else to do
void process_idle();

process_t* process_get_list();

process_t* process_get_current();

vmm_t* process_get_current_vmm();

void process_set_current(process_t* p);

void process_delete(process_t* process);

#endif
