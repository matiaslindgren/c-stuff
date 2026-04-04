#!/usr/bin/env bash
set -o nounset
set -o pipefail
set -o errexit
set -o errtrace
trap 'echo error:$? line:$LINENO cmd:$BASH_COMMAND' ERR


self_dir=$(dirname "$0")
source ${self_dir}/common.bash $@

dataset_tool="$1"

$dataset_tool rcv1 ${root_dir}/test-data/datasets/rcv1 ${test_dir}
