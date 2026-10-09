#ifndef TETRIS_H
#define TETRIS_H

#include <stdbool.h>
#include <stdint.h>

#define TETRIS_COLS 10
#define TETRIS_ROWS 20
#define TETRIS_PIECE_COUNT 7
#define TETRIS_ROTATIONS 4
#define TETRIS_SHAPE_SIZE 4
#define TETRIS_FALL_PERIOD_MS 500u
#define TETRIS_POINTS_PER_LINE 100u

typedef enum
{
  TETRIS_I,
  TETRIS_O,
  TETRIS_T,
  TETRIS_S,
  TETRIS_Z,
  TETRIS_J,
  TETRIS_L
} TetrisPieceType;

/* Each rotation is aligned to the top-left of its occupied bounding box. */
typedef struct
{
  uint8_t cells[TETRIS_SHAPE_SIZE][TETRIS_SHAPE_SIZE];
  uint8_t width;
  uint8_t height;
} TetrisShape;

typedef struct
{
  TetrisPieceType type;
  uint8_t rotation;
  int x;
  int y;
} TetrisActivePiece;

typedef enum
{
  TETRIS_RUNNING,
  TETRIS_PAUSED,
  TETRIS_GAME_OVER
} TetrisPhase;

/* Hardware button codes are translated to these commands by Task_Game. */
typedef enum
{
  TETRIS_CMD_LEFT,
  TETRIS_CMD_RIGHT,
  TETRIS_CMD_ROTATE,
  TETRIS_CMD_DOWN,
  TETRIS_CMD_PAUSE
} TetrisCommand;

/* Owned exclusively by the game task. board contains only locked blocks. */
typedef struct
{
  uint8_t board[TETRIS_ROWS][TETRIS_COLS];
  TetrisActivePiece active;
  TetrisPhase phase;
  uint32_t score;
  uint32_t fall_elapsed_ms;
} TetrisGame;

/* Returns NULL for an invalid piece type or rotation. */
const TetrisShape *tetris_shape(TetrisPieceType type, uint8_t rotation);

void tetris_init(TetrisGame *game);
bool tetris_can_place(const TetrisGame *game, TetrisPieceType type, uint8_t rotation, int x, int y);

/* true means the command was applied and the game copy should be published.
 * DOWN locks a piece if it cannot move down. PAUSE restarts after game over.
 * Movement is ignored while paused or after game over.
 */
bool tetris_command(TetrisGame *game, TetrisCommand command);

/* elapsed_ms is real elapsed time, not the number of task-loop iterations.
 * Delayed updates catch up the current piece, but never fast-forward a newly
 * spawned piece. Pausing/resuming and spawning reset the fall interval.
 */
bool tetris_update(TetrisGame *game, uint32_t elapsed_ms);

/* Combines locked blocks and the active piece; coordinates are in cells. */
uint8_t tetris_cell(const TetrisGame *game, int x, int y);

#endif /* TETRIS_H */
