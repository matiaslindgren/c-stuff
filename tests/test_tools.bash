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
  'jq'
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

png_chunk_types=(
  'IHDR'
  'PLTE'
  'IDAT'
  'IEND'
  'bKGD'
  'cHRM'
  'dSIG'
  'eXIf'
  'gAMA'
  'hIST'
  'iCCP'
  'iTXt'
  'pHYs'
  'sBIT'
  'sPLT'
  'sRGB'
  'sTER'
  'tEXt'
  'tIME'
  'tRNS'
  'zTXt'
)

function test_png_tool {
  local png_tool=$1

  for input in $(find ${self_dir}/../test-data -name '*.png'); do
    local info_output=${test_dir}/stufflib_output.json
    $png_tool info $input > $info_output
    for chunk_type in ${png_chunk_types[*]}; do
      local grep_count=$(grep --count $chunk_type $input)
      local info_count=$(jq ".chunks.${chunk_type} // 0" $info_output)
      if [ $info_count -ne $grep_count ]; then
        printf "grep says '%s' contains %d chunks of type %s, but stufflib png says %d\n" $input $grep_count $chunk $count
      fi
    done
  done

  for segment_threshold in 10 20 30; do
    local input=${self_dir}/../docs/img/tokyo.png
    local expect=${self_dir}/../docs/img/tokyo_segmented_${segment_threshold}p.png
    local output=${test_dir}/stufflib_output.png
    $png_tool segment --threshold-percent=$segment_threshold $input $output
    if ! cmp $output $expect; then
      printf "'%s' failed to segment input with threshold %d\n" $png_tool $segment_threshold
      return 1
    fi
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

function test_txt_tool {
  local txt_tool=$1

  local output=${test_dir}/stufflib_output.txt
  local expect=${test_dir}/concat_output.txt

  for lhs in Makefile README.md LICENSE; do
    for rhs in Makefile README.md LICENSE; do
      local lhs_path=${self_dir}/../${lhs}
      local rhs_path=${self_dir}/../${rhs}
      $txt_tool concat $lhs_path $rhs_path > $output
      cat $lhs_path $rhs_path > $expect
      if ! cmp $output $expect; then
        printf "'%s' failed to concatenate 2 files\n" $png_tool
        return 1
      fi
      $txt_tool concat $lhs_path $rhs_path $lhs_path > $output
      cat $lhs_path $rhs_path $lhs_path > $expect
      if ! cmp $output $expect; then
        printf "'%s' failed to concatenate 3 files\n" $png_tool
        return 1
      fi
    done
  done

  return 0;
}

printf "test txt tool\n"

if ! test_txt_tool ${build_dir}/debug/tools/txt; then
  exit 1
fi
if ! test_txt_tool ${build_dir}/release/tools/txt; then
  exit 1
fi
