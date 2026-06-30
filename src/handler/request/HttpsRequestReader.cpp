#include "enum/HttpEnum.hpp"
#include "utils/Constants.hpp"
#include <handler/request/HttpsRequestReader.hpp>
#include <openssl/ssl.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

std::string HttpsRequestReader::read_request(SSL *ssl) {
  constexpr int buffer_size = 4096;
  char tmp[buffer_size];
  std::string buffer;

  while (buffer.find("\r\n\r\n") == std::string::npos) {
    ssize_t bytes_received = SSL_read(ssl, tmp, buffer_size - 1);
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
  while (buffer.length() < total_length) {
    if (buffer.length() >= config::MAX_BODY_SIZE) {
      return HttpResponseStatusMessage(
          HttpResponseStatusMessageEnum::CONTENT_TOO_LARGE);
    }

    ssize_t bytes_received = SSL_read(ssl, tmp, buffer_size - 1);
    if (bytes_received <= 0) {
      return HttpResponseStatusMessage(
          HttpResponseStatusMessageEnum::INTERNAL_SERVER_ERROR);
    }
    tmp[bytes_received] = '\0';
    buffer.append(tmp, static_cast<size_t>(bytes_received));
  }
  return buffer;
}
