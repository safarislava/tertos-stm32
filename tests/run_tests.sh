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
