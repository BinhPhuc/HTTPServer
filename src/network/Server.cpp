#include "network/Server.hpp"
#include "handler/shutdown/shutdown.hpp"
#include "network/EpollEventHandler.hpp"
#include "utils/Constants.hpp"
#include <asm-generic/socket.h>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <spdlog/spdlog.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <unistd.h>

Server::Server(int port, ApiRouter &api_router, size_t num_loops)
    : m_port(port), m_num_loops(num_loops == 0 ? 1 : num_loops),
      m_api_router(api_router), m_ssl_ctx(TLS::create_context()), m_workers() {
  TLS::configure_context(m_ssl_ctx.get());
}

Server::~Server() {
  for (std::thread &worker : m_workers) {
    if (worker.joinable()) {
      worker.join();
    }
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

int Server::create_listen_socket() {
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  get_addr_info_wrapper(NULL, std::to_string(m_port).c_str(), &hints, &res);

  int sockfd = -1;
  struct addrinfo *p;
  for (p = res; p != NULL; p = p->ai_next) {
    sockfd = create_socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockfd == -1) {
      continue;
    }

    set_non_blocking(sockfd);

    if (set_sock_option(sockfd, SOL_SOCKET, SO_REUSEADDR) == -1 ||
        set_sock_option(sockfd, SOL_SOCKET, SO_REUSEPORT) == -1) {
      close(sockfd);
      sockfd = -1;
      continue;
    }

    if (bind_socket(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      sockfd = -1;
      continue;
    }

    break;
  }

  freeaddrinfo(res);

  if (p == NULL || sockfd == -1) {
    spdlog::error("Failed to bind listening socket.");
    return -1;
  }

  if (socket_listen(sockfd, config::MAX_CONNECTIONS) == -1) {
    close(sockfd);
    return -1;
  }

  return sockfd;
}

void Server::run_event_loop(int loop_id, int wakeup_fd) {
  int listen_fd = create_listen_socket();
  if (listen_fd == -1) {
    return;
  }

  int epollfd = create_epoll();
  if (epollfd == -1) {
    close(listen_fd);
    return;
  }

  struct epoll_event listen_event;
  listen_event.events = EPOLLIN;
  listen_event.data.fd = listen_fd;
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_fd, &listen_event) == -1) {
    spdlog::error("Epoll control error (listen socket): {}", strerror(errno));
    close(epollfd);
    close(listen_fd);
    return;
  }

  struct epoll_event wakeup_event;
  wakeup_event.events = EPOLLIN;
  wakeup_event.data.fd = wakeup_fd;
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, wakeup_fd, &wakeup_event) == -1) {
    spdlog::error("Epoll control error (wakeup fd): {}", strerror(errno));
    close(epollfd);
    close(listen_fd);
    return;
  }

  EpollEventHandler event_handler(epollfd, listen_fd, m_api_router,
                                  m_ssl_ctx.get());

  struct epoll_event events[config::MAX_CONNECTIONS];
  spdlog::info("Event loop {} started on port {}.", loop_id, m_port);

  while (ShutdownHandler::running.load()) {
    int num_events = epoll_wait(epollfd, events, config::MAX_CONNECTIONS, -1);

    if (num_events == -1) {
      if (errno == EINTR) {
        continue;
      }
      spdlog::error("Epoll wait error on loop {}: {}", loop_id,
                    strerror(errno));
      break;
    }

    for (int i = 0; i < num_events; i++) {
      if (events[i].data.fd == wakeup_fd) {
        uint64_t drained;
        while (read(wakeup_fd, &drained, sizeof(drained)) > 0) {
        }
        continue;
      }
      event_handler.handle_event(events[i]);
    }
  }

  close(epollfd);
  close(listen_fd);
  spdlog::info("Event loop {} exited.", loop_id);
}

void Server::start() {
  std::vector<int> wakeup_fds;

  for (size_t i = 0; i < m_num_loops; i++) {
    int efd = eventfd(0, EFD_NONBLOCK);
    if (efd == -1) {
      spdlog::error("eventfd creation error: {}", strerror(errno));
      break;
    }
    wakeup_fds.push_back(efd);
    ShutdownHandler::register_wakeup(efd);
  }

  if (wakeup_fds.empty()) {
    spdlog::error("No event loops could be started.");
    return;
  }

  spdlog::info("Starting {} parallel event loops on port {}.",
               wakeup_fds.size(), m_port);

  for (size_t i = 0; i < wakeup_fds.size(); i++) {
    int loop_id = static_cast<int>(i);
    int wakeup_fd = wakeup_fds[i];
    m_workers.emplace_back(
        [this, loop_id, wakeup_fd] { run_event_loop(loop_id, wakeup_fd); });
  }

  for (std::thread &worker : m_workers) {
    if (worker.joinable()) {
      worker.join();
    }
  }
  m_workers.clear();

  for (int fd : wakeup_fds) {
    close(fd);
  }

  spdlog::info("All event loops stopped, shutting down gracefully.");
}
