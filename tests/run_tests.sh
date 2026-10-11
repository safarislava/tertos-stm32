#!/bin/sh
set -eu

task_test_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
task_repo_dir=$(dirname -- "$task_test_dir")
task_build_dir=$(mktemp -d "${TMPDIR:-/tmp}/tetris-tests.XXXXXX")
trap 'rm -rf "$task_build_dir"' EXIT HUP INT TERM

"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -Wpedantic -g \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I"$task_repo_dir/Core/Inc" \
  "$task_repo_dir/Core/Src/tetris.c" \
  "$task_repo_dir/Core/Src/tetris_shapes.c" \
  "$task_test_dir/tetris_test.c" \
  -o "$task_build_dir/tetris_test"

"$task_build_dir/tetris_test"

"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -g \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I"$task_repo_dir/Core/Inc" \
  -I"$task_repo_dir/Middlewares/Third_Party/FreeRTOS/Source/include" \
  -I"$task_repo_dir/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" \
  -I"$task_repo_dir/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F" \
  -I"$task_repo_dir/Drivers/STM32F4xx_HAL_Driver/Inc" \
  -I"$task_repo_dir/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" \
  -I"$task_repo_dir/Drivers/CMSIS/Device/ST/STM32F4xx/Include" \
  -I"$task_repo_dir/Drivers/CMSIS/Include" \
  -DSTM32F407xx -DUSE_HAL_DRIVER \
  "$task_repo_dir/Core/Src/tetris.c" \
  "$task_repo_dir/Core/Src/tetris_shapes.c" \
  "$task_repo_dir/Core/Src/fonts.c" \
  "$task_repo_dir/Core/Src/oled.c" \
  "$task_repo_dir/Core/Src/pca9538.c" \
  "$task_test_dir/test_render.c" \
  -o "$task_build_dir/test_render"

"$task_build_dir/test_render"
