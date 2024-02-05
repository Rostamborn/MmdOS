#ifndef KERNEL_PROMPT_DEF
#define KERNEL_PROMPT_DEF

#include <stdint.h>

void     prompt_init(); // setup prompt
void     prompt_clear();
void     clear_screen();
uint64_t prompt_lockstdin_syscall(uint64_t frame, uint64_t unused1,
                                  uint64_t unused2, uint64_t unused3,
                                  uint64_t unused4);
uint64_t prompt_unlockstdin_syscall(uint64_t frame, uint64_t unused1,
                                    uint64_t unused2, uint64_t unused3,
                                    uint64_t unused4);
#endif
