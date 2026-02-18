#include <http/HttpResponse.hpp>

HttpResponse::HttpResponse()
    : m_status_line(), m_protocol_version("HTTP/1.1"), m_status_code(),
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
  m_headers[key] = value;
}

std::unordered_map<std::string, std::string> HttpResponse::get_headers() const {
  return m_headers;
}

std::string HttpResponse::get_headers(const std::string &key) const {
  return get_headers().at(key);
}

void HttpResponse::set_body(const std::string &body) { m_body = body; }

std::string HttpResponse::get_body() const { return m_body; }
