#pragma once

#include <cstddef>

class HttpResponseSender {
public:
  static int send_all(int sockfd, const char *buf, size_t len);
};
