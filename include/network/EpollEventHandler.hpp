#pragma once

#include "network/ApiRouter.hpp"
#include <string>
#include <sys/epoll.h>
#include <unordered_map>

class EpollEventHandler {
private:
  struct ConnectionState {
    std::string read_buffer;
    std::string write_buffer;
    int fd;
    size_t write_offset;
    bool keep_alive;

    ConnectionState(const std::string &rb = "", const std::string &wb = "",
                    int f = -1, size_t wo = 0, bool ka = true)
        : read_buffer(rb), write_buffer(wb), fd(f), write_offset(wo),
          keep_alive(ka) {}
  };

  int m_epollfd;
  int m_sockfd;
  ApiRouter &m_api_router;
  std::unordered_map<int, ConnectionState> m_connections;

  void handle_new_connection();
  void handle_read_event(int client_fd);
  void handle_write_event(int client_fd);
  void handle_error_event(int client_fd);
  void close_connection(int client_fd);
  bool is_request_complete(const std::string &buffer) const;
  bool exceeds_size_limits(const std::string &buffer, HttpResponse &error) const;
  void queue_response(int client_fd, const HttpResponse &response,
                      bool keep_alive);
  bool compute_keep_alive(const HttpRequest &request) const;

public:
  EpollEventHandler(int epollfd, int sockfd, ApiRouter &api_router);
  void handle_event(const struct epoll_event &event);
};
