#include "keyboard.h"
#include "../lib/spinlock.h"
#include "../terminal/limine_term.h"
#include "idt.h"
#include <stdint.h>

uint8_t    keyboard_buffer = 0;
bool       keyboard_has_char = false;
spinlock_t keyboard_lock = SPINLOCK_INIT;

typedef struct {
    uint8_t shift;
    uint8_t caps_lock;
} Key_Event;

Key_Event keyboard = {0, 0};

typedef enum {
    UNKNOWN = 0xFFFFFFFF,
    ESC = 0xFFFFFFFF - 1,
    CTRL = 0xFFFFFFFF - 2,
    LSHFT = 0xFFFFFFFF - 3,
    RSHFT = 0xFFFFFFFF - 4,
    ALT = 0xFFFFFFFF - 5,
    F1 = 0xFFFFFFFF - 6,
    F2 = 0xFFFFFFFF - 7,
    F3 = 0xFFFFFFFF - 8,
    F4 = 0xFFFFFFFF - 9,
    F5 = 0xFFFFFFFF - 10,
    F6 = 0xFFFFFFFF - 11,
    F7 = 0xFFFFFFFF - 12,
    F8 = 0xFFFFFFFF - 13,
    F9 = 0xFFFFFFFF - 14,
    F10 = 0xFFFFFFFF - 15,
    F11 = 0xFFFFFFFF - 16,
    F12 = 0xFFFFFFFF - 17,
    SCRLCK = 0xFFFFFFFF - 18,
    HOME = 0xFFFFFFFF - 19,
    UP = 0xFFFFFFFF - 20,
    LEFT = 0xFFFFFFFF - 21,
    RIGHT = 0xFFFFFFFF - 22,
    DOWN = 0xFFFFFFFF - 23,
    PGUP = 0xFFFFFFFF - 24,
    PGDOWN = 0xFFFFFFFF - 25,
    END = 0xFFFFFFFF - 26,
    INS = 0xFFFFFFFF - 27,
    DEL = 0xFFFFFFFF - 28,
    CAPS = 0xFFFFFFFF - 29,
    NONE = 0xFFFFFFFF - 30,
    ALTGR = 0xFFFFFFFF - 31,
    NUMLCK = 0xFFFFFFFF - 32,
} Special_Keys;

const uint64_t lower_case[128] = {
    UNKNOWN, ESC,     '1',     '2',     '3',     '4',     '5',     '6',
    '7',     '8',     '9',     '0',     '-',     '=',     '\b',    '\t',
    'q',     'w',     'e',     'r',     't',     'y',     'u',     'i',
    'o',     'p',     '[',     ']',     '\n',    CTRL,    'a',     's',
    'd',     'f',     'g',     'h',     'j',     'k',     'l',     ';',
    '\'',    '`',     LSHFT,   '\\',    'z',     'x',     'c',     'v',
    'b',     'n',     'm',     ',',     '.',     '/',     RSHFT,   '*',
    ALT,     ' ',     CAPS,    F1,      F2,      F3,      F4,      F5,
    F6,      F7,      F8,      F9,      F10,     NUMLCK,  SCRLCK,  HOME,
    UP,      PGUP,    '-',     LEFT,    UNKNOWN, RIGHT,   '+',     END,
    DOWN,    PGDOWN,  INS,     DEL,     UNKNOWN, UNKNOWN, UNKNOWN, F11,
    F12,     UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN};

const uint64_t upper_case[128] = {
    UNKNOWN, ESC,     '!',     '@',     '#',     '$',     '%',     '^',
    '&',     '*',     '(',     ')',     '_',     '+',     '\b',    '\t',
    'Q',     'W',     'E',     'R',     'T',     'Y',     'U',     'I',
    'O',     'P',     '{',     '}',     '\n',    CTRL,    'A',     'S',
    'D',     'F',     'G',     'H',     'J',     'K',     'L',     ':',
    '"',     '~',     LSHFT,   '|',     'Z',     'X',     'C',     'V',
    'B',     'N',     'M',     '<',     '>',     '?',     RSHFT,   '*',
    ALT,     ' ',     CAPS,    F1,      F2,      F3,      F4,      F5,
    F6,      F7,      F8,      F9,      F10,     NUMLCK,  SCRLCK,  HOME,
    UP,      PGUP,    '-',     LEFT,    UNKNOWN, RIGHT,   '+',     END,
    DOWN,    PGDOWN,  INS,     DEL,     UNKNOWN, UNKNOWN, UNKNOWN, F11,
    F12,     UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN};

execution_context* keyboard_handler(execution_context* frame) {
    uint8_t scancode = inb(0x60) & 0x7f; // which key is pressed
    uint8_t pressed = inb(0x60) & 0x80;  // is the key pressed or released

    switch (scancode) {

    case 1: // esc
        break;
    case 29: // ctrl
        break;
    case 56: // alt
        break;
    case 59: // f1
        break;
    case 60: // f2
        break;
    case 61: // f3
        break;
    case 62: // f4
        break;
    case 63: // f5
        break;
    case 64: // f6
        break;
    case 65: // f7
        break;
    case 66: // f8
        break;
    case 67: // f9
        break;
    case 68: // f10
        break;
    case 87: // f11
        break;
    case 88: // f12
        break;
    case 42: // left shift
        if (pressed == 0) {
            keyboard.shift = 1;
        } else { // released = 128
            keyboard.shift = 0;
        }
        break;
    case 58: // caps lock
        if (!keyboard.caps_lock && pressed == 0) {
            keyboard.caps_lock = 1;
        } else if (keyboard.caps_lock && pressed == 0) {
            keyboard.caps_lock = 0;
        }
        break;
    default:
        if (pressed == 0) {
            spinlock_acquire(&keyboard_lock);
            if (keyboard.shift && keyboard.caps_lock) {

                keyboard_buffer = lower_case[scancode];
            } else if (keyboard.shift || keyboard.caps_lock) {

                keyboard_buffer = upper_case[scancode];
            } else {

                keyboard_buffer = lower_case[scancode];
            }
            keyboard_has_char = true;
            spinlock_release(&keyboard_lock);
        }
    }

    return frame;
}

uint8_t keyboard_getch() {
    spinlock_acquire(&keyboard_lock);
    uint8_t res = keyboard_buffer;
    keyboard_buffer = 0;
    keyboard_has_char = false;
    spinlock_release(&keyboard_lock);

    return res;
}

bool keyboard_checkch() { return keyboard_has_char; }

uint64_t keyboard_getch_syscall(uint64_t frame, uint64_t p1, uint64_t p2,
                                uint64_t p3, uint64_t p4) {
    if (keyboard_checkch()) {
        return keyboard_getch();
    }

    return -1;
}

void keyboard_init() { irq_install_handler(1, &keyboard_handler); }
