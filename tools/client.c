#if 0
  #include <arpa/inet.h>
  #include <errno.h>
  #include <netdb.h>
  #include <netinet/in.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <sys/socket.h>
  #include <sys/types.h>
  #include <unistd.h>

  #define PORT "3490"  // the port client will be connecting to

  #define MAXDATASIZE 100  // max number of bytes we can get at once

  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wcast-align"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr sa[const static 1]) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
  int sockfd = 0;
  ssize_t numbytes = 0;
  char buf[MAXDATASIZE] = {};
  struct addrinfo hints = {};
  struct addrinfo *servinfo = nullptr;
  struct addrinfo *p = nullptr;
  int rv = 0;
  char s[INET6_ADDRSTRLEN] = {};

  if (argc != 2) {
    fprintf(stderr, "usage: client hostname\n");
    exit(1);
  }

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // loop through all the results and connect to the first we can
  for (p = servinfo; p != nullptr; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("client: connect");
      continue;
    }

    break;
  }

  if (p == nullptr) {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }

  inet_ntop(p->ai_family,
            get_in_addr((struct sockaddr *)p->ai_addr),
            s,
            sizeof s);
  printf("client: connecting to %s\n", s);

  freeaddrinfo(servinfo);  // all done with this structure

  if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
    perror("recv");
    exit(1);
  }

  buf[numbytes] = '\0';

  printf("client: received '%s'\n", buf);

  close(sockfd);

  return 0;
}

  #pragma GCC diagnostic pop
#endif
int main() { return 0; }
