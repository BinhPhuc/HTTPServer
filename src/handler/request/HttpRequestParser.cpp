#include "handler/request/HttpRequestParser.hpp"
#include "http/HttpRequest.hpp"

HttpRequest HttpRequestParser::parse(const std::string &raw_request) {
  HttpRequest request;

  size_t pos = 0;
  size_t line_end = raw_request.find("\r\n", pos);
  if (line_end == std::string::npos) {
    // Invalid request
    // TODO: Return HttpResponse with error status code 400 Bad Request
    return request;
  }

  // Parse request line
  std::string request_line = raw_request.substr(pos, line_end - pos);
  request.set_request_line(request_line);

  // Parse method, path, version
  size_t method_end = request_line.find(' ');
  size_t path_end = request_line.find(' ', method_end + 1);
  if (method_end == std::string::npos || path_end == std::string::npos) {
    // Invalid request line
    // TODO: Return HttpResponse with error status code 400 Bad Request
    return request;
  }
  std::string method = request_line.substr(0, method_end);
  std::string path =
      request_line.substr(method_end + 1, path_end - method_end - 1);
  size_t query_pos = path.find("?");
  std::string path_only = path;
  if (query_pos != std::string::npos) {
    path_only = path.substr(0, query_pos);
  }
  std::string version = request_line.substr(path_end + 1);
  request.set_method(method);
  request.set_path(path);
  request.set_path_only(path_only);
  request.set_version(version);

  // Parse headers
  pos = line_end + 2;
  while (true) {
    line_end = raw_request.find("\r\n", pos);
    if (line_end == std::string::npos || line_end == pos) {
      break;
    }
    std::string header_line = raw_request.substr(pos, line_end - pos);
    size_t colon_pos = header_line.find(':');
    if (colon_pos != std::string::npos) {
      std::string key = header_line.substr(0, colon_pos);
      std::string value = header_line.substr(colon_pos + 1);
      size_t first_non_space = value.find_first_not_of(' ');
      if (first_non_space != std::string::npos) {
        value = value.substr(first_non_space);
      }
      request.set_header(key, value);
    }
    pos = line_end + 2;
  }

  // Parse body
  if (pos < raw_request.size()) {
    std::string body = raw_request.substr(pos);
    request.set_body(body);
  }

  return request;
}
