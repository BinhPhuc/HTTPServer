#pragma once

#include <netdb.h>

class SocketHelper {
public:
  static void set_non_blocking(int sockfd);
  static int create_socket(int domain, int type, int protocol);
  static void get_addr_info(const char *node, const char *service,
                            const struct addrinfo *hints, struct addrinfo **res);
  static int set_sock_option(int sockfd, int level, int optname);
  static int bind_socket(int sockfd, const struct sockaddr *addr,
                         socklen_t addrlen);
  static int socket_listen(int sockfd, int backlog);
  static int create_epoll();
};
