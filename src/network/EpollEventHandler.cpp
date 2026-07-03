#include "network/EpollEventHandler.hpp"
#include "enum/Phase.hpp"
#include "handler/request/HttpRequestParser.hpp"
#include "handler/request/HttpRequestReader.hpp"
#include "handler/response/HttpResponseBuilder.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "tls/TLS.hpp"
#include "utils/Constants.hpp"
#include "utils/Helper.hpp"
#include <cerrno>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <spdlog/spdlog.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

EpollEventHandler::EpollEventHandler(int epollfd, int sockfd,
                                     ApiRouter &api_router)
    : m_epollfd(epollfd), m_sockfd(sockfd), m_api_router(api_router),
      m_connections(), m_ssl_ctx(TLS::create_context()) {
  TLS::configure_context(m_ssl_ctx.get());
}

void EpollEventHandler::handle_event(const struct epoll_event &event) {
  int current_fd = event.data.fd;
  uint32_t current_event = event.events;

  if (current_fd == m_sockfd) {
    handle_new_connection();
    return;
  }

  if (current_event & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
    handle_error_event(current_fd);
    return;
  }

  ConnectionState &conn = m_connections[current_fd];
  if (conn.phase == PhaseEnum::HANSHAKING) {
    handshaking(current_fd);
  } else if (conn.phase == PhaseEnum::READING) {
    handle_read_event(current_fd);
  } else if (conn.phase == PhaseEnum::WRITING) {
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

  int flags = fcntl(client_fd, F_GETFL, 0);
  if (flags == -1) {
    spdlog::error("Fcntl get flags error: {}", strerror(errno));
  } else if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    spdlog::error("Fcntl set non-blocking error: {}", strerror(errno));
  }

  struct epoll_event client_event;
  client_event.events = EPOLLIN;
  client_event.data.fd = client_fd;

  if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, client_fd, &client_event) == -1) {
    spdlog::error("Epoll control error: {}", strerror(errno));
    close(client_fd);
    return;
  }

  SSL_ptr ssl(SSL_new(m_ssl_ctx.get()));
  TLS::set_fd(ssl.get(), client_fd);

  m_connections[client_fd] = ConnectionState{
      "", "", PhaseEnum::HANSHAKING, client_fd, 0, std::move(ssl)};
  handshaking(client_fd);
}

void EpollEventHandler::handshaking(int client_fd) {
  ConnectionState &conn = m_connections[client_fd];
  SSL_ptr &ssl = conn.ssl;

  int ret = SSL_accept(ssl.get());
  if (ret == 1) {
    conn.phase = PhaseEnum::READING;
    spdlog::info("TLS handshake completed for fd {}", client_fd);
  } else {
    int error = SSL_get_error(ssl.get(), ret);
    if (error == SSL_ERROR_WANT_READ) {
      struct epoll_event read_event;
      read_event.events = EPOLLIN;
      read_event.data.fd = client_fd;
      if (epoll_ctl(m_epollfd, EPOLL_CTL_MOD, client_fd, &read_event) == -1) {
        spdlog::error("Epoll control error: {}", strerror(errno));
        close_connection(client_fd);
        return;
      }
    } else if (error == SSL_ERROR_WANT_WRITE) {
      struct epoll_event write_event;
      write_event.events = EPOLLOUT;
      write_event.data.fd = client_fd;
      if (epoll_ctl(m_epollfd, EPOLL_CTL_MOD, client_fd, &write_event) == -1) {
        spdlog::error("Epoll control error: {}", strerror(errno));
        close_connection(client_fd);
        return;
      }
    } else {
      spdlog::error("TLS handshake failed for fd {}", client_fd);
      close_connection(client_fd);
      return;
    }
  }
}

void EpollEventHandler::handle_read_event(int client_fd) {
  ConnectionState &conn = m_connections[client_fd];
  SSL_ptr &ssl = conn.ssl;

  constexpr size_t buffer_size = 4096;
  char buffer[buffer_size];

  int bytes_received = SSL_read(ssl.get(), buffer, buffer_size - 1);

  if (bytes_received <= 0) {
    if (bytes_received == 0) {
      spdlog::info("Connection closed by peer: {}", client_fd);
      close_connection(client_fd);
    } else {
      int error = SSL_get_error(ssl.get(), bytes_received);
      if (error == SSL_ERROR_WANT_READ) {
        struct epoll_event read_event;
        read_event.events = EPOLLIN;
        read_event.data.fd = client_fd;
        if (epoll_ctl(m_epollfd, EPOLL_CTL_MOD, client_fd, &read_event) == -1) {
          spdlog::error("Epoll control error: {}", strerror(errno));
          close_connection(client_fd);
          return;
        }
      } else if (error == SSL_ERROR_WANT_WRITE) {
        struct epoll_event write_event;
        write_event.events = EPOLLOUT;
        write_event.data.fd = client_fd;
        if (epoll_ctl(m_epollfd, EPOLL_CTL_MOD, client_fd, &write_event) ==
            -1) {
          spdlog::error("Epoll control error: {}", strerror(errno));
          close_connection(client_fd);
          return;
        }
      } else {
        spdlog::error("SSL read error on fd {}: {}", client_fd, error);
        close_connection(client_fd);
      }
    }
    return;
  }

  conn.read_buffer.append(buffer, static_cast<size_t>(bytes_received));

  HttpResponse error;
  if (exceeds_size_limits(conn.read_buffer, error)) {
    queue_response(client_fd, error, false);
    return;
  }

  if (!is_request_complete(conn.read_buffer)) {
    return;
  }

  size_t header_end = conn.read_buffer.find("\r\n\r\n");
  std::string headers = conn.read_buffer.substr(0, header_end + 4);
  int content_length = HttpRequestReader::get_content_length(headers);
  size_t total_length = header_end + 4 + static_cast<size_t>(content_length);
  std::string raw_request = conn.read_buffer.substr(0, total_length);

  HttpResponse response;
  bool keep_alive = true;
  try {
    HttpRequest request = HttpRequestParser::parse(raw_request);
    response = m_api_router.dispatch(request);
    keep_alive = compute_keep_alive(request);
    response.set_header("Connection", keep_alive ? "keep-alive" : "close");
  } catch (const std::exception &e) {
    spdlog::error("Error handling request on fd {}: {}", client_fd, e.what());
    response =
        HttpResponseBuilder::internal_server_error("Internal Server Error");
    response.set_header("Connection", "close");
    keep_alive = false;
  }

  conn.read_buffer.erase(0, total_length);

  queue_response(client_fd, response, keep_alive);
}

void EpollEventHandler::handle_write_event(int client_fd) {
  ConnectionState &conn = m_connections[client_fd];
  SSL_ptr &ssl = conn.ssl;

  const std::string &response = conn.write_buffer;
  size_t offset = conn.write_offset;

  ssize_t bytes_sent =
      SSL_write(ssl.get(), response.data() + offset,
                static_cast<int>(response.size()) - static_cast<int>(offset));

  if (bytes_sent <= 0) {
    int error = SSL_get_error(ssl.get(), static_cast<int>(bytes_sent));
    if (error == SSL_ERROR_WANT_READ) {
      struct epoll_event read_event;
      read_event.events = EPOLLIN;
      read_event.data.fd = client_fd;
      if (epoll_ctl(m_epollfd, EPOLL_CTL_MOD, client_fd, &read_event) == -1) {
        spdlog::error("Epoll control error: {}", strerror(errno));
        close_connection(client_fd);
        return;
      }
    } else if (error == SSL_ERROR_WANT_WRITE) {
      struct epoll_event write_event;
      write_event.events = EPOLLOUT;
      write_event.data.fd = client_fd;
      if (epoll_ctl(m_epollfd, EPOLL_CTL_MOD, client_fd, &write_event) == -1) {
        spdlog::error("Epoll control error: {}", strerror(errno));
        close_connection(client_fd);
        return;
      }
    } else if (error == SSL_ERROR_ZERO_RETURN) {
      spdlog::info("Connection closed by peer during write: {}", client_fd);
      close_connection(client_fd);
    } else if (error == SSL_ERROR_SYSCALL) {
      spdlog::error("SSL write syscall error on fd {}: {}", client_fd,
                    strerror(errno));
      close_connection(client_fd);
    } else {
      spdlog::error("SSL write error on fd {}: {}", client_fd, error);
      close_connection(client_fd);
    }
    return;
  }

  conn.write_offset += static_cast<size_t>(bytes_sent);

  if (conn.write_offset >= response.size()) {
    if (!conn.keep_alive) {
      close_connection(client_fd);
      return;
    }

    conn.write_buffer.clear();
    conn.write_offset = 0;
    conn.phase = PhaseEnum::READING;

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

bool EpollEventHandler::exceeds_size_limits(const std::string &buffer,
                                            HttpResponse &error) const {
  size_t header_end = buffer.find("\r\n\r\n");
  if (header_end == std::string::npos) {
    if (buffer.size() > static_cast<size_t>(config::MAX_HEADER_SIZE)) {
      error = HttpResponseBuilder::bad_request("Request header too large");
      return true;
    }
    return false;
  }

  std::string headers = buffer.substr(0, header_end + 4);
  if (headers.size() > static_cast<size_t>(config::MAX_HEADER_SIZE)) {
    error = HttpResponseBuilder::bad_request("Request header too large");
    return true;
  }

  int content_length = HttpRequestReader::get_content_length(headers);
  bool multipart = HttpRequestReader::is_multipart_request(headers);
  size_t max_body = multipart ? static_cast<size_t>(config::MAX_UPLOAD_SIZE)
                              : static_cast<size_t>(config::MAX_BODY_SIZE);
  if (content_length > 0 && static_cast<size_t>(content_length) > max_body) {
    error = HttpResponseBuilder::content_too_large("Request body too large");
    return true;
  }

  return false;
}

void EpollEventHandler::queue_response(int client_fd,
                                       const HttpResponse &response,
                                       bool keep_alive) {
  ConnectionState &conn = m_connections[client_fd];
  conn.write_buffer = HttpResponseBuilder::build_response(response);
  conn.write_offset = 0;
  conn.keep_alive = keep_alive;
  conn.phase = PhaseEnum::WRITING;

  struct epoll_event write_event;
  write_event.events = EPOLLOUT;
  write_event.data.fd = client_fd;

  if (epoll_ctl(m_epollfd, EPOLL_CTL_MOD, client_fd, &write_event) == -1) {
    spdlog::error("Epoll control error: {}", strerror(errno));
    close_connection(client_fd);
  }
}

bool EpollEventHandler::compute_keep_alive(const HttpRequest &request) const {
  std::string version = request.get_version();
  bool keep_alive = (version == "HTTP/1.1");

  std::string connection = request.get_header("Connection");
  if (!connection.empty()) {
    std::string lower = utils::lowercase(connection);
    if (lower.find("keep-alive") != std::string::npos) {
      keep_alive = true;
    } else if (lower.find("close") != std::string::npos) {
      keep_alive = false;
    }
  }

  return keep_alive;
}
