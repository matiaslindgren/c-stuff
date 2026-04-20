#!/usr/bin/env bash
set -o nounset
set -o pipefail
set -o errexit
set -o errtrace
trap 'echo error:$? line:$LINENO cmd:$BASH_COMMAND' ERR


self_dir=$(dirname "$0")
source ${self_dir}/common.bash $@

logging_tool="$1"

function count_log_lines {
  local level="$1"
  SL_LOGGING_LEVEL=$level $logging_tool 2>&1 | wc -l | tr -d ' '
}

function test_log_level_trace {
  local n
  n=$(count_log_lines trace)
  if [ "$n" -ne 3 ]; then
    echo "FAIL test_log_level_trace: expected 3 lines, got $n"
    exit 1
  fi
}

function test_log_level_info {
  local n
  n=$(count_log_lines info)
  if [ "$n" -ne 2 ]; then
    echo "FAIL test_log_level_info: expected 2 lines, got $n"
    exit 1
  fi
}

function test_log_level_error {
  local n
  n=$(count_log_lines error)
  if [ "$n" -ne 1 ]; then
    echo "FAIL test_log_level_error: expected 1 line, got $n"
    exit 1
  fi
}

function test_log_level_none {
  local n
  n=$(count_log_lines none)
  if [ "$n" -ne 0 ]; then
    echo "FAIL test_log_level_none: expected 0 lines, got $n"
    exit 1
  fi
}

function test_log_json_structure {
  local line
  line=$(SL_LOGGING_LEVEL=error $logging_tool 2>&1 | head -1)
  if ! echo "$line" | jq -e '.level and .file and (.line | type == "number") and .msg' > /dev/null; then
    echo "FAIL test_log_json_structure: invalid JSON: $line"
    exit 1
  fi
}

test_log_level_trace
test_log_level_info
test_log_level_error
test_log_level_none
test_log_json_structure
