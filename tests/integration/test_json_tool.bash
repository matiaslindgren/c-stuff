#!/usr/bin/env bash
set -o nounset
set -o pipefail
set -o errexit
set -o errtrace
trap 'echo error:$? line:$LINENO cmd:$BASH_COMMAND' ERR


self_dir=$(dirname "$0")
source ${self_dir}/common.bash $@

json_tool="$1"

# test valid json files
for json_file in $(find ${root_dir}/test-data/json/valid -name '*.json' -type f); do
  if ! $json_tool check $json_file; then
    printf "'%s' failed to parse valid json file '%s'\n" $json_tool $json_file
    exit 1
  fi
done

# test invalid json files
for json_file in $(find ${root_dir}/test-data/json/invalid -name '*.json' -type f); do
  if $json_tool check $json_file; then
    printf "'%s' failed to reject invalid json file '%s'\n" $json_tool $json_file
    exit 1
  fi
done

if [ -f ${root_dir}/compile_commands.json ]; then
  json_file=${root_dir}/compile_commands.json
  if ! $json_tool check $json_file; then
    printf "'%s' failed to parse valid json file '%s'\n" $json_tool $json_file
    exit 1
  fi
fi
