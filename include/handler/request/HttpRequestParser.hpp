#pragma once

#include "http/HttpRequest.hpp"
#include <string>
#include <utility>

class HttpRequestParser {
private:
public:
  static std::pair<HttpRequest, bool> parse(const std::string &raw_request);
};
