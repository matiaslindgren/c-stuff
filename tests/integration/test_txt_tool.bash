#!/usr/bin/env bash
set -ue

self_dir=$(dirname "$0")
source ${self_dir}/common.bash $@
txt_dir=${root_dir}/test-data/txt

txt_tool="$1"

wikifiles=(
${txt_dir}/wikipedia/water_hi.txt
${txt_dir}/wikipedia/water_is.txt
${txt_dir}/wikipedia/water_ja.txt
${txt_dir}/wikipedia/water_ko.txt
${txt_dir}/wikipedia/water_ku.txt
${txt_dir}/wikipedia/water_uk.txt
${txt_dir}/wikipedia/water_vi.txt
${txt_dir}/wikipedia/water_zh.txt
)

output=${test_dir}/stufflib_output.txt
expect=${test_dir}/concat_output.txt

for lhs in ${wikifiles[*]}; do
  for rhs in ${wikifiles[*]}; do
    lhs_path=${root_dir}/${lhs}
    rhs_path=${root_dir}/${rhs}
    $txt_tool concat $lhs_path $rhs_path > $output
    cat $lhs_path $rhs_path > $expect
    if ! cmp $output $expect; then
      printf "'%s' failed to concatenate 2 files\n" $txt_tool
      exit 1
    fi
    $txt_tool concat $lhs_path $rhs_path $lhs_path > $output
    cat $lhs_path $rhs_path $lhs_path > $expect
    if ! cmp $output $expect; then
      printf "'%s' failed to concatenate 3 files\n" $txt_tool
      exit 1
    fi
  done
done

for count_test in ${txt_dir}/empty:0 ${txt_dir}/one.txt:0 ${txt_dir}/hello.txt:1 ${txt_dir}/numbers.txt:100; do
  path=$(echo $count_test | cut -f1 -d:)
  newlines=$(echo $count_test | cut -f2 -d:)
  counted=$($txt_tool count $'\n' $path)
  if [ "$counted" -ne "$newlines" ]; then
    printf "'%s' counted %d newlines in '%s' but expected %d\n" $txt_tool $counted $path $newlines
    exit 1
  fi
done

path=${txt_dir}/wikipedia/water_ja.txt
counted=$($txt_tool count は $path)
if [ "$counted" -ne 5 ]; then
  printf "'%s' counted %d of 'は' in '%s' but expected %d\n" $txt_tool $counted $path 5
  exit 1
fi
