#include <handler/request/HttpsRequestReader.hpp>
#include <openssl/ssl.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

std::string HttpsRequestReader::read_request(SSL *ssl) {
  // Flow:
  // 1. Read data from socket until \r\n\r\n is found (end of headers)
  // 2. Parse headers to find Content-Length
  // 3. Read remaining body based on Content-Length
  constexpr int buffer_size = 4096;
  char tmp[buffer_size];
  std::string buffer;
  // Read data from begin to \r\n\r\n
  while (buffer.find("\r\n\r\n") == std::string::npos) {
    ssize_t bytes_received = SSL_read(ssl, tmp, buffer_size - 1);
    if (bytes_received <= 0) {
      return "";
    }
    tmp[bytes_received] = '\0';
    buffer.append(tmp, static_cast<size_t>(bytes_received));
  }
  size_t header_end = buffer.find("\r\n\r\n");
  if (header_end == std::string::npos) {
    return "";
  }
  std::string headers = buffer.substr(0, header_end + 4);
  int content_length = get_content_length(headers);
  size_t total_length = header_end + 4 + static_cast<size_t>(content_length);

  // Read the remaining body if any
  while (buffer.length() < total_length) {
    ssize_t bytes_received = SSL_read(ssl, tmp, buffer_size - 1);
    if (bytes_received <= 0) {
      return "";
    }
    tmp[bytes_received] = '\0';
    buffer.append(tmp, static_cast<size_t>(bytes_received));
  }
  return buffer;
}
