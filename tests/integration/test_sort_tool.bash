#!/usr/bin/env bash
set -ue

self_dir=$(dirname "$0")
source ${self_dir}/common.bash $@

sort_tool="$1"

function test_sort_tool {
  local variance="$1"
  local test_size="$2"

  local input=${test_dir}/sort_input.txt
  local expect=${test_dir}/expected_output.txt
  local output=${test_dir}/stufflib_output.txt

  python3 -c "
from random import randint
for _ in range(${test_size}):
  print(randint(-${variance}, ${variance}))
  " > $input

  LC_ALL=C sort $input > $expect
  $sort_tool ascii $input > $output
  if ! cmp --print-bytes $output $expect; then
    printf "'%s' failed to sort input as ascii strings\n" $sort_tool
    return 1
  fi

  LC_ALL=C sort --general-numeric-sort $input > $expect
  $sort_tool numeric $input > $output
  if ! cmp --print-bytes $output $expect; then
    printf "'%s' failed to sort input as numeric\n" $sort_tool
    return 1
  fi

  return 0
}

if [[ $sort_tool = */debug/* ]]; then
  if ! test_sort_tool 1 '10**3'; then
    exit 1
  fi
  if ! test_sort_tool 10 '10**4'; then
    exit 1
  fi
  if ! test_sort_tool '10**6' '10**4'; then
    exit 1
  fi
fi

if [[ $sort_tool = */release/* ]]; then
  if ! test_sort_tool 1 '10**6'; then
    exit 1
  fi
  if ! test_sort_tool 10 '10**6'; then
    exit 1
  fi
  if ! test_sort_tool '10**6' '10**6'; then
    exit 1
  fi
fi
