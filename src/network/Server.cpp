#include "utils/Constants.hpp"
#include "utils/Logger.hpp"
#include <asm-generic/socket.h>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <network/Server.hpp>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

Server::Server(int port) : m_port(port), m_sockfd(-1) {}

Server::~Server() { stopServer(); }

void Server::stopServer() {
  if (m_sockfd != -1) {
    Logger::getInstance(config::SERVER_LOG_PATH)
        .log(INFO, "Stopping server on port " + std::to_string(m_port) + ".");

    close(m_sockfd);
    m_sockfd = -1;
  }
  Logger::getInstance(config::SERVER_LOG_PATH).log(INFO, "Server stopped.");
}

void Server::start() {
  // getaddrinfo -> get socket fd -> bind -> listen -> accept
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int status;
  if ((status = getaddrinfo(NULL, std::to_string(m_port).c_str(), &hints,
                            &res) != 0)) {
    Logger::getInstance(config::SERVER_LOG_PATH)
        .log(ERROR, "getaddrinfo error: " + std::string(gai_strerror(status)));
    return;
  }

  struct addrinfo *p;
  for (p = res; p != NULL; p = p->ai_next) {
    // create socket
    if ((m_sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==
        -1) {
      Logger::getInstance(config::SERVER_LOG_PATH)
          .log(ERROR, "Socket creation error: " + std::string(strerror(errno)));
      continue;
    }

    int yes = 1;
    if (setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) ==
        -1) {
      Logger::getInstance(config::SERVER_LOG_PATH)
          .log(ERROR, "Setsockopt error: " + std::string(strerror(errno)));
      close(m_sockfd);
      continue;
    }

    // bind
    if (bind(m_sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(m_sockfd);
      Logger::getInstance(config::SERVER_LOG_PATH)
          .log(ERROR, "Bind error: " + std::string(strerror(errno)));
      continue;
    }

    break; // bind success
  }

  freeaddrinfo(res);

  if (p == NULL) {
    Logger::getInstance(config::SERVER_LOG_PATH)
        .log(ERROR, "Failed to bind socket.");
    return;
  }

  // listen
  if (listen(m_sockfd, config::MAX_CONNECTIONS) == -1) {
    Logger::getInstance(config::SERVER_LOG_PATH)
        .log(ERROR, "Listen error: " + std::string(strerror(errno)));
    return;
  }

  Logger::getInstance(config::SERVER_LOG_PATH)
      .log(INFO, "Server started on port " + std::to_string(m_port) + ".");

  struct sockaddr_storage their_addr;
  while (1) {
    // accept connections
    socklen_t addr_size = sizeof(their_addr);
    int new_fd = accept(m_sockfd, (struct sockaddr *)&their_addr, &addr_size);
    if (new_fd == -1) {
      Logger::getInstance(config::SERVER_LOG_PATH)
          .log(ERROR, "Accept error: " + std::string(strerror(errno)));
      continue;
    }

    // simple HTTP response
    std::string body = "Hello World! Your Server is running.\n";
    std::string response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: " +
                           std::to_string(body.size()) +
                           "\r\n"
                           "\r\n" +
                           body;
    if (send(new_fd, response.c_str(), response.size(), 0) == -1) {
      Logger::getInstance(config::SERVER_LOG_PATH)
          .log(ERROR, "Send error: " + std::string(strerror(errno)));
    }
    close(new_fd);
  }
}
