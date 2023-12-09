#include "fb.h"
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "limine.h"
#include "limine_term.h"
#include "pic.h"
#include "print.h"
#include "prompt.h"
#include "timer.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
// NOTE(Arman): *We can't use stdlib at all. We have to write our own functions*

void _start(void) {
    gdt_init();
    idt_init();
    timer_init();
    prompt_init();
    keyboard_init();
    // draw_line();

    init_serial();
    log_to_serial("Hello, world!\n");

    log_to_serial_digit(123);
    // uint8_t a = 1 / 0; // I can not belive the interrupt system works

    // hcf(); // halt, catch fire
    for (;;)
        ;
}
