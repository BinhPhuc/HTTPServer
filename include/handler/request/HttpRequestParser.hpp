#pragma once

#include "http/HttpRequest.hpp"
#include <string>

class HttpRequestParser {
private:
public:
  static std::pair<HttpRequest, bool> parse(const std::string &raw_request);
};
