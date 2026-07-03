#pragma once

#include "utils/Constants.hpp"
#include <memory>
#include <openssl/ssl.h>

struct OpenSSLDeleter {
  void operator()(SSL *ssl) const {
    if (ssl) {
      SSL_shutdown(ssl);
      SSL_free(ssl);
    }
  }
  void operator()(SSL_CTX *ctx) const {
    if (ctx) {
      SSL_CTX_free(ctx);
    }
  }
  void operator()(BIO *bio) const {
    if (bio) {
      BIO_free_all(bio);
    }
  }
};

using SSL_CTX_ptr = std::unique_ptr<SSL_CTX, OpenSSLDeleter>;
using SSL_ptr = std::unique_ptr<SSL, OpenSSLDeleter>;
using BIO_ptr = std::unique_ptr<BIO, OpenSSLDeleter>;

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
