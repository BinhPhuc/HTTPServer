#pragma once

#include <string>

class HttpRequestReader {
public:
  static int get_content_length(const std::string &headers);
  static std::string read_request(int sockfd);
};
