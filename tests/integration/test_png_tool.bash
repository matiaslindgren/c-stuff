#!/usr/bin/env bash
set -ue

self_dir=$(dirname "$0")
source ${self_dir}/common.bash $@

png_tool="$1"

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

for input in $(find ${root_dir}/test-data/png -name '*.png'); do
  info_output=${test_dir}/stufflib_output.json
  control=${input/%.png/.json}
  $png_tool info $input > $info_output
  for chunk_type in ${png_chunk_types[*]}; do
    control_count=$(jq ".${chunk_type} // 0" $control)
    info_count=$(jq ".chunks.${chunk_type} // 0" $info_output)
    if [ $info_count -ne $control_count ]; then
      printf "control json file says '%s' contains %d chunks of type %s, but stufflib png says %d\n" $input $control_count $chunk_type $info_count
    fi
  done
done

for segment_threshold in 10 20 30; do
  input=${root_dir}/docs/img/tokyo.png
  expect=${root_dir}/docs/img/tokyo_segmented_${segment_threshold}p.png
  output=${test_dir}/stufflib_output.png
  $png_tool segment --threshold-percent=$segment_threshold $input $output
  if ! cmp $output $expect; then
    printf "'%s' failed to segment input with threshold %d\n" $png_tool $segment_threshold
    exit 1
  fi
done
