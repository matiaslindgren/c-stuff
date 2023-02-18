#!/usr/bin/env bash
set -ue

if [ $# -eq 1 ]; then
  if [ "$1" == "-v" ]; then
    set -x
  fi
fi

required_commands=(
  'python3'
  'sort'
  'grep'
  'cmp'
  'cut'
  'find'
)
ok=1
for command in ${required_commands[*]}; do
  if [ -z $(type -p $command) ]; then
    printf "required command '%s' not found on path\n" $command
    ok=0
  fi
done
if [ $ok -eq 0 ]; then
  exit 1
fi


test_dir=$(mktemp --directory)

function rm_test_dir {
  rm -rf $test_dir
}
trap rm_test_dir EXIT

self_dir=$(dirname "$0")
build_dir=${self_dir}/../build

function test_sort_tool {
  local sort_tool=$1
  local variance="$2"
  local test_size="$3"

  local input=${test_dir}/sort_input.txt
  local expect=${test_dir}/expected_output.txt
  local output=${test_dir}/stufflib_output.txt

  python3 -c "
from random import randint
for _ in range(${test_size}):
  print(randint(-${variance}, ${variance}))
  " > $input

  LC_ALL=C sort $input > $expect
  $sort_tool strings $input > $output
  if ! cmp --print-bytes $output $expect; then
    printf "'%s' failed to sort input as strings\n" $sort_tool
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

printf "test sort tool\n"

if ! test_sort_tool ${build_dir}/debug/tools/sort 1 '10**3'; then
  exit 1
fi
if ! test_sort_tool ${build_dir}/debug/tools/sort '10**6' '10**3'; then
  exit 1
fi
if ! test_sort_tool ${build_dir}/release/tools/sort 1 '10**6'; then
  exit 1
fi
if ! test_sort_tool ${build_dir}/release/tools/sort 10 '10**6'; then
  exit 1
fi
if ! test_sort_tool ${build_dir}/release/tools/sort '10**6' '10**6'; then
  exit 1
fi

function test_png_tool {
  local png_tool=$1
  for input in $(find ${self_dir}/../test-data -name '*.png'); do
    local info_output=${test_dir}/stufflib_output.txt
    $png_tool info $input > $info_output
    local chunk_counts=$(python3 -c "
with open('$info_output') as f:
  sections = f.read().split('\n\n')
assert sections and sections[0].startswith('CHUNKS'), 'first section should be CHUNKS'
for line in sections[0].splitlines()[1:]:
  chunk, count = line.strip().split(':')
  print(chunk, int(count), sep=':')
  ")
    if [ $? -ne 0 ]; then
      return 1;
    fi
    for line in $chunk_counts; do
      local chunk=$(printf "$line" | cut -f1 -d':')
      local count=$(printf "$line" | cut -f2 -d':')
      local grep_count=$(grep --count $chunk $input)
      if [ $? -ne 0 ]; then
        return 1;
      fi
      if [ $grep_count -ne $count ]; then
        printf "grep says '%s' contains %d chunks of type %s, but stufflib png says %d\n" $input $grep_count $chunk $count
      fi
    done
  done
  return 0;
}

printf "test png tool\n"

if ! test_png_tool ${build_dir}/debug/tools/png; then
  exit 1
fi
if ! test_png_tool ${build_dir}/release/tools/png; then
  exit 1
fi
