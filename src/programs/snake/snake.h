#ifndef SNAKE_H
#define SNAKE_H

#define SNAKE_WIDTH 70
#define SNAKE_HEIGHT 40

#define SNAKE_MAX_SIZE (SNAKE_WIDTH*SNAKE_HEIGHT)

#define SNAKE_BORDER '*'
#define SNAKE_EMPTY ' '
#define SNAKE_BLOCK '#'
#define SNAKE_HEAD '@'
#define SNAKE_BODY '*'
#define SNAKE_FRUIT '$'

void snake_game_loop();

#endif
