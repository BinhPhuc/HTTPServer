#include "handler/request/HttpRequestReader.hpp"
#include "enum/HttpEnum.hpp"
#include "utils/Constants.hpp"
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

std::string HttpRequestReader::read_request(int sockfd) {
  // Flow:
  // 1. Read data from socket until \r\n\r\n is found (end of headers)
  // 2. Parse headers to find Content-Length
  // 3. Read remaining body based on Content-Length
  constexpr int buffer_size = 4096;
  char tmp[buffer_size];
  std::string buffer;
  // Read data from begin to \r\n\r\n
  while (buffer.find("\r\n\r\n") == std::string::npos) {
    ssize_t bytes_received = recv(sockfd, tmp, buffer_size - 1, 0);
    if (bytes_received <= 0) {
      return HttpResponseStatusMessage(
          HttpResponseStatusMessageEnum::INTERNAL_SERVER_ERROR);
    }
    tmp[bytes_received] = '\0';
    buffer.append(tmp, static_cast<size_t>(bytes_received));
  }
  size_t header_end = buffer.find("\r\n\r\n");
  if (header_end == std::string::npos) {
    return HttpResponseStatusMessage(
        HttpResponseStatusMessageEnum::BAD_REQUEST);
  }
  std::string headers = buffer.substr(0, header_end + 4);
  // Assume that every reqeust has Content-Length header, if not, return 400 Bad
  // Request
  int content_length = get_content_length(headers);
  bool is_multipart = is_multipart_request(headers);
  if (is_multipart) {
    if (content_length > config::MAX_UPLOAD_SIZE) {
      return HttpResponseStatusMessage(
          HttpResponseStatusMessageEnum::CONTENT_TOO_LARGE);
    }
  } else {
    if (content_length > config::MAX_BODY_SIZE) {
      return HttpResponseStatusMessage(
          HttpResponseStatusMessageEnum::CONTENT_TOO_LARGE);
    }
  }
  if (content_length <= 0) {
    return HttpResponseStatusMessage(
        HttpResponseStatusMessageEnum::BAD_REQUEST);
  }
  size_t total_length = header_end + 4 + static_cast<size_t>(content_length);

  // Read the remaining body if any
  while (buffer.length() < total_length) {
    if (buffer.length() >= config::MAX_BODY_SIZE) {
      return HttpResponseStatusMessage(
          HttpResponseStatusMessageEnum::CONTENT_TOO_LARGE);
    }
    ssize_t bytes_received = recv(sockfd, tmp, buffer_size - 1, 0);
    if (bytes_received <= 0) {
      return HttpResponseStatusMessage(
          HttpResponseStatusMessageEnum::INTERNAL_SERVER_ERROR);
    }
    tmp[bytes_received] = '\0';
    buffer.append(tmp, static_cast<size_t>(bytes_received));
  }
  return buffer;
}
