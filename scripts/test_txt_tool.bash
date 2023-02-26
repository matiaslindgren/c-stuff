#!/usr/bin/env bash
set -ue

self_dir=$(dirname "$0")
source ${self_dir}/_test_common.bash $@

test_dir=$(mktemp --directory)

function rm_test_dir {
  rm -rf $test_dir
}
trap rm_test_dir EXIT

build_dir=${self_dir}/../build

function test_txt_tool {
  local txt_tool=$1

  local wikifiles=(
    ./test-data/txt/wikipedia/water_hi.txt
    ./test-data/txt/wikipedia/water_is.txt
    ./test-data/txt/wikipedia/water_ja.txt
    ./test-data/txt/wikipedia/water_ko.txt
    ./test-data/txt/wikipedia/water_ku.txt
    ./test-data/txt/wikipedia/water_uk.txt
    ./test-data/txt/wikipedia/water_vi.txt
    ./test-data/txt/wikipedia/water_zh.txt
  )

  local output=${test_dir}/stufflib_output.txt
  local expect=${test_dir}/concat_output.txt

  for lhs in ${wikifiles[*]}; do
    for rhs in ${wikifiles[*]}; do
      local lhs_path=${self_dir}/../${lhs}
      local rhs_path=${self_dir}/../${rhs}
      $txt_tool concat $lhs_path $rhs_path > $output
      cat $lhs_path $rhs_path > $expect
      if ! cmp $output $expect; then
        printf "'%s' failed to concatenate 2 files\n" $txt_tool
        return 1
      fi
      $txt_tool concat $lhs_path $rhs_path $lhs_path > $output
      cat $lhs_path $rhs_path $lhs_path > $expect
      if ! cmp $output $expect; then
        printf "'%s' failed to concatenate 3 files\n" $txt_tool
        return 1
      fi
    done
  done

  for count_test in ./test-data/txt/empty:0 ./test-data/txt/one.txt:0 ./test-data/txt/hello.txt:1 ./test-data/txt/numbers.txt:100; do
    local path=$(echo $count_test | cut -f1 -d:)
    local newlines=$(echo $count_test | cut -f2 -d:)
    local counted=$($txt_tool count $'\n' $path)
    if [ "$counted" -ne "$newlines" ]; then
      printf "'%s' counted %d newlines in '%s' but expected %d\n" $txt_tool $counted $path $newlines
      return 1
    fi
  done

  local path=./test-data/txt/wikipedia/water_ja.txt
  local counted=$($txt_tool count は $path)
  if [ "$counted" -ne 5 ]; then
    printf "'%s' counted %d of 'は' in '%s' but expected %d\n" $txt_tool $counted $path 5
    return 1
  fi

  return 0;
}

if ! test_txt_tool ${build_dir}/debug/tools/txt; then
  exit 1
fi
if ! test_txt_tool ${build_dir}/release/tools/txt; then
  exit 1
fi
