#include <algorithm>
#include <cctype>
#include <http/HttpRequest.hpp>

HttpRequest::HttpRequest()
    : m_request_line(), m_method(), m_path(), m_version(), m_headers(),
      m_body() {}

HttpRequest::~HttpRequest() { m_headers.clear(); }

void HttpRequest::set_request_line(const std::string &line) {
  m_request_line = line;
}

std::string HttpRequest::get_request_line() const { return m_request_line; }

void HttpRequest::set_method(const std::string &method) { m_method = method; }

std::string HttpRequest::get_method() const { return m_method; }

void HttpRequest::set_path(const std::string &path) { m_path = path; }

std::string HttpRequest::get_path() const { return m_path; }

void HttpRequest::set_version(const std::string &version) {
  m_version = version;
}

std::string HttpRequest::get_version() const { return m_version; }

void HttpRequest::set_header(const std::string &key, const std::string &value) {
  // Because HTTP headers can have multiple values for the same key
  // (case-insensitive) Normalize to loowercase for consistent storage
  std::string normalized_key = key;
  std::transform(normalized_key.begin(), normalized_key.end(), normalized_key.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  m_headers[normalized_key].push_back(value);
}

std::unordered_map<std::string, std::vector<std::string>>
HttpRequest::get_headers() const {
  return m_headers;
}

std::vector<std::string>
HttpRequest::get_headers(const std::string &key) const {
  return get_headers().at(key);
}

void HttpRequest::set_body(const std::string &body) { m_body = body; }

std::string HttpRequest::get_body() const { return m_body; }
