#!/usr/bin/env bash
set -ue

self_dir=$(dirname "$0")
source ${self_dir}/common.bash $@

dataset_tool="$1"

$dataset_tool rcv1 ${root_dir}/test-data/datasets/rcv1 ${test_dir}
