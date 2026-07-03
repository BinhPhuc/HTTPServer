#include "network/Server.hpp"
#include "handler/shutdown/shutdown.hpp"
#include "network/EpollEventHandler.hpp"
#include "utils/Constants.hpp"
#include <asm-generic/socket.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <spdlog/spdlog.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

Server::Server(int port, ApiRouter &api_router)
    : m_port(port), m_sockfd(-1), m_epollfd(-1), m_api_router(api_router) {}

Server::~Server() { stop_server(); }

void Server::stop_server() {
  if (m_epollfd != -1) {
    close(m_epollfd);
    m_epollfd = -1;
  }
  if (m_sockfd != -1) {
    spdlog::info("Stopping server on port {}.", m_port);
    close(m_sockfd);
    m_sockfd = -1;
  }
  spdlog::info("Server stopped.");
}

void set_non_blocking(int sockfd) {
  int flags = fcntl(sockfd, F_GETFL, 0);
  if (flags == -1) {
    spdlog::error("Fcntl get flags error: {}", strerror(errno));
    return;
  }
  if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
    spdlog::error("Fcntl set non-blocking error: {}", strerror(errno));
  }
}

int create_socket(int domain, int type, int protocol) {
  int sockfd = socket(domain, type, protocol);
  if (sockfd == -1) {
    spdlog::error("Socket creation error: {}", strerror(errno));
  }
  return sockfd;
}

void get_addr_info_wrapper(const char *node, const char *service,
                           const struct addrinfo *hints,
                           struct addrinfo **res) {
  int status = getaddrinfo(node, service, hints, res);
  if (status != 0) {
    spdlog::error("getaddrinfo error: {}", gai_strerror(status));
  }
}

int set_sock_option(int sockfd, int level, int optname) {
  int yes = 1;
  int status = setsockopt(sockfd, level, optname, &yes, sizeof(int));
  if (status == -1) {
    spdlog::error("Setsockopt error: {}", strerror(errno));
  }
  return status;
}

int bind_socket(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  int status = bind(sockfd, addr, addrlen);
  if (status == -1) {
    spdlog::error("Bind error: {}", strerror(errno));
  }
  return status;
}

int socket_listen(int sockfd, int backlog) {
  int status = listen(sockfd, backlog);
  if (status == -1) {
    spdlog::error("Listen error: {}", strerror(errno));
  }
  return status;
}

int create_epoll() {
  int epollfd = epoll_create1(0);
  if (epollfd == -1) {
    spdlog::error("Epoll creation error: {}", strerror(errno));
  }
  return epollfd;
}

bool Server::initialize_socket() {
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  get_addr_info_wrapper(NULL, std::to_string(m_port).c_str(), &hints, &res);

  struct addrinfo *p;
  for (p = res; p != NULL; p = p->ai_next) {
    m_sockfd = create_socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (m_sockfd == -1) {
      continue;
    }

    set_non_blocking(m_sockfd);

    if (set_sock_option(m_sockfd, SOL_SOCKET, SO_REUSEADDR) == -1) {
      close(m_sockfd);
      continue;
    }

    if (bind_socket(m_sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(m_sockfd);
      continue;
    }

    break;
  }

  freeaddrinfo(res);

  if (p == NULL) {
    spdlog::error("Failed to bind socket.");
    return false;
  }

  if (socket_listen(m_sockfd, config::MAX_CONNECTIONS) == -1) {
    close(m_sockfd);
    return false;
  }

  ShutdownHandler::listen_fd.store(m_sockfd);

  return true;
}

bool Server::initialize_epoll() {
  m_epollfd = create_epoll();
  if (m_epollfd == -1) {
    return false;
  }

  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = m_sockfd;

  if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, m_sockfd, &event) == -1) {
    spdlog::error("Epoll control error: {}", strerror(errno));
    close(m_epollfd);
    return false;
  }

  return true;
}

void Server::run_event_loop() {
  struct epoll_event events[config::MAX_CONNECTIONS];
  EpollEventHandler event_handler(m_epollfd, m_sockfd, m_api_router);

  spdlog::info("Server started on port {}.", m_port);

  while (ShutdownHandler::running.load()) {
    int num_events = epoll_wait(m_epollfd, events, config::MAX_CONNECTIONS, -1);

    if (num_events == -1) {
      if (errno == EINTR) {
        continue;
      }
      spdlog::error("Epoll wait error: {}", strerror(errno));
      break;
    }

    for (int i = 0; i < num_events; i++) {
      event_handler.handle_event(events[i]);
    }
  }

  spdlog::info("Event loop exited, shutting down gracefully.");
}

void Server::start() {
  if (!initialize_socket()) {
    return;
  }

  if (!initialize_epoll()) {
    close(m_sockfd);
    return;
  }

  run_event_loop();
}
