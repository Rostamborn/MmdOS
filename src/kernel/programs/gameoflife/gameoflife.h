#ifndef GAMEOFLIFE_H
#define GAMEOFLIFE_H

#include <stdint.h>
#define GOL_WIDTH 40
#define GOL_HEIGHT 70
#define GOL_BORDER '*'
#define GOL_CELL '#'
#define GOL_EMPTY ' '
#define GOL_CURSOR 'X'

// typedef struct {
//     uint8_t alive;
// } gol_cell;

void game_loop();

#endif
