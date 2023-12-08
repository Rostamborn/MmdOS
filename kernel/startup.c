#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "limine.h"
#include "limine_term.h"
#include "pic.h"
#include "print.h"
#include "timer.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
// NOTE(Arman): *We can't use stdlib at all. We have to write our own functions*

struct limine_framebuffer_request frame_buf_req = {
    .id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0};

void _start(void) {
    gdt_init();
    idt_init();
    timer_init();
    keyboard_init();

    if (frame_buf_req.response == NULL ||
        frame_buf_req.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *framebuffer =
        frame_buf_req.response->framebuffers[0];

    // Draw a diagonal line
    for (size_t i = 0; i < 100; i++) {
        uint32_t *fb_ptr = framebuffer->address;
        fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xfff000;
    }

    limine_write("Hello, world!\n");
    init_serial();
    log_to_serial("Hello, world!\n");
    limine_write("\n");
    printf("%s, %s! %d %c\n", "Hello", "World", 12345, 'h');
    log_to_serial_digit(123);
    uint8_t a = 1 / 0; // I can not belive the interrupt system works

    // hcf(); // halt, catch fire
    for (;;)
        ;
}
