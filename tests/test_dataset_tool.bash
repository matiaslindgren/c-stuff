#!/usr/bin/env bash
set -ue

self_dir=$(dirname "$0")
source ${self_dir}/common.bash $@

dataset_tool="$1"

test_dir=$(mktemp --directory)

function rm_test_dir {
  rm -rf $test_dir
}
trap rm_test_dir EXIT

build_dir=${self_dir}/../build

$dataset_tool rcv1 ${self_dir}/../test-data/datasets/rcv1 ${test_dir}
