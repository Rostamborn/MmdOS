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

int16_t snake_rand(int16_t min, int16_t max) {
    static uint16_t seed = 0;
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return min + (seed % (max - min + 1));
}

int16_t snake_new_fruit_pos() {
    int16_t i = snake_rand(1, SNAKE_HEIGHT-1);
    int16_t j = snake_rand(1, SNAKE_WIDTH-1);

    return (i * SNAKE_WIDTH) + j;
}

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

void snake_restart(uint8_t* cells, int16_t* snake, uint8_t* state, int16_t* head, enum snake_dir* direction, int16_t* fruit) {
    *state = 0;
    snake_init_snake(snake);
    snake_init_cells(cells);
    *head = 0;
    *direction = Stop;
    *fruit = 1010;
}

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
    int16_t i = 0;
    int16_t j = 0;

    while (h > 0) {
        h -= 70;
        i++;
    }
    i -= 1;
    j = h + SNAKE_WIDTH;

    switch (direction) {
        case Left:
            if (j-1 < 0) {
                j = SNAKE_WIDTH;
            }
            res = ((i * SNAKE_WIDTH) + j-1);
            break;
        case Right:
            if (j+1 > SNAKE_WIDTH) {
                j = 0;
            }
            res = ((i * SNAKE_WIDTH) + j+1);
            break;
        case Up:
            if (i-1 < 0) {
                i = SNAKE_HEIGHT;
            }
            res = ((i-1) * SNAKE_WIDTH) + j;
            break;
        case Down:
            if (i+1 > SNAKE_HEIGHT) {
                i = 0;
            }
            res = ((i+1) * SNAKE_WIDTH) + j;
            break;
        case Stop:
            res = 0;
            break;
    }

    return res;
}

void snake_update(uint8_t* cells, int16_t* snake, enum snake_dir* direction, uint8_t* state, int16_t* head, int16_t* fruit) {
    int16_t next_cell = snake_cal_next(snake, *head, *direction, state);

    if (cells[next_cell] == Snake_Body || cells[next_cell] == Block) {
        snake_restart(cells, snake, state, head, direction, fruit);
    } else {
        if (cells[next_cell] == Fruit) {
            *fruit = snake_new_fruit_pos();
            *head += 1;
            snake[*head] = next_cell;

        } else {
            int16_t h = 0;
            while (h < *head) {
                snake[h] = snake[h+1];
                h += 1;
            }
            snake[*head] = next_cell;
        }
    }
    
    for (int16_t i = 0; i < SNAKE_HEIGHT*SNAKE_WIDTH; i++) {
        cells[i] = Empty;
    }

    for (int16_t i = 0; i < *head; i++) {
        int16_t index = snake[i];
        cells[index] = Snake_Body;
    }
    int16_t index = snake[*head];
    cells[index] = Snake_Head;
    cells[*fruit] = Fruit;
}

void snake_key_handler(uint8_t* cells, int16_t* snake, enum snake_dir* direction, uint8_t* state, int16_t* head, int16_t* fruit) {
    uint8_t key = keyboard_getch();

    switch (key) {
        case 'w':
            if (*direction != Down) {
                *direction = Up;
            }
            break;
        case 's':
            if (*direction != Up) {
                *direction = Down;
            }
            break;
        case 'a':
            if (*direction != Right) {
                *direction = Left;
            }
            break;
        case 'd':
            if (*direction != Left) {
                *direction = Right;
            }
            break;
        case 'f':
            if (*state == 0) {
                *state = 1;
                *direction = Up;
            } else if (*state == 1) {
                snake_restart(cells, snake, state, head, direction, fruit);
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
        if (timer_get_uptime() - delta_time < 70) {

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
