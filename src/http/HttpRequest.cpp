#include <http/HttpRequest.hpp>
#include <utility>
#include <utils/Helper.hpp>

HttpRequest::HttpRequest()
    : m_request_line(), m_path_only(), m_method(), m_path(), m_version(),
      m_header(), m_body() {}

HttpRequest::~HttpRequest() { m_header.clear(); }

void HttpRequest::set_request_line(const std::string &line) {
  m_request_line = line;
}

std::string HttpRequest::get_request_line() const { return m_request_line; }

void HttpRequest::set_path_only(const std::string &path_only) {
  m_path_only = path_only;
}

std::string HttpRequest::get_path_only() const { return m_path_only; }

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
  std::string normalized_key = utils::lowercase(key);
  auto it = m_header.find(normalized_key);
  if (it == m_header.end()) {
    m_header[normalized_key] = value;
  } else {
    it->second += ", " + value;
  }
}

std::string HttpRequest::get_header(const std::string &key) const {
  std::string normalized_key = utils::lowercase(key);
  auto it = m_header.find(normalized_key);
  return it == m_header.end() ? "" : it->second;
}

void HttpRequest::set_body(const std::string &body) { m_body = body; }

void HttpRequest::set_body(std::string &&body) { m_body = std::move(body); }

const std::string &HttpRequest::get_body() const { return m_body; }
