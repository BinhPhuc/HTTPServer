#pragma once

#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include <functional>

class ApiRouter {
public:
  ApiRouter();
  using Handler = std::function<HttpResponse(const HttpRequest &)>;
  void add(const std::string &method, const std::string &path, Handler handler);
  HttpResponse dispatch(const HttpRequest &request);

private:
  std::unordered_map<std::string, Handler> m_routes;
};
