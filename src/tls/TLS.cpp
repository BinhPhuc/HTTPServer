#include <openssl/err.h>
#include <openssl/ssl.h>
#include <spdlog/spdlog.h>
#include <tls/TLS.hpp>

std::string getOpenSSLError() {
  unsigned long err = ERR_get_error();

  if (err == 0) {
    return "Unknown OpenSSL error";
  }

  char buffer[256];
  ERR_error_string_n(err, buffer, sizeof(buffer));
  return std::string(buffer);
}

SSL_CTX_ptr TLS::create_context() {
  SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
  if (!ctx) {
    spdlog::error("Unable to create SSL context");
    throw std::runtime_error("Unable to create SSL context: " +
                             getOpenSSLError());
  }
  return SSL_CTX_ptr(ctx);
}

void TLS::configure_context(SSL_CTX *ctx, const std::string &cert_file,
                            const std::string &key_file) {
  SSL_CTX_set_ecdh_auto(ctx, 1);
  if (SSL_CTX_use_certificate_file(ctx, cert_file.c_str(), SSL_FILETYPE_PEM) <=
      0) {
    spdlog::error("Unable to load certificate file: {}", cert_file);
    throw std::runtime_error("Unable to load certificate file: " +
                             getOpenSSLError());
  }

  if (SSL_CTX_use_PrivateKey_file(ctx, key_file.c_str(), SSL_FILETYPE_PEM) <=
      0) {
    spdlog::error("Unable to load private key file: {}", key_file);
    throw std::runtime_error("Unable to load private key file: " +
                             getOpenSSLError());
  }
}

int TLS::set_fd(SSL *ssl, int fd) {
  int ret = SSL_set_fd(ssl, fd);
  if (ret <= 0) {
    spdlog::error("Unable to set file descriptor for SSL: {}",
                  getOpenSSLError());
    throw std::runtime_error("Unable to set file descriptor for SSL: " +
                             getOpenSSLError());
  }
  return ret;
}

int TLS::accept(SSL *ssl) {
  int ret = SSL_accept(ssl);
  if (ret <= 0) {
    int err = SSL_get_error(ssl, ret);
    spdlog::error("TLS handshake failed (err={}): {}", err, getOpenSSLError());
  }
  return ret;
}
