#pragma once

#include "enum/Phase.hpp"
#include "network/ApiRouter.hpp"
#include "tls/TLS.hpp"
#include <openssl/err.h>
#include <string>
#include <sys/epoll.h>
#include <unordered_map>

class EpollEventHandler {
private:
  struct ConnectionState {
    std::string read_buffer;
    std::string write_buffer;
    PhaseEnum phase;
    int fd;
    size_t write_offset;
    SSL_ptr ssl;
    bool keep_alive;

    ConnectionState(const std::string &rb = "", const std::string &wb = "",
                    PhaseEnum ph = PhaseEnum::HANSHAKING, int f = -1,
                    size_t wo = 0, SSL_ptr ssl_ptr = nullptr, bool ka = true)
        : read_buffer(rb), write_buffer(wb), phase(ph), fd(f), write_offset(wo),
          ssl(std::move(ssl_ptr)), keep_alive(ka) {}
  };

  int m_epollfd;
  int m_sockfd;
  ApiRouter &m_api_router;
  std::unordered_map<int, ConnectionState> m_connections;
  SSL_CTX *m_ssl_ctx;

  void handle_new_connection();
  void handle_read_event(int client_fd);
  void handle_write_event(int client_fd);
  void handle_error_event(int client_fd);
  void close_connection(int client_fd);
  void start_graceful_close(int client_fd);
  void handle_close_event(int client_fd);
  void handshaking(int client_fd);
  bool is_request_complete(const std::string &buffer) const;
  bool exceeds_size_limits(const std::string &buffer,
                           HttpResponse &error) const;
  void queue_response(int client_fd, const HttpResponse &response,
                      bool keep_alive);
  bool compute_keep_alive(const HttpRequest &request) const;

public:
  EpollEventHandler(int epollfd, int sockfd, ApiRouter &api_router,
                    SSL_CTX *ssl_ctx);
  EpollEventHandler(const EpollEventHandler &) = delete;
  EpollEventHandler &operator=(const EpollEventHandler &) = delete;
  void handle_event(const struct epoll_event &event);
};
