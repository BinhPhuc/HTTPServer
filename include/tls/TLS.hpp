#pragma once

#include "utils/Constants.hpp"
#include <memory>
#include <openssl/ssl.h>

using SSL_CTX_ptr = std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)>;

class TLS {
public:
  static SSL_CTX_ptr create_context();
  static void
  configure_context(SSL_CTX *ctx,
                    const std::string &cert_file = config::CERT_FILE,
                    const std::string &key_file = config::KEY_FILE);
  static int set_fd(SSL *ssl, int fd);
  static int accept(SSL *ssl);
};
