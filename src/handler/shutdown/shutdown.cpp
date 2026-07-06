#include "handler/shutdown/shutdown.hpp"
#include <cstdint>
#include <unistd.h>

std::atomic<bool> ShutdownHandler::running{true};
std::atomic<int> ShutdownHandler::wakeup_fds[ShutdownHandler::MAX_WAKEUP_FDS];
std::atomic<size_t> ShutdownHandler::wakeup_count{0};

void ShutdownHandler::register_wakeup(int fd) {
  size_t idx = wakeup_count.load();
  if (idx < MAX_WAKEUP_FDS) {
    wakeup_fds[idx].store(fd);
    wakeup_count.store(idx + 1);
  }
}

void ShutdownHandler::signal_handler(int signum) {
  (void)signum;
  running.store(false);

  size_t count = wakeup_count.load();
  if (count > MAX_WAKEUP_FDS) {
    count = MAX_WAKEUP_FDS;
  }

  uint64_t one = 1;
  for (size_t i = 0; i < count; ++i) {
    int fd = wakeup_fds[i].load();
    if (fd != -1) {
      ssize_t n = write(fd, &one, sizeof(one));
      (void)n;
    }
  }
}
