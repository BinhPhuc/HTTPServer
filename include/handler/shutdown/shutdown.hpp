#pragma once

#include <openssl/err.h>

class ShutdownHandler {
public:
  static void close_connection(SSL *ssl, int fd, bool drain_input);
  static void signal_handler(int signum);
};
