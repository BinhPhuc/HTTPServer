#pragma once

#include <string>
#include <unordered_map>

class HttpResponse {
public:
  HttpResponse();
  ~HttpResponse();

  void set_status_line(const std::string &line);
  std::string get_status_line() const;

  void set_protocol_version(const std::string &version);
  std::string get_protocol_version() const;

  void set_status_code(const std::string &code);
  std::string get_status_code() const;

  void set_status_message(const std::string &message);
  std::string get_status_message() const;

  void set_header(const std::string &key,
                  const std::string &value); // set mean add more headers
  std::unordered_map<std::string, std::string> get_headers() const;
  std::string get_headers(const std::string &key) const;

  void set_body(const std::string &body);
  std::string get_body() const;

private:
  std::string m_status_line;
  std::string m_protocol_version;
  std::string m_status_code;
  std::string m_status_message;
  std::unordered_map<std::string, std::string> m_headers;
  std::string m_body;
};
