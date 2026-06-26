#include "handler/response/HttpsResponseSender.hpp"
#include <cstddef>
#include <openssl/ssl.h>
#include <sys/socket.h>

int HttpsResponseSender::send_all(SSL *ssl, const char *buf, size_t len) {
  if (len == 0 || buf == nullptr) {
    return -1;
  }
  size_t total_sent = 0;
  const size_t total_len = static_cast<size_t>(len);
  while (total_sent < total_len) {
    const size_t remaining = total_len - total_sent;
    const ssize_t sent =
        SSL_write(ssl, buf + total_sent, static_cast<int>(remaining));
    if (sent <= 0) {
      return -1; // Send error or connection closed
    }
    total_sent += static_cast<size_t>(sent);
  }
  return 0; // Success
}
