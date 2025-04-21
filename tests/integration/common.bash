#!/usr/bin/env bash
set -ue

if [ $# -gt 0 ]; then
  for arg in $@; do
    if [ "$arg" == "-v" ]; then
      set -x
    fi
  done
fi

required_commands=(
  'python3'
  'sort'
  'cmp'
  'cut'
  'find'
  'jq'
  'wc'
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

root_dir=$(dirname $0)/../../
test_dir=$(mktemp --directory)
trap "rm -rf $test_dir" EXIT SIGTERM
