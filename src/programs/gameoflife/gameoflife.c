#include "gameoflife.h"
#include "../../kernel/interrupts/keyboard.h"
#include "../../kernel/interrupts/timer.h"
#include "../../kernel/lib/logger.h"
#include "../../kernel/lib/print.h"
#include "../../kernel/terminal/prompt.h"

#include <stdint.h>

// compute index
#define GOL_CI(i, j) ((i * GOL_HEIGHT) + j)

void gol_init_cells(uint8_t* cells) {
    for (uint16_t i = 0; i < GOL_WIDTH * GOL_HEIGHT; i++) {
        cells[i] = 0;
    }
}

uint8_t gol_get_alive_neighbors(uint8_t* cells, uint8_t i, uint8_t j) {
    uint8_t count = 0;
    // NOTE: for some reason I dont get, macros don't work.
    // uint16_t indeces[8] = {GOL_CI(i, j-1), GOL_CI(i, j+1),
    //     GOL_CI(i-1, j), GOL_CI(i+1, j), GOL_CI(i-1, j-1),
    //     GOL_CI(i-1, j+1), GOL_CI(i+1, j-1), GOL_CI(i+1, j+1)};

    uint16_t indeces[8] = {
        (i * GOL_HEIGHT + (j - 1)),       (i * GOL_HEIGHT + (j + 1)),
        ((i - 1) * GOL_HEIGHT + j),       ((i + 1) * GOL_HEIGHT + j),
        ((i - 1) * GOL_HEIGHT + (j - 1)), ((i - 1) * GOL_HEIGHT + (j + 1)),
        ((i + 1) * GOL_HEIGHT + (j - 1)), ((i + 1) * GOL_HEIGHT + (j + 1))};

    for (uint8_t i = 0; i < 8; i++) {
        uint16_t index = indeces[i];
        if (index < 0 || (index > GOL_WIDTH * GOL_HEIGHT)) {
            continue;
        }

        if (cells[index]) {
            count++;
        }
    }
    return count;
}

void gol_update(uint8_t* cells) {
    uint16_t toBeKilled[GOL_WIDTH * GOL_HEIGHT] = {0};
    uint16_t inx1 = 0;
    uint16_t toBeRevived[GOL_WIDTH * GOL_HEIGHT] = {0};
    uint16_t inx2 = 0;

    for (uint16_t i = 0; i < GOL_WIDTH; i++) {
        for (uint16_t j = 0; j < GOL_HEIGHT; j++) {
            uint16_t index = GOL_CI(i, j);
            uint8_t  neighbor_count = gol_get_alive_neighbors(cells, i, j);

            if (cells[index]) {
                if ((neighbor_count < 2) || (neighbor_count > 3)) {
                    toBeKilled[inx1] = index;
                    inx1++;
                }
            } else if (!cells[index]) {
                if (neighbor_count == 3) {
                    toBeRevived[inx2] = index;
                    inx2++;
                }
            }
        }
    }

    for (uint16_t i = 0; i < inx1; i++) {
        uint16_t index = toBeKilled[i];
        cells[index] = 0;
    }

    for (uint16_t i = 0; i < inx2; i++) {
        uint16_t index = toBeRevived[i];
        cells[index] = 1;
    }
}

void gol_draw(uint8_t* cells, uint8_t* gol_cursor) {
    char line[GOL_HEIGHT + 2] = {0};
    line[GOL_HEIGHT + 1] = '\0';
    line[GOL_HEIGHT] = '\n';
    for (uint64_t i = 0; i < GOL_WIDTH; i++) {
        for (uint16_t j = 0; j < GOL_HEIGHT; j++) {
            uint16_t index = GOL_CI(i, j);
            if (i == 0 || i == GOL_WIDTH - 1 || j == 0 || j == GOL_HEIGHT - 1) {
                line[j] = GOL_BORDER;
            } else if ((gol_cursor[0] != 0 || gol_cursor[1] != 0) &&
                       gol_cursor[0] == i && gol_cursor[1] == j) {
                line[j] = GOL_CURSOR;
            } else if (cells[index]) {
                line[j] = GOL_CELL;
            } else {
                line[j] = GOL_EMPTY;
            }
        }
        kprintf(line);
    }
}

void gol_key_handler(uint8_t* cells, uint8_t* cursor, uint8_t* state) {
    uint8_t key = keyboard_getch();

    switch (key) {
    case 'w':
        cursor[0]--;
        if (cursor[0] > GOL_WIDTH) {
            cursor[0] = GOL_WIDTH;
        }
        break;
    case 's':
        cursor[0]++;
        if (cursor[0] < 0) {
            cursor[0] = 0;
        }
        break;
    case 'a':
        cursor[1]--;
        if (cursor[1] < 0) {
            cursor[1] = 0;
        }
        break;
    case 'd':
        cursor[1]++;
        if (cursor[1] > GOL_HEIGHT) {
            cursor[1] = GOL_HEIGHT;
        }
        break;
    case 'e':
        cells[GOL_CI(cursor[0], cursor[1])] =
            !cells[GOL_CI(cursor[0], cursor[1])];
        break;
    case 'f':
        if (*state == 0) {
            *state = 1;
            cursor[0] = 0;
            cursor[1] = 0;
        } else if (*state == 1) {
            *state = 0;
            cursor[0] = GOL_WIDTH / 2;
            cursor[1] = GOL_HEIGHT / 2;
            gol_init_cells(cells);
        }
        break;
    case 'q':
        *state = 2;
    }
}

void game_loop() {
    uint64_t success;
    while (1) {
        success = prompt_lockstdin_syscall(0, 0, 0, 0, 0);
        if (success) {
            break;
        }
    }

    klog("Game of life started", "");
    uint64_t delta_time = timer_get_uptime();

    uint8_t gol_cells[GOL_WIDTH * GOL_HEIGHT] = {0};
    uint8_t gol_cursor[2] = {GOL_WIDTH / 2, GOL_HEIGHT / 2};
    uint8_t gol_state = 0; // 0 = config, 1 = run, 2 = quit

    while (1) {
        if (timer_get_uptime() - delta_time < 200) {

        } else {
            delta_time = timer_get_uptime();
            prompt_clear();
            gol_key_handler(gol_cells, gol_cursor, &gol_state);
            switch (gol_state) {
            case 1: // run mode
                gol_update(gol_cells);
                break;
            case 2: // quit mode
                goto exit;
            }
            gol_draw(gol_cells, gol_cursor);
        }
    }
exit:
    prompt_unlockstdin_syscall(0, 0, 0, 0, 0);
}
