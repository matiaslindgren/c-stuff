#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "stufflib/macros/macros.h"
#include "stufflib/misc/misc.h"

#define SL_SOCKET_BACKLOG_LEN 1024

static bool sl_server_running = false;

void stop_server_on_sigterm(int) {
  SL_LOG_INFO("received SIGTERM, shutting down");
  sl_server_running = false;
}

int main(void) {
  int fd_listen = 0;
  if (0 > (fd_listen = socket(AF_INET, SOCK_STREAM, 0))) {
    SL_LOG_ERROR("failed creating socket");
    perror("error");  // TODO into SL_LOG
    return 1;
  }

  struct sockaddr_in serveraddr = {
      .sin_family = AF_INET,
      .sin_addr.s_addr = htonl(INADDR_ANY),
      .sin_port = htons(8080),
  };

  if (0 > bind(fd_listen, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) {
    SL_LOG_ERROR("failed binding address to socket %d", fd_listen);
    perror("error");  // TODO into SL_LOG
    return 1;
  }

  if (0 > listen(fd_listen, SL_SOCKET_BACKLOG_LEN)) {
    SL_LOG_ERROR("failed listening to socket %d", fd_listen);
    perror("error");  // TODO into SL_LOG
    return 1;
  }

  {
    struct sigaction sa = {
        .sa_flags = 0,
        .sa_handler = stop_server_on_sigterm,
    };
    sigemptyset(&sa.sa_mask);
    if (0 > sigaction(SIGTERM, &sa, nullptr)) {
      SL_LOG_ERROR("failed installing sigterm handler");
      perror("error");  // TODO into SL_LOG
      return 1;
    }
  }

  sl_server_running = true;
  SL_LOG_INFO("start serving");
  for (char msg_buffer[4096] = {}; sl_server_running;) {
    bool ok = true;
    int fd_conn = 0;
    if (0 >
        (fd_conn = accept(fd_listen, (struct sockaddr *)nullptr, nullptr))) {
      SL_LOG_ERROR("failed accepting incoming connection");
      perror("error");  // TODO into SL_LOG
      ok = false;
      goto end;
    }
    SL_LOG_INFO("handling TCP request");
    if (0 > snprintf(msg_buffer, SL_ARRAY_LEN(msg_buffer), "hello\r\n")) {
      SL_LOG_ERROR("failed writing message buffer");
      perror("error");  // TODO into SL_LOG
      ok = false;
      goto end;
    }
    if (0 > send(fd_conn, msg_buffer, strlen(msg_buffer), 0)) {
      SL_LOG_ERROR("failed sending message");
      perror("error");  // TODO into SL_LOG
      ok = false;
      goto end;
    }
  end:
    if (fd_conn >= 0 && 0 > close(fd_conn)) {
      SL_LOG_ERROR("failed closing %d", fd_conn);
      perror("error");  // TODO into SL_LOG
      ok = false;
    }
    if (!ok) {
      return 1;
    }
  }

  return 0;
}
