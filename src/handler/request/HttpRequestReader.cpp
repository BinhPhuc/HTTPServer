#include "handler/request/HttpRequestReader.hpp"
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

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
  return std::stoi(value);
}

std::string HttpRequestReader::read_request(int sockfd) {
  // Flow:
  // 1. Read data from socket until \r\n\r\n is found (end of headers)
  // 2. Parse headers to find Content-Length
  // 3. Read remaining body based on Content-Length
  constexpr int buffer_size = 4096;
  int tmp[buffer_size];
  std::string buffer;
  // Read data from begin to \r\n\r\n
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

  // Read the remaining body if any
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
