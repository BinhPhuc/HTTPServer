#pragma once

#include "http/HttpResponse.hpp"
#include <string>
class HttpResponseBuilder {
public:
  static HttpResponse ok(const std::string &body);
  static HttpResponse not_found(const std::string &body);
  static HttpResponse bad_request(const std::string &body);
  static std::string build_response(const HttpResponse &response);
};
