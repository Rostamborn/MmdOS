#include "limine.h"
#include "cpu.h"
#include "limine_terminal.h"
#include "gdt.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
// NOTE(Arman): *We can't use stdlib at all. We have to write our own functions*


// struct limine_framebuffer_request frame_buf_req = {
//     .id = LIMINE_FRAMEBUFFER_REQUEST,
//     .revision = 0
// };

void _start(void) {
    gdt_init();
    // if (frame_buf_req.response == NULL || frame_buf_req.response->framebuffer_count < 1) {
    //     hcf();
    // }

    // struct limine_framebuffer *framebuffer = frame_buf_req.response->framebuffers[0];

    // Draw a diagonal line
    // for (size_t i = 0; i < 100; i++) {
    //     uint32_t *fb_ptr = framebuffer->address;
    //     fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xfff000;
    // }

    limine_write("Hello, world!\n");
    init_serial();
    log_to_serial("Hello, world!\n");
    print_digit(123);

    hcf();
}
