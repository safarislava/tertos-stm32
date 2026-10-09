#include "tetris.h"

#include <stdlib.h>
#include <string.h>

bool tetris_can_place(const TetrisGame *game, TetrisPieceType type, uint8_t rotation, int x, int y)
{
  const TetrisShape *shape = tetris_shape(type, rotation);
  if (shape == NULL || x < 0 || y < 0 || x > TETRIS_COLS - shape->width || y > TETRIS_ROWS - shape->height)
  {
    return false;
  }

  for (int row = 0; row < shape->height; ++row)
  {
    for (int col = 0; col < shape->width; ++col)
    {
      if (shape->cells[row][col] != 0 && game->board[y + row][x + col] != 0)
      {
        return false;
      }
    }
  }

  return true;
}

static void spawn_piece(TetrisGame *game)
{
  game->active.type = (TetrisPieceType)(rand() % TETRIS_PIECE_COUNT);
  const TetrisShape *shape = tetris_shape(game->active.type, 0);
  game->active.rotation = 0;
  game->active.x = (TETRIS_COLS - shape->width) / 2;
  game->active.y = 0;
  game->fall_elapsed_ms = 0;
  if (!tetris_can_place(game, game->active.type, 0, game->active.x, 0))
  {
    game->phase = TETRIS_GAME_OVER;
  }
}

void tetris_init(TetrisGame *game)
{
  memset(game, 0, sizeof(*game));
  game->phase = TETRIS_RUNNING;
  spawn_piece(game);
}

static uint32_t clear_lines(TetrisGame *game)
{
  uint32_t cleared = 0;
  int destination = TETRIS_ROWS - 1;

  for (int row = TETRIS_ROWS - 1; row >= 0; --row)
  {
    bool full = true;
    for (int col = 0; col < TETRIS_COLS; ++col)
    {
      if (game->board[row][col] == 0)
      {
        full = false;
        break;
      }
    }

    if (full)
    {
      ++cleared;
      continue;
    }

    if (destination != row)
    {
      memcpy(game->board[destination], game->board[row], sizeof(game->board[row]));
    }
    --destination;
  }

  while (destination >= 0)
  {
    memset(game->board[destination], 0, sizeof(game->board[destination]));
    --destination;
  }

  return cleared;
}

/* Returns true if the old piece was locked, including a game-over spawn. */
static bool step_down(TetrisGame *game)
{
  if (tetris_can_place(game, game->active.type, game->active.rotation, game->active.x, game->active.y + 1))
  {
    ++game->active.y;
    return false;
  }

  const TetrisShape *shape = tetris_shape(game->active.type, game->active.rotation);
  for (int row = 0; row < shape->height; ++row)
  {
    for (int col = 0; col < shape->width; ++col)
    {
      if (shape->cells[row][col] != 0)
      {
        game->board[game->active.y + row][game->active.x + col] = 1;
      }
    }
  }

  uint32_t cleared = clear_lines(game);
  game->score += cleared * TETRIS_POINTS_PER_LINE;
  spawn_piece(game);
  return true;
}

static bool try_position(TetrisGame *game, int x, int y, uint8_t rotation)
{
  if (!tetris_can_place(game, game->active.type, rotation, x, y))
  {
    return false;
  }

  game->active.x = x;
  game->active.y = y;
  game->active.rotation = rotation;
  return true;
}

bool tetris_command(TetrisGame *game, TetrisCommand command)
{
  if (command == TETRIS_CMD_PAUSE)
  {
    if (game->phase == TETRIS_GAME_OVER)
    {
      tetris_init(game);
    }
    else
    {
      game->phase = game->phase == TETRIS_PAUSED ? TETRIS_RUNNING : TETRIS_PAUSED;
    }
    game->fall_elapsed_ms = 0;
    return true;
  }

  if (game->phase != TETRIS_RUNNING)
  {
    return false;
  }

  switch (command)
  {
  case TETRIS_CMD_LEFT:
    return try_position(game, game->active.x - 1, game->active.y, game->active.rotation);
  case TETRIS_CMD_RIGHT:
    return try_position(game, game->active.x + 1, game->active.y, game->active.rotation);
  case TETRIS_CMD_ROTATE:
    return try_position(game, game->active.x, game->active.y,
                        (uint8_t)((game->active.rotation + 1) % TETRIS_ROTATIONS));
  case TETRIS_CMD_DOWN:
    (void)step_down(game);
    return true;
  default:
    return false;
  }
}

bool tetris_update(TetrisGame *game, uint32_t elapsed_ms)
{
  if (game->phase != TETRIS_RUNNING)
  {
    return false;
  }

  /* The wider sum also handles a long update without overflowing milliseconds. */
  uint64_t remaining = (uint64_t)game->fall_elapsed_ms + elapsed_ms;
  bool changed = false;
  while (remaining >= TETRIS_FALL_PERIOD_MS)
  {
    remaining -= TETRIS_FALL_PERIOD_MS;
    changed = true;
    if (step_down(game))
    {
      /* A new piece gets a full interval even after a delayed task update. */
      return true;
    }
  }

  game->fall_elapsed_ms = (uint32_t)remaining;
  return changed;
}

uint8_t tetris_cell(const TetrisGame *game, int x, int y)
{
  if (x < 0 || y < 0 || x >= TETRIS_COLS || y >= TETRIS_ROWS)
  {
    return 0;
  }
  if (game->board[y][x] != 0)
  {
    return 1;
  }
  if (game->phase == TETRIS_GAME_OVER)
  {
    return 0;
  }

  const TetrisShape *shape = tetris_shape(game->active.type, game->active.rotation);
  int col = x - game->active.x;
  int row = y - game->active.y;
  if (shape == NULL || col < 0 || row < 0 || col >= shape->width || row >= shape->height)
  {
    return 0;
  }

  return shape->cells[row][col];
}
