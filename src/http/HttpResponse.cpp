#include <algorithm>
#include <http/HttpResponse.hpp>

HttpResponse::HttpResponse()
    : m_status_line(), m_protocol_version(), m_status_code(),
      m_status_message(), m_headers(), m_body() {}

HttpResponse::~HttpResponse() { m_headers.clear(); }

void HttpResponse::set_status_line(const std::string &line) {
  m_status_line = line;
}

std::string HttpResponse::get_status_line() const { return m_status_line; }

void HttpResponse::set_protocol_version(const std::string &version) {
  m_protocol_version = version;
}

std::string HttpResponse::get_protocol_version() const {
  return m_protocol_version;
}

void HttpResponse::set_status_code(const std::string &code) {
  m_status_code = code;
}

std::string HttpResponse::get_status_code() const { return m_status_code; }

void HttpResponse::set_status_message(const std::string &message) {
  m_status_message = message;
}

std::string HttpResponse::get_status_message() const {
  return m_status_message;
}

void HttpResponse::set_header(const std::string &key,
                              const std::string &value) {
  // Because HTTP headers can have multiple values for the same key
  // (case-insensitive) Normalize to loowercase for consistent storage
  m_headers[key].push_back(value);
}

std::unordered_map<std::string, std::vector<std::string>>
HttpResponse::get_headers() const {
  return m_headers;
}

std::vector<std::string>
HttpResponse::get_headers(const std::string &key) const {
  // Also normalize the key to lowercase when retrieving
  std::string normalized_key = key;
  std::transform(normalized_key.begin(), normalized_key.end(),
                 normalized_key.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return get_headers().at(key);
}

void HttpResponse::set_body(const std::string &body) { m_body = body; }

std::string HttpResponse::get_body() const { return m_body; }
