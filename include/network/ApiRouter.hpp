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
  std::string get_root_folder() const;
  void set_root_folder(const std::string &root_folder);

private:
  std::unordered_map<std::string, Handler> m_routes;
  std::string m_root_folder;
  std::vector<std::string> split_path(const std::string &path) const;
  HttpResponse
  handle_static_file_request(const HttpRequest &request,
                             const std::vector<std::string> &static_files);
  HttpResponse handle_api_request(const HttpRequest &request);
};
