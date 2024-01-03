#ifndef KERNEL_PROMPT_DEF
#define KERNEL_PROMPT_DEF

#include <stdint.h>

void prompt_init();               // setup prompt
void prompt_enter_handler();      // handle enter key press
void prompt_char_handler(char c); // handle char key press
void prompt_backspace_handler();  // handle backspace press
#endif