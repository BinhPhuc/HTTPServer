#include "network/Server.hpp"
#include "network/EpollEventHandler.hpp"
#include "network/SocketHelper.hpp"
#include "utils/Constants.hpp"
#include <asm-generic/socket.h>
#include <cerrno>
#include <cstring>
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

bool Server::initialize_socket() {
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  SocketHelper::get_addr_info(NULL, std::to_string(m_port).c_str(), &hints, &res);

  struct addrinfo *p;
  for (p = res; p != NULL; p = p->ai_next) {
    m_sockfd = SocketHelper::create_socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (m_sockfd == -1) {
      continue;
    }

    SocketHelper::set_non_blocking(m_sockfd);

    if (SocketHelper::set_sock_option(m_sockfd, SOL_SOCKET, SO_REUSEADDR) == -1) {
      close(m_sockfd);
      continue;
    }

    if (SocketHelper::bind_socket(m_sockfd, p->ai_addr, p->ai_addrlen) == -1) {
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

  if (SocketHelper::socket_listen(m_sockfd, config::MAX_CONNECTIONS) == -1) {
    close(m_sockfd);
    return false;
  }

  return true;
}

bool Server::initialize_epoll() {
  m_epollfd = SocketHelper::create_epoll();
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

  while (true) {
    int num_events = epoll_wait(m_epollfd, events, config::MAX_CONNECTIONS, -1);

    if (num_events == -1) {
      spdlog::error("Epoll wait error: {}", strerror(errno));
      break;
    }

    for (int i = 0; i < num_events; i++) {
      event_handler.handle_event(events[i]);
    }
  }
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
