#!/usr/bin/env bash
set -ue

self_dir=$(dirname "$0")
source ${self_dir}/common.bash $@

listen_port=8080
send_message="hello TCP server"

# wait until TCP sockets from previous runs end their TIME_WAIT state
while $(netstat -n | grep -Eq '^tcp.*127\.0\.0\.1[:.]'${listen_port}); do
  sleep 1
done

server_tool="$1"
nohup $server_tool 127.0.0.1 $listen_port > server.out 2>&1 &
server_pid=$!

function cleanup {
  set +e
  kill $server_pid
  rm -f server.out
  rm -f client.out
}
trap cleanup EXIT

while [ ! -s server.out ]; do
  sleep 1
done

function check_no_server_errors {
  if [ "$(jq .level server.out | grep error --count)" -gt 0 ]; then
    cat server.out
    exit 1
  fi
}

check_no_server_errors

python3 -c "\
import socket
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect(('127.0.0.1', $listen_port))
    s.sendall(b'$send_message')
    data = s.recv(1024)
    print(data.decode('utf-8').strip())
" > client.out 2>&1

check_no_server_errors
if ! grep --silent "$send_message" client.out; then
  cat client.out
  exit 1
fi
