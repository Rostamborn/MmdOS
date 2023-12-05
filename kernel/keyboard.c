#include "keyboard.h"
#include "idt.h"
#include <stdint.h>
#include "limine_term.h"

interrupt_frame* keyboard_handler(interrupt_frame* frame) {
    uint8_t scancode = inb(0x60); // which key is pressed
    uint8_t pressed = inb(0x60) & 0x80; // is the key pressed or released

    limine_write("Key: ");
    limine_write_digit(scancode);
    // log_to_serial("Key: ");
    // log_to_serial_digit(scancode);
    // log_to_serial_digit(pressed);


    return frame;
}

void keyboard_init() {
    irq_install_handler(1, &keyboard_handler);
}
