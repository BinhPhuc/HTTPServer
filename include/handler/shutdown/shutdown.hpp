#pragma once

#include <atomic>
#include <cstddef>

class ShutdownHandler {
public:
  static constexpr size_t MAX_WAKEUP_FDS = 128;

  static std::atomic<bool> running;
  static std::atomic<int> wakeup_fds[MAX_WAKEUP_FDS];
  static std::atomic<size_t> wakeup_count;

  static void register_wakeup(int fd);

  static void signal_handler(int signum);
};
