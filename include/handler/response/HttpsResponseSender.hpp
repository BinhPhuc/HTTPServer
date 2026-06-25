#pragma once

#include <cstddef>
#include <openssl/crypto.h>

class HttpsResponseSender {
public:
  static int send_all(SSL *ssl, const char *buf, size_t len);
};
