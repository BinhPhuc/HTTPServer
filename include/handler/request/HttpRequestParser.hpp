#pragma once

#include "http/HttpRequest.hpp"
#include <string>

class HttpRequestParser {
private:
public:
  static HttpRequest parse(const std::string &raw_request);
};
