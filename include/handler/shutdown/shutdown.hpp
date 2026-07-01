#pragma once

#include <atomic>
#include <openssl/err.h>

class ShutdownHandler {
public:
  static std::atomic<bool> running;
  static std::atomic<int> listen_fd;

  static void close_connection(SSL *ssl, int fd, bool drain_input);
  static void signal_handler(int signum);
};
