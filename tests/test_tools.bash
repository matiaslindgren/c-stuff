#!/usr/bin/env bash
set -ue

required_programs=(
  'python3'
  'sort'
  'grep'
  'cmp'
)
ok=1
for program in ${required_programs[*]}; do
  if [ -z $(type -p $program) ]; then
    printf "required program '%s' not found on path\n" $program
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

printf "test sort\n"

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
