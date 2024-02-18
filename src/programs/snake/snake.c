#include "snake.h"
#include "../../kernel/interrupts/keyboard.h"
#include "../../kernel/interrupts/timer.h"
#include "../../kernel/lib/logger.h"
#include "../../kernel/lib/print.h"
#include "../../kernel/terminal/prompt.h"
#include <stdint.h>

void snake_draw() {

}

void snake_update() {

}

void snake_game_loop() {
    uint64_t success;
    while (1) {
        success = prompt_lockstdin_syscall(0, 0, 0, 0, 0);
        if (success) {
            break;
        }
    }

    klog("Snake Started", "");

    uint8_t gol_cells[SNAKE_WIDTH * SNAKE_HEIGHT] = {0};
    uint64_t delta_time = timer_get_uptime();
    uint8_t snake_state = 0;

    while(1) {
        if (timer_get_uptime() - delta_time < 200) {

        } else {
            delta_time = timer_get_uptime();
            prompt_clear();
            snake_key_handler();
            switch(snake_state) {
            case 1: // run mode
                break;
            case 2: // quit mode
                goto exit
            }

        }
    }

    
exit:
    prompt_unlockstdin_syscall(0, 0, 0, 0, 0);
}
