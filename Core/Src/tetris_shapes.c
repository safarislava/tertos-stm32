#include "tetris.h"

#include <stddef.h>

/* Clockwise rotations. Unspecified cells are zero-initialized. */
static const TetrisShape shapes[TETRIS_PIECE_COUNT][TETRIS_ROTATIONS] = {
  [TETRIS_I] = {
    {.cells = {{1, 1, 1, 1}}, .width = 4, .height = 1},
    {.cells = {{1}, {1}, {1}, {1}}, .width = 1, .height = 4},
    {.cells = {{1, 1, 1, 1}}, .width = 4, .height = 1},
    {.cells = {{1}, {1}, {1}, {1}}, .width = 1, .height = 4},
  },
  [TETRIS_O] = {
    {.cells = {{1, 1}, {1, 1}}, .width = 2, .height = 2},
    {.cells = {{1, 1}, {1, 1}}, .width = 2, .height = 2},
    {.cells = {{1, 1}, {1, 1}}, .width = 2, .height = 2},
    {.cells = {{1, 1}, {1, 1}}, .width = 2, .height = 2},
  },
  [TETRIS_T] = {
    {.cells = {{0, 1, 0}, {1, 1, 1}}, .width = 3, .height = 2},
    {.cells = {{1, 0}, {1, 1}, {1, 0}}, .width = 2, .height = 3},
    {.cells = {{1, 1, 1}, {0, 1, 0}}, .width = 3, .height = 2},
    {.cells = {{0, 1}, {1, 1}, {0, 1}}, .width = 2, .height = 3},
  },
  [TETRIS_S] = {
    {.cells = {{0, 1, 1}, {1, 1, 0}}, .width = 3, .height = 2},
    {.cells = {{1, 0}, {1, 1}, {0, 1}}, .width = 2, .height = 3},
    {.cells = {{0, 1, 1}, {1, 1, 0}}, .width = 3, .height = 2},
    {.cells = {{1, 0}, {1, 1}, {0, 1}}, .width = 2, .height = 3},
  },
  [TETRIS_Z] = {
    {.cells = {{1, 1, 0}, {0, 1, 1}}, .width = 3, .height = 2},
    {.cells = {{0, 1}, {1, 1}, {1, 0}}, .width = 2, .height = 3},
    {.cells = {{1, 1, 0}, {0, 1, 1}}, .width = 3, .height = 2},
    {.cells = {{0, 1}, {1, 1}, {1, 0}}, .width = 2, .height = 3},
  },
  [TETRIS_J] = {
    {.cells = {{1, 0, 0}, {1, 1, 1}}, .width = 3, .height = 2},
    {.cells = {{1, 1}, {1, 0}, {1, 0}}, .width = 2, .height = 3},
    {.cells = {{1, 1, 1}, {0, 0, 1}}, .width = 3, .height = 2},
    {.cells = {{0, 1}, {0, 1}, {1, 1}}, .width = 2, .height = 3},
  },
  [TETRIS_L] = {
    {.cells = {{0, 0, 1}, {1, 1, 1}}, .width = 3, .height = 2},
    {.cells = {{1, 0}, {1, 0}, {1, 1}}, .width = 2, .height = 3},
    {.cells = {{1, 1, 1}, {1, 0, 0}}, .width = 3, .height = 2},
    {.cells = {{1, 1}, {0, 1}, {0, 1}}, .width = 2, .height = 3},
  },
};

const TetrisShape *tetris_shape(TetrisPieceType type, uint8_t rotation)
{
  if ((unsigned int)type >= TETRIS_PIECE_COUNT || rotation >= TETRIS_ROTATIONS)
  {
    return NULL;
  }

  return &shapes[type][rotation];
}
