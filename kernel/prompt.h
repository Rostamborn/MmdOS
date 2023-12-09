
#ifndef KERNEL_PROMPT_DEF
#define KERNEL_PROMPT_DEF

#include <stdint.h>

#define PROMPT_BUFFER_SIZE 1024

static uint8_t line_len = 0;
static char buffer[PROMPT_BUFFER_SIZE];
static uint16_t buffer_pointer = 0;
static uint8_t line_num = 1;
void prompt_init();               // setup prompt
void prompt_enter_handler();      // handle enter key press
void prompt_char_handler(char c); // handle char key press
void prompt_backspace_handler();  // handle backspace press
#endif