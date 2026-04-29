#!/usr/bin/env bash
set -o nounset
set -o pipefail
set -o errexit
set -o errtrace
trap 'echo error:$? line:$LINENO cmd:$BASH_COMMAND' ERR


self_dir=$(dirname "$0")
source ${self_dir}/../common.bash $@

png_fuzzer="$1"

corpus=${test_dir}/fuzz_corpus/png/
mkdir -pv $corpus
cp -v $(find ${root_dir} -type f -name '*.png') $corpus/

max_len=$((2 * 1024 ** 3))

$png_fuzzer $corpus \
  -timeout=30 \
  -max_len=$max_len \
  -max_total_time=$FUZZER_TIMEOUT_SEC \
  -print_final_stats=1 \
  -verbosity=0
