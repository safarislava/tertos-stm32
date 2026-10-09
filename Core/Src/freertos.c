/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "kb.h"
#include "queue.h"
#include "tetris_runtime.h"
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
static QueueHandle_t tetrisGameQueueHandle;
static TetrisGame tetrisGame;
/* USER CODE END Variables */
osThreadId Task_UserInputHandle;
osThreadId Task_GameHandle;
osMessageQId userInputQueueHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static bool apply_button(enum Button key);
/* USER CODE END FunctionPrototypes */

void StartTaskUserInput(void const * argument);
void StartTaskGame(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{
}

__weak unsigned long getRunTimeCounterValue(void)
{
  return 0;
}
/* USER CODE END 1 */

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of userInputQueue */
  osMessageQDef(userInputQueue, 16, uint16_t);
  userInputQueueHandle = osMessageCreate(osMessageQ(userInputQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  tetrisGameQueueHandle = xQueueCreate(1, sizeof(TetrisGame));
  configASSERT(userInputQueueHandle != NULL);
  configASSERT(tetrisGameQueueHandle != NULL);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of Task_UserInput */
  osThreadDef(Task_UserInput, StartTaskUserInput, osPriorityNormal, 0, 128);
  Task_UserInputHandle = osThreadCreate(osThread(Task_UserInput), NULL);

  /* definition and creation of Task_Game */
  osThreadDef(Task_Game, StartTaskGame, osPriorityNormal, 0, 512);
  Task_GameHandle = osThreadCreate(osThread(Task_Game), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  configASSERT(Task_UserInputHandle != NULL);
  configASSERT(Task_GameHandle != NULL);
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartTaskUserInput */
/**
  * @brief  Function implementing the Task_UserInput thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartTaskUserInput */
void StartTaskUserInput(void const * argument)
{
  /* USER CODE BEGIN StartTaskUserInput */
  (void)argument;
  /* Infinite loop */
  while (1)
  {
    enum Button key = Get_Char();
    if (key != B_NONE)
    {
      osMessagePut(userInputQueueHandle, key,10);
    }
    osDelay(1);
  }
  /* USER CODE END StartTaskUserInput */
}

/* USER CODE BEGIN Header_StartTaskGame */
/**
* @brief Function implementing the Task_Game thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskGame */
void StartTaskGame(void const * argument)
{
  /* USER CODE BEGIN StartTaskGame */
  (void)argument;
  srand(HAL_GetTick());
  tetris_init(&tetrisGame);
  xQueueOverwrite(tetrisGameQueueHandle, &tetrisGame);

  uint32_t previousTime = HAL_GetTick();
  while (1)
  {
    /* Wake for input or at least every 10 ms to advance gravity. */
    osEvent event = osMessageGet(userInputQueueHandle, 10);
    uint32_t now = HAL_GetTick();
    uint32_t elapsedMs = now - previousTime;
    previousTime = now;

    /* Account for elapsed time before applying the command received now. */
    bool changed = tetris_update(&tetrisGame, elapsedMs);
    if (event.status == osEventMessage)
    {
      changed = apply_button((enum Button)event.value.v) || changed;
    }
    if (changed)
    {
      xQueueOverwrite(tetrisGameQueueHandle, &tetrisGame);
    }
  }
  /* USER CODE END StartTaskGame */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
static bool apply_button(enum Button key)
{
  switch (key)
  {
  case B_UP:
    return tetris_command(&tetrisGame, TETRIS_CMD_ROTATE);
  case B_DOWN:
    return tetris_command(&tetrisGame, TETRIS_CMD_DOWN);
  case B_LEFT:
    return tetris_command(&tetrisGame, TETRIS_CMD_LEFT);
  case B_RIGHT:
    return tetris_command(&tetrisGame, TETRIS_CMD_RIGHT);
  case B_PAUSE:
    return tetris_command(&tetrisGame, TETRIS_CMD_PAUSE);
  default:
    return false;
  }
}

bool tetris_receive_game(TetrisGame *game, TickType_t wait_ticks)
{
  return xQueueReceive(tetrisGameQueueHandle, game, wait_ticks) == pdPASS;
}
/* USER CODE END Application */
