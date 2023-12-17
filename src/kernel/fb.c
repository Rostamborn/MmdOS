#include "fb.h"
#include "limine.h"
#include "panic.h"
#include <stddef.h>

struct limine_framebuffer_request frame_buf_req = {
    .id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0};

void draw_line() {
    if (frame_buf_req.response == NULL ||
        frame_buf_req.response->framebuffer_count < 1) {
        panic("No Frame Buffer found");
    }

    struct limine_framebuffer *framebuffer =
        frame_buf_req.response->framebuffers[0];

    // Draw a diagonal line
    for (size_t i = 0; i < 100; i++) {
        uint32_t *fb_ptr = framebuffer->address;
        fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xfff000;
    }
}

// void delete(int8_t line_num, int8_t line_len) {
//     if (frame_buf_req.response == NULL ||
//         frame_buf_req.response->framebuffer_count < 1) {
//         panic("No Frame Buffer found");
//     }

//     struct limine_framebuffer *framebuffer =
//         frame_buf_req.response->framebuffers[0];

//     uint32_t *fb_ptr = framebuffer->address;
//     // fb_ptr[(line_num - 1) * ((framebuffer->pitch) * 4) +
//     //        ((line_len + 1) * 10)] = 0xfff000;
//     // fb_ptr[line_num * ((framebuffer->pitch) * 4) + ((line_len + 2) * 10)]
//     =
//     //     0xfff000;

//     for (int i = (line_num - 1) * ((framebuffer->pitch) * 4);
//          i <= line_num * ((framebuffer->pitch) * 4);
//          i = i + (framebuffer->pitch / 4)) {
//         for (int j = ((line_len + 3) * 9); j <= ((line_len + 4) * 9); j++) {
//             fb_ptr[i + j] = 0x000000;
//         }
//     }
// }
