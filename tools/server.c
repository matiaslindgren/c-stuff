#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "stufflib/args/args.h"
#include "stufflib/macros/macros.h"
#include "stufflib/misc/misc.h"

#define SL_SOCKET_BACKLOG_LEN 1024

static bool sl_server_running = false;

void stop_server_on_sigterm(int) {
  SL_LOG_INFO("received SIGTERM, shutting down");
  sl_server_running = false;
}

void print_usage(const struct sl_args args[const static 1]) {
  fprintf(stderr, "usage: %s [-h | --help] host port\n", args->argv[0]);
}

int main(int argc, char *const argv[argc + 1]) {
  struct sockaddr_in serveraddr = {
      .sin_family = AF_INET,
  };

  bool ok = true;
  {
    struct sl_args args = {.argc = argc, .argv = argv};
    if (sl_args_contains_help_flag(&args)) {
      print_usage(&args);
      return 2;
    }

    if (sl_args_count_positional(&args) != 2) {
      ok = false;
    } else {
      switch (inet_pton(AF_INET, argv[1], &(serveraddr.sin_addr))) {
        case -1: {
          SL_LOG_ERROR("error during parsing of host IPv4 address");
          ok = false;
        } break;
        case 0: {
          SL_LOG_ERROR("host address is not IPv4");
          ok = false;
        } break;
        case 1: {
          SL_LOG_TRACE("input %s is a valid IPv4 address", argv[1]);
          ok = true;
        } break;
      }

      if (ok) {
        const unsigned long port_arg = strtoul(argv[2], nullptr, 10);
        if (0 < port_arg && port_arg < (1UL << 16)) {
          serveraddr.sin_port = htons((int)port_arg);
        } else {
          SL_LOG_ERROR("input port must be in [1, 65535]");
          ok = false;
        }
      }
    }

    if (!ok) {
      print_usage(&args);
      return 1;
    }
  }

  int fd_listen = -1;

  if (0 > (fd_listen = socket(AF_INET, SOCK_STREAM, 0))) {
    SL_LOG_ERROR("failed creating socket");
    perror("error");  // TODO into SL_LOG
    ok = false;
    goto end;
  }

  if (0 > bind(fd_listen, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) {
    SL_LOG_ERROR("failed binding address to socket %d", fd_listen);
    perror("error");  // TODO into SL_LOG
    ok = false;
    goto end;
  }

  if (0 > listen(fd_listen, SL_SOCKET_BACKLOG_LEN)) {
    SL_LOG_ERROR("failed listening to socket %d", fd_listen);
    perror("error");  // TODO into SL_LOG
    ok = false;
    goto end;
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
      ok = false;
      goto end;
    }
  }

  sl_server_running = true;
  SL_LOG_INFO("listening at %s:%s", argv[1], argv[2]);

  for (char msg_buffer[4096] = {}; sl_server_running;) {
    int fd_conn = -1;
    if (0 >
        (fd_conn = accept(fd_listen, (struct sockaddr *)nullptr, nullptr))) {
      if (errno == EINTR) {
        SL_LOG_INFO("received interrupt signal, will not accept connections");
        goto end_loop;
      } else {
        SL_LOG_ERROR("failed accepting incoming connection");
        perror("error");  // TODO into SL_LOG
        ok = false;
        goto end_loop;
      }
    }

    SL_LOG_INFO("handling TCP request on fd %d", fd_conn);

    {
      ssize_t msg_len = 0;
      if (0 >
          (msg_len = recv(fd_conn, msg_buffer, SL_ARRAY_LEN(msg_buffer), 0))) {
        SL_LOG_ERROR("failed reading message on %d", fd_conn);
        perror("error");  // TODO into SL_LOG
        ok = false;
        goto end_loop;
      }

      if (msg_len >= (ssize_t)SL_ARRAY_LEN(msg_buffer)) {
        SL_LOG_ERROR(
            "message sent by client is too large (%zd), truncating to %zu",
            msg_len,
            SL_ARRAY_LEN(msg_buffer));
        msg_len = (ssize_t)(SL_ARRAY_LEN(msg_buffer) - 1);
      }
      msg_buffer[msg_len] = 0;
    }

    SL_LOG_TRACE("received message '%s' on %d", msg_buffer, fd_conn);

#if 0
    if (0 > snprintf(msg_buffer, SL_ARRAY_LEN(msg_buffer), "hello\r\n")) {
      SL_LOG_ERROR("failed writing message buffer");
      perror("error");  // TODO into SL_LOG
      ok = false;
      goto end_loop;
    }
#endif

    if (0 > send(fd_conn, msg_buffer, strlen(msg_buffer), 0)) {
      SL_LOG_ERROR("failed sending message on %d", fd_conn);
      perror("error");  // TODO into SL_LOG
      ok = false;
      goto end_loop;
    }

  end_loop:
    if (fd_conn != -1 && 0 > close(fd_conn)) {
      SL_LOG_ERROR("failed closing %d", fd_conn);
      perror("error");  // TODO into SL_LOG
      ok = false;
    }

    if (!ok) {
      sl_server_running = false;
    }
  }

end:
  if (fd_listen != -1 && 0 > close(fd_listen)) {
    SL_LOG_ERROR("failed closing %d", fd_listen);
    perror("error");  // TODO into SL_LOG
    ok = false;
  }

  return ok ? 0 : 1;
}
