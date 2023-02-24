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

  local output=${test_dir}/stufflib_output.txt
  local expect=${test_dir}/concat_output.txt

  for lhs in Makefile README.md LICENSE; do
    for rhs in Makefile README.md LICENSE; do
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

  return 0;
}

if ! test_txt_tool ${build_dir}/debug/tools/txt; then
  exit 1
fi
if ! test_txt_tool ${build_dir}/release/tools/txt; then
  exit 1
fi
