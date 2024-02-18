#include "snake.h"
#include "../../kernel/interrupts/keyboard.h"
#include "../../kernel/interrupts/timer.h"
#include "../../kernel/lib/logger.h"
#include "../../kernel/lib/print.h"
#include "../../kernel/lib/panic.h"
#include "../../kernel/terminal/prompt.h"
#include <stdint.h>

#define SNAKE_CI(i, j) ((i * SNAKE_WIDTH) + j)

enum cell_type {
    Empty,
    Block,
    Snake_Body,
    Snake_Head,
    Fruit,
};

enum snake_dir {
    Up,
    Down,
    Right,
    Left,
    Stop,
};

void snake_init_cells(uint8_t* cells) {
    for (int16_t i = 0; i < SNAKE_WIDTH * SNAKE_HEIGHT; i++) {
        cells[i] = Empty;
    }
    cells[(SNAKE_HEIGHT*SNAKE_WIDTH)/2 - 10] = Snake_Head;
    cells[1010] = Fruit;
}

void snake_init_snake(int16_t* snake) {
    for (int16_t i = 0; i < SNAKE_MAX_SIZE; i++) {
        snake[i] = 0;
    }
    snake[0] = (SNAKE_HEIGHT*SNAKE_WIDTH)/2 - 10;
}

// void snake_draw_map() {
//     char line[SNAKE_HEIGHT + 2] = {0};
//     line[SNAKE_HEIGHT + 1] = '\0';
//     line[SNAKE_HEIGHT] = '\n';
//     for (uint64_t i = 0; i < SNAKE_WIDTH; i++) {
//         for (int16_t j = 0; j < SNAKE_HEIGHT; j++) {
//             int16_t index = SNAKE_CI(i, j);
//             if (i == 0 || i == SNAKE_WIDTH - 1 || j == 0 || j == SNAKE_HEIGHT - 1) {
//                 line[j] = SNAKE_BORDER;
//             } else {
//                 line[j] = SNAKE_EMPTY;
//             }
//         }
//         kprintf(line);
//     }
// }

void snake_draw(uint8_t* cells) {
    char line[SNAKE_WIDTH + 2] = {0};
    line[SNAKE_WIDTH + 1] = '\0';
    line[SNAKE_WIDTH] = '\n';
    for (uint64_t i = 0; i < SNAKE_HEIGHT; i++) {
        for (int16_t j = 0; j < SNAKE_WIDTH; j++) {
            int16_t index = SNAKE_CI(i, j);

            if (i == 0 || i == SNAKE_HEIGHT - 1 || j == 0 || j == SNAKE_WIDTH - 1) {
                line[j] = SNAKE_BORDER;
                continue;
            }

            switch (cells[index]) {
                case Empty:
                    line[j] = SNAKE_EMPTY;
                    break;
                case Block:
                    line[j] = SNAKE_BLOCK;
                    break;
                case Snake_Head:
                    line[j] = SNAKE_HEAD;
                    break;
                case Snake_Body:
                    line[j] = SNAKE_BODY;
                    break;
                case Fruit:
                    line[j] = SNAKE_FRUIT;
                    break;
            }
        }
        kprintf(line);
    }
}

int16_t snake_cal_next(int16_t* snake, int16_t head, enum snake_dir direction, uint8_t* state) {
    int16_t res = 0;
    int16_t h = snake[head];
    klog("cal_next", "snake[head]: %d", h);
    int16_t i = 0;
    int16_t j = 0;

    while (h > 0) {
        h -= 70;
        i++;
    }
    i -= 1;
    j = h + SNAKE_WIDTH;
    klog("cal_next", "i: %d, j: %d", i, j);

    switch (direction) {
        case Left:
            // res = SNAKE_CI(i, j-1);
            if (j-1 < 0) {
                j = SNAKE_WIDTH;
            }
            res = ((i * SNAKE_WIDTH) + j-1);
            break;
        case Right:
            // res = SNAKE_CI(i, j+1);
            if (j+1 > SNAKE_WIDTH) {
                j = 0;
            }
            res = ((i * SNAKE_WIDTH) + j+1);
            break;
        case Up:
            // res = SNAKE_CI(i-1, j);
            if (i-1 < 0) {
                i = SNAKE_HEIGHT;
            }
            res = ((i-1) * SNAKE_WIDTH) + j;
            break;
        case Down:
            // res = SNAKE_CI(i+1, j);
            if (i+1 > SNAKE_HEIGHT) {
                i = 0;
            }
            res = ((i+1) * SNAKE_WIDTH) + j;
            break;
        case Stop:
            res = 0;
            break;
    }

    klog("cal_next", "res: %d", res);
    return res;
}

void snake_update(uint8_t* cells, int16_t* snake, enum snake_dir* direction, uint8_t* state, int16_t* head, int16_t* fruit) {
    int16_t next_cell = snake_cal_next(snake, *head, *direction, state);

    if (cells[next_cell] == Snake_Body || cells[next_cell] == Block) {
        *state = 0;
        *head = 0;
        snake_init_snake(snake);
        snake_init_cells(cells);
        *direction = Stop;
    } else {
        if (cells[next_cell] == Fruit) {
            *fruit = 0;

            int16_t h = *head;
            // cells[snake[0]] = Empty;
            while ((h-1) >= 0) {
                snake[h-1] = snake[h];
                h -= 1;
            }
            *head += 1;
            snake[*head] = next_cell;
            // cells[*head] = Snake_Body;
            // cells[next_cell] = Snake_Head;

        } else {
            int16_t h = *head;
            // cells[snake[0]] = Empty;
            while ((h-1) >= 0) {
                snake[h-1] = snake[h];
                h -= 1;
            }
            snake[*head] = next_cell;
            // cells[*head] = Snake_Body;
            // cells[next_cell] = Snake_Head;
        }
    }
    klog("update", "snake[0]: %d, snake[1]: %d", snake[0], snake[1]);
    
    for (int16_t i = 0; i < SNAKE_HEIGHT*SNAKE_WIDTH; i++) {
        if (i == *fruit) {
            continue;
        }
        cells[i] = Empty;
    }

    for (int16_t i = 0; i < *head; i++) {
        int16_t index = snake[i];
        cells[index] = Snake_Body;
    }
    int16_t index = snake[*head];
    cells[index] = Snake_Head;
}

void snake_key_handler(uint8_t* cells, int16_t* snake, enum snake_dir* direction, uint8_t* state, int16_t* head, int16_t* fruit) {
    uint8_t key = keyboard_getch();

    switch (key) {
        case 'w':
            *direction = Up;
            break;
        case 's':
            *direction = Down;
            break;
        case 'a':
            *direction = Left;
            break;
        case 'd':
            *direction = Right;
            break;
        case 'f':
            if (*state == 0) {
                *state = 1;
                *direction = Up;
            } else if (*state == 1) {
                *state = 0;
                snake_init_snake(snake);
                snake_init_cells(cells);
                *head = 0;
                *direction = Stop;
                *fruit = 1010;
            }
            break;
        case 'q':
            *state = 2;
            break;
    }
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

    uint64_t delta_time = timer_get_uptime();
    uint8_t snake_state = 0; // 0: menu, 1: run, 2: quit
    int16_t head = 0;
    int16_t fruit = 1010;

    int16_t snake[SNAKE_MAX_SIZE] = {0};
    uint8_t cells[SNAKE_WIDTH*SNAKE_HEIGHT] = {0};

    enum snake_dir direction = Up;

    snake_init_snake(snake);
    snake_init_cells(cells);
    cells[fruit] = Fruit;

    while(1) {
        if (timer_get_uptime() - delta_time < 50) {

        } else {
            delta_time = timer_get_uptime();
            prompt_clear();
            snake_key_handler(cells, snake, &direction, &snake_state, &head, &fruit);
            switch(snake_state) {
            case 1: // run mode
                snake_update(cells, snake, &direction, &snake_state, &head, &fruit);
                break;
            case 2: // quit mode
                goto exit;
            }
            snake_draw(cells);
        }
    }

    
exit:
    prompt_unlockstdin_syscall(0, 0, 0, 0, 0);
}
