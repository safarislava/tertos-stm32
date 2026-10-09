#ifndef TETRIS_RUNTIME_H
#define TETRIS_RUNTIME_H

#include "FreeRTOS.h"
#include "tetris.h"

/* Receives the latest game snapshot in the display task.
 * wait_ticks is a FreeRTOS timeout, e.g. portMAX_DELAY or pdMS_TO_TICKS(100).
 */
bool tetris_receive_game(TetrisGame *game, TickType_t wait_ticks);

#endif /* TETRIS_RUNTIME_H */
