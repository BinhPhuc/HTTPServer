#pragma once

#include <atomic>

class ShutdownHandler {
public:
  static std::atomic<bool> running;
  static std::atomic<int> listen_fd;

  static void signal_handler(int signum);
};
