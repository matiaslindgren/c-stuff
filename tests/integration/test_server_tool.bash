#!/usr/bin/env bash
set -ue

self_dir=$(dirname "$0")
source ${self_dir}/common.bash $@

listen_port=8080
bind_host=127.0.0.1
message_content="hello TCP server"

case "$OSTYPE" in
  darwin*)
    cmd__socket_in_use="netstat -a -n -p tcp | grep --quiet '${bind_host}.${listen_port}'"
    ;;
  linux*)
    cmd__socket_in_use="netstat --all --numeric --tcp | grep --quiet '${bind_host}:${listen_port}'"
    ;;
  *)
    printf "unknown OSTYPE='%s'\n" "$OSTYPE"
    exit 1
    ;;
esac

# wait until TCP sockets from previous runs end their TIME_WAIT state
cmd__wait_until_socket_is_free="\
  while $cmd__socket_in_use; do \
    echo 'waiting for port ${listen_port} to be released'; \
    sleep 5; \
  done"
if ! timeout 1m bash -c "$cmd__wait_until_socket_is_free"; then
  printf "timed out waiting for '%s'\n" "$cmd__socket_in_use"
  exit 1
fi

server_tool="$1"
nohup $server_tool $bind_host $listen_port > server.out 2>&1 &
server_pid=$!

function cleanup {
  set +e

  kill $server_pid

  if [ -o xtrace ]; then
    cat server.out
  fi
  rm -f server.out

  if [ -o xtrace ]; then
    cat client.out
  fi
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
    s.connect(('$bind_host', $listen_port))
    s.sendall(b'$message_content')
    data = s.recv(1024)
    print(data.decode('utf-8').strip())
" > client.out 2>&1

check_no_server_errors
if ! grep --quiet "$message_content" client.out; then
  cat client.out
  exit 1
fi
