#include "tetris.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned int occupied_cells(const TetrisGame *game)
{
  unsigned int count = 0;
  for (int y = 0; y < TETRIS_ROWS; ++y)
  {
    for (int x = 0; x < TETRIS_COLS; ++x)
    {
      count += game->board[y][x] != 0;
    }
  }
  return count;
}

static TetrisGame game_with_piece(TetrisPieceType type, uint8_t rotation, int x, int y)
{
  TetrisGame game;
  srand(12345u);
  tetris_init(&game);
  game.active.type = type;
  game.active.rotation = rotation;
  game.active.x = x;
  game.active.y = y;
  assert(tetris_can_place(&game, type, rotation, x, y));
  return game;
}

static void test_shape_geometry(void)
{
  for (int type = 0; type < TETRIS_PIECE_COUNT; ++type)
  {
    for (uint8_t rotation = 0; rotation < TETRIS_ROTATIONS; ++rotation)
    {
      const TetrisShape *shape = tetris_shape((TetrisPieceType)type, rotation);
      const TetrisShape *next = tetris_shape((TetrisPieceType)type, (uint8_t)((rotation + 1) % TETRIS_ROTATIONS));
      assert(shape != NULL);
      assert(shape->width >= 1 && shape->width <= TETRIS_SHAPE_SIZE);
      assert(shape->height >= 1 && shape->height <= TETRIS_SHAPE_SIZE);
      unsigned int blocks = 0;
      bool touches_top = false;
      bool touches_left = false;
      bool touches_bottom = false;
      bool touches_right = false;
      for (int y = 0; y < TETRIS_SHAPE_SIZE; ++y)
      {
        for (int x = 0; x < TETRIS_SHAPE_SIZE; ++x)
        {
          assert(shape->cells[y][x] <= 1);
          if (shape->cells[y][x] == 0)
          {
            continue;
          }
          assert(x < shape->width && y < shape->height);
          ++blocks;
          touches_top |= y == 0;
          touches_left |= x == 0;
          touches_bottom |= y == shape->height - 1;
          touches_right |= x == shape->width - 1;
        }
      }
      assert(blocks == 4);
      assert(touches_top && touches_left && touches_bottom && touches_right);

      /* Validate the precomputed data against a geometric clockwise rotation. */
      assert(next->width == shape->height && next->height == shape->width);
      for (int y = 0; y < shape->height; ++y)
      {
        for (int x = 0; x < shape->width; ++x)
        {
          assert(next->cells[x][shape->height - 1 - y] == shape->cells[y][x]);
        }
      }
    }
  }
  assert(tetris_shape((TetrisPieceType)-1, 0) == NULL);
  assert(tetris_shape(TETRIS_PIECE_COUNT, 0) == NULL);
  assert(tetris_shape(TETRIS_I, TETRIS_ROTATIONS) == NULL);
}

static void test_initial_state_and_centered_spawns(void)
{
  unsigned int seen = 0;
  srand(42);
  for (int spawn = 0; spawn < 256; ++spawn)
  {
    TetrisGame game;
    tetris_init(&game);
    const TetrisShape *shape = tetris_shape(game.active.type, 0);
    assert(occupied_cells(&game) == 0);
    assert(game.phase == TETRIS_RUNNING);
    assert(game.active.rotation == 0 && game.active.y == 0);
    assert(game.active.x == (TETRIS_COLS - shape->width) / 2);
    assert(game.score == 0 && game.fall_elapsed_ms == 0);
    seen |= 1u << game.active.type;
  }
  assert(seen == (1u << TETRIS_PIECE_COUNT) - 1u);

  TetrisGame first;
  TetrisGame second;
  srand(42);
  tetris_init(&first);
  srand(42);
  tetris_init(&second);
  assert(first.active.type == second.active.type);
}

static void test_bounds_and_shape_holes(void)
{
  TetrisGame game = game_with_piece(TETRIS_T, 0, 3, 4);
  assert(!tetris_can_place(&game, TETRIS_T, 0, -1, 4));
  assert(!tetris_can_place(&game, TETRIS_T, 0, 8, 4));
  assert(!tetris_can_place(&game, TETRIS_T, 0, 3, -1));
  assert(!tetris_can_place(&game, TETRIS_T, 0, 3, 19));
  assert(!tetris_can_place(&game, TETRIS_T, 0, INT_MAX, INT_MAX));
  assert(!tetris_can_place(&game, TETRIS_T, 0, INT_MIN, INT_MIN));
  assert(!tetris_can_place(&game, TETRIS_PIECE_COUNT, 0, 0, 0));
  assert(!tetris_can_place(&game, TETRIS_T, 4, 0, 0));

  game.board[4][3] = 1; /* Empty corner of the T bounding box. */
  assert(tetris_can_place(&game, TETRIS_T, 0, 3, 4));
  game.board[4][4] = 1; /* An occupied T cell. */
  assert(!tetris_can_place(&game, TETRIS_T, 0, 3, 4));
}

static void test_movement_and_walls(void)
{
  TetrisGame game = game_with_piece(TETRIS_O, 0, 0, 5);
  TetrisGame before;
  memcpy(&before, &game, sizeof(before));
  assert(!tetris_command(&game, TETRIS_CMD_LEFT));
  assert(memcmp(&before, &game, sizeof(game)) == 0);
  assert(tetris_command(&game, TETRIS_CMD_RIGHT));
  assert(game.active.x == 1 && game.active.y == 5);
  assert(occupied_cells(&game) == 0);

  game.board[5][3] = 1;
  assert(!tetris_command(&game, TETRIS_CMD_RIGHT));
  assert(game.active.x == 1);
  assert(tetris_command(&game, TETRIS_CMD_LEFT));
  assert(game.active.x == 0);

  game.active.x = TETRIS_COLS - 2;
  assert(!tetris_command(&game, TETRIS_CMD_RIGHT));
}

static void test_rotation_cycle(void)
{
  TetrisGame game = game_with_piece(TETRIS_J, 0, 3, 4);
  for (uint8_t rotation = 1; rotation <= TETRIS_ROTATIONS; ++rotation)
  {
    assert(tetris_command(&game, TETRIS_CMD_ROTATE));
    assert(game.active.rotation == rotation % TETRIS_ROTATIONS);
    assert(game.active.x == 3 && game.active.y == 4);
    assert(occupied_cells(&game) == 0);
  }

  game = game_with_piece(TETRIS_O, 0, 3, 4);
  assert(tetris_command(&game, TETRIS_CMD_ROTATE));
  assert(game.active.rotation == 1);
}

static void test_rejected_rotations(void)
{
  TetrisGame game = game_with_piece(TETRIS_I, 0, 3, TETRIS_ROWS - 1);
  assert(!tetris_command(&game, TETRIS_CMD_ROTATE));
  assert(game.active.rotation == 0 && game.active.y == TETRIS_ROWS - 1);

  game = game_with_piece(TETRIS_I, 1, TETRIS_COLS - 1, 3);
  assert(!tetris_command(&game, TETRIS_CMD_ROTATE));
  assert(game.active.rotation == 1 && game.active.x == TETRIS_COLS - 1);

  game = game_with_piece(TETRIS_I, 0, 3, 3);
  game.board[4][3] = 1;
  assert(!tetris_command(&game, TETRIS_CMD_ROTATE));
  assert(game.active.rotation == 0 && game.board[4][3] == 1);
}

static void test_gravity_and_remainder(void)
{
  TetrisGame game = game_with_piece(TETRIS_O, 0, 4, 0);
  assert(!tetris_update(&game, 0));
  assert(!tetris_update(&game, 499));
  assert(game.active.y == 0 && game.fall_elapsed_ms == 499);
  assert(tetris_update(&game, 1));
  assert(game.active.y == 1 && game.fall_elapsed_ms == 0);
  assert(tetris_update(&game, 1249));
  assert(game.active.y == 3 && game.fall_elapsed_ms == 249);
  assert(tetris_update(&game, 251));
  assert(game.active.y == 4 && game.fall_elapsed_ms == 0);
  assert(occupied_cells(&game) == 0);
}

static void test_input_preserves_gravity_deadline(void)
{
  TetrisGame game = game_with_piece(TETRIS_J, 0, 3, 0);
  assert(!tetris_update(&game, 499));
  assert(tetris_command(&game, TETRIS_CMD_LEFT));
  assert(tetris_command(&game, TETRIS_CMD_ROTATE));
  assert(tetris_command(&game, TETRIS_CMD_DOWN));
  assert(game.fall_elapsed_ms == 499);
  assert(tetris_update(&game, 1));
  assert(game.active.y == 2);
}

static void test_lock_and_next_piece(void)
{
  TetrisGame game = game_with_piece(TETRIS_O, 0, 4, TETRIS_ROWS - 2);
  srand(42);
  TetrisPieceType expected = (TetrisPieceType)(rand() % TETRIS_PIECE_COUNT);
  srand(42);
  game.fall_elapsed_ms = 499;
  assert(tetris_command(&game, TETRIS_CMD_DOWN));
  assert(occupied_cells(&game) == 4);
  assert(game.board[18][4] == 1 && game.board[18][5] == 1);
  assert(game.board[19][4] == 1 && game.board[19][5] == 1);
  assert(game.active.type == expected && game.active.y == 0);
  assert(game.active.x == (TETRIS_COLS - tetris_shape(expected, 0)->width) / 2);
  assert(game.phase == TETRIS_RUNNING);
  assert(game.fall_elapsed_ms == 0);
  assert(!tetris_update(&game, 499));
  assert(game.active.y == 0);
  assert(tetris_update(&game, 1));
  assert(game.active.y == 1);
}

static void test_collision_at_different_heights(void)
{
  TetrisGame game = game_with_piece(TETRIS_O, 0, 5, 6);
  game.board[2][1] = 1; /* A tall stack in another column does not stop this piece. */
  assert(tetris_can_place(&game, TETRIS_O, 0, 5, 7));
  game.board[8][5] = 1;
  assert(tetris_command(&game, TETRIS_CMD_DOWN));
  assert(game.board[2][1] == 1 && game.board[8][5] == 1);
  assert(game.board[6][5] == 1 && game.board[7][6] == 1);
  assert(occupied_cells(&game) == 6);
  assert(game.active.y == 0);
}

static void test_one_to_four_lines(void)
{
  for (uint32_t lines = 1; lines <= 4; ++lines)
  {
    TetrisGame game = game_with_piece(TETRIS_I, 1, 0, TETRIS_ROWS - 4);
    for (int row = TETRIS_ROWS - (int)lines; row < TETRIS_ROWS; ++row)
    {
      for (int col = 1; col < TETRIS_COLS; ++col)
      {
        game.board[row][col] = 1;
      }
    }
    game.board[10][7] = 1;
    game.score = 500;
    assert(tetris_command(&game, TETRIS_CMD_DOWN));
    assert(game.score == 500 + lines * TETRIS_POINTS_PER_LINE);
    assert(game.board[10 + lines][7] == 1);
    assert(occupied_cells(&game) == 1 + 4 - lines);
    for (uint32_t row = 0; row < lines; ++row)
    {
      for (int col = 0; col < TETRIS_COLS; ++col)
      {
        assert(game.board[row][col] == 0);
      }
    }
  }
}

static void test_nonadjacent_lines(void)
{
  TetrisGame game = game_with_piece(TETRIS_I, 1, 0, 16);
  for (int col = 1; col < TETRIS_COLS; ++col)
  {
    game.board[17][col] = 1;
    game.board[19][col] = 1;
  }
  game.board[5][7] = 1;
  assert(tetris_command(&game, TETRIS_CMD_DOWN));
  assert(game.score == 200);
  assert(game.board[7][7] == 1);
  assert(game.board[18][0] == 1 && game.board[19][0] == 1);
  assert(occupied_cells(&game) == 3);
}

static void test_game_over_and_restart(void)
{
  TetrisGame game = game_with_piece(TETRIS_O, 0, 0, 18);
  for (int x = 3; x <= 6; ++x)
  {
    game.board[0][x] = 1; /* Blocks any centered spawn. */
  }
  game.score = 300;
  assert(tetris_command(&game, TETRIS_CMD_DOWN));
  assert(game.phase == TETRIS_GAME_OVER);
  assert(occupied_cells(&game) == 8);
  assert(!tetris_update(&game, UINT32_MAX));
  assert(!tetris_command(&game, TETRIS_CMD_LEFT));
  assert(!tetris_command(&game, TETRIS_CMD_DOWN));
  assert(tetris_cell(&game, 4, 1) == 0);

  assert(tetris_command(&game, TETRIS_CMD_PAUSE));
  assert(game.phase == TETRIS_RUNNING);
  assert(occupied_cells(&game) == 0 && game.score == 0);
  assert(game.fall_elapsed_ms == 0);
}

static void test_pause_and_resume(void)
{
  TetrisGame game = game_with_piece(TETRIS_O, 0, 4, 3);
  assert(!tetris_update(&game, 499));
  assert(tetris_command(&game, TETRIS_CMD_PAUSE));
  assert(game.phase == TETRIS_PAUSED && game.fall_elapsed_ms == 0);
  assert(!tetris_update(&game, 10000));
  assert(!tetris_command(&game, TETRIS_CMD_LEFT));
  assert(!tetris_command(&game, TETRIS_CMD_ROTATE));
  assert(!tetris_command(&game, TETRIS_CMD_DOWN));
  assert(game.active.x == 4 && game.active.y == 3);
  assert(tetris_command(&game, TETRIS_CMD_PAUSE));
  assert(game.phase == TETRIS_RUNNING);
  assert(!tetris_update(&game, 499));
  assert(tetris_update(&game, 1));
  assert(game.active.y == 4);

  game.board[15][1] = 1;
  game.score = 200;
  assert(tetris_command(&game, TETRIS_CMD_PAUSE));
  tetris_init(&game);
  assert(game.phase == TETRIS_RUNNING && occupied_cells(&game) == 0);
  assert(game.score == 0);
}

static void test_long_update_does_not_drop_new_piece(void)
{
  TetrisGame game = game_with_piece(TETRIS_O, 0, 4, 0);
  assert(!tetris_update(&game, 499));
  assert(tetris_update(&game, UINT32_MAX));
  assert(occupied_cells(&game) == 4);
  assert(game.active.y == 0 && game.fall_elapsed_ms == 0);
  assert(game.phase == TETRIS_RUNNING);
}

static void test_cells_and_game_copy(void)
{
  TetrisGame game = game_with_piece(TETRIS_O, 0, 4, 2);
  game.board[10][1] = 1;
  TetrisGame snapshot = game;
  unsigned int blocks = 0;
  for (int y = 0; y < TETRIS_ROWS; ++y)
  {
    for (int x = 0; x < TETRIS_COLS; ++x)
    {
      blocks += tetris_cell(&snapshot, x, y);
    }
  }
  assert(blocks == 5);
  assert(snapshot.board[2][4] == 0 && tetris_cell(&snapshot, 4, 2) == 1);
  assert(tetris_cell(&snapshot, -1, 0) == 0);
  assert(tetris_cell(&snapshot, TETRIS_COLS, 0) == 0);
  assert(tetris_cell(&snapshot, 0, TETRIS_ROWS) == 0);
  assert(tetris_cell(&snapshot, INT_MAX, INT_MIN) == 0);

  game.board[10][1] = 0;
  assert(tetris_command(&game, TETRIS_CMD_LEFT));
  assert(snapshot.board[10][1] == 1 && snapshot.active.x == 4);
  assert(tetris_cell(&snapshot, 5, 2) == 1);
}

static void test_random_play_invariants(void)
{
  TetrisGame game;
  srand(54321);
  tetris_init(&game);
  uint32_t choices = 123456789;
  for (int iteration = 0; iteration < 20000; ++iteration)
  {
    choices = choices * 1664525u + 1013904223u;
    (void)tetris_command(&game, (TetrisCommand)(choices % 5));
    (void)tetris_update(&game, choices % 2000);
    assert(game.fall_elapsed_ms < TETRIS_FALL_PERIOD_MS);
    if (game.phase != TETRIS_GAME_OVER)
    {
      assert(tetris_can_place(&game, game.active.type, game.active.rotation, game.active.x, game.active.y));
    }
    for (int y = 0; y < TETRIS_ROWS; ++y)
    {
      unsigned int filled = 0;
      for (int x = 0; x < TETRIS_COLS; ++x)
      {
        assert(game.board[y][x] <= 1);
        filled += game.board[y][x];
      }
      assert(filled < TETRIS_COLS);
    }
  }
}

#define RUN_TEST(name) do { name(); puts(#name " passed"); } while (0)

int main(void)
{
  RUN_TEST(test_shape_geometry);
  RUN_TEST(test_initial_state_and_centered_spawns);
  RUN_TEST(test_bounds_and_shape_holes);
  RUN_TEST(test_movement_and_walls);
  RUN_TEST(test_rotation_cycle);
  RUN_TEST(test_rejected_rotations);
  RUN_TEST(test_gravity_and_remainder);
  RUN_TEST(test_input_preserves_gravity_deadline);
  RUN_TEST(test_lock_and_next_piece);
  RUN_TEST(test_collision_at_different_heights);
  RUN_TEST(test_one_to_four_lines);
  RUN_TEST(test_nonadjacent_lines);
  RUN_TEST(test_game_over_and_restart);
  RUN_TEST(test_pause_and_resume);
  RUN_TEST(test_long_update_does_not_drop_new_piece);
  RUN_TEST(test_cells_and_game_copy);
  RUN_TEST(test_random_play_invariants);
  puts("All 17 Tetris tests passed.");
  return 0;
}
