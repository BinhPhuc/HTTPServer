#include "handler/request/HttpRequestReader.hpp"
#include <charconv>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <system_error>

int HttpRequestReader::get_content_length(const std::string &headers) {
  std::string key = "Content-Length:";
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
  (void)ptr;
  if (ec == std::errc{}) {
    return content_length;
  }
  return 0;
}

bool HttpRequestReader::is_multipart_request(const std::string &headers) {
  std::string key = "Content-Type:";
  size_t pos = headers.find(key);
  if (pos == std::string::npos) {
    return false;
  }
  pos += key.length();
  while (pos < headers.length() &&
         (headers[pos] == ' ' || headers[pos] == '\t')) {
    pos++;
  }
  size_t end_pos = headers.find("\r\n", pos);
  std::string value = headers.substr(pos, end_pos - pos);
  return value.find("multipart/form-data") != std::string::npos;
}

std::string HttpRequestReader::read_request(int sockfd) {
  constexpr int buffer_size = 4096;
  int tmp[buffer_size];
  std::string buffer;
  while (buffer.find("\r\n\r\n") == std::string::npos) {
    ssize_t bytes_received = recv(sockfd, tmp, buffer_size - 1, 0);
    if (bytes_received <= 0) {
      return "";
    }
    tmp[bytes_received] = '\0';
    buffer.append(reinterpret_cast<char *>(tmp),
                  static_cast<size_t>(bytes_received));
  }
  size_t header_end = buffer.find("\r\n\r\n");
  if (header_end == std::string::npos) {
    return "";
  }
  std::string headers = buffer.substr(0, header_end + 4);
  int content_length = get_content_length(headers);
  size_t total_length = header_end + 4 + static_cast<size_t>(content_length);

  while (buffer.length() < total_length) {
    ssize_t bytes_received = recv(sockfd, tmp, buffer_size - 1, 0);
    if (bytes_received <= 0) {
      return "";
    }
    tmp[bytes_received] = '\0';
    buffer.append(reinterpret_cast<char *>(tmp),
                  static_cast<size_t>(bytes_received));
  }
  return buffer;
}
