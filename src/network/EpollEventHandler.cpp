#include "network/EpollEventHandler.hpp"
#include "handler/request/HttpRequestParser.hpp"
#include "handler/request/HttpRequestReader.hpp"
#include "handler/response/HttpResponseBuilder.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "network/SocketHelper.hpp"
#include "utils/Constants.hpp"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <spdlog/spdlog.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

EpollEventHandler::EpollEventHandler(int epollfd, int sockfd,
                                     ApiRouter &api_router)
    : m_epollfd(epollfd), m_sockfd(sockfd), m_api_router(api_router), m_connections() {}

void EpollEventHandler::handle_event(const struct epoll_event &event) {
  int current_fd = event.data.fd;
  uint32_t current_event = event.events;

  if (current_fd == m_sockfd) {
    handle_new_connection();
  } else if (current_event & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
    handle_error_event(current_fd);
  } else if (current_event & EPOLLIN) {
    handle_read_event(current_fd);
  } else if (current_event & EPOLLOUT) {
    handle_write_event(current_fd);
  }
}

void EpollEventHandler::handle_new_connection() {
  struct sockaddr_storage their_addr;
  socklen_t addr_size = sizeof(their_addr);
  int client_fd = accept(m_sockfd, (struct sockaddr *)&their_addr, &addr_size);

  if (client_fd == -1) {
    spdlog::error("Accept error: {}", strerror(errno));
    return;
  }

  SocketHelper::set_non_blocking(client_fd);

  struct epoll_event client_event;
  client_event.events = EPOLLIN;
  client_event.data.fd = client_fd;

  if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, client_fd, &client_event) == -1) {
    spdlog::error("Epoll control error: {}", strerror(errno));
    close(client_fd);
    return;
  }

  m_connections[client_fd] = ConnectionState{"", "", client_fd, 0};
  spdlog::info("New connection accepted: {}", client_fd);
}

void EpollEventHandler::handle_read_event(int client_fd) {
  constexpr size_t buffer_size = 4096;
  char buffer[buffer_size];

  ssize_t bytes_received = recv(client_fd, buffer, buffer_size - 1, 0);

  if (bytes_received <= 0) {
    if (bytes_received == 0) {
      spdlog::info("Connection closed by peer: {}", client_fd);
    } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
      spdlog::error("Receive error on fd {}: {}", client_fd, strerror(errno));
    }
    close_connection(client_fd);
    return;
  }

  m_connections[client_fd].read_buffer.append(
      buffer, static_cast<size_t>(bytes_received));

  if (is_request_complete(m_connections[client_fd].read_buffer)) {
    std::string raw_request = m_connections[client_fd].read_buffer;
    HttpRequest request = HttpRequestParser::parse(raw_request);
    HttpResponse response = m_api_router.dispatch(request);
    std::string response_str = HttpResponseBuilder::build_response(response);

    m_connections[client_fd].write_buffer = response_str;
    m_connections[client_fd].read_buffer.clear();
    m_connections[client_fd].write_offset = 0;

    struct epoll_event write_event;
    write_event.events = EPOLLOUT;
    write_event.data.fd = client_fd;

    if (epoll_ctl(m_epollfd, EPOLL_CTL_MOD, client_fd, &write_event) == -1) {
      spdlog::error("Epoll control error: {}", strerror(errno));
      close_connection(client_fd);
    }
  }
}

void EpollEventHandler::handle_write_event(int client_fd) {
  ConnectionState &conn = m_connections[client_fd];
  const std::string &response = conn.write_buffer;
  size_t offset = conn.write_offset;

  ssize_t bytes_sent =
      send(client_fd, response.data() + offset, response.size() - offset, 0);

  if (bytes_sent == -1) {
    spdlog::error("Send error on fd {}: {}", client_fd, strerror(errno));
    close_connection(client_fd);
    return;
  }

  conn.write_offset += static_cast<size_t>(bytes_sent);

  if (conn.write_offset >= response.size()) {
    struct epoll_event read_event;
    read_event.events = EPOLLIN;
    read_event.data.fd = client_fd;

    if (epoll_ctl(m_epollfd, EPOLL_CTL_MOD, client_fd, &read_event) == -1) {
      spdlog::error("Epoll control error: {}", strerror(errno));
      close_connection(client_fd);
    }
  }
}

void EpollEventHandler::handle_error_event(int client_fd) {
  spdlog::error("Epoll error on fd {}: {}", client_fd, strerror(errno));
  close_connection(client_fd);
}

void EpollEventHandler::close_connection(int client_fd) {
  epoll_ctl(m_epollfd, EPOLL_CTL_DEL, client_fd, NULL);
  close(client_fd);
  m_connections.erase(client_fd);
}

bool EpollEventHandler::is_request_complete(const std::string &buffer) const {
  size_t header_end = buffer.find("\r\n\r\n");
  if (header_end == std::string::npos) {
    return false;
  }

  std::string headers = buffer.substr(0, header_end + 4);
  int content_length = HttpRequestReader::get_content_length(headers);
  size_t total_length = header_end + 4 + static_cast<size_t>(content_length);

  return buffer.length() >= total_length;
}
