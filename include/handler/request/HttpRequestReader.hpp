#pragma once

#include "handler/request/BaseReader.hpp"
#include <string>

class HttpRequestReader : public BaseReader {
public:
  static std::string read_request(int sockfd);
};
