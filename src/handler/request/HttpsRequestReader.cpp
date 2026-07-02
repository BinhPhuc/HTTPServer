#include "enum/ConnectionState.hpp"
#include "utils/Constants.hpp"
#include <cerrno>
#include <handler/request/HttpsRequestReader.hpp>
#include <openssl/ssl.h>
#include <spdlog/spdlog.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

HttpsRequestReader::HttpsRequestReader(SSL *ssl) : ssl(ssl), m_buffer() {}

std::string HttpsRequestReader::read_request() {
  constexpr int buffer_size = 4096;
  char tmp[buffer_size];

  while (m_buffer.find("\r\n\r\n") == std::string::npos) {
    if (m_buffer.length() > config::MAX_HEADER_SIZE) {
      return ConnectionState(ConnectionStateEnum::BAD);
    }
    ssize_t bytes_received = SSL_read(ssl, tmp, buffer_size - 1);
    if (bytes_received == 0) {
      return ConnectionState(ConnectionStateEnum::CLOSED);
    }
    if (bytes_received < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return ConnectionState(ConnectionStateEnum::TIMEOUT);
      }
      return ConnectionState(ConnectionStateEnum::ERROR);
    }
    tmp[bytes_received] = '\0';
    m_buffer.append(tmp, static_cast<size_t>(bytes_received));
  }
  size_t header_end = m_buffer.find("\r\n\r\n");
  if (header_end == std::string::npos) {
    return ConnectionState(ConnectionStateEnum::BAD);
  }
  std::string headers = m_buffer.substr(0, header_end + 4);
  int content_length = get_content_length(headers);
  bool is_multipart = is_multipart_request(headers); // POST request
  if (is_multipart) {
    if (content_length > config::MAX_UPLOAD_SIZE) {
      return ConnectionState(ConnectionStateEnum::TOO_LARGE);
    }
    if (content_length <= 0) {
      return ConnectionState(ConnectionStateEnum::BAD);
    }
  } else {
    if (content_length > config::MAX_BODY_SIZE) {
      return ConnectionState(ConnectionStateEnum::TOO_LARGE);
    }
  }
  if (content_length < 0) {
    return ConnectionState(ConnectionStateEnum::BAD);
  }
  size_t total_length = header_end + 4 + static_cast<size_t>(content_length);
  while (m_buffer.length() < total_length) {
    if (is_multipart && m_buffer.length() >= config::MAX_UPLOAD_SIZE) {
      return ConnectionState(ConnectionStateEnum::TOO_LARGE);
    }
    if (!is_multipart && m_buffer.length() >= config::MAX_BODY_SIZE) {
      return ConnectionState(ConnectionStateEnum::TOO_LARGE);
    }

    ssize_t bytes_received = SSL_read(ssl, tmp, buffer_size - 1);
    if (bytes_received == 0) {
      return ConnectionState(ConnectionStateEnum::CLOSED);
    }
    if (bytes_received < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return ConnectionState(ConnectionStateEnum::TIMEOUT);
      }
      return ConnectionState(ConnectionStateEnum::ERROR);
    }
    tmp[bytes_received] = '\0';
    m_buffer.append(tmp, static_cast<size_t>(bytes_received));
  }
  std::string request = m_buffer.substr(0, total_length);
  m_buffer.erase(0, total_length);
  return request;
}
