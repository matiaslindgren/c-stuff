#!/usr/bin/env bash
set -ue

self_dir=$(dirname "$0")
source ${self_dir}/common.bash $@

server_tool="$1"
nohup $server_tool > server.out 2>&1 &
server_pid=$!

while [ ! -s server.out ]; do
  sleep 1
done

function cleanup {
  kill $server_pid
  rm -f server.out
  rm -f client.out
}
trap cleanup EXIT TERM

function check_no_server_errors {
  if [ "$(jq .level server.out | grep error --count)" -gt 0 ]; then
    cat server.out
    exit 1
  fi
}

check_no_server_errors

python3 -c '\
import socket
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect(("127.0.0.1", 8080))
    data = s.recv(1024)
    print(data.decode("utf-8").strip())
' > client.out 2>&1

check_no_server_errors
if ! grep --silent hello client.out; then
  cat client.out
  exit 1
fi
