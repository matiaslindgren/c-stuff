#!/usr/bin/env bash
set -ue

self_dir=$(dirname "$0")
source ${self_dir}/common.bash $@

test_dir=$(mktemp --directory)

function rm_test_dir {
  rm -rf $test_dir
}
trap rm_test_dir EXIT

build_dir=${self_dir}/../build

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

  for input in $(find ${self_dir}/../test-data/png -name '*.png'); do
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

if ! test_png_tool ${build_dir}/debug/tools/png; then
  exit 1
fi
if ! test_png_tool ${build_dir}/release/tools/png; then
  exit 1
fi
