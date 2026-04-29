#!/usr/bin/env bash
set -o nounset
set -o pipefail
set -o errexit
set -o errtrace
trap 'echo error:$? line:$LINENO cmd:$BASH_COMMAND' ERR


self_dir=$(dirname "$0")
source ${self_dir}/../common.bash $@

json_fuzzer="$1"

corpus=${test_dir}/fuzz_corpus/json/
mkdir -pv $corpus
cp -v $(find ${root_dir} -type f -name '*.json') $corpus/

max_len=$((1 * 1024 ** 3))

$json_fuzzer $corpus \
  -timeout=10 \
  -max_len=$max_len \
  -max_total_time=$FUZZER_TIMEOUT_SEC \
  -print_final_stats=1 \
  -verbosity=0
