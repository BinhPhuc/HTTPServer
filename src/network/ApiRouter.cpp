#include "network/ApiRouter.hpp"
#include "handler/response/HttpResponseBuilder.hpp"

ApiRouter::ApiRouter(): m_routes() {}

void ApiRouter::add(const std::string &method, const std::string &path,
                    Handler handler) {
  std::string key = method + " " + path;
  m_routes[key] = handler;
}

HttpResponse ApiRouter::dispatch(const HttpRequest &request) {
  std::string key = request.get_method() + " " + request.get_path();
  auto it = m_routes.find(key);
  if (it != m_routes.end()) {
    return it->second(request);
  } else {
    return HttpResponseBuilder::not_found("Not Found");
  }
}
