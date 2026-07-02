#include "handler/shutdown/shutdown.hpp"
#include <sys/socket.h>
#include <unistd.h>

std::atomic<bool> ShutdownHandler::running{true};
std::atomic<int> ShutdownHandler::listen_fd{-1};

void ShutdownHandler::signal_handler(int signum) {
  (void)signum;
  running.store(false);
  int fd = listen_fd.load();
  if (fd != -1) {
    ::shutdown(fd, SHUT_RDWR);
  }
}
