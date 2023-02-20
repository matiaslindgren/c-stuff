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
