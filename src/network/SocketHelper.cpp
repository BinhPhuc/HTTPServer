#include "network/SocketHelper.hpp"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <spdlog/spdlog.h>
#include <sys/epoll.h>
#include <sys/socket.h>

void SocketHelper::set_non_blocking(int sockfd) {
  int flags = fcntl(sockfd, F_GETFL, 0);
  if (flags == -1) {
    spdlog::error("Fcntl get flags error: {}", strerror(errno));
    return;
  }
  if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
    spdlog::error("Fcntl set non-blocking error: {}", strerror(errno));
  }
}

int SocketHelper::create_socket(int domain, int type, int protocol) {
  int sockfd = socket(domain, type, protocol);
  if (sockfd == -1) {
    spdlog::error("Socket creation error: {}", strerror(errno));
  }
  return sockfd;
}

void SocketHelper::get_addr_info(const char *node, const char *service,
                                  const struct addrinfo *hints,
                                  struct addrinfo **res) {
  int status = getaddrinfo(node, service, hints, res);
  if (status != 0) {
    spdlog::error("getaddrinfo error: {}", gai_strerror(status));
  }
}

int SocketHelper::set_sock_option(int sockfd, int level, int optname) {
  int yes = 1;
  int status = setsockopt(sockfd, level, optname, &yes, sizeof(int));
  if (status == -1) {
    spdlog::error("Setsockopt error: {}", strerror(errno));
  }
  return status;
}

int SocketHelper::bind_socket(int sockfd, const struct sockaddr *addr,
                               socklen_t addrlen) {
  int status = bind(sockfd, addr, addrlen);
  if (status == -1) {
    spdlog::error("Bind error: {}", strerror(errno));
  }
  return status;
}

int SocketHelper::socket_listen(int sockfd, int backlog) {
  int status = listen(sockfd, backlog);
  if (status == -1) {
    spdlog::error("Listen error: {}", strerror(errno));
  }
  return status;
}

int SocketHelper::create_epoll() {
  int epollfd = epoll_create1(0);
  if (epollfd == -1) {
    spdlog::error("Epoll creation error: {}", strerror(errno));
  }
  return epollfd;
}
