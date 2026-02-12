#pragma once

#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>

class HttpRequest {
public:
  HttpRequest();
  ~HttpRequest();

  void set_request_line(const std::string &line);
  std::string get_request_line() const;

  void set_method(const std::string &method);
  std::string get_method() const;

  void set_path(const std::string &path);
  std::string get_path() const;

  void set_version(const std::string &version);
  std::string get_version() const;

  void set_header(const std::string &key,
                  const std::string &value); // set mean add more headers
  std::unordered_map<std::string, std::vector<std::string>> get_headers() const;
  std::vector<std::string> get_headers(const std::string &key) const;

  void set_body(const std::string &body);
  std::string get_body() const;

private:
  std::string m_request_line; // 3 fields: method, path, version separated by sp
  std::string m_method;
  std::string m_path;
  std::string m_version;
  std::unordered_map<std::string, std::vector<std::string>> m_headers;
  std::string m_body;
};
