
#include <charconv>
#include <handler/request/HttpsRequestReader.hpp>
#include <openssl/ssl.h>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <sys/types.h>
#include <system_error>

int HttpsRequestReader::get_content_length(const std::string &headers) {
  std::string_view key = "Content-Length:";
  size_t pos = headers.find(key);
  if (pos == std::string::npos) {
    return 0;
  }
  pos += key.length();
  while (pos < headers.length() &&
         (headers[pos] == ' ' || headers[pos] == '\t')) {
    pos++;
  }
  size_t end_pos = headers.find("\r\n", pos);
  std::string value = headers.substr(pos, end_pos - pos);
  int content_length = 0;
  auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(),
                                   content_length);
  if (ec == std::errc()) {
    return content_length;
  } else if (ec == std::errc::invalid_argument) {
    return 0;
  } else if (ec == std::errc::result_out_of_range) {
    return 0;
  }
  return content_length;
}

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
