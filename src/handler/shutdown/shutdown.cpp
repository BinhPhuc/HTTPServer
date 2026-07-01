#include "handler/shutdown/shutdown.hpp"
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <unistd.h>

void ShutdownHandler::close_connection(SSL *ssl, int fd, bool drain_input) {
  if (drain_input) {
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    char sink[4096];
    while (SSL_read(ssl, sink, sizeof(sink)) > 0) {
    }
  }
  SSL_shutdown(ssl);
  close(fd);
}

void ShutdownHandler::signal_handler(int signum) { exit(signum); }
