#include "limine.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// LIMINE_BASE_REVISION(1)

struct limine_framebuffer_request frame_buf_req = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

static void hcf(void) {
    asm ("cli");
    for (;;) {
        asm ("hlt");
    }
}

void _start(void) {
    if (frame_buf_req.response == NULL || frame_buf_req.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *framebuffer = &frame_buf_req.response->framebuffers[0];

    for (size_t i = 0; i < 100; i++) {
        uint32_t *fb_ptr = framebuffer->address;
        fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xffffff;
    }
    volatile char *video = (volatile char*)0xB8000;
    video[0] = 'H';

    hcf();
}
