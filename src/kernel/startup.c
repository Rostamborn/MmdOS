#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "pmm.h"
#include "limine_term.h"
#include "pic.h"
#include "prompt.h"
#include "src/lib/print.h"
#include "timer.h"
#include <stdbool.h>
// NOTE(Arman): *We can't use stdlib at all. We have to write our own functions*

void _start(void) {
    gdt_init();
    idt_init();
    timer_init();

    // testing hex output
    printf("%x\n", 1000000000);
    int x = 5;
    int *y = &x;
    printf("%p \n", y);
    // ------

    prompt_init();
    keyboard_init();
    pmm_init();
    // draw_line();

    init_serial();
    log_to_serial("Hello, world!\n");

    log_to_serial_digit(123);
    // uint8_t a = 1 / 0; // I can not belive the interrupt system works

    // hcf(); // halt, catch fire
    for (;;)
        ;
}
