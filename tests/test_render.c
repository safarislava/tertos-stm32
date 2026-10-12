#include "tetris.h"
#include "fonts.h"
#include "oled.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Mock HAL I2C handle */
I2C_HandleTypeDef hi2c1;

HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
  (void)hi2c; (void)DevAddress; (void)MemAddress; (void)MemAddSize; (void)pData; (void)Size; (void)Timeout;
  return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
  (void)hi2c; (void)DevAddress; (void)MemAddress; (void)MemAddSize; (void)pData; (void)Size; (void)Timeout;
  if (pData != NULL && Size > 0)
  {
    memset(pData, 0xFF, Size);
  }
  return HAL_OK;
}

void HAL_Delay(uint32_t Delay)
{
  (void)Delay;
}

void UART_Transmit(uint8_t *data)
{
  (void)data;
}

int main(void)
{
  printf("Testing OLED initialization...\n");
  assert(oled_Init() == 1);

  TetrisGame game;
  srand(42);
  tetris_init(&game);

  printf("Testing render RUNNING state...\n");
  tetris_render(&game);

  printf("Testing render PAUSED state...\n");
  game.phase = TETRIS_PAUSED;
  tetris_render(&game);

  printf("Testing render GAME_OVER state...\n");
  game.phase = TETRIS_GAME_OVER;
  game.score = 12300;
  tetris_render(&game);

  printf("Testing filled rectangle...\n");
  oled_DrawFilledRectangle(10, 10, 20, 20, White);

  printf("All OLED render tests passed successfully!\n");
  return 0;
}
